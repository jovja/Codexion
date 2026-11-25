/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjorge-a <jjorge-a@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/27 12:04:37 by jjorge-a          #+#    #+#             */
/*   Updated: 2026/11/25 10:59:41 by jjorge-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int argc, char *argv[])
{
	int	*settings;
	int	i;

	if (argc != 9)
		return (1);
	if (strcmp(argv[8], "fifo") != 0 && strcmp(argv[8], "edf") != 0)
		return (1);
	i = 1;
	settings = malloc(sizeof(int) * 7);
	if (!settings)
		return (1);
	while (i < argc - 1)
	{
		if (!is_number(argv[i]))
			return (main_exit(settings, 1));
		settings[i - 1] = (int) ft_atol(argv[i]);
		if (settings[i - 1] == -1)
			return (main_exit(settings, 1));
		i++;
	}
	if (!run_program(settings, argv[i]))
		return (main_exit(settings, 1));
	return (main_exit(settings, 0));
}
