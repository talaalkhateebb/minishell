#include "minishell.h"

/*
** ============================================================
** env_mod.c — set / unset variables.
**
** env_set   — overwrites existing entry, or appends new one.
** env_unset — removes an entry by key (no-op if missing).
**
** make_entry lives in env_entry.c so this file stays under
** the norm's 5-function ceiling.
** ============================================================
*/

/*
** count_arr — count non-NULL pointers in a NULL-terminated list.
*/
static int	count_arr(char **arr)
{
	int	n;

	n = 0;
	while (arr && arr[n])
		n++;
	return (n);
}

/*
** append_entry — grow sh->envp by one slot and place 'entry' there.
**
** Allocates a new array with one extra slot, shallow-copies the
** old pointers into it, places entry at the end, and NULL-
** terminates. Then frees the old array spine (NOT the strings —
** their ownership moved into new_arr).
**
** Returns 0 on success, 1 on malloc failure. On failure, the
** caller (env_set) is responsible for freeing 'entry'.
*/
static int	append_entry(t_shell *sh, char *entry)
{
	int		n;
	char	**new_arr;
	int		i;

	n = count_arr(sh->envp);
	new_arr = malloc(sizeof(char *) * (n + 2));
	if (!new_arr)
		return (1);
	i = 0;
	while (i < n)
	{
		new_arr[i] = sh->envp[i];
		i++;
	}
	new_arr[n] = entry;
	new_arr[n + 1] = NULL;
	free(sh->envp);
	sh->envp = new_arr;
	return (0);
}

/*
** env_set — add or update a variable.
**
** Steps:
**   1. Build the new "KEY=VALUE" (or "KEY" if value is NULL).
**   2. If key already exists, free old slot, replace pointer. Done.
**   3. Else, append to the array (grow by 1).
**
** Returns 0 on success, 1 on malloc failure.
** Called by: builtin_export, builtin_cd (PWD/OLDPWD updates).
*/
int	env_set(t_shell *sh, const char *key, const char *value)
{
	int		idx;
	char	*new_entry;

	new_entry = env_make_entry(key, value);
	if (!new_entry)
		return (1);
	idx = env_find_index(sh, key);
	if (idx >= 0)
	{
		free(sh->envp[idx]);
		sh->envp[idx] = new_entry;
		return (0);
	}
	if (append_entry(sh, new_entry) != 0)
	{
		free(new_entry);
		return (1);
	}
	return (0);
}

/*
** env_unset — remove a variable by key.
**
** Always returns 0 (missing key is not an error — just a no-op).
**
** Steps:
**   1. Find the slot. If not found, return 0.
**   2. Free that string.
**   3. Shift every later pointer down by one, including the NULL
**      terminator, so the array stays valid.
*/
int	env_unset(t_shell *sh, const char *key)
{
	int	idx;
	int	i;

	idx = env_find_index(sh, key);
	if (idx < 0)
		return (0);
	free(sh->envp[idx]);
	i = idx;
	while (sh->envp[i])
	{
		sh->envp[i] = sh->envp[i + 1];
		i++;
	}
	return (0);
}
