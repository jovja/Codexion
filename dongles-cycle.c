/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles-cycle.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjorge-a <jjorge-a@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 19:44:15 by jjorge-a          #+#    #+#             */
/*   Updated: 2026/11/24 18:07:58 by jjorge-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// TODO Divide new part

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

// everyone re-checks
void	grant_ready(t_simu *simu)
{
	int			idx;
	t_request	req;

	if (simu->stop)
		return ;
	while (1)
	{
		idx = pick_candidate(simu, now_ms());
		if (idx == -1)
			return ;
		req = simu->queue.requests[idx];
		simu->dongles[req.left].holder = &simu->coders[req.coder_id];
		simu->dongles[req.right].holder = &simu->coders[req.coder_id];
		simu->coders[req.coder_id].granted = 1;
		heap_remove_id(&simu->queue, req.coder_id, simu->request_order);
		logger("has taken a dongle", req.coder_id, simu);
		logger("has taken a dongle", req.coder_id, simu);
		pthread_cond_broadcast(&simu->sched_ready);
	}
	return ;
}

void	build_request(t_request *req, t_coder *coder, t_simu *simu)
{
	req->coder_id = coder->index;
	coder_pair(simu, coder->index, &req->left, &req->right);
	req->arrival_ms = now_ms();
	req->deadline = -1;
	if (simu->request_order)
		req->deadline = coder->last_compile + simu->burnout_ms;
	return ;
}
// new part
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

int	abandon_request(t_simu *simu, t_request req)
{
	heap_remove_id(&simu->queue, req.coder_id, simu->request_order);
	pthread_cond_broadcast(&simu->sched_ready);
	pthread_mutex_unlock(&simu->sched_lock);
	return (0);
}

int	request_dongle(t_coder *coder, t_simu *simu)
{
	t_request	req;

	build_request(&req, coder, simu);
	coder->granted = 0;
	pthread_mutex_lock(&simu->sched_lock);
	heap_insert(&simu->queue, req, simu->request_order);
	while (!coder->granted)
	{
		if (simu->stop)
			return (abandon_request(simu, req));
		grant_ready(simu);
		if (!coder->granted)
			wait_for_pair(simu, req);
	}
	pthread_mutex_unlock(&simu->sched_lock);
	return (1);
}
// end new part

void	release_dongles(t_coder *coder, t_simu *simu)
{
	int		left;
	int		right;
	long	now;

	pthread_mutex_lock(&simu->sched_lock);
	coder_pair(simu, coder->index, &left, &right);
	now = now_ms();
	if (simu->dongles[left].holder == coder)
	{
		simu->dongles[left].holder = NULL;
		simu->dongles[left].available_in = now + simu->dongles[left].cooldown;
	}
	if (simu->dongles[right].holder == coder)
	{
		simu->dongles[right].holder = NULL;
		simu->dongles[right].available_in = now + simu->dongles[right].cooldown;
	}
	coder->granted = 0;
	pthread_cond_broadcast(&simu->sched_ready);
	grant_ready(simu);
	pthread_mutex_unlock(&simu->sched_lock);
	return ;
}
