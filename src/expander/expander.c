#include "minishell.h"

/*
** Expander — Person A (frontend), Days 8–9.
**
** For each T_WORD and the body of "..." segments:
**   - find $[A-Za-z_][A-Za-z0-9_]* → replace with env value (empty if unset)
**   - $? → replace with sh->last_status as a string
**   - inside '...' → no expansion
**   - inside "..." → $-expansion only
**   - after expansion, strip the quote characters
**
** Heredoc: if delimiter was quoted, do NOT expand inside the body.
**          (See the expand_heredoc flag on t_redir.)
*/
