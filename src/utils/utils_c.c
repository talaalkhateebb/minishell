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
