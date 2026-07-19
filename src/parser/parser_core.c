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

static int	is_redir(t_token_type type)
{
	return (type == T_REDIR_IN || type == T_REDIR_OUT
		|| type == T_APPEND || type == T_HEREDOC);
}

/*
** A redirection operator must be followed by a word. For a heredoc, a
** quoted delimiter (<< 'EOF') turns expansion off inside the body, so we
** record that before stripping the quotes.
*/
static int	handle_redir(t_cmd *cur, t_token **toks, t_shell *sh)
{
	t_token	*op;
	t_token	*target;

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
	return (redir_append(cur, op->type, expand_word(target->value, sh), 0));
}

static int	handle_pipe(t_cmd **cur, t_token **toks, t_shell *sh)
{
	if (!(*cur)->argv[0] && !(*cur)->redirs)
		return (syntax_error("|", sh), 1);
	if (!(*toks)->next)
		return (syntax_error("|", sh), 1);
	(*cur)->next = cmd_new();
	if (!(*cur)->next)
		return (1);
	*cur = (*cur)->next;
	*toks = (*toks)->next;
	return (0);
}

t_cmd	*parse_tokens(t_token *toks, t_shell *sh)
{
	t_cmd	*head;
	t_cmd	*cur;
	int		fail;

	head = cmd_new();
	if (!head)
		return (NULL);
	cur = head;
	fail = 0;
	while (toks && !fail)
	{
		if (toks->type == T_WORD)
		{
			fail = argv_append(cur, expand_word(toks->value, sh));
			toks = toks->next;
		}
		else if (toks->type == T_PIPE)
			fail = handle_pipe(&cur, &toks, sh);
		else if (is_redir(toks->type))
			fail = handle_redir(cur, &toks, sh);
	}
	if (fail)
		return (free_cmds(head), NULL);
	return (head);
}
