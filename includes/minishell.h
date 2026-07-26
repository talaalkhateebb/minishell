/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   minishell.h                                          :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include <errno.h>
# include <fcntl.h>
# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <termios.h>
# include <unistd.h>
# include <readline/history.h>
# include <readline/readline.h>

typedef enum e_token_type
{
	T_WORD,
	T_PIPE,
	T_REDIR_IN,
	T_REDIR_OUT,
	T_APPEND,
	T_HEREDOC,
}	t_token_type;

typedef struct s_token
{
	char			*value;
	t_token_type	type;
	struct s_token	*next;
}	t_token;

typedef struct s_redir
{
	t_token_type	type;
	char			*target;
	int				expand_heredoc;
	int				heredoc_fd;
	struct s_redir	*next;
}	t_redir;

typedef struct s_cmd
{
	char			**argv;
	t_redir			*redirs;
	struct s_cmd	*next;
}	t_cmd;

typedef struct s_shell
{
	char	**envp;
	int		last_status;
}	t_shell;

extern volatile sig_atomic_t	g_signal;

/* === Frontend (Person A) — exposed to backend === */
t_cmd	*parse_line(const char *line, t_shell *sh);
void	free_cmds(t_cmd *cmds);

/* === Lexer — internal to the frontend === */
t_token	*tokenize(const char *line, int *err);
void	free_tokens(t_token *tokens);
t_token	*tok_new(char *value, t_token_type type);
void	tok_add_back(t_token **head, t_token *tok);
int		is_space(char c);
int		is_op_char(char c);

/* === Parser — internal to the frontend === */
t_cmd	*parse_tokens(t_token *toks, t_shell *sh);
t_cmd	*cmd_new(void);
int		argv_append(t_cmd *cmd, char *value);
int		word_append(t_cmd *cmd, t_token *tok, t_shell *sh);
int		redir_append(t_cmd *cmd, t_token_type type, char *target, int expand);
void	syntax_error(const char *near, t_shell *sh);

/* === Expander — internal to the frontend === */
char	*expand_word(const char *s, t_shell *sh);
int		expand_to_argv(t_cmd *cmd, const char *s, t_shell *sh);
char	*strip_quotes(const char *s);
int		is_quoted(const char *s);
char	*append_str(char *res, const char *add);
char	*append_char(char *res, char c);
char	*expand_dollar(const char *s, int *i, t_shell *sh);
char	*expand_heredoc_line(const char *s, t_shell *sh);

/* === Env API — A owns, B reads === */
int		env_init(t_shell *sh, char **envp);
void	env_free(t_shell *sh);
char	*env_get(t_shell *sh, const char *key);
int		env_set(t_shell *sh, const char *key, const char *value);
int		env_unset(t_shell *sh, const char *key);
char	**env_to_array(t_shell *sh);
int		env_find_index(t_shell *sh, const char *key);
int		env_key_match(const char *entry, const char *key);

/* === Backend (Person B) — exposed to frontend === */
int		execute(t_cmd *cmds, t_shell *sh);

/* === Backend (Person B) — internal, shared between executor.c/pipes.c/
**     redirections.c/heredoc.c only. Not called from frontend code. === */
int		is_builtin(const char *cmd);
int		run_builtin(t_cmd *cmd, t_shell *sh);
char	*find_executable(char *cmd, t_shell *sh);
int		has_slash(const char *s);
int		is_exec_file(const char *path);
int		report_exec_error(char *cmd);
int		run_pipeline(t_cmd *cmds, t_shell *sh);
int		apply_redirs(t_cmd *cmd);
int		process_heredocs(t_cmd *cmds, t_shell *sh);
void	close_heredocs(t_cmd *cmds);
int		count_cmds(t_cmd *cmds);
void	close_pipes(int (*pipes)[2], int n);
int		wait_children(pid_t *pids, int n);

/* === Signals — A owns === */
void	setup_signals_interactive(void);
void	setup_signals_exec(void);
void	setup_signals_child(void);
void	setup_signals_heredoc(void);
int		report_signal(int sig);

/* === Built-ins A (env-touching) === */
int		builtin_echo(char **argv);
int		builtin_env(t_shell *sh);
int		builtin_export(char **argv, t_shell *sh);
int		builtin_unset(char **argv, t_shell *sh);
void	print_exports(t_shell *sh);

/* === Built-ins B (filesystem / exit) === */
int		builtin_cd(char **argv, t_shell *sh);
int		builtin_pwd(void);
int		builtin_exit(char **argv, t_shell *sh);
int		builtin_dot(char **argv);

/* === Shared utils (split: utils_a / utils_b) === */
char	*ms_strdup(const char *s);
size_t	ms_strlen(const char *s);
int		ms_strcmp(const char *a, const char *b);
int		ms_strncmp(const char *a, const char *b, size_t n);
char	*ms_substr(const char *s, size_t start, size_t len);
char	*ms_itoa(int n);
int		is_var_char(char c);
int		is_var_start(char c);
void	free_array(char **arr);
char	*ms_readline(const char *prompt);
int		is_interactive(void);
void	disable_echoctl(void);
void	put_str(int fd, const char *s);
void	put_err(const char *prefix, const char *msg);

#endif
