#!/bin/sh
set -eu

i=package/minibox-mfp/files/etc/init.d/minibox-scand
p=package/minibox-mfp/files/usr/libexec/minibox-mfp/scan-policy

# Runtime-v2 uses the native C daemon directly. Legacy socat/scanimage wrappers
# are intentionally not part of the active service contract anymore.
test -f "$i"
test -f "$p"
grep -q '/usr/sbin/minibox-scand 8080' "$i"
! grep -q 'TCP-LISTEN:9290' "$i"
grep -q 'scanner-transport=userspace-libusb' "$p"
grep -q 'network-protocol=escl' "$p"

grep -q 'minibox_m1522_scan_backend' src/minibox-scand/m1522_backend.c
grep -q 'HP-SOAP-SCAN' docs/M1522-SCAN-PROTOCOL.md
grep -q '03f0:4517' docs/M1522-SCAN-PROTOCOL.md

echo 'scan contract: native eSCL/libusb runtime OK'
