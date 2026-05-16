#include "minishell.h"

/*
** ============================================================
** expander.c — top of the expander module. Called by parse_line
** AFTER build_cmds, BEFORE returning the t_cmd list.
**
** For every command in the pipeline:
**   - expand every argv[i]     → $VAR / $? / quotes
**   - expand every redir.target → same, except heredoc delimiters
**     which only get quotes stripped (no $ expansion in the delim)
** ============================================================
*/

/*
** expand_argv — expand each argument of one command in place.
**
** If expansion fails (malloc), we leave the original argv[i]
** untouched. The shell will still try to run the command; better
** that than crashing.
*/
static void	expand_argv(t_cmd *c, t_shell *sh)
{
	int		i;
	char	*new_val;

	if (!c->argv)
		return ;
	i = 0;
	while (c->argv[i])
	{
		new_val = expand_word(c->argv[i], sh);
		if (new_val)
		{
			free(c->argv[i]);
			c->argv[i] = new_val;
		}
		i++;
	}
}

/*
** expand_one_redir — handle a single redirection.
**
** For T_HEREDOC: detect whether the delimiter was quoted (this
** decides whether the heredoc body gets $-expanded later, at
** read time in the backend). Then strip quotes from the
** delimiter itself — but DO NOT $-expand it.
**
** For T_REDIR_IN / T_REDIR_OUT / T_APPEND: full expansion.
*/
static void	expand_one_redir(t_redir *r, t_shell *sh)
{
	char	*new_target;

	if (r->type == T_HEREDOC)
	{
		if (delim_is_quoted(r->target))
			r->expand_heredoc = 0;
		new_target = strip_quotes_only(r->target);
	}
	else
		new_target = expand_word(r->target, sh);
	if (new_target)
	{
		free(r->target);
		r->target = new_target;
	}
}

/*
** expand_redirs — walk a command's redirection list and expand each.
*/
static void	expand_redirs(t_cmd *c, t_shell *sh)
{
	t_redir	*r;

	r = c->redirs;
	while (r)
	{
		expand_one_redir(r, sh);
		r = r->next;
	}
}

/*
** expand_cmds — public entry. Walk every command in the pipeline
** and expand its argv and redirections.
*/
void	expand_cmds(t_cmd *cmds, t_shell *sh)
{
	t_cmd	*c;

	c = cmds;
	while (c)
	{
		expand_argv(c, sh);
		expand_redirs(c, sh);
		c = c->next;
	}
}
