/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   heredoc2.c                                           :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Opening the heredoc.
**
** The body is read into a pipe, and the real stdin is saved first because
** the heredoc SIGINT handler closes fd 0 to break readline() out of its
** read — fill_heredoc() has to be able to put it back.
**
** On failure the pipe is closed here, so -1 always means "nothing was
** opened" and the caller has nothing to undo. That is what lets it answer
** with a single value instead of a cleanup and a status together.
*/
int	heredoc_open(int *fds)
{
	int	saved_stdin;

	if (pipe(fds) == -1)
		return (-1);
	saved_stdin = dup(STDIN_FILENO);
	if (saved_stdin == -1)
	{
		close(fds[0]);
		close(fds[1]);
	}
	return (saved_stdin);
}
