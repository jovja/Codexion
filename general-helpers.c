/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   general-helpers.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjorge-a <jjorge-a@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/16 17:14:14 by jjorge-a          #+#    #+#             */
/*   Updated: 2026/11/25 11:35:04 by jjorge-a         ###   ########.fr       */
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
