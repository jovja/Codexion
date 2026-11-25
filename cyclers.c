/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cyclers.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjorge-a <jjorge-a@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/27 15:11:07 by jjorge-a          #+#    #+#             */
/*   Updated: 2026/11/25 14:42:16 by jjorge-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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
		lock_pair(simu, req.left, req.right);
		simu->dongles[req.left].holder = &simu->coders[req.coder_id];
		if (req.left != req.right)
			simu->dongles[req.right].holder = &simu->coders[req.coder_id];
		unlock_pair(simu, req.left, req.right);
		simu->coders[req.coder_id].granted = 1;
		heap_remove_id(&simu->queue, req.coder_id, simu->request_order);
		logger("has taken a dongle", req.coder_id, simu);
		if (req.left != req.right)
			logger("has taken a dongle", req.coder_id, simu);
		pthread_cond_broadcast(&simu->sched_ready);
	}
	return ;
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
	if (simu->n_coders == 1)
		return (0);
	return (1);
}

void	release_dongles(t_coder *coder, t_simu *simu)
{
	int		left;
	int		right;
	long	now;

	pthread_mutex_lock(&simu->sched_lock);
	coder_pair(simu, coder->index, &left, &right);
	now = now_ms();
	lock_pair(simu, left, right);
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
	unlock_pair(simu, left, right);
	coder->granted = 0;
	pthread_cond_broadcast(&simu->sched_ready);
	grant_ready(simu);
	pthread_mutex_unlock(&simu->sched_lock);
	return ;
}

// put it self on the heap
// compile->debug->refract
// safety: loop exited while holding
void	*cycle(void *arg)
{
	t_coder	*t;

	t = arg;
	while (!t->sim->stop && t->compiles_done < t->sim->compiles_required)
	{
		if (request_dongle(t, t->sim) == 0)
			break ;
		t->last_compile = now_ms();
		logger("is compiling", t->index, t->sim);
		go_to_sleep(t->compile_ms);
		t->compiles_done++;
		release_dongles(t, t->sim);
		if (t->sim->stop)
			break ;
		logger("is debugging", t->index, t->sim);
		go_to_sleep(t->debug_ms);
		if (t->sim->stop)
			break ;
		logger("is refactoring", t->index, t->sim);
		go_to_sleep(t->refract_ms);
	}
	if (t->granted)
		release_dongles(t, t->sim);
	return (NULL);
}
