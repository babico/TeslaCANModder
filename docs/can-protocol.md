# CAN Protocol

TeslaCANModder communicates with the vehicle by intercepting and modifying CAN bus frames on the Tesla X179 connector.

## Bus Assignments

The ESP32 3-CAN configuration maps to the Tesla X179 connector as follows:

| Bus | MCP2515 | X179 Pins | Function | CAN Speed |
| --- | ------- | --------- | -------- | --------- |
| **0** | MCP2515 #1 | 13–14 | FSD / Autopilot CAN | 500 kbps |
| **1** | MCP2515 #2 | 9–10 | Vehicle Control CAN | 500 kbps |
| **2** | MCP2515 #3 | 2–3 | Body Control CAN | 500 kbps |

> 1-CAN builds use only Bus 0 (FSD). Vehicle and body commands require 3-CAN builds.

## CAN IDs

| CAN ID | Hex | Name | Bus | Used By |
| ------ | --- | ---- | --- | ------- |
| 69 | 0x045 | Legacy stalk position | FSD | Legacy |
| 130 | 0x082 | UI_tripPlanning (preconditioning) | Vehicle | HW3, HW4 |
| 281 | 0x119 | Window vent control | Vehicle | HW3, HW4 |
| 306 | 0x132 | BMS_hvBusStatus (voltage/current) | Vehicle | HW3, HW4 |
| 627 | 0x273 | UI_vehicleControl (summon, lock, etc.) | Vehicle | HW3, HW4 |
| 644 | 0x284 | Sentry mode control | Vehicle | HW3, HW4 |
| 658 | 0x292 | BMS_socStatus (state of charge) | Vehicle | HW3, HW4 |
| 755 | 0x2F3 | Climate control | Vehicle | HW3, HW4 |
| 786 | 0x312 | BMS_thermalStatus (cell temps) | Vehicle | HW3, HW4 |
| 787 | 0x313 | UI_trackModeSettings | Vehicle | HW3, HW4 |
| 792 | 0x318 | GTW_carState (OTA detection) | Vehicle | HW3, HW4 |
| 819 | 0x333 | Charge control | Vehicle | HW3, HW4 |
| 820 | 0x334 | Drive config (pedal/regen/stop) | Vehicle | HW3, HW4 |
| 826 | 0x33A | BMS_energyStatus (Wh/km) | Vehicle | HW3, HW4 |
| 880 | 0x370 | EPAS_sysStatus (torque sensor) | Vehicle | HW3, HW4 |
| 921 | 0x399 | ISA speed chime | FSD | HW4 |
| 920 | 0x398 | GTW_carConfig (auto HW detect) | Vehicle | HW3, HW4 |
| 947 | 0x3B3 | Trunk/Glovebox control | Vehicle | HW3, HW4 |
| 1006 | 0x3EE | Legacy FSD mux | FSD | Legacy |
| 1016 | 0x3F8 | Follow distance (profile mapping) | FSD | HW3, HW4 |
| 1021 | 0x3FD | FSD mux (FSD/nag/profile/offset) | FSD | HW3, HW4 |

## FSD Mux Frame (CAN ID 1021)

The main FSD frame uses a multiplexer in the lower 3 bits of byte 0:

| Mux ID | Function | Modified Bits |
| ------ | -------- | ------------- |
| **0** | FSD enable | bit 38 (FSD active), bit 46, bit 60 |
| **1** | Nag suppress | bit 19 (nag flag cleared), bit 47 (HW4) |
| **2** | Speed profile / offset | Profile bytes (HW4) or offset bytes (HW3) |

### HW4 Handler

- **Mux 0:** Sets bits 38, 46, 60 to enable FSD when UI has FSD selected
- **Mux 1:** Clears bit 19 and sets bit 47 to suppress nag
- **Mux 2:** Writes speed profile value

### HW3 Handler

- **Mux 0:** Sets bits 38, 46, writes speed profile into v12/v13 fields
- **Mux 1:** Clears bit 19 to suppress nag
- **Mux 2:** Writes speed offset value

### Legacy Handler

- Uses CAN ID **1006** instead of 1021
- **Mux 0:** Sets bit 46, writes speed profile
- **Mux 1:** Clears bit 19 to suppress nag

## ISA Speed Chime (HW4 only)

CAN ID 921 — ISA speed chime suppression:

- Sets `data[1] |= 0x20` to suppress the chime
- Recalculates checksum in `data[7]`
- Only active when `isaChimeSuppress` is enabled

## Follow Distance → Profile Mapping

CAN ID 1016 — The follow distance stalk position is read from `data[5] bits 7:5` and mapped to a speed profile (0–3) unless profile is manually pinned.

## EPAS Torque Spoofing (Nag Killer)

CAN ID 0x370 — Intercepts the EPAS torque sensor frame and echoes it with zeroed steering torque:

- `data[2]`, `data[3]` = 0x00 (zero torque)
- Counter (nibble in `data[1]`) incremented by 1
- Checksum in `data[7]` = sum of bytes 0–6 + 0x70 + 0x03

This is more aggressive than bit-19 nag suppression and eliminates the "apply steering force" prompt.

## BMS Telemetry Frames

Battery management data is decoded from multiple CAN IDs:

| CAN ID | Frame | Decoded Fields |
| ------ | ----- | -------------- |
| 0x132 | BMS_hvBusStatus | Voltage (bytes 0–1 × 0.01V), Current (bytes 2–3 signed × 0.1A) |
| 0x292 | BMS_socStatus | State of Charge (bits 9:0 × 0.1%) |
| 0x312 | BMS_thermalStatus | Temp min (byte 4 − 40°C), Temp max (byte 5 − 40°C) |
| 0x33A | BMS_energyStatus | Energy consumption (bytes 0–1 × 0.1 Wh/km) |

## Preconditioning Frame

CAN ID 0x082 — UI_tripPlanning frame injected on the Vehicle bus to trigger battery preconditioning:

- `data[0] = 0x05` to activate, `0x00` to deactivate
- Requires periodic injection at 500ms intervals

## Track Mode Frame

CAN ID 0x313 — UI_trackModeSettings injected on the Vehicle bus:

- `data[0]` bits 1:0 = `0x01` to enable Track Mode
- Sent as a 20-frame burst at 20ms intervals

## OTA Detection

CAN ID 0x318 — GTW_carState is monitored for OTA update status. When an OTA is detected, all CAN TX injection is automatically paused until the update completes.

## Auto Hardware Detection

CAN ID 0x398 — GTW_carConfig is read at boot to automatically detect whether the vehicle uses HW3 or HW4. Falls back to the manually configured variant if this frame is not received.

## Vehicle Control Frame (CAN ID 627)

Used for summon, vehicle commands, seat heating, wiper, display, and power control. All use the cached 0x273 frame as a base with specific bits modified.

### Summon

- Bit 4 of byte 0: Summon active
- Bit 5 of byte 0: Direction (0 = forward, 1 = reverse)
- Bit 0 of byte 0: Mode (start/stop)
- Sends 30-frame bursts when summon is activated

### Seat Heating

Five seats, each with 2-bit level (0=off, 1=low, 2=med, 3=high):

| Seat | Byte | Bits | Mask |
| ---- | ---- | ---- | ---- |
| Front-left | 5 | 42–43 | 0x0C |
| Front-right | 5 | 44–45 | 0x30 |
| Rear-left | 5 | 46–47 | 0xC0 |
| Rear-center | 6 | 48–49 | 0x03 |
| Rear-right | 6 | 50–51 | 0x0C |

### Wiper

Wiper speed in byte 7 bits 56–58 (mask 0x07): 0=off, 1=low, 2=medium, 3=high. Sends 20-frame burst.

### Display Brightness

Byte 4 (bits 32–39) sets display brightness (0–127, factor 0.5). Sends 20-frame burst.

### Power Control

| Function | Byte | Bit | Value |
| -------- | ---- | --- | ----- |
| Accessory power | 0 | 0 | 1=on, 0=off |
| Power off | 3 | 31 | 1=off |
| Drive-ready | 7 | 62 | 1=enable |

## Frame Routing

The dispatch system routes frames based on CAN ID and configured variant:

1. **RX from Bus 0 (FSD):** IDs 921, 1006, 1016, 1021, 69 → variant handler → modified frame sent back
2. **RX from Bus 1 (Vehicle):** IDs 627, 0x082, 0x132, 0x292, 0x312, 0x313, 0x318, 0x33A, 0x370, 0x398 → cached/decoded/intercepted
3. **TX to Bus 1 (Vehicle):** Vehicle commands, preconditioning, track mode, nag killer echo
4. **Streaming:** All received frames optionally forwarded to serial/BLE as JSON
5. **OTA Guard:** TX automatically paused when 0x318 indicates OTA in progress

## Testing CAN Communication

```bash
# Enable raw CAN mode to see all frames
can:raw:on

# Start frame streaming
stream:on

# Expected frames by variant:
# HW4: 921, 1016, 1021, 627
# HW3: 1016, 1021, 627
# Legacy: 69, 1006
```

If no frames appear, check CAN-H/CAN-L connections and verify the vehicle is powered on.
