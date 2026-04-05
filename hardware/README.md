# TeslaCANModder Hardware Firmware

Simplified Arduino Uno firmware for Tesla CAN bus modification. Supports runtime variant switching (HW4/HW3/Legacy) with full web UI control.

## Hardware Requirements

- Arduino Uno R3 (CH340 or ATmega16U2)
- MCP2515 CAN module with TJA1050 transceiver (8 MHz crystal)
- HC-05 Bluetooth module (optional)
- 9V-36V to 5V/3A USB converter
- Tesla X179 connector

## Wiring

### MCP2515
- VCC → 5V
- GND → GND
- CS → D10
- INT → D2
- SCK/MISO/MOSI → SPI pins

### HC-05 (Optional)
- RX → D4
- TX → D5 (via 1kΩ + 2kΩ voltage divider)
- VCC → 3.3V regulator
- GND → GND

### X179 Power & CAN
- Pin 1 → Converter VIN+
- Pin 20 → Converter VIN-
- Converter USB → Arduino USB
- Pin 13 → MCP2515 CAN-H
- Pin 14 → MCP2515 CAN-L

## Build

```powershell
# USB + Bluetooth
.\pio.ps1 run -e uno

# USB only
.\pio.ps1 run -e uno_usb

# Upload
.\pio.ps1 run -e uno -t upload
```

## Memory Usage

| Build | RAM | Flash |
|-------|-----|-------|
| USB + Bluetooth | 1558 bytes (76%) | 11150 bytes (35%) |
| USB only | 1378 bytes (67%) | 9668 bytes (30%) |

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

All commands are newline-terminated ASCII over USB or Bluetooth.

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
{"t":"boot","hw":"ArduinoUnoR3CH340","variant":"hw4","cap":"usb+bluetooth",...}
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
│   │   ├── config.h       - Pin definitions, build flags
│   │   ├── types.h        - Frame, State, Features, Variant
│   │   └── driver.h       - MCP2515 hardware interface
│   ├── protocol/
│   │   ├── can.h          - CAN IDs, basic frame helpers
│   │   ├── fsd.h          - FSD frame manipulation
│   │   ├── summon.h       - ASS summon control
│   │   └── vehicle.h      - 0x273 vehicle control
│   ├── handler/
│   │   ├── hw4.h          - HW4 message handler
│   │   ├── hw3.h          - HW3 message handler
│   │   ├── legacy.h       - Legacy message handler
│   │   └── dispatch.h     - Message routing and filters
│   ├── command/
│   │   ├── system.h       - System commands
│   │   ├── fsd.h          - FSD commands
│   │   └── vehicle.h      - Vehicle control commands
│   └── io/
│       └── serial.h       - Serial/BT I/O, JSON, command router
├── src/
│   └── main.cpp           - setup() + loop()
└── platformio.ini         - Build configuration
```

### Module Responsibilities

**Core Infrastructure (core/):**
- **config.h** - Hardware configuration and compile-time flags
- **types.h** - Core data structures (Frame, State, Features, Variant)
- **driver.h** - Low-level MCP2515 SPI communication

**Protocol Layer (protocol/):**
- **can.h** - CAN ID definitions and basic frame helpers
- **fsd.h** - FSD-specific frame encoding/decoding
- **summon.h** - ASS summon frame manipulation
- **vehicle.h** - 0x273 vehicle control frame manipulation

**Handler Layer (handler/):**
- **hw4.h** - HW4-specific CAN message processing
- **hw3.h** - HW3-specific CAN message processing
- **legacy.h** - Legacy vehicle CAN message processing
- **dispatch.h** - Routes messages to variant handlers, manages filters

**Command Layer (command/):**
- **system.h** - System command execution (streaming, raw CAN)
- **fsd.h** - FSD command execution (fsd/nag/profile/offset/isa-chime/summon/variant)
- **vehicle.h** - Vehicle control command execution (mirrors/locks/lights/wipers/seats/display/power)

**I/O Layer (io/):**
- **serial.h** - Serial/Bluetooth communication, JSON serialization, command routing

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
