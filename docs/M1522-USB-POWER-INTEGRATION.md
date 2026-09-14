# HP LaserJet M1522n <-> MiniBox USB and power integration

This note records the electrical integration assumptions for the MiniBox MFP project.

## Source service documentation

HP LaserJet M1522 MFP Series Service Manual, part number **CB534-90945**, Edition 1, 01/2008.

Relevant service-manual sections:

- Figure 6-4: Formatter connectors.
- Table 6-5: Formatter connectors.
- Figure 6-12: Circuit diagram (Problem-solve diagrams, printed page 203).
- ECU connector table: the formatter receives communication, +5 V, +3.3 V and GND on J531, and +24 V plus GND on J532.

The service manual identifies formatter connector **J33A** as the **High-speed USB 2.0 port**.

## Functional USB topology

```text
MiniBox V1.0                         HP LaserJet M1522n
+----------------------+            +-------------------------+
| USB HOST             |            | USB DEVICE / Formatter  |
|                      |            | J33A                    |
| VBUS 5 V  ---------- +----------> | VBUS sense/device side  |
| D+      <------------+----------> | D+                      |
| D-      <------------+----------> | D-                      |
| GND     -------------+----------- | GND                     |
+----------------------+            +-------------------------+
```

Important consequence: in normal USB operation the **MiniBox is the USB host** and the M1522n is the USB device. Therefore the ordinary USB cable is **not a valid source of power for MiniBox**. The host side is expected to provide USB VBUS to the peripheral side, not the opposite direction.

## MiniBox power requirement

Gainstrong MiniBox V1.0 is specified for **5 VDC via micro-USB**. Public Gainstrong/OpenWrt hardware references list 5 V input; Gainstrong documentation also lists a 0.5 A input rating for this class of MiniBox V1.0 hardware.

Design target for integration should therefore be a clean regulated **5 V rail with at least 0.5 A continuous capability**, with additional margin preferred for USB-host transients and Wi-Fi activity.

## Power available inside the M1522n

The HP service documentation shows these low-voltage rails between the engine controller and formatter:

```text
Engine Controller PCA
        |
        +---- J531 ----> Formatter: communication + 5 V + 3.3 V + GND
        |
        +---- J532 ----> Formatter: communication + 24 V + GND
```

The manual therefore proves that **+5 V, +3.3 V and +24 V rails exist internally**. It does **not** specify sufficient spare-current margin for an added external load such as MiniBox.

## Recommended project direction

Do **not** attempt to back-power MiniBox through the M1522n USB data connector.

Preferred candidate architecture for a single-cable appliance:

```text
M1522n internal switched low-voltage rail
                 |
                +24 V
                 |
        [protected DC/DC buck]
          24 V -> regulated 5 V
                 |
          fuse/current limit
                 |
          MiniBox micro-USB 5 V
```

Why +24 V -> 5 V is the preferred candidate instead of directly tapping the M1522n +5 V rail:

- it avoids adding MiniBox directly to the formatter 5 V logic rail;
- a dedicated buck converter isolates the added load from logic-rail voltage drop;
- it gives us our own filtering, current limit and fuse;
- MiniBox requires only a few watts, so conversion from the printer low-voltage supply is practical if measurements confirm adequate reserve.

This is a **candidate design, not yet electrically approved**. The HP manual confirms the rail exists, but does not publish spare-current capacity.

## Required measurements before implementation

Before hard-wiring MiniBox power inside the M1522n:

1. Identify the exact low-voltage +24 V and GND points corresponding to J532/J30 from the service diagrams and physical formatter/ECU connectors.
2. Measure the +24 V rail in Ready/Sleep.
3. Measure it during printer startup.
4. Measure it during printing/fuser operation.
5. Measure it during scanning.
6. Temporarily test a separately fused 24 V -> 5 V buck converter with an electronic/dummy load equivalent to MiniBox before connecting MiniBox.
7. Verify the 5 V output remains within the MiniBox input tolerance through all printer operating states.
8. Verify printer shutdown removes MiniBox power in the desired way and that no reverse-current path is created through USB.

Do not connect the modification to mains/high-voltage/fuser circuitry. The intended integration point is **only the isolated low-voltage DC side** after the printer power supply.

## Project decision status

- USB data topology: **LOCKED** - MiniBox host -> M1522n device via libusb.
- Power through USB cable from M1522n to MiniBox: **REJECTED**.
- Internal printer-powered MiniBox: **FEASIBLE IN PRINCIPLE**.
- Preferred source: **M1522n internal +24 V low-voltage rail -> protected 5 V buck converter**.
- Final approval: **PENDING physical voltage/current-margin measurements**.
