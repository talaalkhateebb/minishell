*This project has been created as part of the 42 curriculum by talkhati, salzghou.*

# Minishell

## Description

Minishell is a small POSIX-style shell — a reduced reimplementation of `bash`.
It reads a command line, parses it, expands variables, applies redirections,
builds pipelines, and executes the result, either as a built-in or as an
external program found through `PATH`.

The goal of the project is to gain practical knowledge of **processes** and
**file descriptors**: `fork`, `execve`, `waitpid`, `pipe`, `dup2`, and the
signal handling an interactive shell needs.

### Features

- Interactive prompt with command history
- Quoting: `'single'` (no interpretation) and `"double"` (only `$` interpreted)
- Environment variable expansion (`$VAR`) and exit status expansion (`$?`)
- Redirections: `<`, `>`, `>>`, and heredocs `<<` (with expansion when the
  delimiter is unquoted)
- Pipelines of arbitrary length (`cmd1 | cmd2 | cmd3`)
- Built-ins: `echo` (with `-n`), `cd`, `pwd`, `export`, `unset`, `env`, `exit`
- Signals behaving as in bash: `ctrl-C` gives a new prompt, `ctrl-D` exits,
  `ctrl-\` is ignored
- Exit statuses matching bash, including `128 + signo` for signalled children

## Instructions

### Build

```sh
make
```

Requires GNU `readline`. On macOS, install it with `brew install readline` —
the Makefile locates it automatically via `brew --prefix`. On Linux, install
`libreadline-dev`.

Other rules: `make clean`, `make fclean`, `make re`.

### Run

```sh
./minishell
```

Then type commands as you would in bash:

```sh
minishell$ echo "hello $USER" | cat
minishell$ ls -la > out.txt
minishell$ cat << EOF
minishell$ export FOO=bar && echo $FOO   # note: && is not implemented
```

### Tests

`run_tests.sh` is a differential test harness: it runs each case in **both**
the real bash and `./minishell` and reports any difference in output.

```sh
./run_tests.sh        # summary
./run_tests.sh -v     # show a diff for each failure
```

It also prints the list of behaviours deliberately *not* implemented, because
they fall outside the mandatory part (`&&`, `||`, subshells, wildcards, fd
duplication such as `2>&1`).

## Architecture

The shell is a pipeline of four stages, split across `src/`:

| Stage | Files | Role |
|---|---|---|
| Lexer | `lexer/` | line → token list, quote-aware |
| Parser | `parser/` | tokens → `t_cmd` list, expanding as it goes |
| Expander | `expander/` | `$VAR` / `$?` substitution and quote removal |
| Executor | `executor/`, `pipes/`, `redirections/`, `heredoc/` | fork, redirect, exec, wait |

Supporting modules: `env/` (the shell's own environment array), `signals/`,
`builtins_a/` and `builtins_b/`, and `utils/`.

One design point worth stating, because it is easy to get wrong: **the lexer
keeps the quote characters inside the token value**. It only decides where a
word ends. The expander later needs to know which stretches were single- versus
double-quoted in order to decide what to expand, so it is the expander that
strips the quotes, in the same pass as the substitution. That is what makes
`echo "hello"world'!'` stay a single word and come out as `helloworld!`.

## Resources

- GNU Bash Reference Manual — https://www.gnu.org/software/bash/manual/
- POSIX Shell Command Language —
  https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html
- GNU Readline documentation —
  https://tiswww.case.edu/php/chet/readline/readline.html
- `man 2 fork`, `man 2 execve`, `man 2 dup2`, `man 2 pipe`, `man 2 waitpid`,
  `man 7 signal`, `man 2 sigaction`

### Use of AI

> **⚠ REVIEW AND CORRECT THIS SECTION BEFORE SUBMITTING.** It is a required
> disclosure and it must describe what actually happened. The draft below was
> itself written by the AI assistant and reflects its own view of the work.

An AI assistant (Claude) was used on this project for the following tasks:

- **Testing.** The differential harness `run_tests.sh` was AI-generated.

All AI-generated code must be read and understood by the authors before
defending the project; the subject requires being able to justify and modify
any part of it on request.
