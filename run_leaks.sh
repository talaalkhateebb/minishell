#!/bin/bash
# Leak check for minishell — run this on Ubuntu, where valgrind works.
#
# The important flag is --trace-children=yes: without it valgrind only
# watches the parent shell, and every child that exits without exec'ing (a
# built-in in a pipeline, a failed redirection, "command not found") is
# never inspected at all. Those children inherit the whole shell — the
# environment, the command list, the input line, the history — so they are
# exactly where the leaks hide.
#
#   ./run_leaks.sh            # run the whole battery
#   ./run_leaks.sh -i         # interactive: valgrind an ordinary session
#
# Exits non-zero if any case leaks.

set -u

VG="valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes \
--trace-children=yes --error-exitcode=42 --suppressions=readline.supp \
--log-file=/tmp/ms_vg_%p.log"

if [ ! -x ./minishell ]; then
	echo "build first: make" >&2
	exit 1
fi
if ! command -v valgrind >/dev/null; then
	echo "valgrind not installed: sudo apt install valgrind" >&2
	exit 1
fi

if [ "${1:-}" = "-i" ]; then
	rm -f /tmp/ms_vg_*.log
	# shellcheck disable=SC2086
	$VG ./minishell
	grep -l "definitely lost: [1-9]\|indirectly lost: [1-9]\|Open file descriptor" \
		/tmp/ms_vg_*.log 2>/dev/null && echo "^ see those logs"
	exit 0
fi

# Each case is fed on stdin. The forking ones matter most.
CASES=(
	'echo hello world'
	'export FOO=bar
export
unset FOO'
	'cd /tmp
pwd
cd -
pwd'
	'echo "$HOME"'"'"'lit'"'"'$?${PATH}$$'
	'echo $NOPE unquoted $HOME/x ~ ~/y'
	'ls Makefile'
	'nosuchcommand'
	'/etc'
	'./Makefile'
	'ls | cat | wc -l'
	'ls | export FOO=1'
	'echo a | nosuchcommand | cat'
	'cat << EOF
line $HOME
EOF'
	'cat << "EOF" | cat
raw $HOME
EOF'
	'cat << E1 | cat << E2
a
E1
b
E2'
	'echo x > /tmp/ms_leak_out
cat < /tmp/ms_leak_out
echo y >> /tmp/ms_leak_out
cat /tmp/ms_leak_out'
	'> /tmp/ms_leak_only'
	'cat < /nonexistent'
	'echo x > /nonexistent_dir/f'
	'echo |'
	'echo ${}'
	'echo "unclosed'
	'exit 3'
	'exit notanumber'
	'env | head -3'
	'env ls Makefile'
	'env nosuchcommand'
	'echo $NOPE > out'
	'unset PATH
ls'
)

rm -f /tmp/ms_vg_*.log
fails=0

for i in "${!CASES[@]}"; do
	rm -f /tmp/ms_vg_*.log
	printf '%s\nexit\n' "${CASES[$i]}" | \
		timeout 60 $VG ./minishell >/dev/null 2>&1

	bad=""
	for log in /tmp/ms_vg_*.log; do
		[ -e "$log" ] || continue
		# "still reachable" is not a leak for our purposes; readline holds
		# state it never frees. definite/indirect are ours.
		if grep -qE "definitely lost: [1-9]|indirectly lost: [1-9]" "$log"; then
			bad="$bad leak"
		fi
		# --track-fds: 0/1/2 are inherited, anything else is ours.
		if grep -q "Open file descriptor" "$log" && \
		   grep -A2 "Open file descriptor" "$log" | grep -qv "inherited from parent"; then
			if grep -cE "Open file descriptor [3-9]" "$log" | grep -qv '^0$'; then
				bad="$bad fd"
			fi
		fi
	done

	if [ -n "$bad" ]; then
		fails=$((fails + 1))
		printf 'LEAK  [%s ]  %s\n' "$bad" "$(printf '%s' "${CASES[$i]}" | head -1)"
		cat /tmp/ms_vg_*.log
	else
		printf 'clean         %s\n' "$(printf '%s' "${CASES[$i]}" | head -1)"
	fi
done

rm -f /tmp/ms_leak_out /tmp/ms_leak_only /tmp/ms_vg_*.log out
echo
if [ "$fails" -eq 0 ]; then
	echo "all ${#CASES[@]} cases clean"
	exit 0
fi
echo "$fails of ${#CASES[@]} cases leaked"
exit 1
