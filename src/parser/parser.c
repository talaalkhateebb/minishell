#include "minishell.h"

/*
** Day-0 stub: parse_line returns a hardcoded `echo hi` t_cmd so the
** project compiles end-to-end. Replace with the real lexer → parser
** → expander pipeline.
*/

static t_cmd	*build_stub_echo_hi(void)
{
	t_cmd	*cmd;

	cmd = malloc(sizeof(t_cmd));
	if (!cmd)
		return (NULL);
	cmd->argv = malloc(sizeof(char *) * 3);
	if (!cmd->argv)
	{
		free(cmd);
		return (NULL);
	}
	cmd->argv[0] = ms_strdup("echo");
	cmd->argv[1] = ms_strdup("hi");
	cmd->argv[2] = NULL;
	cmd->redirs = NULL;
	cmd->next = NULL;
	return (cmd);
}

t_cmd	*parse_line(const char *line, t_shell *sh)
{
	(void)line;
	(void)sh;
	return (build_stub_echo_hi());
}

static void	free_argv(char **argv)
{
	int	i;

	if (!argv)
		return ;
	i = 0;
	while (argv[i])
	{
		free(argv[i]);
		i++;
	}
	free(argv);
}

static void	free_redirs(t_redir *redirs)
{
	t_redir	*next;

	while (redirs)
	{
		next = redirs->next;
		free(redirs->target);
		free(redirs);
		redirs = next;
	}
}

void	free_cmds(t_cmd *cmds)
{
	t_cmd	*next;

	while (cmds)
	{
		next = cmds->next;
		free_argv(cmds->argv);
		free_redirs(cmds->redirs);
		free(cmds);
		cmds = next;
	}
}
