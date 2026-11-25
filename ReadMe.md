# Codexion

Coders competing for dongles, the dining-philosophers problem rendered as a
build farm: each coder is a thread, there is one dongle between every adjacent
pair, and a coder may only compile while holding both of its dongles.

## Usage

    make
    ./codexion <coders> <burnout> <compile> <debug> <refactor> \
               <compiles_required> <cooldown> <fifo|edf>

    ./codexion 3 1000 200 150 150 5 50 edf

| Argument | Meaning |
|---|---|
| number_of_coders | coders on the ring, equal to the number of dongles |
| time_to_burnout | ms without starting a compile before a coder burns out |
| time_to_compile | ms a compile takes, during which both dongles are held |
| time_to_debug | ms of debugging, dongles released |
| time_to_refactor | ms of refactoring, dongles released |
| number_of_compiles_required | compiles per coder before the simulation ends |
| dongle_cooldown | ms a dongle stays unavailable after being released |
| scheduler | `fifo` (arrival order) or `edf` (earliest deadline first) |

Output is one line per event: `<elapsed_ms> <coder_number> <message>`. The run
ends when every coder finished its required compiles, or when a coder burns out.
Invalid arguments exit 1 without printing.

## Topology and ownership

Coder `i` owns dongle `i` (left) and dongle `(i + 1) % n` (right)
(`dongle-helpers.c:coder_pair`). With a single coder both indices are 0, which
is the subject's degenerate ring of one dongle; the same code path handles it,
and cooldown is applied once.

A dongle has exactly three states: **free**, **held** (`holder` points at the
coder), and **cooling** (`holder` is `NULL` but `available_in` is in the future).

## Scheduling model

Requests live in one hand-rolled binary heap on the simulation state, because
the subject forbids a standard-library priority queue. Each coder has at most
one outstanding request, so the heap is sized to the coder count and never
reallocates.

`is_better` is the entire scheduling policy: FIFO compares arrival time, EDF
compares the deadline `last_compile + time_to_burnout` and then arrival, and
both fall back to the coder index. That last tie-break makes output
deterministic, so repeated runs of the same arguments are reproducible.

`pick_candidate` returns the request to grant, and only a request that is the
best-priority waiter for **both** of its dongles can be granted. `grant_ready`
is the only function that ever writes `holder` or `available_in`.

## Blocking cases handled

**Deadlock prevention — Coffman's conditions.** All four must hold at once for a
deadlock to exist, and the design breaks two of them:

- *Mutual exclusion* is intended: a dongle serves one coder at a time.
- *Hold and wait* is broken. A pair is granted atomically — `grant_ready` sets
  both holders in one decision under one mutex, so a coder never owns one dongle
  while waiting for the other. The classic philosopher deadlock requires exactly
  that half-acquisition, and it cannot occur here.
- *No preemption* is not needed. Holders release voluntarily at the end of the
  compile, so no forced take and no forced take ordering exist.
- *Circular wait* is broken by a single lock order: `sched_lock` is always taken
  before `print_lock`, and never the other way round. There is no second
  scheduler lock, so no pair of locks can be acquired in conflicting order.

**Starvation prevention.** Only the best-priority eligible request is granted,
never a later arrival that happens to be ready first, so a waiting coder cannot
be overtaken indefinitely. Equal priorities resolve by coder index. The single
exception is documented and bounded: a better-priority waiter that is blocked
*only* by cooldown may be skipped, because it cannot act yet; when its cooldown
expires it becomes the best eligible contender for both its dongles and wins at
the next arbitration tick. A waiter blocked by a live holder is never skipped.

**Cooldown handling.** On release, `available_in = now + dongle_cooldown`, and
no predicate ever grants a dongle whose `available_in` is still in the future.
Release happens before debugging, so cooldown overlaps debugging and
refactoring instead of idling the ring.

**Precise burnout detection.** A monitor thread polls every 3 ms and compares
`now - last_compile` against `time_to_burnout`. `last_compile` is stamped when
dongles are granted, which is what makes both the burnout window and the EDF
deadline mean "time since the last compile started". Detection triggers one
message, sets the stop flag, and broadcasts so parked waiters leave immediately
rather than sleeping through the end of the run.

**Log serialization.** Every line goes through `logger`, which holds
`print_lock` for the whole timestamp-plus-message write, so two messages can
never interleave on one line. It also refuses to print once the stop flag is
set, which is why no acquisition line appears after termination.

**Lost wakeups.** Cooldown expiry wakes nobody by itself — no event fires when
`available_in` becomes past — so waiters use `pthread_cond_timedwait` bounded by
the earliest expiry in their pair, capped at 3 ms to match the monitor's tick.
`ETIMEDOUT` is treated as a normal re-arbitration tick, not an error.

## Thread synchronization mechanisms

| Primitive | Protects | Why |
|---|---|---|
| `pthread_mutex_t sched_lock` | the request heap, `dongles[].holder`, `dongles[].available_in`, `granted`, `stop` | grants must be atomic; a request spans two dongles, so no per-dongle lock can express "both mine are free and I am next" |
| `pthread_cond_t sched_ready` | the same state | a condition variable carries no data; it only tells waiters to re-test their predicate |
| `pthread_mutex_t print_lock` | stdout | two messages must never interleave |
| `pthread_mutex_t dongles[].lock` | each dongle's own state fields | per-dongle state protection, initialized in `fill_dongles` and destroyed in `clean_data` |

**How a coder waits.** `request_dongle` builds a request, inserts it into the
heap under `sched_lock`, then loops: if the run has stopped it removes its own
request and returns failure; otherwise it runs the arbiter, and if it was not
granted it waits on `sched_ready` with a cooldown-aware timeout. The wait is a
`while` loop over the predicate, never an `if`, because spurious wakeups are
legal and a broadcast wakes threads whose condition is still false — those must
go back to sleep instead of proceeding.

**How a coder is granted.** A grant is usually performed by another coder's
release, so the granted thread cannot be handed anything directly; `grant_ready`
sets `holder` on both dongles, marks `granted`, logs the two acquisition lines,
removes the request, and broadcasts. Reading `dongles[left].holder == coder` is
the equivalent invariant: a coder is granted exactly when both its dongles name
it as holder.

**Why broadcast, not signal.** Pair grants mean an arbitrary single wakeup can
select a waiter whose second dongle is still busy, while the rightful winner
stays asleep. `pthread_cond_broadcast` makes every waiter re-test, which removes
that class of lost wakeup at negligible cost.

**How races are prevented.** `dongle_is_free` and `pair_is_free` are only called
with `sched_lock` held, so `holder` and `available_in` can never be read while
another thread is mid-grant; two coders cannot both observe the same dongle as
free. Predicates take `now` as an argument and never read the clock, which keeps
them testable and keeps sort keys frozen: `arrival_ms` and `deadline` are
computed once in `build_request`, and the comparator only reads stored fields.

**Coders and the monitor.** The monitor reads `compiles_done` and `last_compile`
without the scheduler lock; those are per-coder counters written only by their
own thread, and `stop` is written under `sched_lock` and read as a plain flag.
Termination is coordinated by `stop_and_wake`, which sets the flag and broadcasts
under `sched_lock`, so the monitor's decision and any parked waiter's exit are
not a race.

**Cleanup.** All mutexes and the condition variable are destroyed in reverse
creation order, the single request heap is freed once, and thread creation
failure goes through `threads_exit`, which broadcasts before joining so no
waiter is left unjoined. No heap allocation is left unfreed.

## Files

| File | Role |
|---|---|
| `main.c` | argument parsing, coder and monitor creation, joining |
| `initializer.c` | simulation, coder and dongle initialization |
| `dongles-cycle.c` | request building, arbitration, granting, release |
| `dongle-helpers.c` | ring topology and availability predicates |
| `helpers3.c` | eligibility scans: `best_for_dongle`, `best_for_both`, `pick_candidate` |
| `heapers.c` | binary heap: insert, sift, remove head, remove by id |
| `helpers.c` | parsing and the comparator |
| `helpers2.c` | logging, burnout check, thread helpers, stop broadcast |
| `helpers3`/`time-helpers.c` | clock helpers, cooldown-aware wait bound |
| `cyclers.c` | the compile → debug → refactor loop |
| `cleaners.c` | mutex destruction and freeing |

## References

- `pthread_cond_wait` / `pthread_cond_timedwait` / `pthread_cond_broadcast`:
  https://man7.org/linux/man-pages/man3/pthread_cond_wait.3p.html,
  https://man7.org/linux/man-pages/man3/pthread_cond_timedwait.3p.html,
  https://man7.org/linux/man-pages/man3/pthread_cond_broadcast.3p.html
- `pthread_mutex_lock`: https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3p.html
- `pthread_create` / `pthread_join`: https://man7.org/linux/man-pages/man3/pthread_create.3p.html
