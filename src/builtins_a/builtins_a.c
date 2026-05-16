#include "minishell.h"

/*
** ============================================================
** builtins_a.c — echo, env, unset, and the shared is_valid_name.
**
** echo  — print args separated by spaces, newline unless -n.
**         Bash quirk: a run of -nnnn... flags all count, and
**         multiple -n flags chain.
** env   — list every "KEY=VALUE" entry (skips KEY-only entries,
**         which are exported-but-unset variables).
** unset — remove each named variable from sh->envp.
**
** is_valid_name is here because both unset and export need it.
** export lives in builtin_export.c / export_args.c so this file
** stays under the norm's 5-function ceiling.
** ============================================================
*/

/*
** has_n_flag — is this argument exactly "-n", "-nn", "-nnn", ...?
**
** Bash accepts any run of n's after the dash; the presence of the
** flag suppresses the trailing newline. Anything else (e.g. "-na",
** "--n", "-") is treated as a literal argument to print.
*/
static int	has_n_flag(const char *s)
{
	size_t	i;

	if (s[0] != '-' || s[1] != 'n')
		return (0);
	i = 1;
	while (s[i] == 'n')
		i++;
	return (s[i] == '\0');
}

/*
** builtin_echo — implement `echo [-n] [args...]`.
**
** Algorithm:
**   1. Skip leading -n flags (each suppresses the newline).
**   2. Print remaining args, with a single space between them.
**   3. Append \n unless any -n was seen.
**
** Returns 0 always — Bash's echo doesn't really fail.
*/
int	builtin_echo(char **argv)
{
	int	i;
	int	newline;

	i = 1;
	newline = 1;
	while (argv[i] && has_n_flag(argv[i]))
	{
		newline = 0;
		i++;
	}
	while (argv[i])
	{
		write(1, argv[i], ms_strlen(argv[i]));
		if (argv[i + 1])
			write(1, " ", 1);
		i++;
	}
	if (newline)
		write(1, "\n", 1);
	return (0);
}

/*
** builtin_env — implement `env` (no options).
**
** Walks sh->envp and prints every entry that contains '='.
** Entries without '=' are exported-but-unset variables (only
** visible via `export`), so `env` skips them — matching Bash.
*/
int	builtin_env(t_shell *sh)
{
	int		i;
	size_t	j;

	if (!sh->envp)
		return (0);
	i = 0;
	while (sh->envp[i])
	{
		j = 0;
		while (sh->envp[i][j] && sh->envp[i][j] != '=')
			j++;
		if (sh->envp[i][j] == '=')
		{
			write(1, sh->envp[i], ms_strlen(sh->envp[i]));
			write(1, "\n", 1);
		}
		i++;
	}
	return (0);
}

/*
** is_valid_name — does s look like a shell variable name?
**
** Rule: starts with [A-Za-z_], continues with [A-Za-z0-9_].
** Empty string or wrong characters → 0 (invalid).
**
** Re-uses is_var_start / is_var_char from expand_var.c so the
** rules stay in sync between expansion and validation.
*/
int	is_valid_name(const char *s)
{
	int	i;

	if (!s || !s[0] || !is_var_start(s[0]))
		return (0);
	i = 1;
	while (s[i])
	{
		if (!is_var_char(s[i]))
			return (0);
		i++;
	}
	return (1);
}

/*
** builtin_unset — implement `unset NAME...`.
**
** For each argument:
**   - if the name is invalid, print an error and remember to
**     return 1 (but keep processing the rest)
**   - otherwise call env_unset (which is a no-op if not present)
*/
int	builtin_unset(char **argv, t_shell *sh)
{
	int	i;
	int	rc;

	rc = 0;
	i = 1;
	while (argv[i])
	{
		if (!is_valid_name(argv[i]))
		{
			write(2, "minishell: unset: `", 19);
			write(2, argv[i], ms_strlen(argv[i]));
			write(2, "': not a valid identifier\n", 26);
			rc = 1;
		}
		else
			env_unset(sh, argv[i]);
		i++;
	}
	return (rc);
}
