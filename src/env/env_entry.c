#include "minishell.h"

/*
** ============================================================
** env_entry.c — builds "KEY=VALUE" strings from key+value parts.
**
** Centralised so env_set and (later) builtin_export both call the
** same code. Two cases: with value ("KEY=VALUE") and without
** value ("KEY", representing an exported-but-unset variable).
** ============================================================
*/

/*
** copy_chars — write 'len' chars of 'src' into 'dst' starting at 'at'.
**
** Tiny helper to keep env_make_entry under the norm's 25-line cap.
** dst must have room for at+len bytes. We don't write '\0' here.
*/
static void	copy_chars(char *dst, size_t at, const char *src, size_t len)
{
	size_t	i;

	i = 0;
	while (i < len)
	{
		dst[at + i] = src[i];
		i++;
	}
}

/*
** make_entry_kv — build "KEY=VALUE".
**
** Allocates klen + vlen + 2 bytes:
**   klen for the key, 1 for '=', vlen for the value, 1 for '\0'.
*/
static char	*make_entry_kv(const char *key, const char *value)
{
	size_t	klen;
	size_t	vlen;
	char	*out;

	klen = ms_strlen(key);
	vlen = ms_strlen(value);
	out = malloc(klen + vlen + 2);
	if (!out)
		return (NULL);
	copy_chars(out, 0, key, klen);
	out[klen] = '=';
	copy_chars(out, klen + 1, value, vlen);
	out[klen + 1 + vlen] = '\0';
	return (out);
}

/*
** env_make_entry — public entry point. Picks "KEY=VALUE" or "KEY"
** based on whether value is NULL.
**
** Returns malloc'd memory. Caller frees it (or stores it in
** sh->envp where env_free will free it).
*/
char	*env_make_entry(const char *key, const char *value)
{
	size_t	klen;
	char	*out;

	if (value)
		return (make_entry_kv(key, value));
	klen = ms_strlen(key);
	out = malloc(klen + 1);
	if (!out)
		return (NULL);
	copy_chars(out, 0, key, klen);
	out[klen] = '\0';
	return (out);
}
