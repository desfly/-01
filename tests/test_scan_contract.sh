#!/bin/sh
set -eu
f=package/minibox-mfp/files/usr/libexec/minibox-mfp/scand
b=package/minibox-mfp/files/usr/libexec/minibox-mfp/scan-usb
i=package/minibox-mfp/files/etc/init.d/minibox-scand
[ -x "$f" ] || test -f "$f"
[ -x "$b" ] || test -f "$b"
grep -q 'MINIBOX_SCAN_BACKEND' "$f"
grep -q 'scanimage' "$b"
grep -q 'TCP-LISTEN:9290' "$i"
! grep -Eq 'usblp|cupsd|CUPS' "$f" "$b" "$i"
echo 'scan contract: OK'
