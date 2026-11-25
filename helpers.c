/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helpers.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjorge-a <jjorge-a@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/16 17:14:14 by jjorge-a          #+#    #+#             */
/*   Updated: 2026/11/24 13:39:33 by jjorge-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	ft_isdigit(int c)
{
	if (c >= 48 && c <= 57)
		return (1);
	return (0);
}

// check if it's all digits
int	is_number(char *nptr)
{
	int	i;

	i = 0;
	while (nptr[i] == 32 || (nptr[i] >= 9 && nptr[i] <= 13))
		i++;
	while (nptr[i])
	{
		if (!ft_isdigit(nptr[i]))
			return (0);
		i++;
	}
	return (1);
}

long	ft_atol(const char *nptr)
{
	int		i;
	int		signal;
	long	number;

	i = 0;
	signal = 1;
	number = 0;
	while (nptr[i] == 32 || (nptr[i] >= 9 && nptr[i] <= 13))
		i++;
	if (nptr[i] == '-' || nptr[i] == '+')
	{
		if (nptr[i] == '-')
			return (-1);
		i++;
	}
	while (ft_isdigit(nptr[i]))
	{
		number = number * 10 + (nptr[i] - '0');
		i++;
	}
	if (number <= INT_MAX)
		return (signal * number);
	return (-1);
}

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
