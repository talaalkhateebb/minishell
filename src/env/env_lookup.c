#include "minishell.h"

/*
** ============================================================
** env_lookup.c — finding and reading env variables.
**
** Two public functions:
**   env_find_index(sh, "USER")  → 0 (the array slot) or -1
**   env_get(sh, "USER")         → "tala" (pointer inside the
**                                 stored "USER=tala" string)
**
** key_matches is the underlying check: does this stored entry
** belong to the given key?
** ============================================================
*/

/*
** key_matches — does "entry" represent variable "key"?
**
** entry  looks like "USER=tala" or sometimes just "USER" (an
**        exported-but-unset variable).
** key    is the name we are searching for, e.g. "USER".
** keylen is the length of key, precomputed by the caller so we
**        don't recompute it on every call.
**
** Returns 1 if entry starts with "key" followed by either '='
** or end-of-string (\0). Returns 0 otherwise.
**
** Why this exact rule? Because we must distinguish "USER" from
** "USERNAME": "USERNAME=x" should NOT match key "USER" — the
** char after "USER" inside the entry is 'N', not '=' or '\0'.
*/
static int	key_matches(const char *entry, const char *key, size_t keylen)
{
	size_t	i;

	i = 0;
	while (i < keylen)
	{
		if (entry[i] != key[i])
			return (0);
		i++;
	}
	return (entry[keylen] == '=' || entry[keylen] == '\0');
}

/*
** env_find_index — locate a variable's slot in sh->envp.
**
** Returns the position (0, 1, 2, ...) of the entry with the
** matching key, or -1 if no entry matches.
**
** Used by env_get (to read), env_set (to overwrite), and
** env_unset (to remove). Centralising the search keeps the
** three operations consistent.
*/
int	env_find_index(t_shell *sh, const char *key)
{
	int		i;
	size_t	keylen;

	if (!sh->envp || !key)
		return (-1);
	keylen = ms_strlen(key);
	i = 0;
	while (sh->envp[i])
	{
		if (key_matches(sh->envp[i], key, keylen))
			return (i);
		i++;
	}
	return (-1);
}

/*
** env_get — read the value of a variable.
**
** Returns a POINTER INTO our env array — do not free it, do not
** modify it. The pointer is valid until the next env_set or
** env_unset that affects this slot.
**
** Behaviour:
**   not found        → returns NULL
**   "KEY=value"      → returns pointer to first char of "value"
**   "KEY" (no '=')   → returns pointer to the trailing '\0',
**                      which reads as an empty string ""
**
** Called by:
**   - the expander, every time it sees "$KEY"
**   - builtin_export, when checking existing values
**
** Example:  sh->envp[0] = "USER=tala"
**           env_get(sh, "USER")  →  "tala"
*/
char	*env_get(t_shell *sh, const char *key)
{
	int		idx;
	char	*entry;

	idx = env_find_index(sh, key);
	if (idx < 0)
		return (NULL);
	entry = sh->envp[idx];
	while (*entry && *entry != '=')
		entry++;
	if (*entry == '=')
		return (entry + 1);
	return (entry);
}
