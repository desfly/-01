# Gainstrong MiniBox V1.0 — stock OpenWrt reference

This directory records the stock firmware evidence supplied from the running MiniBox and from the `box(3).zip` root-filesystem snapshot.

## Source archive

- File supplied: `box(3).zip`
- SHA-256: `569986431dfd7d27bc255216d70cebfe760a35279c3112342cf78a4acdd56871`
- Archive size: about 3.4 MiB
- Purpose: immutable reference for the vendor OpenWrt userspace and the stock web/sysupgrade path.

## Stock release identity extracted from the archive

`/etc/openwrt_release` contains:

```text
DISTRIB_ID='OpenWrtIT'
DISTRIB_RELEASE='sunrise'
DISTRIB_REVISION='49971'
DISTRIB_CODENAME='net'
DISTRIB_TARGET='ar71xx/generic'
DISTRIB_DESCRIPTION='OpenWrtIT Net sunrise'
DISTRIB_TAINTS='no-all'
```

`/etc/openwrt_version` contains `sunrise`.

Board detection in `/lib/ar71xx.sh` maps:

- TP-Link-style HWID prefix `3C0002*` -> `MINIBOX_V1`
- machine string `MiniBox V1.0` -> board id `minibox-v1`

The stock status LED mapping in `/etc/diag.sh` is `minibox-v1:green:system`.

## Stock network definition

In `/etc/board.d/02_network`, `minibox-v1` is in the group that executes:

```sh
ucidef_set_interfaces_lan_wan "eth0" "eth2"
ucidef_add_switch "switch0" \
        "0@eth0" "1:lan:4" "2:lan:3" "3:lan:2" "4:lan:1"
```

This is historical stock-board evidence only. The finished MiniBox-MFP project uses the canonical runtime architecture in `docs/PROJECT-ARCHITECTURE.md`: Wi-Fi client/uplink, MFP over USB/libusb, and RJ45 as the optional wired LAN/gateway function.

## Exact stock web-upgrade path recovered from the archive

The supplied rootfs contains the complete relevant updater stack:

- `/lib/upgrade/platform.sh`
- `/lib/upgrade/common.sh`
- `/sbin/sysupgrade`
- `/usr/lib/lua/luci/controller/admin/system.lua`
- `/usr/lib/lua/luci/view/admin_system/flashops.htm`
- `/usr/lib/lua/luci/view/admin_system/upgrade.htm`

For `minibox-v1`, `/lib/upgrade/platform.sh` proves that the stock validator requires all of the following:

1. TP-Link image magic/version `0100`.
2. Image HWID must exactly match the HWID read from the running `firmware` MTD partition.
3. Image MID must exactly match the MID read from the running `firmware` MTD partition.
4. TP-Link bootloader-size field must be `00000000`; an image containing a bootloader is rejected.
5. `PART_NAME=firmware`.

`minibox-v1` has no special write case in `platform_do_upgrade()`, so it falls through to `default_do_upgrade()`, which writes the accepted image to the `firmware` MTD partition.

The LuCI controller uploads the candidate to `/tmp/firmware.img`, validates it with `sysupgrade -T`, and only if that test succeeds offers the confirmation page. On confirmation it runs `/sbin/sysupgrade` on the same image.

This makes the stock LuCI updater a viable installation route for a correctly constructed MiniBox factory image; it is not enough for the image merely to fit in flash.

## Runtime screenshot evidence — 2026-09-14

The supplied LuCI status screenshot shows the running stock unit at `192.168.55.250` and confirms:

- hostname: `OpenWrt`
- model: `Минибокс V1.0` / MiniBox V1.0
- firmware: `OpenWrtIT Net sunrise 49971 / LuCI Master (git-16.298.34250-95358ab)`
- kernel: `4.4.14`
- memory total shown by LuCI: `60488 kB` (approximately 64 MiB RAM)
- `wlan0` is the WAN IPv4 interface
- `wlan0` type: DHCP
- `wlan0` address: `192.168.55.250`
- mask: `255.255.255.0`

Screenshot SHA-256: `80b3bb75917e5808656ce5c298398ac77b5491c239255fecff31c91d736aba59`.

The screenshot is runtime evidence for the current stock system and Wi-Fi-client connectivity. It must not be interpreted as a requirement to preserve the vendor LuCI UI in the final minimal firmware.

## Remaining mandatory value before a stock-web flash candidate

The exact stock MID must be copied into the generated TP-Link header. The validator compares MID byte-for-byte, so the Build-0105 factory candidate must not guess it.

The previous firmware analysis already established HWID `0x3c000201`, hardware revision `1`, load/entry `0x80060000`, and no bootloader payload. See `docs/STOCK-WEB-UPGRADE-ANALYSIS.md` for the consolidated acceptance checklist.
