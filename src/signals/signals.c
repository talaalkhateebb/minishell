/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   signals.c                                            :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Signals — Person A (frontend). Four modes:
**
**   interactive — waiting at the prompt: Ctrl-C wipes the line and
**                 redraws a fresh prompt, Ctrl-\ does nothing.
**   exec        — a child is running: the PARENT ignores both, so the
**                 signal reaches only the child.
**   child       — inside the fork, just before exec: back to defaults,
**                 so `cat` and `sleep` are killable.
**   heredoc     — reading heredoc lines: Ctrl-C aborts the heredoc.
**
** The single global g_signal only ever stores the signal number.
*/

static void	sigint_interactive(int sig)
{
	g_signal = sig;
	write(1, "\n", 1);
	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();
}

void	setup_signals_interactive(void)
{
	struct sigaction	sa;

	sa.sa_handler = sigint_interactive;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_IGN);
}

void	setup_signals_exec(void)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
}

void	setup_signals_child(void)
{
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
}

/*
** Bash prints the newline itself when a foreground child dies on Ctrl-C,
** and "Quit: 3" on Ctrl-\. Returns the 128 + signo exit status.
*/
int	report_signal(int sig)
{
	if (sig == SIGINT)
		put_str(1, "\n");
	else if (sig == SIGQUIT)
		put_str(1, "Quit: 3\n");
	return (128 + sig);
}
