/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   runner.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjorge-a <jjorge-a@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/25 10:54:21 by jjorge-a          #+#    #+#             */
/*   Updated: 2026/11/25 12:14:01 by jjorge-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	*monitor_fn(void	*arg)
{
	int		i;
	t_simu	*sim;
	int		all_done;

	sim = arg;
	while (!sim->stop)
	{
		i = -1;
		all_done = 1;
		while (++i < sim->n_coders)
		{
			check_burnout(sim, i);
			if (sim->coders[i].compiles_done < sim->compiles_required)
				all_done = 0;
		}
		if (all_done)
			stop_and_wake(sim);
		go_to_sleep(3);
	}
	return (NULL);
}

// take into account the free before joining threads
int	start_up(pthread_t	*threads, t_coder	*coders, t_simu	*simu)
{
	int			i;
	pthread_t	monitor;

	i = 0;
	while (i < simu->n_coders)
	{
		if (create_thread(&threads[i], &cycle, &coders[i]) != 0)
			return (threads_exit(simu, threads, i, 0));
		i++;
	}
	if (create_thread(&monitor, &monitor_fn, simu) != 0)
		return (threads_exit(simu, threads, i, 0));
	if (!join_all(threads, simu->n_coders))
		return (0);
	if (pthread_join(monitor, NULL) != 0)
		return (0);
	return (1);
}

int	run_program(const	int	*settings, char *request_order)
{
	t_dongle	*dongles;
	pthread_t	*threads;
	t_coder		*coders;
	t_simu		simu;

	threads = malloc(sizeof(pthread_t) * settings[0]);
	dongles = malloc(sizeof(t_dongle) * settings[0]);
	coders = malloc(sizeof(t_coder) * settings[0]);
	if (!dongles || !threads || !coders)
		return (free_arrays(threads, coders, dongles, 0));
	simu.request_order = (strcmp(request_order, "fifo"));
	if (!fill_data(settings, coders, dongles, &simu))
		return (free_arrays(threads, coders, dongles, 0));
	if (!start_up(threads, coders, &simu))
		return (clean_data(&simu, threads, 0));
	if (!simu.burned_out)
		return (clean_data(&simu, threads, 1));
	return (clean_data(&simu, threads, 1));
}
