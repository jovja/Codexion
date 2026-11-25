/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   initializer.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjorge-a <jjorge-a@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/27 15:28:34 by jjorge-a          #+#    #+#             */
/*   Updated: 2026/11/24 18:00:29 by jjorge-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	fill_simu(const	int	*s, t_coder *c, t_dongle *d, t_simu *simu)
{
	simu->stop = 0;
	simu->burned_out = 0;
	simu->n_coders = s[0];
	simu->burnout_ms = s[1];
	simu->start_ms = now_ms();
	simu->compiles_required = s[5];
	simu->coders = c;
	simu->dongles = d;
	simu->queue.size = 0;
	simu->queue.cap = s[0];
	if (pthread_mutex_init(&simu->print_lock, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&simu->sched_lock, NULL) != 0)
		return (0);
	if (pthread_cond_init(&simu->sched_ready, NULL) != 0)
		return (0);
	simu->queue.requests = malloc(sizeof(t_request) * s[0]);
	return (simu->queue.requests != NULL);
}

void	fill_threads(const	int	*settings, t_coder *coders, t_simu *simu)
{
	int			i;

	i = 0;
	while (i < settings[0])
	{
		coders[i].index = i;
		coders[i].sim = simu;
		coders[i].compile_ms = settings[2];
		coders[i].debug_ms = settings[3];
		coders[i].refract_ms = settings[4];
		coders[i].last_compile = simu->start_ms;
		coders[i].compiles_done = 0;
		coders[i].granted = 0;
		i++;
	}
	return ;
}

int	fill_dongles(const	int	*settings, t_dongle *dongles)
{
	int			i;

	i = 0;
	while (i < settings[0])
	{
		dongles[i].cooldown = settings[6];
		dongles[i].holder = NULL;
		dongles[i].available_in = 0;
		if (pthread_mutex_init(&dongles[i].lock, NULL) != 0)
			return (dongle_rollback(dongles, i, 0));
		i++;
	}
	return (1);
}

// populate data
int	fill_data(const	int	*s, t_coder *c, t_dongle *d, t_simu *simu)
{
	if (!fill_dongles(s, d))
		return (0);
	if (!fill_simu(s, c, d, simu))
		return (dongle_rollback(d, s[0], 0));
	fill_threads(s, c, simu);
	return (1);
}
