#include "minishell.h"

/*
** ============================================================
** parser_argv.c — t_cmd construction helpers.
**
** cmd_new       — allocate an empty t_cmd (argv=NULL, no redirs).
** cmd_add_arg   — append one argument to argv (grow by 1).
** redir_new     — make a t_redir node.
** cmd_add_redir — append a t_redir to the command's list.
**
** No realloc available, so cmd_add_arg malloc's a fresh argv of
** size+1 each time, copies pointers, and frees the old spine.
** O(n²) for n args, but n is tiny in practice (commands rarely
** have 20+ args). Don't over-engineer.
** ============================================================
*/

/*
** cmd_new — allocate a fresh t_cmd with all fields zeroed.
*/
t_cmd	*cmd_new(void)
{
	t_cmd	*cmd;

	cmd = malloc(sizeof(t_cmd));
	if (!cmd)
		return (NULL);
	cmd->argv = NULL;
	cmd->redirs = NULL;
	cmd->next = NULL;
	return (cmd);
}

/*
** grow_argv — return a new argv with 'dup' added at the end.
**
** Walks the existing argv to count, allocates +2 slots (one for
** the new arg, one for the trailing NULL), copies the old
** pointers, places dup, and frees the OLD ARRAY SPINE (not the
** strings inside it — they move ownership into the new array).
**
** Returns NULL on malloc failure. In that case the caller must
** free 'dup' since we didn't store it.
*/
static char	**grow_argv(char **old, char *dup)
{
	int		n;
	char	**new_arr;
	int		i;

	n = 0;
	while (old && old[n])
		n++;
	new_arr = malloc(sizeof(char *) * (n + 2));
	if (!new_arr)
		return (NULL);
	i = 0;
	while (i < n)
	{
		new_arr[i] = old[i];
		i++;
	}
	new_arr[n] = dup;
	new_arr[n + 1] = NULL;
	free(old);
	return (new_arr);
}

/*
** cmd_add_arg — append a copy of 'arg' to cmd->argv.
**
** We copy because the source string lives in a t_token that will
** be freed shortly. After this returns 0, ownership of the copy
** is in cmd->argv.
*/
int	cmd_add_arg(t_cmd *cmd, const char *arg)
{
	char	*dup;
	char	**new_argv;

	dup = ms_strdup(arg);
	if (!dup)
		return (1);
	new_argv = grow_argv(cmd->argv, dup);
	if (!new_argv)
	{
		free(dup);
		return (1);
	}
	cmd->argv = new_argv;
	return (0);
}

/*
** redir_new — allocate one t_redir node.
**
** Takes ownership of 'target' (will be freed with free_redirs).
** expand_heredoc defaults to 1; the expander will toggle it to 0
** if the heredoc delimiter was quoted.
*/
t_redir	*redir_new(t_token_type type, char *target)
{
	t_redir	*r;

	r = malloc(sizeof(t_redir));
	if (!r)
		return (NULL);
	r->type = type;
	r->target = target;
	r->expand_heredoc = 1;
	r->next = NULL;
	return (r);
}

/*
** cmd_add_redir — append a redirection to the command.
**
** Copies 'target' so the source token can be freed independently.
** Appends to the END of cmd->redirs (order matters: redirections
** apply left-to-right).
*/
int	cmd_add_redir(t_cmd *cmd, t_token_type type, const char *target)
{
	char	*dup;
	t_redir	*r;
	t_redir	*cur;

	dup = ms_strdup(target);
	if (!dup)
		return (1);
	r = redir_new(type, dup);
	if (!r)
	{
		free(dup);
		return (1);
	}
	if (!cmd->redirs)
		cmd->redirs = r;
	else
	{
		cur = cmd->redirs;
		while (cur->next)
			cur = cur->next;
		cur->next = r;
	}
	return (0);
}
