/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   lexer_utils.c                                        :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	is_space(char c)
{
	return (c == ' ' || c == '\t' || c == '\n'
		|| c == '\v' || c == '\f' || c == '\r');
}

int	is_op_char(char c)
{
	return (c == '|' || c == '<' || c == '>');
}

t_token	*tok_new(char *value, t_token_type type)
{
	t_token	*tok;

	if (!value)
		return (NULL);
	tok = malloc(sizeof(t_token));
	if (!tok)
	{
		free(value);
		return (NULL);
	}
	tok->value = value;
	tok->type = type;
	tok->next = NULL;
	return (tok);
}

void	tok_add_back(t_token **head, t_token *tok)
{
	t_token	*cur;

	if (!tok)
		return ;
	if (!*head)
	{
		*head = tok;
		return ;
	}
	cur = *head;
	while (cur->next)
		cur = cur->next;
	cur->next = tok;
}

void	free_tokens(t_token *tokens)
{
	t_token	*next;

	while (tokens)
	{
		next = tokens->next;
		free(tokens->value);
		free(tokens);
		tokens = next;
	}
}
