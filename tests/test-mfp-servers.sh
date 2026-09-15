#!/bin/sh
set -eu
cc -std=c99 -Wall -Wextra -Werror -pedantic -DMINIBOX_TEST_PRINT_SINK src/minibox-printerd/main.c src/minibox-ipp/ipp.c -o /tmp/minibox-printerd
cc -std=c99 -Wall -Wextra -Werror -pedantic src/minibox-scand/main.c -o /tmp/minibox-scand
MINIBOX_TEST_PRINT_FILE=/tmp/printed.bin /tmp/minibox-printerd 18631 >/tmp/printerd.log 2>&1 & P=$!
/tmp/minibox-scand 18080 >/tmp/scand.log 2>&1 & S=$!
trap 'kill $P $S 2>/dev/null || true' EXIT INT TERM
sleep 1
curl -fsS http://127.0.0.1:18631/health | grep -q 'printerd ok'
curl -fsS http://127.0.0.1:18080/health | grep -q 'scand ok'
curl -fsS http://127.0.0.1:18080/eSCL/ScannerCapabilities | grep -q 'HP LaserJet M1522n'
curl -fsS http://127.0.0.1:18080/eSCL/ScannerStatus | grep -q '<scan:State>Idle</scan:State>'
printf '\002\000\000\013\000\000\000\001\003' >/tmp/ipp.req
code=$(curl -sS -o /tmp/ipp.out -w '%{http_code}' -H 'Content-Type: application/ipp' --data-binary @/tmp/ipp.req http://127.0.0.1:18631/ipp/print); [ "$code" = 200 ]
printf '\002\000\000\000\000\000\000\001\003' >/tmp/ipp.ok
cmp /tmp/ipp.ok /tmp/ipp.out
# Minimal Print-Job envelope: header + end-of-attributes + raw PCL document.
printf '\002\000\000\002\000\000\000\002\003\033EHello MiniBox\014\033E' >/tmp/print.req
printf '\033EHello MiniBox\014\033E' >/tmp/document.expected
code=$(curl -sS -o /tmp/print.out -w '%{http_code}' -H 'Content-Type: application/ipp' --data-binary @/tmp/print.req http://127.0.0.1:18631/ipp/print); [ "$code" = 200 ]
printf '\002\000\000\000\000\000\000\002\003' >/tmp/print.ok
cmp /tmp/print.ok /tmp/print.out
cmp /tmp/document.expected /tmp/printed.bin
code=$(curl -sS -o /tmp/scan.out -w '%{http_code}' -X POST http://127.0.0.1:18080/eSCL/ScanJobs); [ "$code" = 501 ]
echo 'MFP server transport contract OK'
