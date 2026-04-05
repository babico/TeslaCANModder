# Setup Guide

Complete hardware installation and software configuration for TeslaCANModder.

## Quick Start

1. Wire MCP2515 to Arduino Uno (see Wiring Guide)
2. Open Flasher tab → select firmware variant → flash
3. Open Dashboard → Connect USB → verify boot message
4. Select your variant (HW4 / HW3 / Legacy) in the connection bar
5. Enable features (FSD, Nag, etc.) — all OFF by default
6. Install in vehicle via X179 connector

## Required Hardware

- **Arduino Uno R3** — CH340 or ATmega16U2 USB chip. Runs the firmware.
- **MCP2515 CAN Module** — 8 MHz crystal + TJA1050 transceiver. Connects to VehicleBus via X179.
- **9V-36V to 5V Buck Converter** — 3A minimum. Powers Arduino from vehicle 12V rail.
- **Tesla X179 Connector** — Behind center screen. Provides 12V power + CAN bus access.

### Optional

- **HC-05 Bluetooth Module** — Wireless phone control. Requires voltage divider on RX pin.
- **MCP2515 CAN Module #2** — Second CAN bus (e.g. Powertrain). Shares SPI, uses D9/D3.

## Firmware Variants

| Variant | Bluetooth | Dual CAN | Use Case |
|---------|-----------|----------|----------|
| USB Only | No | No | Lightest firmware |
| USB + Bluetooth | Yes | No | Wireless control via HC-05 |
| USB + Dual CAN | No | Yes | Monitor two CAN buses |
| Full | Yes | Yes | Everything enabled |

All variants support all FSD/vehicle features. Bluetooth and Dual CAN only affect I/O capability.

## Vehicle Variant Selection

| Variant | Vehicles | Features |
|---------|----------|----------|
| HW4 | 2023+ with HW4 (FSD v14+) | FSD, Nag, Profile, ISA Chime, Summon |
| HW3 | 2019–2023 with HW3 | FSD, Nag, Profile, Speed Offset, Summon |
| Legacy | Older vehicles / simple CAN | FSD, Nag only |

Select your variant in the connection bar after connecting. The firmware saves it to EEPROM.

## Software Setup

### Mobile App (Android / iOS)
1. Install the app from source or build with Expo
2. Connect via Bluetooth (mobile) or USB (web)
3. All controls are available from the Dashboard tab

### Web (Chrome / Edge)
1. Open the web app in Chrome or Edge (Web Serial API required)
2. Click "Connect USB" and select the Arduino serial port
3. Use the Flasher tab to update firmware directly from browser

### PlatformIO CLI
```bash
cd hardware
pio run -e uno_usb -t upload
```
