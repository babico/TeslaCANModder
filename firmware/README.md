# TeslaCANModder Hardware Firmware

Firmware for Tesla CAN bus modification supporting ESP32-S DevKit. Supports runtime variant switching (HW4/HW3/Legacy) with full client control.

## Hardware Requirements

### ESP32-S DevKit

- ESP32-S DevKit (30-pin or 38-pin) — Built-in WiFi + BLE
- MCP2515 CAN module with TJA1050 transceiver (8 MHz crystal) × 1–3
- 9V-36V to 5V/3A Buck converter
- Tesla X179 connector

## Wiring

See [docs/guides/hardware-setup.md](../docs/guides/hardware-setup.md) for full ESP32 + MCP2515 wiring (per-bus CS/INT, SPI sharing, X179 connector pinout).

## Build

```powershell
# ESP32
.\.pio.ps1 run -e esp32         # Serial only
.\.pio.ps1 run -e esp32_wifi    # WiFi REST API
.\.pio.ps1 run -e esp32_ble     # BLE
.\.pio.ps1 run -e esp32_wifi_ble # WiFi + BLE

# Enable extra CAN buses (FSD is always on)
$env:PLATFORMIO_BUILD_FLAGS = "-DBUS_VEHICLE_ACTIVE=1 -DBUS_BODY_ACTIVE=1"
.\.pio.ps1 run -e esp32_wifi

# Upload
.\pio.ps1 run -e esp32 -t upload
```

## Features

### Variants

- `hw4` - HW4 (2026.2.3+) with ISA chime control
- `hw3` - HW3 with speed offset control
- `legacy` - Pre-HW3 vehicles

### Controls (All OFF by default)

- FSD enable/disable
- Nag suppression
- Speed profile (0-4)
- Speed offset (HW3: 0-100%)
- ISA chime suppression (HW4)
- CAN frame streaming
- Raw CAN listen mode

## Commands

All commands are newline-terminated ASCII over USB, BLE, or WiFi.

### System

- `ping` - Health check
- `status` - Full state dump
- `stream:on` / `stream:off` - Enable or disable CAN frame streaming
- `can:raw:on` / `can:raw:off` - Enable or disable raw CAN listen mode

### Variant

- `variant:hw4` / `variant:hw3` / `variant:legacy`

### FSD / Feature Controls

- `fsd:on` / `fsd:off` / `fsd:toggle`
- `nag:on` / `nag:off` / `nag:toggle`
- `profile:0` to `profile:4` (or `sp:0` to `sp:4`)
- `profile:auto` - Track stalk input instead of user override
- `offset:auto` - Track HW3 UI speed offset instead of user override
- `offset:0` to `offset:100` (HW3 only)
- `isa-chime:on` / `isa-chime:off` / `isa-chime:toggle` (HW4 only)

### Summon

- `summon` - Start summon using the last known direction
- `summon:forward` / `summon:fwd`
- `summon:reverse` / `summon:rev`
- `summon:stop`

### Mirrors

- `mirror:fold`
- `mirror:unfold`
- `mirror:heat`
- `mirror:autofold`
- `mirror:dip`

### Locks

- `lock`
- `unlock`
- `lock:child`
- `horn`

### Trunk / Frunk

- `frunk:open` / `frunk`
- `frunk:close`
- `trunk:open` / `trunk`
- `trunk:close`
- `glovebox`

### Lighting

- `light:fog:front`
- `light:fog:rear`
- `light:highbeam:auto`
- `light:ambient`
- `light:home`
- `light:dome:off`
- `light:dome:on`
- `light:dome:auto`

### Wipers

- `wiper:off`
- `wiper:1`
- `wiper:2`
- `wiper:3`

### Seats

- `seat:fl:0` to `seat:fl:3`
- `seat:fr:0` to `seat:fr:3`
- `seat:rl:0` to `seat:rl:3`
- `seat:rr:0` to `seat:rr:3`
- `seat:rc:0` to `seat:rc:3`

### Display

- `maindisplay:0` to `maindisplay:127`

### Power

- `power:acc:on`
- `power:acc:off`
- `power:off`
- `power:ready`

### Windows

- `window:vent:N` (N = 0..100)
- `vent:open`
- `vent:close`

### Sentry

- `sentry:on`
- `sentry:off`

### Climate

- `climate:keep`
- `climate:off`

### Charge

- `charge:start`
- `charge:stop`
- `charge:port` / `chargeport`

### Drive Configuration

- `pedal:standard` / `pedal:std`
- `pedal:chill`
- `pedal:sport`
- `regen:off`
- `regen:low`
- `regen:standard` / `regen:std`
- `regen:max`
- `stop:creep`
- `stop:roll`
- `stop:hold`

## Protocol

JSON messages over serial (115200 baud):

### Boot

```json
{"t":"boot","hw":"ESP32S_DevKit","variant":"hw4","cap":"usb+wifi+ble",...}
```

### Status (every 500ms)

```json
{"t":"status","variant":"hw4","fsd":0,"sp":1,"offset":0,"isaChime":0,"nag":0,...}
```

### Frame (when streaming)

```json
{"t":"frame","dir":"rx","id":1021,"dlc":8,"d":"0102030405060708",...}
```

### Ack/Error

```json
{"t":"ack","cmd":"fsd:on"}
{"t":"error","msg":"Invalid variant"}
```

## Code Structure

```
hardware/
├── lib/
│   ├── core/
│   │   ├── types.h        - Frame, State, Features, Variant
│   │   ├── forward.h      - Forward declarations
│   │   ├── config/        - Pin definitions per platform
│   │   ├── driver/        - MCP2515 hardware interface per platform
│   │   └── persist/       - Settings save/load per platform
│   ├── infra/
│   │   ├── can.h          - CAN IDs, bus defs, frame helpers
│   │   └── burst.h        - Non-blocking burst send
│   ├── feature/
│   │   ├── <name>/cmd.h      - Command parser
│   │   ├── <name>/protocol.h - CAN frame builder
│   │   └── ...            - 27 feature folders
│   ├── handler/
│   │   ├── hw4.h          - HW4 message handler
│   │   ├── hw3.h          - HW3 message handler
│   │   ├── legacy.h       - Legacy message handler
│   │   └── dispatch/      - Platform-specific message routing
│   └── io/
│       ├── serial/        - Serial/BT command router per platform
│       ├── wifi/          - WiFi REST API + dashboard (ESP32)
│       └── ble/           - BLE GATT service (ESP32)
├── src/
│   └── esp32/main.cpp
└── platformio.ini         - Build configuration
```

### Module Responsibilities

**Core Infrastructure (core/):**

- **types.h** - Core data structures (Frame, State, Features, Variant)
- **forward.h** - Forward declarations for cross-layer references
- **config/** - Hardware pin assignments per platform
- **driver/** - Low-level MCP2515 SPI communication per platform
- **persist/** - Settings save/load (NVS on ESP32)

**Infrastructure (infra/):**

- **can.h** - CAN ID definitions, bus constants, frame helpers
- **burst.h** - Non-blocking burst send (`startBurst` + `burstTick`)

**Feature Layer (feature/):**

Each feature has its own folder with `cmd.h` (command parser) and optionally
`protocol.h` (CAN frame builder). Examples:

- **charge/** - Charge start/stop/port control
- **trunk/** - Trunk/frunk/glovebox one-shot commands
- **fsd/** - FSD enable/disable (cmd only, handler-based)
- **profile/** - Speed profile + follow distance mapping

**Handler Layer (handler/):**

- **hw4.h** - HW4-specific CAN message processing
- **hw3.h** - HW3-specific CAN message processing
- **legacy.h** - Legacy vehicle CAN message processing
- **dispatch/** - Routes messages to variant handlers, manages filters

**I/O Layer (io/):**

- **serial/** - Serial/Bluetooth command dispatch per platform
- **wifi/** - WiFi REST API server + embedded dashboard (ESP32)
- **ble/** - BLE GATT Nordic UART service (ESP32)

## Default State

- Variant: HW4
- FSD: OFF
- Nag Suppression: OFF
- Speed Profile: 1 (Standard)
- Speed Offset: 0
- ISA Chime Suppression: OFF
- Frame Streaming: OFF
- Raw CAN Listen: OFF

## License

GPL-3.0 - See legacy reference projects for original implementations.
