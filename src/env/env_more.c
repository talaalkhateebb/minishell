#include "minishell.h"

int	env_unset(t_shell *sh, const char *key)
{
	(void)sh;
	(void)key;
	return (0);
}

char	**env_to_array(t_shell *sh)
{
	return (sh->envp);
}
