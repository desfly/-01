# MiniBox V1 stock web-upgrade analysis

## Verified stock image facts

Source dump: `firmware(7).bin`

- firmware partition size: `0x00fd0000`
- stock OpenWrt revision: `r49971`
- stock kernel: Linux `4.4.14`
- vendor: `OpenWrt`
- HWID: `0x3c000201`
- hardware revision: `1`
- kernel load address: `0x80060000`
- kernel entry point: `0x80060000`
- kernel offset: `0x00000200`
- actual SquashFS offset: `0x001426dc`
- rootfs offset field: `0x00100000`
- image recipe limit: `0x00f80000`
- bootloader offset/length in firmware image: `0 / 0`
- stock machine: `MiniBox V1.0`
- board id: `minibox-v1`

## Build-0105 checks

Before using a stock-web candidate, require:

1. TP-Link v1 image header.
2. HWID exactly `0x3c000201`.
3. HW revision exactly `1`.
4. Kernel load and entry `0x80060000`.
5. Kernel begins at `0x200`.
6. No bootloader payload.
7. Firmware image remains inside the stock firmware partition.
8. Kernel and rootfs ranges do not overlap.
9. TP-Link checksum/header format must be accepted by the stock updater.
10. Stock web UI must receive `squashfs-factory.bin`, not `squashfs-sysupgrade.bin`.

## Exact stock updater files to extract

From the immutable stock SquashFS or from the running stock MiniBox, collect:

- `/lib/upgrade/platform.sh`
- `/sbin/sysupgrade`
- `/lib/upgrade/common.sh`
- `/lib/upgrade/*.sh`
- LuCI/system firmware-upload controller files
- CGI/uhttpd firmware-upload handlers

Search them for:

- `platform_check_image`
- `platform_do_upgrade`
- `get_magic_long`
- `tplink`
- `HWID`
- `md5`
- `sysupgrade`
- `mtd`
- `firmware`

## Current evidence limitation

The saved project evidence contains the structural analysis of `firmware(7).bin`, but the raw binary itself is not currently available as a materializable Library file. Therefore this document does not claim that the exact stock updater scripts were re-extracted in this session.

Next evidence source: either the original stock dump or read-only copies of the updater files from the running stock MiniBox over SSH.
