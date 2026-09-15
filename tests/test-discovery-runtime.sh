#!/bin/sh
set -eu
CC=${CC:-cc}
TMP="${TMPDIR:-/tmp}/minibox-discovery-test.$$"
trap 'rm -rf "$TMP"' EXIT INT TERM
mkdir -p "$TMP"
$CC -std=c11 -Wall -Wextra -Werror -Isrc/minibox-discoveryd \
  src/minibox-discoveryd/main.c src/minibox-discoveryd/service.c -o "$TMP/minibox-discoveryd"
cp overlay/etc/minibox/services.d/*.service "$TMP/"
out="$($TMP/minibox-discoveryd "$TMP")"
printf '%s\n' "$out"
printf '%s\n' "$out" | grep -F 'type=_ipp._tcp' >/dev/null
printf '%s\n' "$out" | grep -F 'type=_uscan._tcp' >/dev/null
printf '%s\n' "$out" | grep -F 'loaded 2 service(s)' >/dev/null
