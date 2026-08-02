#!/bin/bash
# ============================================================================
# run_tests.sh — differential test harness for minishell
#
# Every case is fed to BOTH the real bash and ./minishell, on stdin, and the
# combined stdout+stderr+exit-status is compared. The shell's own name is
# normalised ("bash: " / "minishell: " -> "SH: ") so only real behavioural
# differences are reported.
#
#   ./run_tests.sh        summary
#   ./run_tests.sh -v     show a diff for every failure
#
# Cases are newline-separated command lists, never ';'-separated: `;` is
# outside the mandatory subject and minishell deliberately does not implement
# it (see the list printed at the end of a run).
#
# NOTE ON BASH VERSIONS: macOS ships bash 3.2, which differs from the bash 5
# that 42 evaluates against — `exit abc` (2 vs 255), `cd a b` (error vs
# accepted) and "Is a directory" capitalisation all changed after 3.2.
# minishell follows bash 5. Point the harness at a modern bash with:
#   BASH_BIN=/opt/homebrew/bin/bash ./run_tests.sh
# ============================================================================

cd "$(dirname "$0")" || exit 1

MS=./minishell
BASH_BIN=${BASH_BIN:-bash}
VERBOSE=0
[ "$1" = "-v" ] && VERBOSE=1

if [ ! -x "$MS" ]; then
	echo "run_tests.sh: $MS not built — run 'make' first." >&2
	exit 1
fi

PASS=0
FAIL=0
FAILED_NAMES=()

# Besides the shell name, this drops two lines bash only emits when it is
# reading a SCRIPT rather than a terminal: the echo of the offending line
# ("bash: `echo hi >'") and the "unexpected end of file" that follows a
# construct it would have asked PS2 to complete. Neither appears in an
# interactive bash, so neither is a real difference from minishell.

# The bash name is matched loosely because bash announces itself as whatever
# argv[0] was: plain "bash: " when it is on PATH, but the full
# "/opt/homebrew/bin/bash: line 1: " when BASH_BIN points at a build by path.
# Anchoring on the literal "bash: " silently failed 27 cases whose message
# bodies were in fact identical.
norm() { sed -e 's/^minishell: /SH: /' \
	-e 's|^[^ ]*bash: line [0-9]*: |SH: |' -e 's|^[^ ]*bash: |SH: |' \
	-e "/^SH: \`.*'$/d" -e '/^SH: syntax error: unexpected end of file$/d'; }

# run_expect <name> <script> <expected>
# For the handful of cases where the evaluation sheet asks for something
# bash does not do. Those cannot be diffed against bash, so the expected
# output is spelled out here instead — see the notes at the bottom.
run_expect() {
	local name="$1" script="$2" want="$3" got
	got=$( { printf '%s\n' "$script" | $MS; echo "EXIT=$?"; } 2>&1 | norm )
	if [ "$got" = "$want" ]; then
		PASS=$((PASS + 1))
		return
	fi
	FAIL=$((FAIL + 1))
	FAILED_NAMES+=("$name")
	echo "FAIL: $name"
	if [ "$VERBOSE" = 1 ]; then
		diff <(printf '%s\n' "$want") <(printf '%s\n' "$got") \
			--label expected --label minishell -u | sed 's/^/    /'
	fi
}

# run <name> <script>
run() {
	local name="$1" script="$2" bout mout
	bout=$( { printf '%s\n' "$script" | $BASH_BIN; echo "EXIT=$?"; } 2>&1 | norm )
	mout=$( { printf '%s\n' "$script" | $MS;       echo "EXIT=$?"; } 2>&1 | norm )
	if [ "$bout" = "$mout" ]; then
		PASS=$((PASS + 1))
		return
	fi
	FAIL=$((FAIL + 1))
	FAILED_NAMES+=("$name")
	if [ "$VERBOSE" = 1 ]; then
		echo "--------------------------------------------------------------"
		echo "FAIL: $name"
		echo "  input:"
		printf '%s\n' "$script" | sed 's/^/    | /'
		diff <(printf '%s\n' "$bout") <(printf '%s\n' "$mout") \
			--label bash --label minishell -u | sed 's/^/    /'
	else
		echo "FAIL: $name"
	fi
}

echo "=== minishell differential tests (bash: $($BASH_BIN --version | head -1)) ==="
echo

# ---------------------------------------------------------------- echo
run "echo simple"            'echo hello world'
run "echo -n"                'echo -n hi'
run "echo -nnn"              'echo -nnn hi'
run "echo -n -n"             'echo -n -n hi'
run "echo no args"           'echo'
run "echo empty quotes"      'echo ""'
run "echo mixed quotes"      "echo \"hello\"world'!'"
run "echo flag after arg"    'echo hi -n'
run "echo inner spaces"      'echo "a   b"'
run "echo many args"         'echo 1 2 3 4 5 6 7 8 9 10'

# ------------------------------------------------------------- statuses
run "status ok"              $'ls > /dev/null\necho $?'
run "status false"           $'/usr/bin/false\necho $?'
run "status not found"       $'nosuchcmd_xyz\necho $?'
run_expect "status is a directory" $'/tmp\necho $?' \
	$'SH: /tmp: Is a directory\n126\nEXIT=0'
run "status not executable"  $'/etc/passwd\necho $?'
run "status relative"        $'./nosuchfile_xyz\necho $?'

# ------------------------------------------------------------ expansion
run "expand HOME"            'echo $HOME'
run "expand unset"           'echo [$NOSUCHVAR_XYZ]'
run "expand in dquotes"      'echo "$HOME"'
run "no expand in squotes"   "echo '\$HOME'"
run "expand \$?"             $'nosuchcmd_xyz\necho $?'
run "dollar alone"           'echo $'
run "dollar in dquotes"      'echo "$"'
run "field splitting"        $'export V="a   b"\necho $V'
run "no split when quoted"   $'export V="a   b"\necho "$V"'
run "empty var disappears"   $'export E=""\necho x $E y'
run "var as command"         $'export C=echo\n$C hi'
run "quote inside quote"     'echo "it'"'"'s"'
run "quoted command name"    '"echo" hi'

# ------------------------------------------------- braced expansion ${}
run "braced"                 'echo "[${HOME}]"'
run "braced unset"           'echo "[${NOSUCHVAR_XYZ}]"'
run "braced adjacent"        'echo a${HOME}b'
run "braced splits"          $'export V="a  b"\necho ${V}'
run "braced quoted no split" $'export V="a  b"\necho "${V}"'
run "braced \${?}"           $'nosuchcmd_xyz\necho ${?}'
run "braced in squotes"      "echo '\${HOME}'"
run "bad substitution"       $'echo ${}\necho next'
run "bad subst bad name"     $'echo ${1BAD}\necho next'
run "bad subst in pipe"      $'echo ${} | cat\necho next'

# ---------------------------------------------------------------- pipes
run "pipe simple"            'echo hi | cat'
run "pipe three"             'echo hi | cat | cat'
run "pipe five"              'echo a | cat | cat | cat | cat'
run "pipe not found"         $'echo hi | nosuchcmd_xyz\necho $?'
run "pipe grep"              'printf "a\nb\n" | grep b'
run "pipe wc"                'echo hi | wc -c'
run "pipe pwd"               'pwd | cat'
run "pipe builtin status"    $'echo hi | cd /nope\necho $?'
run "pipe sigpipe"           'yes | head -2'

# --------------------------------------------------------- redirections
run "redir out"              $'echo hi > /tmp/ms_t1\ncat /tmp/ms_t1'
run "redir append"           $'echo a > /tmp/ms_t2\necho b >> /tmp/ms_t2\ncat /tmp/ms_t2'
run "redir in"               $'echo z > /tmp/ms_t3\ncat < /tmp/ms_t3'
run "redir in missing"       $'cat < /tmp/no_such_xyz\necho $?'
run "redir two outputs"      $'echo hi > /tmp/ms_t4 > /tmp/ms_t5\ncat /tmp/ms_t4\ncat /tmp/ms_t5'
run "redir onto directory"   $'echo hi > /tmp\necho $?'
run "redir with no command"  $'> /tmp/ms_t6\necho $?'
run "redir before command"   $'> /tmp/ms_t7 echo hi\ncat /tmp/ms_t7'
run "redir mid command"      $'echo hi > /tmp/ms_t8 world\ncat /tmp/ms_t8'
run "redir into pipe"        $'echo q > /tmp/ms_t9\ncat < /tmp/ms_t9 | cat'
run "redir out of pipe"      $'echo p | cat > /tmp/ms_ta\ncat /tmp/ms_ta'
run "redir target from var"  $'export F=/tmp/ms_tb\necho hi > $F\ncat /tmp/ms_tb'

# -------------------------------------------------------------- heredoc
run "heredoc basic"          $'cat << EOF\nhello\nEOF'
run "heredoc expands"        $'export V=world\ncat << EOF\nhi $V\nEOF'
run "heredoc dquoted delim"  $'export V=world\ncat << "EOF"\nhi $V\nEOF'
run "heredoc squoted delim"  $'export V=world\ncat << \'EOF\'\nhi $V\nEOF'
run "heredoc braced var"     $'export V=w\ncat << EOF\n[${V}]\nEOF'
run "heredoc into pipe"      $'cat << EOF | cat\nx\nEOF'
run "heredoc twice"          $'cat << A << B\n1\nA\n2\nB'
run "heredoc empty body"     $'cat << EOF\nEOF'
run "heredoc keeps quotes"   $'cat << EOF\na"b\'c\nEOF'

# ------------------------------------------------------------- builtins
run "pwd"                    'pwd'
run "cd then pwd"            $'cd /tmp\npwd'
run "cd is logical"          $'cd /tmp\ncd ..\npwd'
run "cd dot"                 $'cd /tmp\ncd .\npwd'
run "cd double dot twice"    $'cd /usr/local/bin\ncd ../..\npwd'
run "cd trailing slash"      $'cd /tmp/\npwd'
run "cd redundant slashes"   $'cd /tmp//.//\npwd'
run "cd dash roundtrip"      $'cd /tmp\ncd -\npwd'
run "cd sets PWD OLDPWD"     $'cd /tmp\necho $PWD\necho $OLDPWD'
run "cd above root"          $'cd /\ncd ..\npwd'
run "cd nonexistent"         $'cd /no_such_dir_xyz\necho $?'
run "cd onto a file"         $'cd /etc/passwd\necho $?'
run "cd empty string"        $'cd ""\necho $?'
run "cd dash unset OLDPWD"   $'cd -\necho $?'
run "pwd ignores fake PWD"   $'export PWD=/nonsense\npwd'
run "cd home"                $'cd\npwd'

# Standing in a directory that was deleted underneath us. bash warns once and
# keeps going (status 0), and because it can no longer canonicalize anything
# it records $PWD as the raw "a/b/..". The second cd is silent and back to
# normal logical resolution. Not diffable against bash: bash's warning comes
# out as "cd: ...", with no shell-name prefix for norm() to rewrite.
CDTMP=$(mktemp -d)
run_expect "cd deleted cwd" \
	$'mkdir -p '"$CDTMP"$'/a/b\ncd '"$CDTMP"$'/a/b\nrm -r ../../a\ncd ..\necho $?\npwd\ncd ..\necho $?\npwd' \
	"SH: error retrieving current directory: getcwd: cannot access parent directories: No such file or directory
0
$CDTMP/a/b/..
0
$CDTMP
EXIT=0"
rm -rf "$CDTMP"
run "export lists PATH"      'export | grep -c "declare -x PATH="'
run "export sets"            $'export FOO=bar\necho $FOO'
run "export invalid name"    $'export 1BAD=x\necho $?'
run "export without value"   $'export ZQ\nenv | grep -c "^ZQ"'
run "export overwrite"       $'export A=1\nexport A=2\necho $A'
run "export quoted value"    $'export "A=b c"\necho [$A]'
run "export OLDPWD at start" 'export | grep -c "^declare -x OLDPWD$"'
run "unset"                  $'export FOO=bar\nunset FOO\necho [$FOO]'
run "unset invalid name"     $'unset 1BAD\necho $?'
run "unset empty name"       $'unset ""\necho $?'
run "unset empty and name"   $'unset "" test\necho $?'
run "unset no args"          $'unset\necho $?'
run "env lists PATH"         'env | grep -c "^PATH="'
run_expect "env missing command" $'env test/\necho $?' \
	$'env: \'test/\': No such file or directory\n127\nEXIT=0'
run_expect "env unknown name"    $'env nosuchcmd\necho $?' \
	$'env: \'nosuchcmd\': No such file or directory\n127\nEXIT=0'
run_expect "env directory"       $'env /tmp\necho $?' \
	$'env: \'/tmp\': Permission denied\n126\nEXIT=0'
run_expect "env not a directory" $'env Makefile/\necho $?' \
	$'env: \'Makefile/\': Not a directory\n126\nEXIT=0'
run_expect "env empty command"   $'env ""\necho $?' \
	$'env: \'\': No such file or directory\n127\nEXIT=0'
run "env runs command"       $'env echo hi\necho $?'
run "env passes exports"     $'export ZFOO=bar\nenv env | grep -c "^ZFOO=bar$"'
run "exit code"              'exit 42'
run "exit modulo 256"        'exit 300'
run "exit negative"          'exit -1'
run "exit too many args"     $'exit 1 2\necho after'

# ------------------------------------------------------- PATH behaviour
run "PATH unset"             $'unset PATH\nls\necho $?'
run "PATH empty"             $'export PATH=""\nls\necho $?'
run "absolute path"          '/bin/echo hi'
run "relative path"          $'cd /bin\n./echo hi'

# --------------------------------------------------------- syntax errors
run "pipe with no command"   '| echo hi'
run "two pipes"              'echo a | | echo b'
run "redir with no target"   'echo hi >'
run "redir after redir"      'echo hi > > x'
run "unclosed dquote"        'echo "abc'
run "unclosed squote"        "echo 'abc"
run "unterminated brace"     'echo a${HOME'

# ---------------------------------------------------------------- misc
run "empty line"             ''
run "spaces only"            '   '
run "leading tabs"           $'\techo\thi'
run "only empty quotes"      $'""\necho $?'

rm -f /tmp/ms_t1 /tmp/ms_t2 /tmp/ms_t3 /tmp/ms_t4 /tmp/ms_t5 /tmp/ms_t6 \
	/tmp/ms_t7 /tmp/ms_t8 /tmp/ms_t9 /tmp/ms_ta /tmp/ms_tb

echo
echo "=============================================================="
echo "  passed: $PASS    failed: $FAIL    total: $((PASS + FAIL))"
echo "=============================================================="
if [ "$FAIL" -gt 0 ] && [ "$VERBOSE" = 0 ]; then
	echo "  re-run with -v to see the diffs"
fi

cat <<'NOTIMPL'

Deliberately NOT implemented — these are outside the mandatory subject and
are not covered above:

  ;             command separator
  &&  ||        logical operators           (bonus)
  ( )           subshells                   (bonus)
  *             wildcards                   (bonus)
  2>&1  >&      file-descriptor duplication
  \             backslash escaping
  <<<           here-strings
  $(...)  ``    command substitution

Checked with run_expect instead of against the local bash:

  /tmp          "Is a directory" — the capital I is what strerror(EISDIR)
                returns, and what the evaluation sheet asks for. The bash
                3.2 shipped with macOS prints its own lowercase string
                instead, so diffing this one case against it would fail.

  env ARG       "env: 'ARG': ..." — env is an external program, so its
                message comes from whichever env the system ships. The GNU
                coreutils env on Ubuntu quotes the operand, like every
                coreutils tool does ("ls: cannot access 'foo'"); the BSD
                env on macOS does not. minishell follows Ubuntu.

  deleted cwd   bash prints its getcwd warning as "cd: ..." with no shell
                name in front of it, so norm() has nothing to rewrite and
                the line can never match minishell's. The message, the
                status and the resulting $PWD are all bash's, verbatim.

NOTIMPL

[ "$FAIL" -eq 0 ]
