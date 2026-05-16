#include "minishell.h"

/*
** Env module — Person A (frontend), Days 1–2.
** env_init / env_free are wired so the project compiles and so $? and
** built-ins have a place to read/write. Real env_get/env_set/env_unset
** are TODO: pick a representation (linked list of key=value, or stay
** with char**) and implement them properly.
*/

static int	count_envp(char **envp)
{
	int	n;

	n = 0;
	while (envp && envp[n])
		n++;
	return (n);
}

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

char	*env_get(t_shell *sh, const char *key)
{
	(void)sh;
	(void)key;
	return (NULL);
}

int	env_set(t_shell *sh, const char *key, const char *value)
{
	(void)sh;
	(void)key;
	(void)value;
	return (0);
}
