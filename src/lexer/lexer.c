/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   lexer.c                                              :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Lexer — Person A (frontend).
**
** Splits the raw line into a t_token list. Whitespace separates words
** (outside quotes); the operators are | < > << >>.
**
** Quote characters are deliberately KEPT in the token value. The lexer
** only decides where a word ends; the expander later needs to know which
** stretches were single- vs double-quoted in order to decide what to
** expand, and strips the quotes itself. That is what makes
** `echo "hello"world'!'` stay ONE token and come out as `helloworld!`.
**
** *err is set to 1 on an unclosed quote.
*/

/* Advances past a "..." or '...' run. Returns 1 if the quote is closed. */
static int	skip_quoted(const char *line, int *i)
{
	char	quote;

	quote = line[*i];
	(*i)++;
	while (line[*i] && line[*i] != quote)
		(*i)++;
	if (!line[*i])
		return (0);
	(*i)++;
	return (1);
}

static int	lex_operator(const char *line, int *i, t_token **head)
{
	t_token_type	type;
	int				len;

	len = 1;
	if (line[*i] == '|')
		type = T_PIPE;
	else if (line[*i] == '<' && line[*i + 1] == '<')
		type = T_HEREDOC;
	else if (line[*i] == '<')
		type = T_REDIR_IN;
	else if (line[*i + 1] == '>')
		type = T_APPEND;
	else
		type = T_REDIR_OUT;
	if (type == T_HEREDOC || type == T_APPEND)
		len = 2;
	tok_add_back(head, tok_new(ms_substr(line, *i, len), type));
	*i += len;
	return (0);
}

/* Returns 0, or the opening quote character when it is never closed. */
static int	lex_word(const char *line, int *i, t_token **head)
{
	int	start;
	int	quote;

	start = *i;
	while (line[*i] && !is_space(line[*i]) && !is_op_char(line[*i]))
	{
		if (line[*i] == '"' || line[*i] == '\'')
		{
			quote = line[*i];
			if (!skip_quoted(line, i))
				return (quote);
		}
		else
			(*i)++;
	}
	tok_add_back(head, tok_new(ms_substr(line, start, *i - start), T_WORD));
	return (0);
}

t_token	*tokenize(const char *line, int *err)
{
	t_token	*head;
	int		i;

	head = NULL;
	*err = 0;
	i = 0;
	while (line[i])
	{
		if (is_space(line[i]))
			i++;
		else if (is_op_char(line[i]))
			lex_operator(line, &i, &head);
		else
		{
			*err = lex_word(line, &i, &head);
			if (*err)
				return (free_tokens(head), NULL);
		}
	}
	return (head);
}
