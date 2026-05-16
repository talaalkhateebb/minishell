#include "minishell.h"

/*
** ============================================================
** lexer_tokens.c — token list helpers.
**
** Linked-list operations: create a node, append to a list,
** free the whole list.
** ============================================================
*/

/*
** token_new — allocate one t_token node.
**
** Takes ownership of 'value' — that string will be freed when
** the token is freed. If malloc fails we DO NOT free value
** (caller knows whether they want to retry or give up).
**
** type is one of t_token_type. next is initialised to NULL.
*/
t_token	*token_new(char *value, t_token_type type)
{
	t_token	*node;

	node = malloc(sizeof(t_token));
	if (!node)
		return (NULL);
	node->value = value;
	node->type = type;
	node->next = NULL;
	return (node);
}

/*
** token_append — add 'node' to the end of the list.
**
** Walks from *head to the tail and links node there. If *head
** is NULL (empty list), the new node becomes the head.
**
** O(n) per append: fine for the small token counts we deal with.
** If perf matters later, the caller can track a tail pointer.
*/
void	token_append(t_token **head, t_token *node)
{
	t_token	*cur;

	if (!*head)
	{
		*head = node;
		return ;
	}
	cur = *head;
	while (cur->next)
		cur = cur->next;
	cur->next = node;
}

/*
** free_tokens — release every node and every value in the list.
**
** Safe to call with head = NULL (the while loop just exits).
** After this, the caller's pointer is dangling — clear it to
** NULL if you intend to reuse the variable.
*/
void	free_tokens(t_token *head)
{
	t_token	*next;

	while (head)
	{
		next = head->next;
		free(head->value);
		free(head);
		head = next;
	}
}
