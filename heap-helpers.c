/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap-helpers.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjorge-a <jjorge-a@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/25 10:57:13 by jjorge-a          #+#    #+#             */
/*   Updated: 2026/11/25 10:57:23 by jjorge-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	heap_find_index(t_queue *h, int coder_id)
{
	int	i;

	i = 0;
	while (i < h->size)
	{
		if (h->requests[i].coder_id == coder_id)
			return (i);
		i ++;
	}
	return (-1);
}

int	is_better(t_request a, t_request b, int req_ord)
{
	if (req_ord == 0)
	{
		if (a.arrival_ms != b.arrival_ms)
			return (a.arrival_ms < b.arrival_ms);
	}
	else
	{
		if (a.deadline != b.deadline)
			return (a.deadline < b.deadline);
		if (a.arrival_ms != b.arrival_ms)
			return (a.arrival_ms < b.arrival_ms);
	}
	return (a.coder_id < b.coder_id);
}
