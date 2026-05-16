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

# libft (optional during day-0 scaffold; required for full project)
LIBFT_DIR	= libft
LIBFT		= $(LIBFT_DIR)/libft.a
HAS_LIBFT	:= $(shell test -f $(LIBFT_DIR)/Makefile && echo 1 || echo 0)
ifeq ($(HAS_LIBFT),1)
	INCLUDES	+= -I $(LIBFT_DIR)
	LIBS		+= -L $(LIBFT_DIR) -lft
endif

SRCS = \
	src/main.c \
	src/lexer/lexer.c \
	src/parser/parser.c \
	src/expander/expander.c \
	src/env/env.c \
	src/env/env_lookup.c \
	src/env/env_entry.c \
	src/env/env_mod.c \
	src/signals/signals.c \
	src/builtins_a/builtins_a.c \
	src/executor/executor.c \
	src/redirections/redirections.c \
	src/pipes/pipes.c \
	src/heredoc/heredoc.c \
	src/builtins_b/builtins_b.c \
	src/utils/utils_a.c \
	src/utils/utils_b.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): libft_step $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) $(LIBS) -o $(NAME)

libft_step:
ifeq ($(HAS_LIBFT),1)
	@$(MAKE) -C $(LIBFT_DIR)
else
	@echo "note: libft/ not present yet — building without it (day-0 scaffold)."
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

.PHONY: all clean fclean re libft_step
