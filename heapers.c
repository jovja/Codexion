/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heapers.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjorge-a <jjorge-a@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/14 15:20:44 by jjorge-a          #+#    #+#             */
/*   Updated: 2026/11/24 17:43:37 by jjorge-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	shift_up(t_queue *heap, int i, int order)
{
	t_request	temp;
	int			p;

	while (i > 0)
	{
		p = (i - 1) / 2;
		if (!is_better(heap->requests[i], heap->requests[p], order))
			return ;
		temp = heap->requests[p];
		heap->requests[p] = heap->requests[i];
		heap->requests[i] = temp;
		i = p;
	}
}

void	shift_down(t_queue *heap, int i, int order)
{
	int			best;
	int			left;
	int			right;
	t_request	temp;

	while (1)
	{
		best = i;
		left = 2 * i + 1;
		right = 2 * i + 2;
		if (left < heap->size && is_better(heap->requests[left],
				heap->requests[best], order))
			best = left;
		if (right < heap->size && is_better(heap->requests[right],
				heap->requests[best], order))
			best = right;
		if (best == i)
			return ;
		temp = heap->requests[best];
		heap->requests[best] = heap->requests[i];
		heap->requests[i] = temp;
		i = best;
	}
	return ;
}

void	heap_remove_head(t_queue *heap, int order)
{
	if (heap->size == 0)
		return ;
	heap->size--;
	if (heap->size > 0)
	{
		heap->requests[0] = heap->requests[heap->size];
		shift_down(heap, 0, order);
	}
}

int	heap_remove_id(t_queue *h, int id, int order)
{
	int	i;
	int	p;

	i = heap_find_index(h, id);
	if (i < 0)
		return (0);
	h->size--;
	if (i == h->size)
		return (1);
	p = (i - 1) / 2;
	h->requests[i] = h->requests[h->size];
	if (i > 0 && is_better(h->requests[i], h->requests[p], order))
		shift_up(h, i, order);
	else
		shift_down(h, i, order);
	return (1);
}

int	heap_insert(t_queue *h, t_request req, int order)
{
	int	i;

	if (h->size == h->cap)
		return (0);
	h->requests[h->size] = req;
	i = h->size;
	h->size++;
	shift_up(h, i, order);
	return (1);
}
