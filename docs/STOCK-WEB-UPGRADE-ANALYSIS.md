# MiniBox V1 stock web-upgrade analysis

## Verified stock image facts

Primary image source: `firmware(7).bin` analysis.

Additional stock userspace evidence: user-supplied `box(3).zip`, SHA-256 `569986431dfd7d27bc255216d70cebfe760a35279c3112342cf78a4acdd56871`.

- firmware partition size: `0x00fd0000`
- stock OpenWrt revision: `r49971`
- stock kernel: Linux `4.4.14`
- stock distribution: `OpenWrtIT Net sunrise`
- stock target: `ar71xx/generic`
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

## Exact stock LuCI/sysupgrade path — now verified

The stock rootfs in `box(3).zip` contains the updater implementation that was previously missing from the project evidence.

### LuCI upload flow

`/usr/lib/lua/luci/controller/admin/system.lua` does the following:

1. receives the uploaded image as `/tmp/firmware.img`;
2. calls `sysupgrade -T /tmp/firmware.img` through `image_supported()`;
3. only if validation succeeds renders the verification page with image size, MD5 and SHA-256;
4. on confirmation runs `/sbin/sysupgrade` on the uploaded image.

Therefore the stock web page is not doing a separate proprietary flash operation. It relies on the ordinary OpenWrt `sysupgrade` validation and write path present in this vendor image.

### `minibox-v1` image acceptance rules

`/lib/upgrade/platform.sh` sets:

```sh
PART_NAME=firmware
CI_LDADR=0x80060000
```

For the `minibox-v1` board, `platform_check_image()` places it in the TP-Link-style image group and requires:

1. header magic/version `0100`;
2. image HWID exactly equal to the HWID read from the current `firmware` MTD partition;
3. image MID exactly equal to the MID read from the current `firmware` MTD partition;
4. TP-Link bootloader-size field equal to `00000000`.

An image containing a bootloader is explicitly rejected.

The helper offsets used by the stock code are:

- HWID: 4 bytes at header word index 16 (offset `0x40`);
- MID: 4 bytes at header word index 17 (offset `0x44`);
- bootloader size: 4 bytes at header word index 37 (offset `0x94`).

### Actual write path

`minibox-v1` has no dedicated case in `platform_do_upgrade()`. It falls through to `default_do_upgrade()` from `/lib/upgrade/common.sh`.

Because `PART_NAME=firmware`, the accepted image is written to the `firmware` MTD partition. This matches the known MiniBox partition layout and means a correctly generated factory image can be installed from the stock LuCI page.

## Runtime evidence from the working stock MiniBox

A LuCI status screenshot captured on 2026-09-14 confirms the live unit is running:

- model: `MiniBox V1.0`
- firmware: `OpenWrtIT Net sunrise 49971 / LuCI Master (git-16.298.34250-95358ab)`
- kernel: `4.4.14`
- RAM shown by LuCI: `60488 kB`
- WAN IPv4 device: `wlan0`
- WAN protocol: DHCP
- current Wi-Fi address: `192.168.55.250`
- netmask: `255.255.255.0`

This confirms the stock unit currently reaches the network as a Wi-Fi client. The final project topology remains defined in `docs/PROJECT-ARCHITECTURE.md`.

## Build-0105 stock-web checks

Before using a stock-web candidate, require all of the following:

1. TP-Link v1 image header with magic/version `0100`.
2. HWID exactly `0x3c000201`.
3. HW revision exactly `1`.
4. MID exactly matching the stock MiniBox firmware header — do not guess this value.
5. Kernel load and entry `0x80060000`.
6. Kernel begins at `0x200`.
7. Bootloader-size field is zero and no bootloader payload is present.
8. Firmware image remains inside the stock `firmware` partition.
9. Kernel and rootfs ranges do not overlap.
10. The finished image passes the equivalent of stock `sysupgrade -T` validation.
11. For the stock LuCI route, upload the factory/TP-Link-header image, not the ordinary ath79 `sysupgrade.bin` container.
12. First flash candidate must still be treated as high-risk until the exact MID and generated header are independently checked.

## Critical conclusion

The stock web-upgrade route is now **structurally confirmed** from the actual vendor updater code. The remaining header-critical unknown is the exact stock MID. Once MID is recovered and Build-0105 reproduces the required TP-Link header fields, we can validate the candidate against the stock acceptance logic before any flash attempt.

For the source archive details and the 2026-09-14 runtime screenshot facts, see `reference/stock-r49971/README.md`.
