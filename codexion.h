/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjorge-a <jjorge-a@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/16 17:12:22 by jjorge-a          #+#    #+#             */
/*   Updated: 2026/11/25 11:38:55 by jjorge-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H

# define CODEXION_H

//libraries
# include <unistd.h>
# include <limits.h>
# include <stdio.h>
# include <pthread.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <errno.h>

typedef struct s_coder
{
	struct s_simu	*sim;
	int				index;
	int				compile_ms;
	int				debug_ms;
	int				refract_ms;
	int				compiles_done;
	long			last_compile;
	int				granted;
}	t_coder;

typedef struct s_dongle
{
	t_coder			*holder;
	pthread_mutex_t	lock;
	int				cooldown;
	long			available_in;
}	t_dongle;

typedef struct s_request
{
	int		coder_id;
	long	deadline;
	long	arrival_ms;
	int		right;
	int		left;
}	t_request;

typedef struct s_queue
{
	int			cap;
	int			size;
	t_request	*requests;
}	t_queue;

typedef struct s_simu
{
	t_coder			*coders;
	t_dongle		*dongles;
	t_queue			queue;
	int				n_coders;
	int				burnout_ms;
	int				compiles_required;
	long			start_ms;
	int				stop;
	int				burned_out;
	int				request_order;
	pthread_mutex_t	print_lock;
	pthread_mutex_t	sched_lock;
	pthread_cond_t	sched_ready;
}	t_simu;

// general-helpers
int			ft_isdigit(int c);
int			is_number(char *nptr);
long		ft_atol(const char *nptr);
void		coder_pair(t_simu *simu, int index, int *left, int *right);

// threads-helpers
void		logger(const char *log, int coder_id, t_simu *simu);
int			create_thread(pthread_t *t, void *(*fn)(void *), void *arg);
void		check_burnout(t_simu *sim, int i);
int			join_all(pthread_t *t, int created);
void		stop_and_wake(t_simu *simu);

// heap helpers
int			is_better(t_request a, t_request b, int req_ord);
int			heap_find_index(t_queue *h, int coder_id);

// dongle-helpers
int			pick_candidate(t_simu *simu, long now);
int			pair_is_free(t_simu *simu, t_request req, long now);

// cleaners
int			free_arrays(pthread_t *t, t_coder *c, t_dongle *d, int error_code);
int			clean_data(t_simu *simu, pthread_t *t, int err_c);
int			main_exit(int *settings, int error_code);
int			threads_exit(t_simu *simu, pthread_t *t, int i, int error_code);
int			dongle_rollback(t_dongle *d, int affected_dongles, int error_code);

// cyclers
void		*cycle(void *arg);
int			request_dongle(t_coder *coder, t_simu *simu);

// dongles-cycle
void		grant_ready(t_simu *simu);
void		wait_for_pair(t_simu *simu, t_request req);
void		release_dongles(t_coder *coder, t_simu *simu);

// runner
int			start_up(pthread_t	*threads, t_coder	*coders, t_simu	*simu);
int			run_program(const	int	*settings, char *request_order);

// request
void		build_request(t_request *req, t_coder *coder, t_simu *simu);
int			abandon_request(t_simu *simu, t_request req);

// time-helpers
long		now_ms(void);
void		go_to_sleep(int remaining_time);
long		wait_ms_for(t_simu *simu, t_request req, long now);

// init
int			fill_data(const	int	*s, t_coder *c, t_dongle *d, t_simu *simu);

// heapers
int			heap_insert(t_queue *h, t_request req, int order);
int			heap_remove_id(t_queue *h, int id, int order);

#endif