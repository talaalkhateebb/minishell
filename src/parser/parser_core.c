/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   parser_core.c                                        :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Token list → command list. Expansion happens here, as each word is
** attached, so the executor only ever sees finished strings.
**
** Returns NULL on a syntax error (sh->last_status already set to 2).
*/

/*
** A redirection operator must be followed by a word. For a heredoc, a
** quoted delimiter (<< 'EOF') turns expansion off inside the body, so we
** record that before stripping the quotes.
*/
static int	handle_redir(t_cmd *cur, t_token **toks, t_shell *sh)
{
	t_token	*op;
	t_token	*target;
	char	*expanded;

	op = *toks;
	target = op->next;
	if (!target || target->type != T_WORD)
	{
		if (target)
			syntax_error(target->value, sh);
		else
			syntax_error("newline", sh);
		return (1);
	}
	*toks = target->next;
	if (op->type == T_HEREDOC)
		return (redir_append(cur, op->type, strip_quotes(target->value),
				!is_quoted(target->value)));
	expanded = expand_redir_target(target, sh);
	if (!expanded)
		return (1);
	return (redir_append(cur, op->type, expanded, 0));
}

/*
** `filled` says whether the command before the pipe had any word or
** redirection *token* — not whether argv ended up non-empty, since a word
** may legitimately have vanished: `$EMPTY | cat` is not a syntax error.
**
** A trailing pipe reports `newline': bash would read a continuation line
** there, but we take the whole command at once, so the line ends instead.
*/
static int	handle_pipe(t_cmd **cur, t_token **toks, t_shell *sh, int filled)
{
	if (!filled)
		return (syntax_error("|", sh), 1);
	if (!(*toks)->next)
		return (syntax_error("newline", sh), 1);
	(*cur)->next = cmd_new();
	if (!(*cur)->next)
		return (1);
	*cur = (*cur)->next;
	*toks = (*toks)->next;
	return (0);
}

static int	handle_word(t_cmd *cur, t_token **toks, t_shell *sh,
	int *filled)
{
	if (!*filled && is_reserved_word((*toks)->value))
		return (syntax_error((*toks)->value, sh), 1);
	if (word_append(cur, *toks, sh))
		return (1);
	*toks = (*toks)->next;
	*filled = 1;
	return (0);
}

static int	parse_loop(t_cmd *cur, t_token *toks, t_shell *sh)
{
	int	fail;
	int	filled;

	fail = 0;
	filled = 0;
	while (toks && !fail)
	{
		if (toks->type == T_WORD)
			fail = handle_word(cur, &toks, sh, &filled);
		else if (toks->type == T_PIPE)
		{
			fail = handle_pipe(&cur, &toks, sh, filled);
			filled = 0;
		}
		else if (is_redir(toks->type))
		{
			fail = handle_redir(cur, &toks, sh);
			filled = 1;
		}
		else
			fail = (syntax_error(toks->value, sh), 1);
	}
	return (fail);
}

t_cmd	*parse_tokens(t_token *toks, t_shell *sh)
{
	t_cmd	*head;
	int		status;

	head = cmd_new();
	if (!head)
		return (NULL);
	if (!parse_loop(head, toks, sh))
		return (head);
	process_heredocs(head, sh);
	status = sh->last_status;
	close_heredocs(head);
	if (sh->syntax_token && status == 2)
	{
		put_str(2, "minishell: syntax error near unexpected token `");
		put_str(2, sh->syntax_token);
		put_str(2, "'\n");
	}
	free(sh->syntax_token);
	sh->syntax_token = NULL;
	return (free_cmds(head), NULL);
}
