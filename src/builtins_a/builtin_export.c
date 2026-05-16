#include "minishell.h"

/*
** ============================================================
** builtin_export.c — the `export` built-in.
**
** Behaviour:
**   - no args      → print every env entry as
**                    declare -x KEY="VALUE"  (sorted)
**   - with args    → for each arg, parse it as KEY=VALUE
**                    (or just KEY), validate, then env_set
**
** Argument parsing lives in export_args.c (process_export_arg).
** This file owns the "print sorted env" case + the top-level
** entry point.
** ============================================================
*/

/*
** env_count — count entries in a NULL-terminated string array.
*/
static int	env_count(char **envp)
{
	int	n;

	n = 0;
	while (envp && envp[n])
		n++;
	return (n);
}

/*
** sort_arr — in-place bubble sort by ms_strcmp on string pointers.
**
** O(n²) — fine for env which rarely exceeds 100 entries. We swap
** pointers, not strings, so each swap is just three assignments.
*/
static void	sort_arr(char **arr, int n)
{
	int		i;
	int		j;
	char	*tmp;

	i = 0;
	while (i < n - 1)
	{
		j = 0;
		while (j < n - 1 - i)
		{
			if (ms_strcmp(arr[j], arr[j + 1]) > 0)
			{
				tmp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = tmp;
			}
			j++;
		}
		i++;
	}
}

/*
** print_one_export — print one entry in declare -x format.
**
** With value:    declare -x KEY="VALUE"
** Without value: declare -x KEY
**
** Note: Bash's export output quotes the value with literal " — we
** match that. Real Bash also escapes internal quotes; we don't,
** to keep the code simple (subject doesn't require perfect match).
*/
static void	print_one_export(const char *entry)
{
	size_t	i;

	write(1, "declare -x ", 11);
	i = 0;
	while (entry[i] && entry[i] != '=')
		i++;
	if (entry[i] == '=')
	{
		write(1, entry, i);
		write(1, "=\"", 2);
		write(1, entry + i + 1, ms_strlen(entry + i + 1));
		write(1, "\"\n", 2);
	}
	else
	{
		write(1, entry, ms_strlen(entry));
		write(1, "\n", 1);
	}
}

/*
** print_sorted_env — copy env pointers, sort the copy, print.
**
** We sort a COPY of the pointer array — never reorder sh->envp
** itself, because the order matters for inheritance to children
** (and for stability across calls).
*/
static int	print_sorted_env(t_shell *sh)
{
	int		n;
	char	**arr;
	int		i;

	n = env_count(sh->envp);
	arr = malloc(sizeof(char *) * n);
	if (!arr)
		return (1);
	i = 0;
	while (i < n)
	{
		arr[i] = sh->envp[i];
		i++;
	}
	sort_arr(arr, n);
	i = 0;
	while (i < n)
	{
		print_one_export(arr[i]);
		i++;
	}
	free(arr);
	return (0);
}

/*
** builtin_export — entry point.
**
** With no args: print the sorted env.
** With args:    each arg goes through process_export_arg, which
**               splits KEY=VALUE, validates, and calls env_set.
**
** Return code: 0 if every arg succeeded, 1 if any failed (e.g.
** invalid identifier). Other args still get processed.
*/
int	builtin_export(char **argv, t_shell *sh)
{
	int	i;
	int	rc;

	if (!argv[1])
		return (print_sorted_env(sh));
	rc = 0;
	i = 1;
	while (argv[i])
	{
		if (process_export_arg(argv[i], sh) != 0)
			rc = 1;
		i++;
	}
	return (rc);
}
