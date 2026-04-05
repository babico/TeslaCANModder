# TeslaCANModder

Browser-based control and monitoring stack for Tesla CAN bus modification using Arduino Uno.

## Quick Start

### Hardware
```powershell
cd hardware
.\pio.ps1 run -e uno          # Build with Bluetooth (default)
.\pio.ps1 run -e uno_usb      # Build USB-only (minimal)
.\pio.ps1 run -e uno_usb_mcp2 # Build USB + 2nd MCP2515
.\pio.ps1 run -e uno_full     # Build all features (BT + 2nd MCP2515)
.\pio.ps1 run -e uno -t upload # Flash to board
```

### Web UI
```bash
cd web
npm install
npm run dev
```

Open http://localhost:5173 and click "Connect USB"

## What's Included

- **Hardware Firmware** - Simplified Arduino Uno firmware (6 files, ~800 lines)
- **Web UI** - React dashboard with Web Serial API
- **Docker Support** - Build firmware and run web UI in containers

## Hardware

- Arduino Uno R3 (CH340 or ATmega16U2)
- MCP2515 CAN module (8 MHz crystal) - Primary bus
- MCP2515 CAN module (optional) - Secondary bus for dual-CAN setups
- HC-05 Bluetooth (optional)
- 9V-36V to 5V converter
- Tesla X179 connector

## Features

### Runtime Variant Switching
- HW4 (2026.2.3+) with ISA chime control
- HW3 with speed offset control
- Legacy for pre-HW3 vehicles

### Controls (All OFF by default, user enables via web)
- FSD enable/disable/toggle
- Nag suppression
- Speed profile (0-4)
- Speed offset (HW3: 0-100%)
- ISA chime suppression (HW4)
- ASS (Autopark Summon System) - Forward/Reverse/Stop
- CAN frame streaming
- Raw CAN listen mode
- Dual CAN bus support (optional)

## Memory Usage

| Build | RAM | Flash |
|-------|-----|-------|
| USB + Bluetooth | 1558 bytes (76%) | 11150 bytes (35%) |
| USB only | 1378 bytes (67%) | 9668 bytes (30%) |

## Protocol

JSON messages over serial (115200 baud):
- `boot` - Board initialization
- `status` - State updates (every 500ms)
- `frame` - CAN frames (when streaming)
- `ack` / `error` - Command responses
- `pong` - Ping response

## Commands

### Core FSD Controls
```
ping, status
variant:hw4, variant:hw3, variant:legacy
fsd:on, fsd:off, fsd:toggle
nag:on, nag:off, nag:toggle
profile:0-4 (or sp:0-4)
offset:0-100 (HW3 only)
isa-chime:on, isa-chime:off, isa-chime:toggle (HW4 only)
stream:on, stream:off
can:raw:on, can:raw:off
```

### ASS Summon
```
summon, summon:forward, summon:fwd
summon:reverse, summon:rev
summon:stop
```

### Vehicle Control (0x273 UI_vehicleControl)
```
# Mirror
mirror:fold, mirror:unfold, mirror:heat
mirror:autofold, mirror:dip

# Lock
lock, unlock, lock:child

# Trunk/Frunk
frunk, frunk:open, frunk:close
trunk, trunk:open, trunk:close
glovebox, horn

# Lighting (toggle switches)
light:fog:front, light:fog:rear
light:highbeam:auto, light:ambient, light:home
light:dome:off, light:dome:on, light:dome:auto

# Wiper
wiper:off, wiper:1, wiper:2, wiper:3

# Seat Heating (0-3: off/low/med/high)
seat:fl:0, seat:fl:1, seat:fl:2, seat:fl:3
seat:fr:0-3, seat:rl:0-3, seat:rr:0-3, seat:rc:0-3

# Display (main center screen, 0-127)
maindisplay:0-127

# Power (vehicle state control)
power:acc:on, power:acc:off, power:off, power:ready
```

### Advanced Features
```
# Window Vent (0x119)
window:vent:open, vent:open
window:vent:close, vent:close

# Sentry Mode (0x284)
sentry:on, sentry:off

# Climate Control (0x2F3 - requires frame caching)
climate:keep, climate:off

# Charge Control (0x333 - requires frame caching)
charge:start, charge:stop
charge:port, chargeport

# Drive Configuration (0x334 - requires frame caching)
# Pedal Response
pedal:standard, pedal:std, pedal:chill, pedal:sport

# Regenerative Braking (0-200)
regen:off, regen:low, regen:std, regen:max

# Stop Mode
stop:creep, stop:roll, stop:hold
```

## Browser Support

- ✅ Desktop Chrome/Edge - Full support (flashing + control)
- ✅ Android Chrome - Runtime control via Bluetooth
- ⚠️ Other browsers - Guide mode only

## Documentation

- `hardware/README.md` - Firmware details
- `web/README.md` - Web UI details

## License

GPL-3.0
