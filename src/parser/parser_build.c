#include "minishell.h"

/*
** ============================================================
** parser_build.c — turn tokens into a list of t_cmd.
**
** After validate_syntax has approved the token list, this file
** walks it and groups WORDs and redirections into commands,
** with each T_PIPE starting a new t_cmd.
**
** Example:
**   tokens: [ls] [-la] [|] [grep] [.c] [>] [out.txt]
**   result:
**     cmd1: argv=["ls", "-la"]            redirs=NULL
**       ↓ next
**     cmd2: argv=["grep", ".c"]           redirs=[> "out.txt"]
**       ↓ next
**     NULL
** ============================================================
*/

/*
** start_pipe — open a new command and advance current pointer.
**
** Called when we encounter a T_PIPE token. Allocates a fresh
** t_cmd, links it after *cur, and makes it the new "current".
*/
static int	start_pipe(t_cmd **cur)
{
	t_cmd	*next;

	next = cmd_new();
	if (!next)
		return (1);
	(*cur)->next = next;
	*cur = next;
	return (0);
}

/*
** consume_redir — record this redirection on the current cmd.
**
** The token list already has the redirection followed by its
** filename (we trust validate_syntax to have guaranteed that).
** Advances *tok past BOTH tokens (the op and its target).
*/
static int	consume_redir(t_cmd *cur, t_token **tok)
{
	if (cmd_add_redir(cur, (*tok)->type, (*tok)->next->value) != 0)
		return (1);
	*tok = (*tok)->next->next;
	return (0);
}

/*
** build_step — handle exactly one token in the main loop.
**
** Dispatches based on token type:
**   T_PIPE  → start a new command
**   T_WORD  → append as argument to current command
**   else    → it's a redirection op, consume it + its target
*/
static int	build_step(t_token **tok, t_cmd **cur)
{
	int	rc;

	rc = 0;
	if ((*tok)->type == T_PIPE)
	{
		rc = start_pipe(cur);
		*tok = (*tok)->next;
	}
	else if ((*tok)->type == T_WORD)
	{
		rc = cmd_add_arg(*cur, (*tok)->value);
		*tok = (*tok)->next;
	}
	else
		rc = consume_redir(*cur, tok);
	return (rc);
}

/*
** build_cmds — public entry. Token list → t_cmd list.
**
** Returns NULL on malloc failure (and frees what was built so far).
** Assumes validate_syntax has already passed.
*/
t_cmd	*build_cmds(t_token *tokens)
{
	t_cmd	*head;
	t_cmd	*cur;
	t_token	*tok;

	head = cmd_new();
	if (!head)
		return (NULL);
	cur = head;
	tok = tokens;
	while (tok)
	{
		if (build_step(&tok, &cur) != 0)
		{
			free_cmds(head);
			return (NULL);
		}
	}
	return (head);
}
