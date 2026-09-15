#!/bin/sh
set -eu
cc -std=c99 -Wall -Wextra -Werror -pedantic src/minibox-printerd/main.c -o /tmp/minibox-printerd
cc -std=c99 -Wall -Wextra -Werror -pedantic src/minibox-scand/main.c -o /tmp/minibox-scand
/tmp/minibox-printerd 18631 >/tmp/printerd.log 2>&1 & P=$!
/tmp/minibox-scand 18080 >/tmp/scand.log 2>&1 & S=$!
trap 'kill $P $S 2>/dev/null || true' EXIT INT TERM
sleep 1
curl -fsS http://127.0.0.1:18631/health | grep -q 'printerd ok'
curl -fsS http://127.0.0.1:18080/health | grep -q 'scand ok'
curl -fsS http://127.0.0.1:18080/eSCL/ScannerCapabilities | grep -q 'HP LaserJet M1522n'
curl -fsS http://127.0.0.1:18080/eSCL/ScannerStatus | grep -q '<scan:State>Idle</scan:State>'
code=$(curl -sS -o /tmp/ipp.out -w '%{http_code}' -X POST http://127.0.0.1:18631/ipp/print); [ "$code" = 501 ]
code=$(curl -sS -o /tmp/scan.out -w '%{http_code}' -X POST http://127.0.0.1:18080/eSCL/ScanJobs); [ "$code" = 501 ]
echo 'MFP server transport contract OK'
