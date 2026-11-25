/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helpers2.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjorge-a <jjorge-a@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 16:51:48 by jjorge-a          #+#    #+#             */
/*   Updated: 2026/11/24 18:03:49 by jjorge-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	create_thread(pthread_t *t, void *(*fn)(void *), void *arg)
{
	return (pthread_create(t, NULL, fn, arg));
}

int	join_all(pthread_t *t, int created)
{
	int	i;
	int	ok;

	i = 0;
	ok = 1;
	while (i < created)
	{
		if (pthread_join(t[i], NULL) != 0)
			ok = 0;
		i++;
	}
	return (ok);
}

void	check_burnout(t_simu *sim, int i)
{
	long	now;

	now = now_ms();
	if (sim->stop)
		return ;
	if (now - sim->coders[i].last_compile >= sim->burnout_ms)
	{
		printf("%ld %d burned out\n", now - sim->start_ms, i + 1);
		stop_and_wake(sim);
		sim->burned_out = 1;
	}
	return ;
}

void	logger(const char *log, int coder_id, t_simu *simu)
{
	pthread_mutex_lock(&simu->print_lock);
	if (simu->stop)
	{
		pthread_mutex_unlock(&simu->print_lock);
		return ;
	}
	printf("%ld %i %s\n", now_ms() - simu->start_ms, coder_id + 1, log);
	fflush(stdout);
	pthread_mutex_unlock(&simu->print_lock);
	return ;
}

void	stop_and_wake(t_simu *simu)
{
	pthread_mutex_lock(&simu->sched_lock);
	simu->stop = 1;
	pthread_cond_broadcast(&simu->sched_ready);
	pthread_mutex_unlock(&simu->sched_lock);
	return ;
}
