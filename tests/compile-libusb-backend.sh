#!/bin/sh
set -eu
pkg-config --exists libusb-1.0
cc -std=c99 -Wall -Wextra -Werror -pedantic $(pkg-config --cflags libusb-1.0) -c src/minibox-usb/libusb_m1522.c -o /tmp/libusb_m1522.o
