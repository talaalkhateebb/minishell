#!/bin/bash
# Differential test harness: runs each case in BOTH bash and ./minishell
# and reports any difference in output.
#
# minishell uses readline, which echoes "minishell$ <input>" when stdin is
# not a tty, so those prompt lines are filtered out before comparing.
#
# Usage: ./run_tests.sh [-v]     (-v prints the diff for every failure)

VERBOSE=0
[ "$1" = "-v" ] && VERBOSE=1

PASS=0
FAIL=0
FAILED_CASES=()

run_case() {
	local desc="$1"
	local script="$2"

	local want got
	# Both shells prefix their diagnostics with their own name; normalise
	# "bash: line 1: " and "minishell: " away so only the message matters.
	want=$(printf '%s\n' "$script" | bash --norc --noprofile 2>&1 \
		| sed -e 's/^bash: line [0-9]*: //' -e 's/^bash: //')
	got=$(printf '%s\n' "$script" | ./minishell 2>&1 \
		| sed -e 's/^minishell: //')

	if [ "$want" = "$got" ]; then
		PASS=$((PASS + 1))
	else
		FAIL=$((FAIL + 1))
		FAILED_CASES+=("$desc")
		if [ $VERBOSE -eq 1 ]; then
			echo "─── FAIL: $desc"
			echo "    input:  $(printf '%s' "$script" | tr '\n' ';')"
			echo "    bash:   $(printf '%s' "$want" | tr '\n' '|')"
			echo "    mini:   $(printf '%s' "$got" | tr '\n' '|')"
		fi
	fi
}

echo "=== echo / pwd ==="
run_case "echo empty"          'echo'
run_case "echo word"           'echo hello'
run_case "echo words"          'echo hello world'
run_case "echo -n"             'echo -n hello'
run_case "echo -n -n"          'echo -n -n -n hello'
run_case "echo dquote"         'echo "hello world"'
run_case "echo squote"         "echo 'hello world'"
run_case "pwd"                 'pwd'

echo "=== quoting ==="
run_case "adjacent dq"         'echo "hello"world'
run_case "adjacent sq"         "echo 'hello'world"
run_case "adjacent both"       "echo \"hello\"'world'"
run_case "inner spaces"        'echo "a    b"'
run_case "spaces sq"           "echo '   '"
run_case "expand dq"           'echo "$USER"'
run_case "no expand sq"        "echo '\$USER'"
run_case "lone dollar"         'echo "$"'
run_case "status in dq"        'echo "$?"'
run_case "mixed concat"        "echo \"hel\"lo'wor'ld"
run_case "empty string"        'echo ""'
run_case "var then text"       'echo $USER-suffix'

echo "=== env / export / unset ==="
run_case "export+read"         'export FOO=bar
echo $FOO'
run_case "unset"               'export FOO=bar
unset FOO
echo $FOO'
run_case "export multi"        'export A=1 B=2 C=3
echo $A$B$C'
run_case "export overwrite"    'export A=1
export A=2
echo $A'
run_case "unset unknown"       'unset NOPE_NOT_SET
echo $?'
run_case "export bad ident"    'export 1BAD=x
echo $?'
run_case "empty value"         'export E=
echo "[$E]"'
run_case "env has export"      'export ZZTEST=hi
env | grep ZZTEST'

echo "=== cd ==="
# /usr, not /tmp: on macOS /tmp is a symlink to /private/tmp, and bash
# tracks the logical path while getcwd() reports the resolved one.
run_case "cd abs"              'cd /usr
pwd'
run_case "cd dash"             'cd /usr
cd /
cd -
pwd'
run_case "cd home"             'cd
pwd'
run_case "cd nonexistent"      'cd /no/such/dir
echo $?'
run_case "cd updates PWD"      'cd /usr
echo $PWD'
run_case "cd dot dot"          'cd /usr/local
cd ..
pwd'

echo "=== redirections ==="
run_case "redir out"           'echo hi > /tmp/ms_t1
cat /tmp/ms_t1'
run_case "redir in"            'echo hi > /tmp/ms_t2
cat < /tmp/ms_t2'
run_case "append"              'echo a > /tmp/ms_t3
echo b >> /tmp/ms_t3
cat /tmp/ms_t3'
run_case "truncate"            'echo aaa > /tmp/ms_t4
echo b > /tmp/ms_t4
cat /tmp/ms_t4'
run_case "no cmd redir"        '> /tmp/ms_t5
cat /tmp/ms_t5
echo $?'
run_case "redir bad path"      'echo hi > /no/such/dir/f
echo $?'
run_case "input missing"       'cat < /no/such/file
echo $?'

echo "=== pipes ==="
run_case "pipe wc"             'echo hi | wc -c'
run_case "pipe cat"            'echo hi | cat'
run_case "pipe chain"          'echo hi | cat | cat'
run_case "pipe grep"           'printf "a\nb\nc\n" | grep b'
run_case "pipe status"         'echo hi | grep nope
echo $?'
run_case "pipe builtin"        'echo hi | cat
pwd'
run_case "pipe with redir"     'echo hi > /tmp/ms_t6
cat < /tmp/ms_t6 | cat'

echo "=== heredoc ==="
run_case "heredoc plain"       'cat << EOF
line one
EOF'
run_case "heredoc expand"      'cat << EOF
user is $USER
EOF'
run_case "heredoc quoted"      "cat << 'EOF'
user is \$USER
EOF"
run_case "heredoc pipe"        'cat << EOF | wc -l
a
b
EOF'

echo "=== exit status ==="
run_case "true"                'true
echo $?'
run_case "false"               'false
echo $?'
run_case "not found"           'nosuchcommand_xyz
echo $?'
run_case "ls missing"          'ls /this/does/not/exist
echo $?'
run_case "status persists"     'false
echo $?
echo $?'

echo "=== syntax errors ==="
# Compared on the error message only: a NON-interactive bash also echoes
# the offending line and aborts the whole script, while minishell reports
# the error and carries on to the next prompt. The interactive behaviour —
# which is what this project is graded on — is the same.
run_msg() {
	local desc="$1" script="$2"
	local want got
	want=$(printf '%s\n' "$script" | bash --norc --noprofile 2>&1 \
		| sed -e 's/^bash: line [0-9]*: //' | head -1)
	got=$(printf '%s\n' "$script" | ./minishell 2>&1 \
		| sed -e 's/^minishell: //' | head -1)
	if [ "$want" = "$got" ]; then
		PASS=$((PASS + 1))
	else
		FAIL=$((FAIL + 1))
		FAILED_CASES+=("$desc")
		[ $VERBOSE -eq 1 ] && printf '─── FAIL: %s\n    bash: %s\n    mini: %s\n' \
			"$desc" "$want" "$got"
	fi
}
run_msg "pipe at start"        '| echo hi'
run_msg "redir no target"      'echo hi >'
run_msg "pipe then pipe"       'echo a | | echo b'

echo
echo "=== deliberately NOT implemented (outside the mandatory subject) ==="
echo "  &&  ||          logical operators        — bonus part"
echo "  ( )             subshells                — bonus part"
echo "  *               wildcard globbing        — bonus part"
echo "  2>&1  >&2       fd duplication           — not in the subject"
echo "  cd a b          bash's 2-arg cd rewrite  — bash extension"
echo "  cmd |<newline>  multi-line continuation  — bash extension"
echo "                  (we reject a trailing pipe as a syntax error,"
echo "                   status 2; bash waits for a continuation line)"

echo
echo "════════════════════════════════"
echo "  passed: $PASS   failed: $FAIL"
if [ $FAIL -gt 0 ]; then
	echo "  failing: ${FAILED_CASES[*]}"
fi
echo "════════════════════════════════"
[ $FAIL -eq 0 ]
