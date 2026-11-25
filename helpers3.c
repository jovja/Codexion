/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helpers3.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjorge-a <jjorge-a@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 16:14:55 by jjorge-a          #+#    #+#             */
/*   Updated: 2026/11/24 17:20:04 by jjorge-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// arbiter lock held, no clock reads
int	best_for_dongle(t_simu *simu, int idx, long now)
{
	int			best;
	int			i;
	t_request	*reqs;

	reqs = simu->queue.requests;
	best = -1;
	i = 0;
	while (i < simu->queue.size)
	{
		if ((reqs[i].left == idx || reqs[i].right == idx)
			&& pair_is_free(simu, reqs[i], now))
		{
			if (best == -1
				|| is_better(reqs[i], reqs[best], simu->request_order))
				best = i;
		}
		i++;
	}
	if (best == -1)
		return (-1);
	return (reqs[best].coder_id);
}

int	best_for_both(t_simu *simu, t_request req, long now)
{
	return (best_for_dongle(simu, req.left, now) == req.coder_id
		&& best_for_dongle(simu, req.right, now) == req.coder_id);
}

// sched_lock held
int	pick_candidate(t_simu *simu, long now)
{
	int			best;
	int			i;
	t_request	*reqs;

	reqs = simu->queue.requests;
	best = -1;
	i = 0;
	while (i < simu->queue.size)
	{
		if (pair_is_free(simu, reqs[i], now)
			&& best_for_both(simu, reqs[i], now))
		{
			if (best == -1
				|| is_better(reqs[i], reqs[best], simu->request_order))
				best = i;
		}
		i++;
	}
	return (best);
}
