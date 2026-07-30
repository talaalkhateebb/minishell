NAME		= minishell

CC			= cc
CFLAGS		= -Wall -Wextra -Werror

INCLUDES	= -I includes
LDFLAGS		=
LIBS		= -lreadline

# macOS: locate Homebrew readline (brew install readline)
UNAME_S		:= $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	BREW_RL	:= $(shell brew --prefix readline 2>/dev/null)
	ifneq ($(BREW_RL),)
		INCLUDES	+= -I $(BREW_RL)/include
		LDFLAGS		+= -L $(BREW_RL)/lib
	endif
endif

# libft is authorized by the subject but not required, and this project does
# not use it — the few helpers we need live in src/utils/ as ms_*. The hooks
# below stay so that dropping a libft/ folder in would just work.
LIBFT_DIR	= libft
LIBFT		= $(LIBFT_DIR)/libft.a
HAS_LIBFT	:= $(shell test -f $(LIBFT_DIR)/Makefile && echo 1 || echo 0)
ifeq ($(HAS_LIBFT),1)
	INCLUDES	+= -I $(LIBFT_DIR)
	LIBS		+= -L $(LIBFT_DIR) -lft
	LIBFT_DEPS	= $(LIBFT)
endif

SRCS = \
	src/main.c \
	src/lexer/lexer.c \
	src/lexer/lexer_utils.c \
	src/parser/parser.c \
	src/parser/parser_build.c \
	src/parser/parser_core.c \
	src/parser/parser_utils.c \
	src/expander/expander.c \
	src/expander/expander_dollar.c \
	src/expander/expander_quotes.c \
	src/expander/expander_utils.c \
	src/env/env.c \
	src/env/env_more.c \
	src/env/env_set.c \
	src/signals/signals.c \
	src/signals/signals_heredoc.c \
	src/builtins_a/builtins_a.c \
	src/builtins_a/builtins_export.c \
	src/builtins_a/builtins_export2.c \
	src/executor/executor.c \
	src/executor/executor_path.c \
	src/executor/executor_error.c \
	src/redirections/redirections.c \
	src/pipes/pipes.c \
	src/pipes/pipes_utils.c \
	src/heredoc/heredoc.c \
	src/builtins_b/builtins_b.c \
	src/builtins_b/builtins_cd.c \
	src/builtins_b/builtins_exit.c \
	src/utils/utils_a.c \
	src/utils/utils_b.c \
	src/utils/utils_c.c \
	src/utils/input.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS) $(LIBFT_DEPS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) $(LIBS) -o $(NAME)

ifeq ($(HAS_LIBFT),1)
$(LIBFT):
	@$(MAKE) -C $(LIBFT_DIR)
endif

%.o: %.c includes/minishell.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)
ifeq ($(HAS_LIBFT),1)
	@$(MAKE) -C $(LIBFT_DIR) clean
endif

fclean: clean
	rm -f $(NAME)
ifeq ($(HAS_LIBFT),1)
	@$(MAKE) -C $(LIBFT_DIR) fclean
endif

re: fclean all

.PHONY: all clean fclean re
