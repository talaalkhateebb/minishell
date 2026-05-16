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

/* Lexer internals — shared between lexer.c, lexer_word, lexer_op, lexer_tokens */
int		is_space(char c);
int		is_op_char(char c);
char	*extract_word(const char *line, size_t start, size_t end);
int		handle_word(const char *line, size_t *i, t_token **head);
int		handle_op(const char *line, size_t *i, t_token **head);
t_token	*token_new(char *value, t_token_type type);
void	token_append(t_token **head, t_token *node);
void	free_tokens(t_token *head);
t_token	*tokenize(const char *line);

/* Parser internals — shared between parser.c, parser_validate, parser_build, parser_argv */
int		validate_syntax(t_token *tokens);
t_cmd	*build_cmds(t_token *tokens);
t_cmd	*cmd_new(void);
int		cmd_add_arg(t_cmd *cmd, const char *arg);
int		cmd_add_redir(t_cmd *cmd, t_token_type type, const char *target);
t_redir	*redir_new(t_token_type type, char *target);

/* Expander — frontend, called by parse_line */
typedef struct s_xstate
{
	char	*dst;
	size_t	pos;
	char	q;
	t_shell	*sh;
}	t_xstate;

void	expand_cmds(t_cmd *cmds, t_shell *sh);
char	*expand_word(const char *w, t_shell *sh);
char	*strip_quotes_only(const char *w);
int		delim_is_quoted(const char *d);
int		handle_quote_state(char c, char *q);
int		is_var_start(char c);
int		is_var_char(char c);
size_t	var_name_len(const char *s);
char	*get_var_value(const char *w, size_t pos, size_t nlen, t_shell *sh);
char	*itoa_status(int n);

/* === Env API — A owns, B reads === */
int		env_init(t_shell *sh, char **envp);
void	env_free(t_shell *sh);
char	*env_get(t_shell *sh, const char *key);
int		env_set(t_shell *sh, const char *key, const char *value);
int		env_unset(t_shell *sh, const char *key);
char	**env_to_array(t_shell *sh);
/* env internals — shared between env_lookup, env_entry, env_mod */
int		env_find_index(t_shell *sh, const char *key);
char	*env_make_entry(const char *key, const char *value);

/* === Backend (Person B) — exposed to frontend === */
int		execute(t_cmd *cmds, t_shell *sh);

/* === Signals — A owns === */
void	setup_signals_interactive(void);
void	setup_signals_exec(void);
void	setup_signals_heredoc(void);

/* === Built-ins A (env-touching) === */
int		builtin_echo(char **argv);
int		builtin_env(t_shell *sh);
int		builtin_export(char **argv, t_shell *sh);
int		builtin_unset(char **argv, t_shell *sh);
int		is_valid_name(const char *s);
int		process_export_arg(const char *arg, t_shell *sh);
int		run_frontend_builtin(t_cmd *c, t_shell *sh);
int		run_backend_builtin(t_cmd *c, t_shell *sh);
int		try_run_builtin(t_cmd *cmds, t_shell *sh);

/* === Built-ins B (filesystem / exit) === */
int		builtin_cd(char **argv, t_shell *sh);
int		builtin_pwd(void);
int		builtin_exit(char **argv, t_shell *sh);

/* === Shared utils (split: utils_a / utils_b) === */
char	*ms_strdup(const char *s);
size_t	ms_strlen(const char *s);
int		ms_strcmp(const char *a, const char *b);

#endif
