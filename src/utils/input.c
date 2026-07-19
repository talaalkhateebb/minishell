/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   input.c                                              :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Line input.
**
** readline() is only right when we are actually talking to a terminal.
** When stdin is a pipe or a file (`echo cmd | ./minishell`, or a grader's
** test script) a real shell prints no prompt and echoes nothing, so we
** read the line plainly instead. That keeps piped output byte-identical
** to bash's, which is what makes differential testing possible at all.
*/

int	is_interactive(void)
{
	return (isatty(STDIN_FILENO));
}

/* Reads one line from stdin, without the '\n'. NULL at EOF with no data. */
static char	*read_plain_line(void)
{
	char	*line;
	char	buf[2];
	int		n;

	line = ms_strdup("");
	buf[1] = '\0';
	n = read(STDIN_FILENO, buf, 1);
	if (n <= 0)
		return (free(line), NULL);
	while (n > 0 && buf[0] != '\n')
	{
		line = append_char(line, buf[0]);
		if (!line)
			return (NULL);
		n = read(STDIN_FILENO, buf, 1);
	}
	return (line);
}

char	*ms_readline(const char *prompt)
{
	if (is_interactive())
		return (readline(prompt));
	return (read_plain_line());
}
