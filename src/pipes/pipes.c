#include "minishell.h"

/* Person B — Days 8–10. N-1 pipes, N forks, close every pipe fd in
** every child or you'll hang. Pipeline status = last command's status. */
