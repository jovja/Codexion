/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjorge-a <jjorge-a@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/25 11:16:29 by jjorge-a          #+#    #+#             */
/*   Updated: 2026/11/25 11:35:07 by jjorge-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	coder_pair(t_simu *simu, int index, int *left, int *right)
{
	*left = index;
	*right = (index + 1) % simu->n_coders;
	return ;
}

int	dongle_is_free(t_dongle *d, long now)
{
	if (d->holder == NULL && d->available_in <= now)
		return (1);
	return (0);
}

int	pair_is_free(t_simu *simu, t_request req, long now)
{
	return (dongle_is_free(&simu->dongles[req.left], now)
		&& dongle_is_free(&simu->dongles[req.right], now));
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

int	abandon_request(t_simu *simu, t_request req)
{
	heap_remove_id(&simu->queue, req.coder_id, simu->request_order);
	pthread_cond_broadcast(&simu->sched_ready);
	pthread_mutex_unlock(&simu->sched_lock);
	return (0);
}
