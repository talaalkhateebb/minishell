#include "minishell.h"

/*
** ============================================================
** expand_var.c — variable-name helpers and value lookup.
**
** Variable rules (Bash):
**   - name starts with [A-Za-z_]
**   - continues with [A-Za-z0-9_]
**   - $? is a special variable: the last exit status as a string
**   - $ followed by anything else (e.g. $1, $-, $$) is NOT
**     handled in mandatory minishell — we leave a literal $.
** ============================================================
*/

/*
** is_var_start — is this a valid first char for a variable name?
*/
int	is_var_start(char c)
{
	return ((c >= 'A' && c <= 'Z')
		|| (c >= 'a' && c <= 'z')
		|| c == '_');
}

/*
** is_var_char — is this a valid non-first char for a name?
*/
int	is_var_char(char c)
{
	return (is_var_start(c) || (c >= '0' && c <= '9'));
}

/*
** var_name_len — how many chars of 's' form a variable name?
**
** Returns 0 if s doesn't start with a valid first-char (so the
** caller knows there is no variable name to expand here).
*/
size_t	var_name_len(const char *s)
{
	size_t	i;

	if (!is_var_start(s[0]))
		return (0);
	i = 0;
	while (is_var_char(s[i]))
		i++;
	return (i);
}

/*
** get_var_value — copy nlen chars starting at w[pos], look them up
** in env, return a fresh malloc'd value string.
**
** If the variable is unset, returns a malloc'd empty string ""
** (matching Bash semantics: undefined → empty, not an error).
** Returns NULL on malloc failure.
**
** Allocating a copy of the name lets us pass it to env_get without
** a stack buffer of arbitrary size.
*/
char	*get_var_value(const char *w, size_t pos, size_t nlen, t_shell *sh)
{
	char	*name;
	char	*value;
	size_t	i;

	name = malloc(nlen + 1);
	if (!name)
		return (NULL);
	i = 0;
	while (i < nlen)
	{
		name[i] = w[pos + i];
		i++;
	}
	name[nlen] = '\0';
	value = env_get(sh, name);
	free(name);
	if (!value)
		return (ms_strdup(""));
	return (ms_strdup(value));
}

/*
** itoa_status — convert a non-negative int to a malloc'd string.
**
** Used for $? expansion. last_status is always 0..255 in practice
** (process exit codes), so we don't need to handle negatives.
*/
char	*itoa_status(int n)
{
	char	buf[12];
	int		i;

	i = 11;
	buf[i] = '\0';
	if (n == 0)
	{
		i--;
		buf[i] = '0';
		return (ms_strdup(&buf[i]));
	}
	while (n > 0)
	{
		i--;
		buf[i] = '0' + (n % 10);
		n /= 10;
	}
	return (ms_strdup(&buf[i]));
}
