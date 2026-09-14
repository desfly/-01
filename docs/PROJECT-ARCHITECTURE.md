# MiniBox MFP — project operating architecture

This document is the canonical description of the intended runtime topology for the MiniBox MFP project.

## Runtime topology

```text
                         Home / office Wi-Fi network
                                   |
                                   | Wi-Fi uplink
                                   | MiniBox = Wi-Fi client
                                   v
                         +---------------------+
                         | Gainstrong MiniBox  |
                         | MiniBox V1.0        |
                         | OpenWrt 25.12.x     |
                         +---------------------+
                           |               |
                 USB/libusb|               |RJ45 LAN
                           |               |DHCP + routing/NAT
                           v               v
                 +------------------+   PC / notebook / LAN device
                 | HP LaserJet      |        |
                 | M1522n           |        +--> local network + Internet
                 | USB 03f0:4517    |
                 +------------------+
```

## Fixed project rules

- MiniBox operates as a **Wi-Fi client** of the home/office network, not as the project's access point.
- Wi-Fi is the **uplink** to the existing network and Internet.
- The HP LaserJet M1522n is connected to MiniBox by **USB**.
- Printer/scanner USB communication is implemented in userspace through **libusb**.
- **Do not use `usblp`** for the project data path.
- **Do not use CUPS** as the printing architecture.
- Network printing and scanning are exposed by MiniBox to network clients.
- The RJ45 Ethernet port has a second, intentional project role: it is a **LAN port for a wired client**.
- A PC, notebook, or other Ethernet device plugged into RJ45 should automatically receive network configuration from MiniBox and obtain access to the network/Internet through the MiniBox Wi-Fi uplink.
- The preferred implementation is **routed/NAT mode**: Wi-Fi = uplink/WAN side; RJ45 = LAN side with DHCP. Do not depend on a transparent STA Wi-Fi L2 bridge.
- Ethernet is **not required for MFP printing/scanning**, but it is a supported network-access function of the finished MiniBox appliance.
- Ethernet may additionally be used for recovery, initial bring-up, and diagnostics.

## Functional data paths

### MFP path

**Network client <-> Wi-Fi/network <-> MiniBox services <-> libusb <-> HP LaserJet M1522n**

### RJ45 network path

**PC/notebook <-> RJ45 <-> MiniBox router/NAT <-> Wi-Fi uplink <-> home/office network <-> Internet**

The two functions coexist: MiniBox is simultaneously an MFP network bridge and a small Wi-Fi-to-Ethernet gateway.

## Post-upgrade hardware acceptance order

After the first successful boot of a new OpenWrt image, validate in this order:

1. OpenWrt boots reliably.
2. ART/calibration data are intact and the Wi-Fi radio works.
3. MiniBox can operate as a client on the target Wi-Fi network and reach the network/Internet.
4. USB host works and detects the HP LaserJet M1522n (`03f0:4517`).
5. Confirm `usblp` is not used and libusb can access the device.
6. Validate printing through the MiniBox MFP userspace service.
7. Validate scanning through the MiniBox MFP userspace service.
8. Validate RJ45 LAN: wired client receives an address by DHCP and reaches the network/Internet through the Wi-Fi uplink.
9. Validate isolation/firewall rules so the RJ45 gateway does not compromise management or MFP services.
10. Validate the minimal management web UI and service autostart.

## Design intent

The finished appliance combines two roles:

1. **Wi-Fi network MFP bridge** for the HP LaserJet M1522n over USB/libusb.
2. **Wi-Fi-to-Ethernet gateway**: a device connected to RJ45 receives wired network/Internet access through the MiniBox Wi-Fi uplink.

Neither role should depend on CUPS or `usblp`, and MFP operation must not depend on an Ethernet cable being connected.
