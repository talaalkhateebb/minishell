#include "minishell.h"

/*
** ============================================================
** env.c  —  the foundation of the env module.
**
** The env is a list of strings shaped "KEY=VALUE", ending in NULL.
** We copy the OS's env on startup so we can freely change ours
** without touching the OS's.
**
** Layout (after env_init):
**     sh->envp[0] -> "USER=tala"
**     sh->envp[1] -> "HOME=/Users/tala"
**     sh->envp[2] -> NULL
**
** This file holds the lifecycle helpers (count / init / free /
** export-as-array). Lookups live in env_lookup.c and modifications
** live in env_mod.c.
** ============================================================
*/

/*
** count_envp — how many strings are in this list?
**
** A string list always ends with NULL. We just walk until we
** hit NULL and return the position we reached.
**
** Example:
**   envp = ["A=1", "B=2", NULL]   ->   returns 2
*/
static int	count_envp(char **envp)
{
	int	n;

	n = 0;
	while (envp && envp[n])
		n++;
	return (n);
}

/*
** env_init — copy the OS's env into our own storage.
**
** Called ONCE at startup, from main().
** Inputs:  sh   — pointer to our shell-wide state
**          envp — the env list main() received from the OS
** Output:  0 on success, 1 on malloc failure.
**
** Steps:
**   1. Reset last_status to 0 (no command has run yet).
**   2. Count the OS env, then malloc room for n+1 pointers
**      (the +1 is for the trailing NULL).
**   3. ms_strdup each entry so we own independent memory.
**   4. Set the last slot to NULL.
**
** After this function, the OS's envp is untouched and sh->envp
** is a freshly-owned copy we can grow/shrink/modify.
*/
int	env_init(t_shell *sh, char **envp)
{
	int	n;
	int	i;

	sh->last_status = 0;
	n = count_envp(envp);
	sh->envp = malloc(sizeof(char *) * (n + 1));
	if (!sh->envp)
		return (1);
	i = 0;
	while (i < n)
	{
		sh->envp[i] = ms_strdup(envp[i]);
		if (!sh->envp[i])
			return (1);
		i++;
	}
	sh->envp[n] = NULL;
	return (0);
}

/*
** env_free — release the env list we built.
**
** Called once on shutdown. Frees each string, then the array.
** Guards against NULL so it's safe to call twice.
*/
void	env_free(t_shell *sh)
{
	int	i;

	if (!sh->envp)
		return ;
	i = 0;
	while (sh->envp[i])
	{
		free(sh->envp[i]);
		i++;
	}
	free(sh->envp);
	sh->envp = NULL;
}

/*
** env_to_array — hand the raw char** to whoever needs it.
**
** Used by the backend's executor when it calls execve(), which
** expects a "char **envp" argument. We just return our internal
** array directly; do NOT free what this returns.
*/
char	**env_to_array(t_shell *sh)
{
	return (sh->envp);
}
