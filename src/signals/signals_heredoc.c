/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   signals_heredoc.c                                    :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Heredoc mode. readline() has no way to be interrupted from a handler,
** so Ctrl-C closes stdin underneath it: the pending readline() returns
** NULL, and heredoc.c sees g_signal == SIGINT and treats it as a cancel.
**
** "^C" is written without a newline, unlike the prompt handler: closing
** stdin makes readline see EOF, and readline breaks the line itself on the
** way out. Adding one here would leave a blank line bash does not print.
*/
static void	sigint_heredoc(int sig)
{
	g_signal = sig;
	write(1, "^C", 2);
	close(STDIN_FILENO);
}

void	setup_signals_heredoc(void)
{
	struct sigaction	sa;

	sa.sa_handler = sigint_heredoc;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_IGN);
}
