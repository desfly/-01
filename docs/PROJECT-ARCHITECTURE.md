# MiniBox MFP — project operating architecture

This document is the canonical description of the intended runtime topology for the MiniBox MFP project.

## Runtime topology

```text
Home / office Wi-Fi network
          |
          | Wi-Fi (MiniBox operates as a client, not as an AP)
          v
+---------------------+
| Gainstrong MiniBox  |
| MiniBox V1.0        |
| OpenWrt 25.12.x     |
+---------------------+
          |
          | USB
          | libusb userspace access
          v
+---------------------+
| HP LaserJet M1522n  |
| USB ID 03f0:4517    |
+---------------------+

Network clients --Wi-Fi--> MiniBox --USB/libusb--> MFP
```

## Fixed project rules

- The normal network transport for the finished device is **Wi-Fi**.
- MiniBox operates as a **Wi-Fi client**, not as the project's access point.
- The MFP is connected to MiniBox by **USB**.
- Printer/scanner USB communication is implemented in userspace through **libusb**.
- **Do not use `usblp`** for the project data path.
- **Do not use CUPS** as the printing architecture.
- Network printing and scanning are exposed by MiniBox over Wi-Fi.
- Ethernet is **not part of the normal runtime topology**. It may be used temporarily for recovery, initial bring-up, or diagnostics, but must not become a functional dependency of MiniBox MFP.

## Post-upgrade hardware acceptance order

After the first successful boot of a new OpenWrt image, validate in this order:

1. OpenWrt boots reliably.
2. ART/calibration data are intact and the Wi-Fi radio works.
3. MiniBox can operate as a client on the target Wi-Fi network.
4. USB host works and detects the HP LaserJet M1522n (`03f0:4517`).
5. Confirm `usblp` is not used and libusb can access the device.
6. Validate printing through the MiniBox MFP userspace service.
7. Validate scanning through the MiniBox MFP userspace service.
8. Validate the minimal management web UI and service autostart.

Ethernet success alone is not an acceptance criterion for the finished project.

## Design intent

The finished appliance is a small Wi-Fi-to-USB network bridge dedicated to the HP LaserJet M1522n. Its essential data path is:

**Wi-Fi network <-> MiniBox services <-> libusb <-> HP LaserJet M1522n.**
