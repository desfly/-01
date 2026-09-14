# MiniBox MFP — hardware validation log — 2026-09-15

This file records the confirmed hardware/runtime state after flashing Build-0105 so the same bring-up steps are not repeated later.

## Confirmed firmware state

- Hardware: Gainstrong MiniBox V1.0 / Atheros AR9330 rev 1.
- Firmware: OpenWrt 25.12.5, kernel 6.12.94.
- Build-0105 was flashed successfully through the old/vendor LuCI web-upgrade page using the factory image.
- The device boots permanently from flash and LuCI is reachable.
- Ethernet management address observed: `192.168.55.251`.
- Wi-Fi client address observed: `192.168.55.250`.

## Wi-Fi client bring-up

Target SSID: `Nitros`.

The preserved configuration initially contained duplicate Wi-Fi client entries. The broken/duplicate entries were removed so only one STA remained.

Final relevant UCI state confirmed on-device:

```text
network.wwan=interface
network.wwan.proto='dhcp'
network.wwan.metric='10'
network.wwan.auto='1'

wireless.radio0=wifi-device
wireless.radio0.type='mac80211'
wireless.radio0.hwmode='11g'
wireless.radio0.path='platform/ahb/18100000.wmac'
wireless.radio0.htmode='HT20'
wireless.radio0.disabled='0'
wireless.radio0.channel='auto'
wireless.radio0.txpower='15'
wireless.radio0.country='US'

wireless.wifinet2=wifi-iface
wireless.wifinet2.device='radio0'
wireless.wifinet2.mode='sta'
wireless.wifinet2.network='wwan'
wireless.wifinet2.ssid='Nitros'
wireless.wifinet2.encryption='psk2'
wireless.wifinet2.disabled='0'
```

The WPA key is intentionally not recorded in Git.

Confirmed after reboot:

- Wi-Fi reconnects automatically without a manual LuCI action.
- `wwan` obtains `192.168.55.250/24` by DHCP.
- Gateway/DNS observed: `192.168.55.1`.
- Internet/NTP works through the Wi-Fi uplink.

Boot timing from logs:

- AR9330 Wi-Fi PHY visible around 25 s after kernel start.
- STA authentication begins around 61.85 s.
- Association completes around 62.05 s.
- DHCP lease follows a few seconds later.

So Wi-Fi autoconnect is working; remaining slowness is mainly overall OpenWrt/service boot time, not a failed STA configuration.

## SSH

Dropbear is enabled on port 22 and is not bound to only one interface.

Confirmed from Windows:

```text
Test-NetConnection 192.168.55.250 -Port 22
TcpTestSucceeded : True
```

SSH to `root@192.168.55.250` works.

Note: an unrelated broken `C:\Users\75\ssh.exe` shadows Windows OpenSSH in PATH. Use the system client explicitly when needed:

```text
C:\Windows\System32\OpenSSH\ssh.exe
```

## USB host and HP M1522n

USB host is working. Kernel log includes:

```text
usb 1-1: new high-speed USB device number 2 using ci_hdrc
```

The device is positively identified from sysfs as:

```text
1-1 03f0:4517 Hewlett-Packard HP LaserJet M1522n MFP
```

The composite USB interfaces are:

```text
1-1:1.0  interface 00  class ff  subclass 02  protocol 01  endpoints 3
1-1:1.1  interface 01  class 07  subclass 01  protocol 02  endpoints 2
1-1:1.2  interface 02  class ff  subclass 01  protocol 01  endpoints 2
```

`1-1:1.1` is the standard bidirectional USB printer interface.

`usblp` is NOT loaded:

```text
lsmod | grep usblp
# no output
```

This is the intended project state: userspace libusb owns the printing path.

## minibox-mfp package and daemon

Installed packages confirmed:

```text
libusb-1.0-0
minibox-mfp
```

Package version observed:

```text
minibox-mfp-0.2.0_rc1-r1
```

Installed files include:

```text
/etc/config/minibox
/etc/init.d/minibox
/usr/sbin/minibox-wifi-setup
/usr/sbin/miniboxd
/www/cgi-bin/minibox-print
/www/cgi-bin/minibox-scan
/www/cgi-bin/minibox-status
/www/minibox/app.js
/www/minibox/index.html
```

Daemon is running:

```text
/usr/sbin/miniboxd --daemon
```

ubus object is present:

```text
minibox
  status {}
  print {"path":"String"}
  scan {}
  cancel {}
```

TCP RAW printing service is listening on all interfaces:

```text
0.0.0.0:9100 LISTEN  miniboxd
```

Relevant `/etc/config/minibox` state:

```text
config core 'main'
        option enabled '1'
        option vid '03f0'
        option pid '4517'
        option transport 'libusb'
        option spool '/tmp/minibox'
        option timeout_ms '30000'
        option raw_port '9100'

config scan 'scan'
        option enabled '0'
        option reason 'hp_proprietary_plugin_required'
```

The binary contains real libusb code paths (`libusb_open`, `libusb_claim_interface`, `libusb_bulk_transfer`, etc.), so it is not just a stub.

## Printing — CONFIRMED WORKING

Important detail: `ubus call minibox print` accepts files from the configured spool directory `/tmp/minibox`. A path outside that directory returned `Permission denied`; this is expected path validation, not a USB failure.

Successful local print test:

```sh
mkdir -p /tmp/minibox
printf '\033E*** MiniBox USB PRINT TEST ***\r\n\r\nHP LaserJet M1522n\r\n\f' > /tmp/minibox/test.prn
ubus call minibox print '{"path":"/tmp/minibox/test.prn"}'
```

The HP physically printed the page.

After that job:

```json
{
  "service": "running",
  "transport": "libusb",
  "printer_online": true,
  "jobs_ok": 1,
  "jobs_failed": 0,
  "scan": "plugin_required"
}
```

Successful network RAW print test from Windows:

```text
Windows -> Wi-Fi -> 192.168.55.250:9100 -> miniboxd -> libusb -> HP M1522n
```

A PowerShell TCP test sent a page containing `WIFI TCP 9100 TEST`, and that page physically printed.

Therefore the complete network printing data path is CONFIRMED WORKING.

## Windows printer installation state

Windows printer installation was started using a Standard TCP/IP port to:

```text
Host/IP: 192.168.55.250
Protocol: RAW
Port: 9100
SNMP: disable
```

The session stopped at the Windows printer-driver selection page. Next action is to select/install an HP LaserJet M1522/M1522n-compatible PCL driver and print the Windows test page.

Do NOT repeat USB/libusb/raw-9100 diagnosis before finishing this Windows driver step: the raw network print path is already proven.

## Scanning state

Scanning is NOT yet implemented/validated in this build.

Current status is explicitly:

```text
scan = plugin_required
```

and `/etc/config/minibox` has:

```text
option enabled '0'
option reason 'hp_proprietary_plugin_required'
```

This is the next major MFP function after finishing the Windows printer-driver setup.

## RJ45 gateway state

Project architecture still requires RJ45 client -> MiniBox -> Wi-Fi uplink -> network/Internet using routed/NAT mode.

This has NOT yet been hardware-tested because no RJ45 cable was available during this session.

Also note: the current preserved configuration has `LAN 192.168.55.251/24` and `WWAN 192.168.55.250/24` in the same subnet. That is not the final routed/NAT topology and must be redesigned before validating the RJ45 gateway role.

## Remaining work, in order

1. Finish Windows printer installation on Standard TCP/IP RAW port 9100 and print a Windows test page.
2. Implement/enable scanning for HP M1522n and remove the current `plugin_required` limitation.
3. Correct the final LAN/WWAN subnet architecture for RJ45 routed/NAT gateway mode.
4. Test RJ45 DHCP + Internet through Wi-Fi when a cable is available.
5. Later optimize boot time; do not confuse the ~60 s boot-to-Wi-Fi timing with a failed Wi-Fi autoconnect.
6. Consider changing regulatory country from the currently observed `US` to the correct deployment country after functional testing.

## Do not repeat these already-proven checks

- Build-0105 permanent flash/boot: proven.
- Wi-Fi STA autoconnect to `Nitros`: proven.
- Internet/NTP through Wi-Fi: proven.
- USB host: proven.
- HP USB identity `03f0:4517`: proven.
- Correct printer USB interface class `07/01/02`: proven.
- `usblp` absent: proven.
- libusb/minibox package installed: proven.
- `miniboxd` running and ubus object present: proven.
- TCP port 9100 listening: proven.
- Local libusb print: proven physically.
- Windows -> Wi-Fi -> TCP 9100 -> MiniBox -> USB -> HP print: proven physically.
