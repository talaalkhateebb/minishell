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

/*
** "^C" is written by hand. readline turns ECHO off while it owns the
** terminal, so the tty cannot echo the intr character itself, and with
** rl_catch_signals off readline no longer echoes it either — yet bash
** shows it. Outside readline (a child in the foreground) the terminal is
** back in canonical mode and ECHOCTL echoes "^C" on its own, which is why
** report_signal() below only adds the newline.
*/
static void	sigint_interactive(int sig)
{
	g_signal = sig;
	write(1, "^C\n", 3);
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
}

/*
** rl_catch_signals must be 0. Left at its default of 1, readline installs
** its OWN SIGINT handler: on Ctrl-C it frees the line, un-preps the
** terminal, re-raises the signal to the handler below — which then
** redisplays onto a terminal readline has already reset — and finally
** re-preps and redraws the prompt a second time. On Linux that leaves a
** duplicated prompt and readline's column bookkeeping out of sync, so the
** next characters typed land in the wrong place. With it off, this handler
** is the only one that runs, and SA_RESTART keeps readline in its read
** loop so the redrawn prompt is the live one.
*/
void	setup_signals_interactive(void)
{
	struct sigaction	sa;

	rl_catch_signals = 0;
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
** Bash prints the newline itself when a foreground child dies on Ctrl-C.
** On Ctrl-\ it prints "Quit", followed by " (core dumped)" only when the
** kernel actually wrote a core — which is what `ulimit -c` decides. Takes
** the raw wait status, not the bare signo, so that test can be made.
** Returns the 128 + signo exit status.
*/
int	report_signal(int status)
{
	int	sig;

	sig = WTERMSIG(status);
	if (sig == SIGINT)
		put_str(1, "\n");
	else if (sig == SIGQUIT)
	{
		put_str(1, "Quit");
		if (WCOREDUMP(status))
			put_str(1, " (core dumped)");
		put_str(1, "\n");
	}
	return (128 + sig);
}
