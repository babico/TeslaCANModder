---
title: CAN Protocol
title_tr: CAN Protokolü
description: CAN bus frame structure, IDs, and bus assignments
category: reference
folder: reference
tags: [can, protocol, frames, bus]
order: 5
icon: 🔌
---

# CAN Protocol

TeslaCANModder communicates with the vehicle by intercepting and modifying CAN bus frames on the Tesla X179 connector.

## Bus Assignments

The ESP32 supports up to three CAN buses on a shared SPI fabric. All three lanes land on the Tesla X179 connector.

| Bus     | MCP2515    | Connector       | Function                | CAN Speed |
| ------- | ---------- | --------------- | ----------------------- | --------- |
| Chassis | MCP2515 #1 | X179 pins 13–14 | Chassis / Autopilot CAN | 500 kbps  |
| Vehicle | MCP2515 #2 | X179 pins 9–10  | Vehicle Control CAN     | 500 kbps  |
| Body    | MCP2515 #3 | X179 pins 2–3   | Body Control CAN        | 500 kbps  |

> Every bus is opt-in via PlatformIO build flags (`BUS_CHASSIS_ACTIVE`, `BUS_VEHICLE_ACTIVE`, `BUS_BODY_ACTIVE`). Builds without `_chassis` run as a passive sniffer — DAS injection requires the Chassis CAN.

## CAN IDs

| CAN ID | Hex   | Name                                                   | Bus     | Used By  |
| ------ | ----- | ------------------------------------------------------ | ------- | -------- |
| 69     | 0x045 | Legacy stalk position                                  | Chassis | Legacy   |
| 130    | 0x082 | UI_tripPlanning (preconditioning)                      | Vehicle | HW3, HW4 |
| 281    | 0x119 | Window vent control                                    | Vehicle | HW3, HW4 |
| 306    | 0x132 | BMS_hvBusStatus (voltage/current)                      | Vehicle | HW3, HW4 |
| 537    | 0x219 | TPMS tire pressure/temperature                         | Vehicle | HW3, HW4 |
| 553    | 0x229 | EPAS_harness (CRC-protected)                           | Chassis | HW3, HW4 |
| 585    | 0x249 | DI_steer (drive mode readback)                         | Chassis | HW3, HW4 |
| 627    | 0x273 | UI_vehicleControl (summon, lock, etc.)                 | Vehicle | HW3, HW4 |
| 644    | 0x284 | Sentry mode control                                    | Vehicle | HW3, HW4 |
| 658    | 0x292 | BMS_socStatus (state of charge)                        | Vehicle | HW3, HW4 |
| 755    | 0x2F3 | Climate control                                        | Vehicle | HW3, HW4 |
| 786    | 0x312 | BMS_thermalStatus (cell temps)                         | Vehicle | HW3, HW4 |
| 787    | 0x313 | UI_trackModeSettings                                   | Vehicle | HW3, HW4 |
| 792    | 0x318 | GTW_carState (OTA detection)                           | Vehicle | HW3, HW4 |
| 819    | 0x333 | Charge control                                         | Vehicle | HW3, HW4 |
| 820    | 0x334 | Drive config (pedal/regen/stop/drive mode inject)      | Vehicle | HW3, HW4 |
| 826    | 0x33A | BMS_energyStatus (Wh/km)                               | Vehicle | HW3, HW4 |
| 880    | 0x370 | EPAS_sysStatus (torque sensor)                         | Vehicle | HW3, HW4 |
| 914    | 0x392 | GTW_version (gateway firmware version, muxed)          | Vehicle | HW3, HW4 |
| 920    | 0x398 | GTW_carConfig (auto HW detect, region, vehicle config) | Vehicle | HW3, HW4 |
| 921    | 0x399 | ISA speed chime                                        | Chassis | HW4      |
| 923    | 0x39B | DAS_status (hands-on request state)                    | Vehicle | HW3, HW4 |
| 2047   | 0x7FF | GTW_carConfig mux (autopilot tier readback)            | Vehicle | HW3, HW4 |
| 947    | 0x3B3 | Trunk/Glovebox control                                 | Vehicle | HW3, HW4 |
| 1006   | 0x3EE | Legacy FSD mux                                         | Chassis | Legacy   |
| 1016   | 0x3F8 | Follow distance (profile mapping)                      | Chassis | HW3, HW4 |
| 1021   | 0x3FD | FSD mux (FSD/nag/profile/offset/ECE R79)               | Chassis | HW3, HW4 |
| 262    | 0x106 | Rear motor RPM                                         | Vehicle | HW3, HW4 |
| 277    | 0x115 | Front motor RPM (dual motor)                           | Vehicle | HW3, HW4 |
| 280    | 0x118 | DI_state (gear/accelerator pedal)                      | Vehicle | HW3, HW4 |
| 297    | 0x129 | Steering angle sensor                                  | Vehicle | HW3, HW4 |
| 599    | 0x257 | Vehicle speed                                          | Vehicle | HW3, HW4 |
| 682    | 0x2AA | Air recirculation control                              | Vehicle | HW3, HW4 |
| 1011   | 0x3F3 | VCRIGHT seatbelt status                                | Vehicle | HW3, HW4 |
| 1013   | 0x3F5 | VCFRONT vehicle lights (turn signals)                  | Vehicle | HW3, HW4 |

## FSD Mux Frame (CAN ID 1021)

The main FSD frame uses a multiplexer in the lower 3 bits of byte 0:

| Mux ID | Function               | Modified Bits                             |
| ------ | ---------------------- | ----------------------------------------- |
| **0**  | FSD enable             | bit 38 (FSD active), bit 46, bit 60       |
| **1**  | Nag suppress           | bit 19 (nag flag cleared), bit 47 (HW4)   |
| **2**  | Speed profile / offset | Profile bytes (HW4) or offset bytes (HW3) |

### HW4 Handler

- **Mux 0:** Sets bits 38, 46, 60 to enable FSD when UI has FSD selected
- **Mux 1:** Clears bit 19 and sets bit 47 to suppress nag
- **Mux 2:** Writes speed profile value; optional HW4 offset override writes `data[1]` low 6 bits (`0..63`, `0=off`)

### HW3 Handler

- **Mux 0:** Sets bits 38, 46, writes speed profile into v12/v13 fields
- **Mux 1:** Clears bit 19 to suppress nag
- **Mux 2:** Writes speed offset value

### Legacy Handler

- Uses CAN ID **1006** instead of 1021
- **Mux 0:** Sets bit 46, writes speed profile
- **Mux 1:** Clears bit 19 to suppress nag

### Force FSD Override

When `forcefsd:on` is enabled, mux-0 FSD edits are applied even if the UI FSD-selected bit is not set. This is an explicit override and should be used with caution.

## ISA Speed Chime (HW4 only)

CAN ID 921 — ISA speed chime suppression:

- Sets `data[1] |= 0x20` to suppress the chime
- Recalculates checksum in `data[7]`
- Only active when `isaChimeSuppress` is enabled

> **Note:** On HW3, CAN ID 0x399 on the Vehicle bus carries DAS_status (blind spot),
> not ISA speed chime. The ISA chime handler gates on `variant == HW4`.

## Follow Distance → Profile Mapping

CAN ID 1016 — The follow distance stalk position is read from `data[5] bits 7:5` and mapped to a speed profile (0–3) unless profile is manually pinned.

## EPAS Torque Spoofing (Nag Killer)

CAN ID 0x370 — Intercepts the EPAS torque sensor frame and echoes it with zeroed steering torque:

- `data[2]`, `data[3]` = 0x00 (zero torque)
- Counter (nibble in `data[1]`) incremented by 1
- Checksum in `data[7]` = sum of bytes 0–6 + 0x70 + 0x03

This is more aggressive than bit-19 nag suppression and eliminates the "apply steering force" prompt.

### Nag Killer Modes

- `legacy`: always echo the modified 0x370 frame while nag killer is enabled.
- `safe`: echo only when DAS requests hands-on steering input.

Safe mode reads `DAS_autopilotHandsOnState` from CAN ID `0x39B` and only emits spoof frames when that signal indicates a hands-on request.

## BMS Telemetry Frames

Battery management data is decoded from multiple CAN IDs:

| CAN ID | Frame             | Decoded Fields                                                 |
| ------ | ----------------- | -------------------------------------------------------------- |
| 0x132  | BMS_hvBusStatus   | Voltage (bytes 0–1 × 0.01V), Current (bytes 2–3 signed × 0.1A) |
| 0x292  | BMS_socStatus     | State of Charge (bits 9:0 × 0.1%)                              |
| 0x312  | BMS_thermalStatus | Temp min (byte 4 − 40°C), Temp max (byte 5 − 40°C)             |
| 0x33A  | BMS_energyStatus  | Energy consumption (bytes 0–1 × 0.1 Wh/km)                     |

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

## GTW Autopilot Tier Readback

CAN ID 0x7FF — GTW_carConfig mux frames are monitored for autopilot tier diagnostics. On mux `2`, `byte[5] bits 4:2` are decoded as:

- `0`: NONE
- `1`: HIGHWAY
- `2`: ENHANCED
- `3`: SELF_DRIVING
- `4`: BASIC

Status payload exposes this as `gtwAutopilotTier` (`-1` when unknown).

## Vehicle Control Frame (CAN ID 627)

Used for summon, vehicle commands, seat heating, wiper, display, and power control. All use the cached 0x273 frame as a base with specific bits modified.

### Summon

- Bit 4 of byte 0: Summon active
- Bit 5 of byte 0: Direction (0 = forward, 1 = reverse)
- Bit 0 of byte 0: Mode (start/stop)
- Sends 30-frame bursts when summon is activated

### Seat Heating

Five seats, each with 2-bit level (0=off, 1=low, 2=med, 3=high):

| Seat        | Byte | Bits  | Mask |
| ----------- | ---- | ----- | ---- |
| Front-left  | 5    | 42–43 | 0x0C |
| Front-right | 5    | 44–45 | 0x30 |
| Rear-left   | 5    | 46–47 | 0xC0 |
| Rear-center | 6    | 48–49 | 0x03 |
| Rear-right  | 6    | 50–51 | 0x0C |

### Wiper

Wiper speed in byte 7 bits 56–58 (mask 0x07): 0=off, 1=low, 2=medium, 3=high. Sends 20-frame burst.

### Display Brightness

Byte 4 (bits 32–39) sets display brightness (0–127, factor 0.5). Sends 20-frame burst.

### Power Control

| Function        | Byte | Bit | Value       |
| --------------- | ---- | --- | ----------- |
| Accessory power | 0    | 0   | 1=on, 0=off |
| Power off       | 3    | 31  | 1=off       |
| Drive-ready     | 7    | 62  | 1=enable    |

## Frame Routing

The dispatch system routes frames based on CAN ID and configured variant:

1. **RX from the Chassis bus:** IDs 921, 1006, 1016, 1021, 69 → variant handler → modified frame sent back
2. **RX from the Vehicle bus:** IDs 627, 0x082, 0x132, 0x292, 0x312, 0x313, 0x318, 0x33A, 0x370, 0x392, 0x398, 0x39B, 0x7FF → cached/decoded/intercepted
3. **TX to the Vehicle bus:** Vehicle commands, preconditioning, track mode, nag killer echo
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

## Field Name Mapping (C++ → JSON → TypeScript)

The same field uses different names across the firmware, JSON wire protocol, and TypeScript UI layer. This table maps the correspondence for contributors:

| C++ (State)            | JSON Key           | TypeScript (boardState) |
| ---------------------- | ------------------ | ----------------------- |
| `fsdEnabled`           | `fsd`              | `fsd`                   |
| `fsdForceEnabled`      | `fsdForce`         | `fsdForce`              |
| `nagSuppress`          | `nag`              | `nag`                   |
| `isaChimeSuppress`     | `isaChime`         | `isaChime`              |
| `speedProfile`         | `sp`               | `profile`               |
| `profileOverride`      | `spPin`            | `profilePinned`         |
| `speedOffset`          | `offset`           | `offset`                |
| `offsetOverride`       | `offPin`           | `offsetPinned`          |
| `driveModeOverride`    | `driveMode`        | `driveMode`             |
| `currentDriveMode`     | `currentDriveMode` | `currentDriveMode`      |
| `eceR79Bypass`         | `eceR79`           | `eceR79`                |
| `regionCode`           | `regionCode`       | `regionCode`            |
| `hasRegion`            | `hasRegion`        | `hasRegion`             |
| `chineseGatewayLocked` | `cnLocked`         | `cnLocked`              |
| `rateLimitEnabled`     | `rateLimit`        | `rateLimit`             |
| `hasTpms`              | `hasTpms`          | `hasTpms`               |
| `tpmsPressure[0]`      | `fl`               | `tpmsPressureFL`        |
| `tpmsPressure[1]`      | `fr`               | `tpmsPressureFR`        |
| `tpmsPressure[2]`      | `rl`               | `tpmsPressureRL`        |
| `tpmsPressure[3]`      | `rr`               | `tpmsPressureRR`        |
| `tpmsTemp[0]`          | `tfl`              | `tpmsTempFL`            |
| `tpmsTemp[1]`          | `tfr`              | `tpmsTempFR`            |
| `tpmsTemp[2]`          | `trl`              | `tpmsTempRL`            |
| `tpmsTemp[3]`          | `trr`              | `tpmsTempRR`            |

## CRC-8/OPENSAFETY

Certain Tesla CAN frames use CRC-8 with polynomial 0x2F and per-ID XOR magic tables. For 0x229 (EPAS_harness), the CRC is stored in `data[0]` high nibble while the low nibble holds the mux counter. For 0x249 (DI_steer) and 0x370 (EPAS_sysStatus), the counter is at `data[1] & 0x0F`.

**Polynomial:** 0x2F (CRC-8/OPENSAFETY)
**Init:** 0x00
**Magic tables:** 16-entry per-ID XOR values indexed by `data[0] & 0x0F`

## TPMS Frame (CAN ID 0x219)

Tire pressure and temperature are decoded from 8 bytes:

| Byte | Field                   | Formula         |
| ---- | ----------------------- | --------------- |
| 0    | Front-left pressure     | raw × 0.025 bar |
| 1    | Front-right pressure    | raw × 0.025 bar |
| 2    | Rear-left pressure      | raw × 0.025 bar |
| 3    | Rear-right pressure     | raw × 0.025 bar |
| 4    | Front-left temperature  | raw − 40 °C     |
| 5    | Front-right temperature | raw − 40 °C     |
| 6    | Rear-left temperature   | raw − 40 °C     |
| 7    | Rear-right temperature  | raw − 40 °C     |

## Drive Mode Injection (CAN ID 0x334)

Drive mode override injects frames at 20 Hz to enforce the selected mode:

| `data[1]` | Mode               |
| --------- | ------------------ |
| 0         | Off (no injection) |
| 1         | Chill              |
| 2         | Standard           |
| 3         | Performance        |

The actual drive mode is read back from CAN ID 0x249 (DI_steer) for confirmation.

## Region Detection (CAN ID 0x398)

The low nibble of byte 2 in GTW_carConfig contains the region code:

| Code | Region        |
| ---- | ------------- |
| 0    | Unknown       |
| 1    | North America |
| 2    | Europe        |
| 3    | China         |
| 4    | Asia-Pacific  |
| 5    | Middle East   |

## ECE R79 Bypass

Applied in mux 1 of the FSD frame (CAN ID 1021). Clears bit 20 (`data[2] &= ~0x10`) to remove the European steering speed limit. Only active when `eceR79Bypass` is enabled AND `isEuropeanMarket(regionCode)` is true.

## Gateway Firmware Version (CAN ID 0x392)

Multiplexed frame carrying the gateway firmware version. Used by the FW Version Compatibility feature.

| Mux | Bytes | Field                     |
| --- | ----- | ------------------------- |
| 0   | 1–2   | Firmware year (e.g. 2024) |
| 0   | 3     | Release number (e.g. 44)  |
| 0   | 4     | Minor version             |
| 1   | 1–4   | Build number (uint32)     |

Mux ID is in byte[0] bits[3:0]. The firmware evaluates compatibility level:

| Level | Meaning                                               |
| ----- | ----------------------------------------------------- |
| 0     | UNKNOWN — not yet decoded                             |
| 1     | OK — firmware year ≥ 2024, fully compatible           |
| 2     | WARN — older firmware, may have reduced compatibility |
| 3     | FAIL — known incompatible firmware version            |

## Vehicle Config (CAN ID 0x398)

In addition to region detection, 0x398 is used to identify the vehicle platform and model year.

| Byte | Bits  | Field                     |
| ---- | ----- | ------------------------- |
| 1    | [7:4] | Platform ID               |
| 2    | [7:0] | Year offset (+ base year) |

Platform IDs:

| ID  | Vehicle    |
| --- | ---------- |
| 1   | Model 3    |
| 2   | Model Y    |
| 3   | Model S    |
| 4   | Model X    |
| 5   | Cybertruck |

## Ring Buffer Frame Distribution

All received CAN frames from MCP2515 buses are pushed into a 256-entry lock-free ring buffer (`canRingBuffer`). Multiple consumers can independently read from the ring using `RingConsumer` structs with sequence-based tracking. Overflow is detected per consumer.
