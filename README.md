*This project has been created as part of the 42 curriculum by jjorge-a*

# Codexion

## Description

Codexion is a multithreaded resource-allocation simulation written in C with
POSIX threads. A ring of coder threads shares one USB dongle between every
adjacent pair, and a coder may only compile while holding **both** of its
dongles. It is the dining-philosophers problem rendered as a build farm:
philosophers become coders, forks become dongles, eating becomes compiling —
and a coder that goes too long without starting a compile **burns out**,
ending the simulation.

The goal is to keep every coder alive until each one completes its required
number of compiles, using mutexes, a condition variable, and a priority queue
implemented as a hand-written binary heap, under one of two request
schedulers: `fifo` (arrival order) or `edf` (earliest deadline first).

## Instructions

### Build

    make          # cc -Wall -Wextra -Werror, produces ./codexion
    make clean    # remove object files
    make fclean   # remove objects and the binary
    make re       # full rebuild

### Run

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
ends when every coder finished its required compiles, or when a coder burns
out. Invalid arguments exit with status 1 without printing.

## Blocking cases handled

- **Deadlock prevention (Coffman's conditions)** — deadlock requires mutual
  exclusion, hold-and-wait, no preemption, and circular wait *all at once*.
  This design breaks **hold-and-wait**: an arbiter grants both dongles
  atomically inside a single critical section, so a coder never owns a dongle
  while waiting for another — it waits on a condition variable holding
  nothing. All scheduler state lives under one mutex, so no circular lock
  ordering between scheduler locks can exist either.
- **Starvation prevention** — requests wait in a binary heap ordered by the
  active policy: `fifo` by arrival time, `edf` by deadline
  (`last_compile_start + time_to_burnout`), with coder id as a deterministic
  tie-break. The arbiter always grants the best *eligible* request, so the
  most urgent waiter is served first under `edf`, and wait time is bounded
  by arrival order under `fifo`.
- **Cooldown handling** — releasing a dongle stamps it
  `available_in = now + cooldown`; a dongle counts as free only when unheld
  *and* cooled down. Waiters sleep on the condition variable with a
  cooldown-aware timeout, so they re-check eligibility exactly when the
  dongle becomes available instead of spinning.
- **Precise burnout detection** — a monitor thread scans every coder roughly
  every 3 ms (the requirement is to report within 10 ms of the missed
  deadline) and is guarded so only the first burnout is ever reported.
- **Log serialization** — one print mutex serializes all output. The stop
  flag is checked *inside* the lock, so no event can be printed after the
  burnout line; the timestamp is also taken inside the lock, so printed
  timestamps never go backwards. Every line is flushed.

## Thread synchronization mechanisms

- **`pthread_mutex_t print_lock`** — serializes stdout between all coder
  threads and the monitor; combined with the in-lock stop check, this is
  what prevents interleaved or post-burnout messages.
- **`pthread_mutex_t sched_lock` + `pthread_cond_t sched_ready`** — the
  arbiter. The request heap and all dongle state (`holder`, `available_in`)
  are only examined or modified while holding `sched_lock`. Coders insert
  their request and then `pthread_cond_timedwait` until granted; grants,
  releases, cooldown expiry, and shutdown all wake the waiters with
  `pthread_cond_broadcast`. Request insertion and the grant scan are one
  atomic critical section, which is the core race-prevention mechanism.
- **One `pthread_mutex_t` per dongle** — each dongle carries its own mutex as
  the subject requires; in this arbiter design the mutual exclusion of
  scheduling decisions is provided by `sched_lock`, with the per-dongle
  mutex marking ownership of each dongle resource.
- **Coder ↔ monitor communication** — shared flags (`stop`, `burned_out`,
  `last_compile`, `compiles_done`). `stop` is always written under
  `sched_lock` together with a broadcast (`stop_and_wake`), so every
  sleeping coder wakes immediately on burnout or completion instead of
  discovering the stop late.

## Resources

- *Operating Systems: Three Easy Pieces* (OSTEP) — concurrency, locks, and
  condition-variable chapters: https://pages.cs.wisc.edu/~remzi/OSTEP/
- *The Little Book of Semaphores* — synchronization patterns:
  https://greenteapress.com/semaphores/LittleBookOfSemaphores.pdf
- `man 7 pthreads`, `man pthread_mutex_lock`, `man pthread_cond_timedwait`
- Dining philosophers problem and Coffman conditions:
  https://en.wikipedia.org/wiki/Dining_philosophers_problem
- LLNL POSIX Threads tutorial: https://hpc-tutorials.llnl.gov/posix/
- Liu & Layland (1973), *Scheduling Algorithms for Multiprogramming in a
  Hard-Real-Time Environment* — the EDF optimality result
- `valgrind --tool=helgrind` and `-fsanitize=thread` for race detection

### AI usage

AI was used as a design and review aid, not to write the codebase: planning
the architecture (staged init/cleanup ladder, the arbiter-based granting
design, the logger design), reviewing code for correctness bugs (rollback
bounds, join semantics, lock ordering), explaining the underlying theory
(binary heaps, Coffman's conditions, EDF vs FIFO, pthread semantics), and
drafting this README. All final code was written and tested by the author.
