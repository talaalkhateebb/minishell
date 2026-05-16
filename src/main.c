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
	add_history(line);
	cmds = parse_line(line, sh);
	if (!cmds)
		return ;
	sh->last_status = execute(cmds, sh);
	free_cmds(cmds);
}

static void	prompt_loop(t_shell *sh)
{
	char	*line;

	while (1)
	{
		line = readline("minishell$ ");
		check_signal_after_readline(sh);
		if (!line)
		{
			write(1, "exit\n", 5);
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
	prompt_loop(&sh);
	env_free(&sh);
	return (sh.last_status);
}
