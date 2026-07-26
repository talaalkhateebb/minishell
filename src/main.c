/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   main.c                                               :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

volatile sig_atomic_t	g_signal = 0;

static void	check_signal_after_readline(t_shell *sh)
{
	if (g_signal == SIGINT)
	{
		sh->last_status = 130;
		g_signal = 0;
	}
}

/* bash: `$_` is the last argument of the last simple command. */
static void	update_underscore(t_cmd *cmds, t_shell *sh)
{
	t_cmd	*last;
	int		i;

	last = cmds;
	while (last && last->next)
		last = last->next;
	if (!last || !last->argv || !last->argv[0])
		return ;
	i = 0;
	while (last->argv[i + 1])
		i++;
	env_set(sh, "_", last->argv[i]);
}

static void	run_one_line(char *line, t_shell *sh)
{
	t_cmd	*cmds;

	if (!*line)
		return ;
	if (is_interactive())
		add_history(line);
	cmds = parse_line(line, sh);
	if (!cmds)
		return ;
	sh->last_status = execute(cmds, sh);
	update_underscore(cmds, sh);
	free_cmds(cmds);
}

static void	prompt_loop(t_shell *sh)
{
	char	*line;

	while (1)
	{
		disable_echoctl();
		line = ms_readline("minishell$ ");
		check_signal_after_readline(sh);
		if (!line)
		{
			if (is_interactive())
				put_str(1, "exit\n");
			break ;
		}
		run_one_line(line, sh);
		free(line);
	}
}

int	main(int argc, char **argv, char **envp)
{
	t_shell	sh;

	(void)argc;
	(void)argv;
	if (env_init(&sh, envp) != 0)
		return (1);
	sh.last_status = 0;
	setup_signals_interactive();
	disable_echoctl();
	prompt_loop(&sh);
	env_free(&sh);
	return (sh.last_status);
}
