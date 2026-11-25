/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleaners.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjorge-a <jjorge-a@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 16:51:56 by jjorge-a          #+#    #+#             */
/*   Updated: 2026/11/24 13:44:27 by jjorge-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	dongle_rollback(t_dongle *d, int affected_dongles, int error_code)
{
	int	i;

	i = 0;
	while (i < affected_dongles)
	{
		pthread_mutex_destroy(&d[i].lock);
		i++;
	}
	return (error_code);
}

int	free_arrays(pthread_t *t, t_coder *c, t_dongle *d, int error_code)
{
	free(t);
	free(c);
	free(d);
	return (error_code);
}

// clean data
int	clean_data(t_simu *simu, pthread_t *t, int error_code)
{
	int	i;

	i = 0;
	while (i < simu->n_coders)
	{
		pthread_mutex_destroy(&simu->dongles[i].lock);
		i++;
	}
	free(simu->queue.requests);
	pthread_cond_destroy(&simu->sched_ready);
	pthread_mutex_destroy(&simu->sched_lock);
	pthread_mutex_destroy(&simu->print_lock);
	return (free_arrays(t, simu->coders, simu->dongles, error_code));
}

int	threads_exit(t_simu *simu, pthread_t *t, int i, int error_code)
{
	stop_and_wake(simu);
	join_all(t, i);
	return (error_code);
}

int	main_exit(int *settings, int error_code)
{
	free(settings);
	return (error_code);
}
