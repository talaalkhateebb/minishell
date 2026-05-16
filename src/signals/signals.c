#include "minishell.h"

/*
** Signals — Person A (frontend), Day 2 + final wiring on Day 18.
** Three modes: interactive (waiting at prompt), exec (child running),
** heredoc (reading heredoc lines).
**
** The single global g_signal (declared in main.c) only ever stores
** the signal number — nothing else.
*/

static void	sigint_interactive(int sig)
{
	g_signal = sig;
	write(1, "\n", 1);
	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();
}

void	setup_signals_interactive(void)
{
	struct sigaction	sa;

	sa.sa_handler = sigint_interactive;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_IGN);
}

void	setup_signals_exec(void)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
}

static void	sigint_heredoc(int sig)
{
	g_signal = sig;
	write(1, "\n", 1);
	close(0);
}

void	setup_signals_heredoc(void)
{
	struct sigaction	sa;

	sa.sa_handler = sigint_heredoc;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_IGN);
}
