#include "minishell.h"

/*
** Day-0 stub for Person B. Just prints argv[0] of each command in the
** pipeline and returns 0. Replace with the real fork/exec/waitpid +
** redirections + pipes + heredoc machinery.
*/

int	execute(t_cmd *cmds, t_shell *sh)
{
	t_cmd	*cur;

	(void)sh;
	cur = cmds;
	while (cur)
	{
		if (cur->argv && cur->argv[0])
		{
			write(1, "[stub] ", 7);
			write(1, cur->argv[0], ms_strlen(cur->argv[0]));
			write(1, "\n", 1);
		}
		cur = cur->next;
	}
	return (0);
}
