#include "minishell.h"

size_t	ms_strlen(const char *s)
{
	size_t	i;

	i = 0;
	while (s[i])
		i++;
	return (i);
}

char	*ms_strdup(const char *s)
{
	size_t	len;
	char	*out;
	size_t	i;

	len = ms_strlen(s);
	out = malloc(len + 1);
	if (!out)
		return (NULL);
	i = 0;
	while (i < len)
	{
		out[i] = s[i];
		i++;
	}
	out[len] = '\0';
	return (out);
}
