/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cyclers.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jjorge-a <jjorge-a@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/27 15:11:07 by jjorge-a          #+#    #+#             */
/*   Updated: 2026/11/24 17:53:45 by jjorge-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	compiling(int compile_time)
{
	go_to_sleep(compile_time);
	return ;
}

void	debuging(int debug_time)
{
	go_to_sleep(debug_time);
	return ;
}

void	refracting(int refrac_time)
{
	go_to_sleep(refrac_time);
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
		compiling(t->compile_ms);
		t->compiles_done++;
		release_dongles(t, t->sim);
		if (t->sim->stop)
			break ;
		logger("is debugging", t->index, t->sim);
		debuging(t->debug_ms);
		if (t->sim->stop)
			break ;
		logger("is refactoring", t->index, t->sim);
		refracting(t->refract_ms);
	}
	if (t->granted)
		release_dongles(t, t->sim);
	return (NULL);
}
