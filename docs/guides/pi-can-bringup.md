---
title: Raspberry Pi CAN Bring-up (Listen-Only)
title_tr: Raspberry Pi CAN Başlatma (Sadece Dinleme)
description: Runbook for driving the existing MCP2515 modules from a Raspberry Pi in listen-only mode
category: guides
folder: guides
tags: [hardware, raspberry-pi, mcp2515, socketcan, listen-only, x179]
order: 10
icon: 📡
---

# Raspberry Pi CAN Bring-up (Listen-Only)

> **UNVERIFIED RUNBOOK.** No Raspberry Pi, MCP2515 module, or vehicle was available
> when this document was written. Everything below is a **procedure to execute and
> record on real hardware**, not a report of a working setup. Every electrical,
> kernel, and frame claim is marked UNVERIFIED until the provenance record at the
> bottom is filled in by an operator.

## Scope

This guide wires the **existing 1–3 MCP2515 CAN modules** used by the ESP32 build
(see [Hardware Setup](./hardware-setup.md)) to a Raspberry Pi and brings the
SocketCAN interfaces up **receive-only** so X179 traffic can be observed with
`candump`. It is a bench/bring-up procedure for the Pi portability spike.

Out of scope (deliberately):

- Any CAN transmission, ACK-capable "normal" mode used for injection, or firmware
  command that writes to a bus — see [DO NOT ENABLE TX](#do-not-enable-tx).
- Flashing or powering the shipping ESP32 firmware from the Pi.
- Copying code from the referenced guides. They are read-only references for
  wiring and overlay patterns; prose and values here are re-derived.

## Sources and provenance of this document

| Source                                                                                                            | What was taken                                                                                                               | Status                      |
| ----------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------- | --------------------------- |
| [Using the MCP2515 with the Raspberry Pi](https://38-3d.co.uk/blogs/blog/using-the-mcp2515-with-the-raspberry-pi) | Module `VCC → 3.3 V`, SPI0 pin map, `oscillator=16000000`, `dtoverlay=spi-bcm2835`, 120 Ω note                               | External guide — UNVERIFIED |
| [CANBus-MCP2515-Raspi](https://github.com/tolgakarakurt/CANBus-MCP2515-Raspi)                                     | TJA1050 5 V requirement + trace-cut/jumper mod, `oscillator=8000000`, `spi-dma`, `spi0.0/net/can0` check, common-ground note | External guide — UNVERIFIED |
| `docs/guides/hardware-setup.md` (this repo)                                                                       | X179 bus-to-pin mapping and per-bus CAN purpose                                                                              | Git-derived from repo       |
| `docs/reference/can-protocol.md` (this repo)                                                                      | Expected frame IDs per variant, bus routing, 500 kbps rate                                                                   | Git-derived from repo       |

A community forum thread was identified during research but returned HTTP 403 and
was **not** used. No content was copied from any external guide.

The two external guides do **not** conflict on VCC: both put MCP2515 `VCC` at
3.3 V. The difference is that the [tolgakarakurt guide](https://github.com/tolgakarakurt/CANBus-MCP2515-Raspi)
documents the TJA1050 transceiver as a separate 4.75–5.25 V device that must be
fed 5 V on common modules. That is the trap described below. The guides disagree
on the crystal value (16 MHz vs 8 MHz), which is why the overlay uses a
`<module crystal>` placeholder rather than a fixed number.

## X179 bus mapping (git-derived)

Derived from `docs/guides/hardware-setup.md` ("Tesla X179 Connector — Bus
Mapping") and cross-checked with the frame routing in `docs/reference/can-protocol.md`.

| Bus | SocketCAN name | X179 pins | Function                | Rate     |
| --- | -------------- | --------- | ----------------------- | -------- |
| 0   | `can0`         | 13 / 14   | Chassis / Autopilot CAN | 500 kbps |
| 1   | `can1`         | 9 / 10    | Vehicle Control CAN     | 500 kbps |
| 2   | `can2`         | 2 / 3     | Body Control CAN        | 500 kbps |

`13/14`, `9/10`, and `2/3` are CAN-H/CAN-L pairs. X179 pin 1 is 12 V (VIN+) and
pin 20 is ground in the original harness; **powering the Pi from X179 is not
covered here**. CAN-H/CAN-L carry no ground reference, so a common ground between
Pi and module/vehicle is mandatory (explicitly called out in the tolgakarakurt
guide).

## The TJA1050 5 V trap

> **Read before applying power.** Common "MCP2515 CAN Module TJA1050" boards tie
> the TJA1050 transceiver supply to the module's 3.3 V `VCC` rail. The
> [tolgakarakurt guide](https://github.com/tolgakarakurt/CANBus-MCP2515-Raspi)
> documents the TJA1050 as operating at 4.75–5.25 V (typically 5 V) and shows a
> board modification to add a separate 5 V `Vdd` input. Running the transceiver
> from 3.3 V is out of spec. UNVERIFIED for any specific module in your hands.

Two acceptable approaches — pick one and record which:

1. **Modify the module.** Locate the TJA1050 `Vdd` trace, cut it, scrape the
   solder mask, solder a jumper from a 5 V source to the transceiver `Vdd` pad,
   insulate the joint, and (optionally) replace the 7-pin header with an 8-pin
   right-angle header to expose the new rail. Keep the MCP2515 controller itself
   on 3.3 V. The sequence and photos are in the cited guide; nothing is copied
   here. UNVERIFIED — a cut trace is irreversible, so use a scrap module first.
2. **Use a 3.3 V-rated transceiver module.** A module built around a transceiver
   specified for 3.3 V operation avoids the mod entirely. Verify the actual part
   number and its datasheet voltage range before assuming compatibility;
   "MCP2515" on the label identifies the controller, not the transceiver.
   UNVERIFIED.

**Level shifting:** Raspberry Pi GPIO is 3.3 V and is not 5 V tolerant. The
[38-3D guide](https://38-3d.co.uk/blogs/blog/using-the-mcp2515-with-the-raspberry-pi)
wires module `VCC` to 3.3 V, which keeps SPI/INT lines at Pi-safe levels. If your
module (or mod) leaves **any** line — MISO, INT, CS, SCK, MOSI — at 5 V, put a
bidirectional level shifter on those lines before the Pi header. At minimum
shift MISO and INT. UNVERIFIED.

## Pin table — bus 0 (`can0`, X179 13/14)

Values below follow the 38-3D guide's SPI0 mapping. Physical pin numbers are for
a standard 40-pin Pi header; confirm against `pinout.xyz` for your board.
UNVERIFIED.

| MCP2515 module pin | Pi signal | GPIO | Physical pin | Notes                                       |
| ------------------ | --------- | ---- | ------------ | ------------------------------------------- |
| VCC (controller)   | 3V3       | —    | 1            | 3.3 V to MCP2515, **not** the TJA1050 `Vdd` |
| GND                | GND       | —    | 6 (or any)   | Common ground with module and vehicle       |
| SCK                | SPI0 SCLK | 11   | 23           | Shared across modules                       |
| MOSI / SI          | SPI0 MOSI | 10   | 19           | Shared across modules                       |
| MISO / SO          | SPI0 MISO | 9    | 21           | Shared across modules; level-shift if 5 V   |
| CS                 | SPI0 CE0  | 8    | 24           | Chip select for bus 0                       |
| INT                | GPIO      | 25   | 22           | Interrupt for bus 0                         |

Extra buses reuse SCK/MOSI/MISO and need a **unique CS and INT each**:

| Bus | CS proposal                 | INT proposal     | X179 pair |
| --- | --------------------------- | ---------------- | --------- |
| 0   | GPIO 8 (CE0, pin 24)        | GPIO 25 (pin 22) | 13 / 14   |
| 1   | GPIO 7 (CE1, pin 26)        | GPIO 24 (pin 18) | 9 / 10    |
| 2   | Free GPIO (e.g. 16, pin 36) | GPIO 17 (pin 11) | 2 / 3     |

These CS/INT choices for buses 1 and 2 are a **proposal, not a verified
mapping**. Confirm each GPIO is unused by other overlays/HATs and that your
kernel's `mcp2515` overlay accepts the corresponding `cs=`/`interrupt=`
parameters. UNVERIFIED.

## Device-tree and overlay configuration

1. Enable SPI via `sudo raspi-config` → Interface Options → SPI, or use
   `dtparam=spi=on` below. Install tools: `sudo apt install -y can-utils`.
2. Edit `/boot/config.txt` (on newer Raspberry Pi OS the file lives at
   `/boot/firmware/config.txt` — verify your path). Add:

    ```ini
    dtparam=spi=on
    dtoverlay=mcp2515-can0,oscillator=<module crystal Hz>,interrupt=25
    dtoverlay=spi-bcm2835
    # Only on older kernels (e.g. 4.19.x), per the tolgakarakurt guide:
    # dtoverlay=spi-dma
    ```

    - Replace `<module crystal Hz>` with the **actual crystal on the module**:
      `16000000` in the 38-3D guide, `8000000` in the tolgakarakurt guide. A wrong
      value produces no frames or bad bit timing. UNVERIFIED.
    - `spi-dma` was needed on the old 4.19 kernel in the guide; do not assume it
      is required or even valid on a newer kernel. UNVERIFIED.

3. For additional buses, repeat the `dtoverlay=mcp2515-canN,...` pattern with
   distinct `interrupt=` and `cs=` values from the table above; verify overlay
   parameter names against your kernel's `/boot/overlays/README`. UNVERIFIED.
4. Reboot: `sudo reboot`.

## Verify enumeration

The interface must exist before any CAN configuration:

```bash
ls /sys/bus/spi/devices/spi0.0/net
# expected: can0
# or test the exact path:
test -e /sys/bus/spi/devices/spi0.0/net/can0 && echo present
```

If `can0` is missing, re-check: SPI enabled, `config.txt` edits on the **boot**
partition actually used, `oscillator=` matches the crystal, CS/INT wiring,
common ground, and module power. (Enumeration check is from the tolgakarakurt
guide.) UNVERIFIED.

## Bring up the interface (receive-only)

Baseline command used by both guides:

```bash
sudo ip link set can0 up type can bitrate 500000
```

For listen-only operation, prefer the SocketCAN listen-only flag if your kernel
and the `mcp2515` driver support it:

```bash
sudo ip link set can0 up type can bitrate 500000 listen-only on
```

Confirm support with `ip -details link show can0`; if the flag is rejected or not
echoed back, record that and do **not** assume the controller is prevented from
ACKing. UNVERIFIED — driver support varies by kernel. Bring the link back down
with `sudo ip link set can0 down`.

Do **not** send test frames. The external guides include `cansend` examples;
those are intentionally omitted here. See [DO NOT ENABLE TX](#do-not-enable-tx).

## Acceptance checks

Run these on real hardware and paste the outputs into the provenance record.
A check is only "pass" when recorded on the actual setup.

1. **Interface state / bitrate**

    ```bash
    ip -details link show can0
    ```

    Expect the link `UP`, bitrate `500000` (500 kbps), and — if the flag is
    supported — listen-only enabled. UNVERIFIED.

2. **Frames on the bus**

    ```bash
    candump can0
    ```

    With the vehicle powered and a module on the matching X179 pair, expect to
    see IDs in this family (from `docs/reference/can-protocol.md`, hex as printed
    by `candump`):

    | Variant | Expected IDs (hex)         |
    | ------- | -------------------------- |
    | HW4     | `399`, `3F8`, `3FD`, `273` |
    | HW3     | `3F8`, `3FD`, `273`        |
    | Legacy  | `045`, `3EE`               |

    A quiet bus may show nothing; confirm the vehicle is awake and the CAN-H/L
    pair is correct before declaring failure. UNVERIFIED.

3. **Termination**

    With the vehicle powered **off**, measure resistance between CAN-H and
    CAN-L at the module: two 120 Ω terminators in parallel read about 60 Ω. The
    Tesla harness normally supplies termination — do not add resistors unless a
    measurement says otherwise. For a bench-only two-node test, one 120 Ω across
    H/L is typically needed. UNVERIFIED — the guides only say "ensure 120 Ω
    termination"; they do not authorize modifying the vehicle harness.

4. **Provenance** — fill in the table below completely before treating this
   bring-up as evidence for later tasks.

## Provenance record (operator fills in — all UNVERIFIED by default)

| Field                       | Value (fill in) | How to obtain                                |
| --------------------------- | --------------- | -------------------------------------------- |
| Operator / date             | —               | —                                            |
| Pi model / board revision   | —               | `cat /proc/cpuinfo`                          |
| OS / kernel                 | —               | `cat /etc/os-release`, `uname -a`            |
| MCP2515 crystal             | —               | Read silkscreen: 8 MHz or 16 MHz             |
| Transceiver variant         | —               | Part number + datasheet voltage range        |
| 5 V mod applied?            | —               | Trace cut + jumper, or 3.3 V-rated module    |
| Level shifters              | —               | Which lines, if any                          |
| CS / INT used per bus       | —               | From the "Extra buses" table / actual wiring |
| X179 pairs used             | —               | 13/14, 9/10, 2/3 as applicable               |
| Termination measured (Ω)    | —               | Multimeter across CAN-H / CAN-L, vehicle off |
| Wiring photo                | —               | File path or link to a clear photo           |
| `ip -details` output        | —               | Paste from acceptance check 1                |
| `candump` sample / IDs seen | —               | Paste from acceptance check 2                |
| Pass / fail per check       | —               | Checks 1–4 above                             |

## DO NOT ENABLE TX

**Transmission is forbidden in this phase.** Until the **Task B3 gate** passes,
this bring-up is strictly RX/listen-only:

- Do **not** run `cansend` or any other frame transmitter (`cangen`, `canplayer`,
  Python `bus.send()`, etc.). The `cansend` examples in both cited guides are
  explicitly out of scope for this doc.
- Do **not** issue any firmware command that writes to a CAN bus through a Pi
  bridge (`fsd:on`, `drive:on`, `offset:N`, injection commands, etc.).
- Do **not** leave an interface up in normal (ACK-capable) mode "just to try
  it". Use `listen-only on` where supported; if unsupported, stop and record
  the limitation rather than falling back to unconstrained TX.
- Do **not** modify the vehicle harness or inject frames to "test" termination.

Rationale: an unproven adapter that transmits or ACKs on Tesla's chassis,
vehicle, or body buses can disturb safety-critical ECUs. TX enable is gated on
Task B3's hardware-in-the-loop gate; until that gate is signed off, any frame
transmission is out of scope. If you believe TX is required, **stop and
escalate** rather than proceeding.

## Not covered

- No claim of success; no hardware was exercised to produce this doc.
- No code copied from external guides; the inaccessible forum thread was unused.
- No ESP32 firmware changes, no flashing procedure, no vehicle power-over-X179
  wiring, and no TX/injection procedure of any kind.
