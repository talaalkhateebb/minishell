#include "minishell.h"

/*
** ============================================================
** dispatch.c — built-in dispatcher used by main.c (and later by
** the executor for the pipeline case, in the child).
**
** Built-ins run in TWO places:
**   1. Parent process — when the command is a single command
**      with no pipe (cd, export, unset MUST run in the parent
**      so their changes persist).
**   2. Child process — when the built-in is part of a pipeline,
**      e.g. `echo hi | cat`. The change wouldn't persist anyway
**      because pipeline stages are children.
**
** try_run_builtin handles case 1. The partner's executor will
** call run_frontend_builtin / run_backend_builtin for case 2.
** ============================================================
*/

/*
** run_frontend_builtin — dispatch to A's built-ins (echo/env/
** export/unset).
**
** Returns the built-in's exit code, or -1 if argv[0] isn't one
** of A's built-ins (so the caller can try B's).
*/
int	run_frontend_builtin(t_cmd *c, t_shell *sh)
{
	const char	*name;

	name = c->argv[0];
	if (ms_strcmp(name, "echo") == 0)
		return (builtin_echo(c->argv));
	if (ms_strcmp(name, "env") == 0)
		return (builtin_env(sh));
	if (ms_strcmp(name, "export") == 0)
		return (builtin_export(c->argv, sh));
	if (ms_strcmp(name, "unset") == 0)
		return (builtin_unset(c->argv, sh));
	return (-1);
}

/*
** run_backend_builtin — dispatch to B's built-ins (cd/pwd/exit).
**
** Currently the partner's implementations are stubs returning 0.
** Once the partner fills them in, this will route correctly.
*/
int	run_backend_builtin(t_cmd *c, t_shell *sh)
{
	const char	*name;

	name = c->argv[0];
	if (ms_strcmp(name, "cd") == 0)
		return (builtin_cd(c->argv, sh));
	if (ms_strcmp(name, "pwd") == 0)
		return (builtin_pwd());
	if (ms_strcmp(name, "exit") == 0)
		return (builtin_exit(c->argv, sh));
	return (-1);
}

/*
** try_run_builtin — entry point used by main.c.
**
** Refuses (returns -1) when the command can't safely run in the
** parent:
**   - cmds->next      → it's part of a pipeline, defer to executor
**   - cmds->redirs    → has redirections, defer to executor (the
**                       backend owns redirection setup)
**   - no argv         → empty command, nothing to dispatch
**
** Otherwise tries frontend's built-ins then backend's, returning
** the first match's exit code, or -1 if neither matched.
*/
int	try_run_builtin(t_cmd *cmds, t_shell *sh)
{
	int	rc;

	if (cmds->next || cmds->redirs)
		return (-1);
	if (!cmds->argv || !cmds->argv[0])
		return (-1);
	rc = run_frontend_builtin(cmds, sh);
	if (rc != -1)
		return (rc);
	return (run_backend_builtin(cmds, sh));
}
