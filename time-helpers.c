/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time-helpers.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjorge-a <jjorge-a@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 17:43:21 by jjorge-a          #+#    #+#             */
/*   Updated: 2026/11/25 11:35:51 by jjorge-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// get the time to print and other stuff
void	go_to_sleep(int remaining_time)
{
	struct timeval	tv;
	int				buffer;
	long			target_us;
	long			remaining_us;

	buffer = 2000;
	gettimeofday(&tv, NULL);
	target_us = tv.tv_sec * 1000000 + tv.tv_usec + remaining_time * 1000;
	remaining_us = remaining_time * 1000;
	while (remaining_us > buffer + 500)
	{
		if (remaining_us > 20000)
			usleep(20000);
		else
			usleep(remaining_us - buffer);
		gettimeofday(&tv, NULL);
		remaining_us = target_us - (tv.tv_sec * 1000000 + tv.tv_usec);
	}
	while (remaining_us > 0)
	{
		gettimeofday(&tv, NULL);
		remaining_us = target_us - (tv.tv_sec * 1000000L + tv.tv_usec);
	}
	return ;
}

long	wait_ms_for(t_simu *simu, t_request req, long now)
{
	long	left;
	long	right;
	long	delta;

	left = simu->dongles[req.left].available_in - now;
	right = simu->dongles[req.right].available_in - now;
	if (left < 0)
		left = 0;
	if (right < 0)
		right = 0;
	delta = left;
	if (right > delta)
		delta = right;
	if (delta < 1)
		delta = 1;
	if (delta > 3)
		delta = 3;
	return (delta);
}

long	now_ms(void)
{
	struct timeval	start;

	gettimeofday(&start, NULL);
	return (start.tv_sec * 1000 + start.tv_usec / 1000);
}

struct	timespec	abs_time(long delta_ms)
{
	struct timespec	ts;

	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += delta_ms / 1000;
	ts.tv_nsec += (delta_ms % 1000) * 1000000L;
	if (ts.tv_nsec >= 1000000000L)
	{
		ts.tv_nsec -= 1000000000L;
		ts.tv_sec += 1;
	}
	return (ts);
}

void	wait_for_pair(t_simu *simu, t_request req)
{
	struct timespec	ts;
	int				ret;

	ts = abs_time(wait_ms_for(simu, req, now_ms()));
	ret = pthread_cond_timedwait(&simu->sched_ready, &simu->sched_lock, &ts);
	if (ret != 0 && ret != ETIMEDOUT)
		fprintf(stderr, "sched wait error %d\n", ret);
	return ;
}
