#include "minishell.h"

/*
** ============================================================
** parser.c — top of the frontend pipeline.
**
** parse_line orchestrates the whole frontend:
**   raw line  →  tokenize  →  validate  →  build t_cmd list  →
**                expand $VAR / $?  →  return list to executor.
**
** On any error, returns NULL and sets sh->last_status to 2 (the
** Bash exit code for a syntax error). The caller (main) then
** simply skips execution and prompts again.
**
** This file also owns free_cmds, which walks the result list
** and releases every allocation.
** ============================================================
*/

/*
** parse_line — input string → ready-to-execute t_cmd list.
**
** 1. tokenize             — cut line into tokens.
** 2. validate_syntax      — reject |, |, < without filename, etc.
** 3. build_cmds           — group tokens into t_cmd structures.
** 4. expand_cmds          — replace $VAR and $? using sh->envp.
**
** Returns NULL on failure (sets last_status = 2 for syntax errors).
** On success, caller must call free_cmds to release.
*/
t_cmd	*parse_line(const char *line, t_shell *sh)
{
	t_token	*tokens;
	t_cmd	*cmds;

	tokens = tokenize(line);
	if (!tokens)
	{
		sh->last_status = 2;
		return (NULL);
	}
	if (validate_syntax(tokens) != 0)
	{
		sh->last_status = 2;
		free_tokens(tokens);
		return (NULL);
	}
	cmds = build_cmds(tokens);
	free_tokens(tokens);
	if (!cmds)
		return (NULL);
	expand_cmds(cmds, sh);
	return (cmds);
}

/*
** free_argv — free a NULL-terminated array of malloc'd strings.
*/
static void	free_argv(char **argv)
{
	int	i;

	if (!argv)
		return ;
	i = 0;
	while (argv[i])
	{
		free(argv[i]);
		i++;
	}
	free(argv);
}

/*
** free_redirs — walk and free a t_redir list.
*/
static void	free_redirs(t_redir *redirs)
{
	t_redir	*next;

	while (redirs)
	{
		next = redirs->next;
		free(redirs->target);
		free(redirs);
		redirs = next;
	}
}

/*
** free_cmds — release the whole t_cmd list and all child memory.
**
** Called by main() after the executor returns, and also internally
** when a parser step fails mid-way through building.
*/
void	free_cmds(t_cmd *cmds)
{
	t_cmd	*next;

	while (cmds)
	{
		next = cmds->next;
		free_argv(cmds->argv);
		free_redirs(cmds->redirs);
		free(cmds);
		cmds = next;
	}
}
