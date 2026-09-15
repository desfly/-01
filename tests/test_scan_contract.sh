#!/bin/sh
set -eu
f=package/minibox-mfp/files/usr/libexec/minibox-mfp/scand
b=package/minibox-mfp/files/usr/libexec/minibox-mfp/scan-usb
i=package/minibox-mfp/files/etc/init.d/minibox-scand
p=package/minibox-mfp/files/usr/libexec/minibox-mfp/scan-policy
[ -x "$f" ] || test -f "$f"
[ -x "$b" ] || test -f "$b"
test -f "$p"
grep -q 'MINIBOX_SCAN_BACKEND' "$f"
grep -q 'scanimage' "$b"
grep -q 'TCP-LISTEN:9290' "$i"
grep -q 'scanner-transport=userspace-libusb' "$p"
grep -q 'network-protocol=escl' "$p"
echo 'scan contract: OK'
