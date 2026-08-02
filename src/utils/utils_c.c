/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   utils_c.c                                            :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** write() with a hand-counted length is how off-by-one bugs get into a
** shell's error messages. Everything that prints goes through here so the
** length is always derived from the string itself.
*/
void	put_str(int fd, const char *s)
{
	if (!s)
		return ;
	write(fd, s, ms_strlen(s));
}

void	put_err(const char *prefix, const char *msg)
{
	put_str(2, "minishell: ");
	if (prefix)
	{
		put_str(2, prefix);
		put_str(2, ": ");
	}
	put_str(2, msg);
	put_str(2, "\n");
}

/*
** Reporting a failure and handing back the status that goes with it is the
** single most common thing this shell does, and it used to be written as
** `return (put_err(cmd, "command not found"), 127)` — one return giving
** back two things. Naming the pair keeps every one of those branches a
** single value, and keeps it on one line: several of the functions doing
** it sit at the Norm's 25-line limit and have no room to spell it out.
*/
int	err_ret(const char *prefix, const char *msg, int code)
{
	put_err(prefix, msg);
	return (code);
}

/* A bare name: a letter or '_', then letters, digits and '_' only. */
int	is_identifier(const char *s)
{
	int	i;

	if (!s || !is_var_start(s[0]))
		return (0);
	i = 1;
	while (s[i])
	{
		if (!is_var_char(s[i]))
			return (0);
		i++;
	}
	return (1);
}

int	read_pid(void)
{
	int		fd;
	char	buf[32];
	int		n;
	int		pid;

	fd = open("/proc/self/stat", O_RDONLY);
	if (fd < 0)
		return (-1);
	n = read(fd, buf, sizeof(buf) - 1);
	close(fd);
	if (n <= 0)
		return (-1);
	buf[n] = '\0';
	pid = 0;
	n = 0;
	while (buf[n] && buf[n] >= '0' && buf[n] <= '9')
		pid = pid * 10 + (buf[n++] - '0');
	return (pid);
}
