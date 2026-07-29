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

static int	has_ifs(const char *s)
{
	while (s && *s)
	{
		if (*s == ' ' || *s == '\t' || *s == '\n')
			return (1);
		s++;
	}
	return (0);
}

/* True when `$` appears outside quotes — those expansions undergo splitting. */
static int	has_unquoted_dollar(const char *s)
{
	char	quote;
	int		i;

	quote = 0;
	i = 0;
	while (s && s[i])
	{
		if (!quote && (s[i] == '\'' || s[i] == '"'))
			quote = s[i];
		else if (quote && s[i] == quote)
			quote = 0;
		else if (!quote && s[i] == '$')
			return (1);
		i++;
	}
	return (0);
}

/*
** bash: an unquoted `$VAR` in a redirect that expands to nothing or to more
** than one field is an ambiguous redirect. Message keeps the raw token.
*/
static char	*expand_redir_target(t_token *target, t_shell *sh)
{
	char	*expanded;

	expanded = expand_word(target->value, sh);
	if (!expanded)
		return (NULL);
	if (has_unquoted_dollar(target->value)
		&& (!expanded[0] || has_ifs(expanded)))
	{
		put_err(target->value, "ambiguous redirect");
		sh->last_status = 1;
		return (free(expanded), NULL);
	}
	return (expanded);
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
*/
static int	handle_pipe(t_cmd **cur, t_token **toks, t_shell *sh, int filled)
{
	if (!filled)
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

/*
** bash reserved words that become `syntax error near unexpected token`
** when they are the first token of a simple command (not after a leading
** redirection). Quotes keep them as normal words: `"in" < file` is fine.
*/
static int	is_reserved_word(const char *s)
{
	return (ms_strcmp(s, "in") == 0 || ms_strcmp(s, "do") == 0
		|| ms_strcmp(s, "then") == 0 || ms_strcmp(s, "else") == 0
		|| ms_strcmp(s, "elif") == 0 || ms_strcmp(s, "fi") == 0
		|| ms_strcmp(s, "esac") == 0);
}

static int	parse_loop(t_cmd *cur, t_token *toks, t_shell *sh)
{
	int	fail;
	int	filled;
	int	cmd_start;

	fail = 0;
	filled = 0;
	cmd_start = 1;
	while (toks && !fail)
	{
		if (toks->type == T_WORD)
		{
			if (cmd_start && is_reserved_word(toks->value))
				return (syntax_error(toks->value, sh), 1);
			fail = word_append(cur, toks, sh);
			toks = toks->next;
			filled = 1;
			cmd_start = 0;
		}
		else if (toks->type == T_PIPE)
		{
			fail = handle_pipe(&cur, &toks, sh, filled);
			filled = 0;
			cmd_start = 1;
		}
		else if (is_redir(toks->type))
		{
			fail = handle_redir(cur, &toks, sh);
			filled = 1;
			cmd_start = 0;
		}
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
	if (parse_loop(head, toks, sh))
	{
		/*
		** bash still reads any here-documents collected before the syntax
		** error (`cat << EOF >` prompts for the body, then reports the error).
		*/
		status = sh->last_status;
		if (process_heredocs(head, sh) == -1)
			status = sh->last_status;
		else
			sh->last_status = status;
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
	return (head);
}
