#include "minishell.h"

/*
** ============================================================
** export_args.c — parsing one "KEY=VALUE" (or "KEY") argument
** to the `export` built-in.
**
** Behaviour:
**   - "KEY=VALUE"   → env_set(sh, "KEY", "VALUE")
**   - "KEY="        → env_set(sh, "KEY", "")
**   - "KEY"         → only set if KEY isn't already in env; if it
**                     is, leave the existing value alone (so the
**                     user-typed `export X` after `X=1` keeps X=1)
**   - invalid name  → print error to stderr, return 1
** ============================================================
*/

/*
** dup_range — malloc and copy the first 'len' chars of src.
**
** Like ms_strdup but for a bounded slice. Returns malloc'd
** string ending with '\0'. NULL on malloc failure.
*/
static char	*dup_range(const char *src, size_t len)
{
	char	*out;
	size_t	i;

	out = malloc(len + 1);
	if (!out)
		return (NULL);
	i = 0;
	while (i < len)
	{
		out[i] = src[i];
		i++;
	}
	out[len] = '\0';
	return (out);
}

/*
** print_export_err — Bash-style invalid-identifier error.
*/
static int	print_export_err(const char *arg)
{
	write(2, "minishell: export: `", 20);
	write(2, arg, ms_strlen(arg));
	write(2, "': not a valid identifier\n", 26);
	return (1);
}

/*
** find_eq — index of the first '=' in s, or index of '\0' if none.
*/
static size_t	find_eq(const char *s)
{
	size_t	i;

	i = 0;
	while (s[i] && s[i] != '=')
		i++;
	return (i);
}

/*
** apply_export — having parsed key+eq position, do the actual set.
**
** Three cases, controlled by arg[eq]:
**   - '=' present  → dup the value (possibly empty), env_set, free
**   - '=' absent and key already in env → leave it alone (return 0)
**   - '=' absent and key NOT in env     → env_set with NULL value
**
** Why those rules? They match `export FOO=bar` vs `export FOO`
** vs `export FOO` (with FOO already set) in Bash.
*/
static int	apply_export(const char *arg, size_t eq, const char *key,
		t_shell *sh)
{
	char	*value;
	int		rc;

	if (arg[eq] == '=')
	{
		value = ms_strdup(&arg[eq + 1]);
		if (!value)
			return (1);
		rc = env_set(sh, key, value);
		free(value);
		return (rc);
	}
	if (env_find_index(sh, key) >= 0)
		return (0);
	return (env_set(sh, key, NULL));
}

/*
** process_export_arg — parse and apply one argument to export.
**
** Steps:
**   1. Find the '=' position (or end-of-string).
**   2. Duplicate the part before '=' as 'key'.
**   3. Validate the name; print error and bail if invalid.
**   4. Delegate the actual env update to apply_export.
**   5. Free 'key' and return apply_export's result.
**
** Returns 0 on success, 1 on invalid identifier or malloc fail.
*/
int	process_export_arg(const char *arg, t_shell *sh)
{
	size_t	eq;
	char	*key;
	int		rc;

	eq = find_eq(arg);
	key = dup_range(arg, eq);
	if (!key)
		return (1);
	if (!is_valid_name(key))
	{
		free(key);
		return (print_export_err(arg));
	}
	rc = apply_export(arg, eq, key, sh);
	free(key);
	return (rc);
}
