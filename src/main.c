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
	sh->cmds = cmds;
	sh->last_status = execute(cmds, sh);
	sh->cmds = NULL;
	free_cmds(cmds);
}

static void	prompt_loop(t_shell *sh)
{
	char	*line;

	while (1)
	{
		enable_echoctl();
		line = ms_readline("minishell$ ");
		check_signal_after_readline(sh);
		if (!line)
		{
			if (is_interactive())
				put_str(1, "exit\n");
			break ;
		}
		sh->line = line;
		run_one_line(line, sh);
		sh->line = NULL;
		free(line);
		if (sh->should_exit)
			break ;
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
	sh.cwd_lost = 0;
	sh.pid = read_pid();
	setup_signals_interactive();
	enable_echoctl();
	prompt_loop(&sh);
	rl_clear_history();
	env_free(&sh);
	return (sh.last_status);
}
