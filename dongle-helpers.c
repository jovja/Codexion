/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle-helpers.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjorge-a <jjorge-a@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 14:52:24 by jjorge-a          #+#    #+#             */
/*   Updated: 2026/11/24 17:44:32 by jjorge-a         ###   ########.fr       */
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
