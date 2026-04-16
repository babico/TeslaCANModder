# Getting Started

TeslaCANModder is an open-source CAN bus modification tool for Tesla vehicles. It intercepts and modifies CAN frames to enable features like FSD, nag suppression, speed profiles, summon, and vehicle control commands.

## Supported Boards

| Board | CAN Driver | Buses | Connectivity |
| ----- | --------- | ----- | ----------- |
| **Arduino Uno** | MCP2515 (SPI) | 1–3 | USB, HC-05 Bluetooth |
| **ESP32-S DevKit** | MCP2515 (SPI) | 1–3 | USB, WiFi (AP/STA), BLE |

## Quick Start — Arduino Uno

1. Wire MCP2515 module to Arduino Uno (see [Hardware Setup](hardware-setup))
2. Go to **Flasher** tab → select firmware variant → flash
3. Go to **Dashboard** → Connect USB → verify boot message
4. Select your vehicle variant (HW4 / HW3 / Legacy) in the connection bar
5. Enable features (FSD, Nag, etc.) — all OFF by default
6. Install in vehicle via X179 connector

## Quick Start — ESP32-S DevKit

1. Wire 3× MCP2515 modules to ESP32 via SPI (see [Hardware Setup](hardware-setup))
2. Go to **Flasher** tab → select ESP32 firmware variant → flash
3. Go to **Dashboard** → Connect USB → verify boot message
4. Connect to WiFi AP `TeslaCANModder` (password: `teslacan123`) for wireless control
5. Or pair via BLE using any Nordic UART compatible app
6. Select your vehicle variant (HW4 / HW3 / Legacy)
7. Enable features — all OFF by default
8. Install in vehicle via X179 connector

## Vehicle Variants

| Variant | Vehicles | Key Features |
| -------- | --------- | ------------ |
| **HW4** | 2023+ with HW4 (FSD v14+) | FSD, Nag, Profile, ISA Chime, Summon |
| **HW3** | 2019–2023 with HW3 | FSD, Nag, Profile, Speed Offset, Summon |
| **Legacy** | Pre-HW3 vehicles | FSD, Nag, Profile (limited) |

Select the variant in the Dashboard connection bar or via `variant:hw4` / `variant:hw3` / `variant:legacy` command. Setting is saved to NVS (ESP32) or EEPROM (Arduino).

## Safety Warning

> **WARNING:** This device modifies vehicle CAN bus messages. Use at your own risk.
> Improper use may affect vehicle safety systems. Always test in safe environments.
> Educational and research purposes only.
