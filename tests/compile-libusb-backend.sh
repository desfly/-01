#!/bin/sh
set -eu
pkg-config --exists libusb-1.0
CFLAGS="-std=c99 -Wall -Wextra -Werror -pedantic $(pkg-config --cflags libusb-1.0)"
cc $CFLAGS -c src/minibox-usb/libusb_m1522.c -o /tmp/libusb_m1522.o
cc $CFLAGS -c src/minibox-usb/scan_m1522.c -o /tmp/scan_m1522.o
