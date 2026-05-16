#include "minishell.h"

/*
** ============================================================
** parser_validate.c — syntax error detection on the token list.
**
** We refuse:
**   - empty token list                     (handled in parse_line)
**   - pipe at start                        | echo hi
**   - two pipes in a row                   echo || ls   (mandatory)
**   - pipe at end                          echo |
**   - redirection not followed by a WORD   echo > | wc
**   - redirection at end                   echo >
**
** All errors print: "minishell: syntax error near unexpected
** token `X'" — matching Bash's format.
** ============================================================
*/

/*
** print_err — emit the Bash-style syntax error to stderr.
*/
static void	print_err(const char *near)
{
	write(2, "minishell: syntax error near unexpected token `", 47);
	write(2, near, ms_strlen(near));
	write(2, "'\n", 2);
}

/*
** check_pipe_at — is this PIPE token legal at this position?
**
** prev = the token immediately before (NULL at start).
**
** Pipe is illegal:
**   - at the start of the line              (prev == NULL)
**   - immediately after another pipe        (prev->type == T_PIPE)
**   - at the end of the line                (cur->next == NULL)
*/
static int	check_pipe_at(t_token *cur, t_token *prev)
{
	if (!prev || prev->type == T_PIPE)
	{
		print_err(cur->value);
		return (1);
	}
	if (!cur->next)
	{
		print_err("newline");
		return (1);
	}
	return (0);
}

/*
** check_redir_at — is this redirection followed by a filename?
**
** A redirection (< > << >>) must be immediately followed by a
** T_WORD that names the file (or, for <<, the heredoc delimiter).
** Anything else is a syntax error.
*/
static int	check_redir_at(t_token *cur)
{
	if (!cur->next)
	{
		print_err("newline");
		return (1);
	}
	if (cur->next->type != T_WORD)
	{
		print_err(cur->next->value);
		return (1);
	}
	return (0);
}

/*
** validate_syntax — walk the token list and apply the rules above.
**
** Returns 0 if everything is fine, 1 if any error was printed.
*/
int	validate_syntax(t_token *tokens)
{
	t_token	*prev;
	t_token	*cur;

	if (!tokens)
		return (1);
	prev = NULL;
	cur = tokens;
	while (cur)
	{
		if (cur->type == T_PIPE && check_pipe_at(cur, prev) != 0)
			return (1);
		if (cur->type != T_WORD && cur->type != T_PIPE
			&& check_redir_at(cur) != 0)
			return (1);
		prev = cur;
		cur = cur->next;
	}
	return (0);
}
