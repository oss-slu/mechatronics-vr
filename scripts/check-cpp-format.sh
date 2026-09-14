#!/usr/bin/env bash
# Checks (never modifies) C++ formatting for first-party MechatronicsVR code.
# Same script CI runs, so a local pass == a CI pass.
#
#   scripts/check-cpp-format.sh              # check every file under Source/MechatronicsVR
#   scripts/check-cpp-format.sh a.cpp b.h    # check only these files
#   FIX=1 scripts/check-cpp-format.sh        # rewrite files in place instead of checking
#
# Exit code: 0 = all formatted, 1 = some files need formatting. CI treats 1 as a warning, not a failure.
cd "$(git rev-parse --show-toplevel)"

CLANG_FORMAT="${CLANG_FORMAT:-clang-format}"
SCOPE="Source/MechatronicsVR"

FILES=()
if [ "$#" -gt 0 ]; then
  FILES=("$@")
else
  # Read git's file list one line at a time. (Written this way because macOS ships bash 3.2,
  # which lacks the newer `mapfile` shortcut.)
  while IFS= read -r line; do
    FILES+=("$line")
  done < <(git ls-files -- "$SCOPE/**/*.cpp" "$SCOPE/**/*.h" "$SCOPE/*.cpp" "$SCOPE/*.h")
fi

if [ "${#FILES[@]}" -eq 0 ]; then
  echo "No first-party C++ files to check."
  exit 0
fi

echo "Using: $("$CLANG_FORMAT" --version)"

if [ "${FIX:-0}" = "1" ]; then
  "$CLANG_FORMAT" -i --style=file "${FILES[@]}"
  echo "Formatted ${#FILES[@]} file(s)."
  exit 0
fi

BAD=0
for f in "${FILES[@]}"; do
  # --dry-run prints one "file:line:col: warning: ..." per violation; -Werror makes the exit code non-zero.
  if ! "$CLANG_FORMAT" --dry-run -Werror --style=file "$f" 2>>"${FORMAT_LOG:-/dev/null}" >/dev/null; then
    echo "needs formatting: $f"
    BAD=$((BAD + 1))
  fi
done

if [ "$BAD" -eq 0 ]; then
  echo "All ${#FILES[@]} file(s) are formatted."
  exit 0
fi
echo
echo "$BAD of ${#FILES[@]} file(s) need formatting. Fix with:  FIX=1 scripts/check-cpp-format.sh"
exit 1
