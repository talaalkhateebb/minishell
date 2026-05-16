#include "minishell.h"

/* Person B — Days 6–7. Loop readline("> "); stop when line == delimiter.
** Expand $VAR if delimiter was unquoted. Pipe write end → buffer →
** dup2 read end onto stdin. Ctrl-C cancels the heredoc. */
