#!/bin/bash

# Usage: tools/check-symbol-visibility.sh <liblwnode.so> <liblwnode.map>

set -e

if [ $# -ne 2 ]; then
  echo "usage: $0 <liblwnode.so> <liblwnode.map>" >&2
  exit 1
fi

LIB=$1
MAP=$2

if [ ! -f "$LIB" ]; then
  echo "error: $LIB not found" >&2
  exit 1
fi
if [ ! -f "$MAP" ]; then
  echo "error: $MAP not found" >&2
  exit 1
fi

# Confirm known-internal prefixes are hidden.
# e.g) nm -D --defined-only $LIB | grep -cE "^[0-9a-f]+ T ares_"
ALL_T=$(nm -D --defined-only "$LIB" | awk '/^[0-9a-f]+ T / { print $3 }')

for prefix in uv_ ares_ nghttp2_; do
  count=$(echo "$ALL_T" | grep -c "^${prefix}" || true)
  status=$( [ "$count" -eq 0 ] && echo OK || echo LEAKED )
  printf "  %-12s %s (%d)\n" "${prefix}*" "$status" "$count"
done

# Each glob pattern ends with * — convert to a regex prefix match.
# e.g. "_ZN6lwnode|_ZN6LWNode|_ZN2v8|napi_".
PATTERN=$(awk '
  /global:/  { in_global=1; next }
  /local:/   { in_global=0 }
  in_global && /[A-Za-z_]/ {
    gsub(/[;[:space:]]/, "")
    gsub(/\*$/, "")
    if (length > 0) print
  }
' "$MAP" | paste -sd'|')

if [ -z "$PATTERN" ]; then
  echo "error: no global patterns found in $MAP" >&2
  exit 1
fi

# Collect exported text symbols not matched by any allowed pattern.
LEAKS=$(nm -D --defined-only "$LIB" \
  | awk '/^[0-9a-f]+ T / { print $3 }' \
  | grep -vE "^(${PATTERN})" || true)

if [ -n "$LEAKS" ]; then
  echo "FAIL: symbols exported outside liblwnode.map:"
  echo "$LEAKS"
  exit 1
fi

echo "OK: all exported symbols match liblwnode.map"
