# Vehicle Features

TeslaCANModder modifies CAN bus frames to enable and control various Tesla vehicle features. All features are OFF by default and must be explicitly enabled.

## FSD (Full Self-Driving) Enable

Modifies the FSD mux frame (CAN ID 1021 or 1006) to activate FSD capability.

- **HW4:** Sets bits 38, 46, 60 in mux 0
- **HW3:** Sets bits 38, 46 in mux 0, writes speed profile
- **Legacy:** Sets bit 46 in mux 0 (CAN ID 1006)

**Requires:** FSD must be selected in the vehicle UI. The modification only takes effect when `isFSDSelectedInUI` is true.

```bash
fsd:on     # Enable FSD modification
fsd:off    # Disable FSD modification
```

## Nag Suppression

Suppresses the "hands on wheel" nag prompt by clearing bit 19 in the FSD mux frame (mux ID 1).

- **HW4:** Also sets bit 47
- **HW3/Legacy:** Clears bit 19 only

```bash
nag:on     # Enable nag suppression
nag:off    # Disable nag suppression
```

## Speed Profile

Controls the FSD speed profile (aggressiveness), mapped from follow distance stalk position:

| Profile | Description |
| -------- | ------------ |
| 0 | Most aggressive |
| 1 | Moderate-aggressive |
| 2 | Moderate-conservative |
| 3 | Most conservative |

```bash
profile:0      # Set to most aggressive
profile:auto   # Auto-track from follow distance stalk
```

In auto mode, the profile follows the stalk position. When pinned, it stays at the set value regardless of stalk changes.

## Speed Offset (HW3 only)

Applies a speed offset to the FSD mux frame (mux ID 2). Only available on HW3 vehicles.

```bash
offset:5       # Set speed offset to 5
offset:auto    # Auto-track from UI offset steps
```

## ISA Speed Chime Suppression (HW4 only)

Suppresses the ISA (Intelligent Speed Assistance) speed warning chime by modifying CAN ID 921.

```bash
isa-chime:on   # Suppress ISA chime
isa-chime:off  # Restore original chime
```

## Summon

Sends summon CAN frames via the 0x273 (UI_vehicleControl) frame on the Vehicle Control bus. Sends a 30-frame burst.

```bash
summon:forward   # Summon forward
summon:reverse   # Summon reverse  
summon:stop      # Stop summon
```

> Requires a cached 0x273 frame from the vehicle bus. If not yet received, Console shows "Waiting for 0x273 frame".

## Nag Killer (EPAS Torque Spoofing)

More aggressive than basic nag suppression — intercepts CAN ID 0x370 (EPAS torque sensor) and echoes it back with zeroed torque values. This eliminates the "hands on wheel" nag entirely by spoofing zero steering torque.

- Increments counter byte, zeros torque bytes 2–3
- Recalculates checksum (sum bytes 0–6 + 0x70 + 0x03)
- Setting persisted to NVS/EEPROM

```bash
nagkiller:on      # Enable EPAS torque spoofing
nagkiller:off     # Disable EPAS torque spoofing
nagkiller:toggle  # Toggle state
```

> **Warning:** This is more aggressive than `nag:on` and modifies a safety-related CAN frame. Use with caution.

## Battery Preconditioning

Triggers battery preconditioning for supercharging by injecting CAN ID 0x082 (UI_tripPlanning) on the Vehicle bus. Useful before arriving at a Supercharger to warm the battery for faster charging.

- Injects `data[0] = 0x05` to activate, `0x00` to deactivate
- Periodic injection at 500ms intervals while active
- Setting persisted to NVS/EEPROM

```bash
precondition:on      # Start battery preconditioning
precondition:off     # Stop battery preconditioning
precondition:toggle  # Toggle state
```

## Track Mode

Enables Track Mode by injecting CAN ID 0x313 (UI_trackModeSettings) on the Vehicle bus. Sends a 20-frame burst to activate.

- Sets byte[0] bits 1:0 = 0x01 to enable
- Sends 20 frames at 20ms intervals
- Setting persisted to NVS/EEPROM

```bash
trackmode:on      # Enable Track Mode
trackmode:off     # Disable Track Mode
trackmode:toggle  # Toggle state
```

## BMS Battery Telemetry

Reads real-time battery management system data from the CAN bus. The dashboard displays:

| Metric | CAN ID | Description |
| ------ | ------ | ----------- |
| Voltage | 0x132 | Pack voltage (raw × 0.01V) |
| Current | 0x132 | Pack current (signed, raw × 0.1A) |
| Power | — | Calculated (V × A × 0.001 kW) |
| State of Charge | 0x292 | SoC (10-bit, raw × 0.1%) |
| Temp Min/Max | 0x312 | Cell temperatures (byte − 40 °C) |
| Wh/km | 0x33A | Energy consumption (raw × 0.1 Wh/km) |

```bash
bms   # Query current BMS telemetry snapshot
```

## OTA Safety Check

Automatically pauses all CAN frame injection when an over-the-air (OTA) update is detected on the vehicle. Monitors CAN ID 0x318 (GTW_carState) for OTA status flags.

- TX is paused automatically when OTA is detected
- TX resumes when OTA completes
- No manual intervention needed — fully automatic

## Auto Hardware Detection

Reads CAN ID 0x398 (GTW_carConfig) to automatically detect whether the vehicle is HW3 or HW4. This eliminates the need to manually set the variant.

- Detected at boot from the first 0x398 frame
- Falls back to manually set variant if 0x398 is not received

## CAN Error Monitoring

Tracks CAN bus error flags and timestamps for diagnostic purposes. Error state is visible in the dashboard status panel.

## Vehicle Commands (3 CAN required)

These commands require a 3-CAN build and send frames on the Vehicle Control bus (Bus 1):

### Lock & Horn

- `lock` — Lock all doors
- `unlock` — Unlock all doors
- `lock:child` — Child lock
- `horn` — Sound horn

### Trunk & Frunk

- `frunk` / `frunk:open` — Open frunk
- `frunk:close` — Close frunk
- `trunk:open` — Open trunk
- `trunk:close` — Close trunk
- `glovebox` — Toggle glovebox

### Windows

- `vent:open` — Vent all windows
- `vent:close` — Close all windows

### Mirrors

- `mirror:fold` — Fold mirrors
- `mirror:unfold` — Unfold mirrors
- `mirror:heat` — Heat mirrors
- `mirror:autofold` — Auto-fold setting
- `mirror:dip` — Dip mirrors

### Lights

- `light:fog:front` / `light:fog:rear` — Fog lights
- `light:highbeam:auto` — Auto highbeam
- `light:ambient` — Ambient lighting
- `light:dome:on` / `light:dome:off` / `light:dome:auto` — Dome light

### Sentry

- `sentry:on` — Enable sentry mode
- `sentry:off` — Disable sentry mode

### Climate

- `climate:keep` — Keep climate on
- `climate:off` — Turn climate off

### Charge

- `charge:start` — Start charging
- `charge:stop` — Stop charging
- `chargeport` — Open charge port

### Drive Config

- `pedal:standard` / `pedal:chill` / `pedal:sport` — Pedal response
- `regen:off` / `regen:low` / `regen:standard` / `regen:max` — Regen braking
- `stop:creep` / `stop:roll` / `stop:hold` — Stopping mode

### Wiper

- `wiper:off` / `wiper:1` / `wiper:2` / `wiper:3` — Wiper speed (off, low, medium, high)

### Seat Heating

Controls heated seats for all five positions via CAN ID 0x273. Each seat has levels 0–3 (off, low, med, high).

- `seat:fl:0` – `seat:fl:3` — Front-left
- `seat:fr:0` – `seat:fr:3` — Front-right
- `seat:rl:0` – `seat:rl:3` — Rear-left
- `seat:rr:0` – `seat:rr:3` — Rear-right
- `seat:rc:0` – `seat:rc:3` — Rear-center

### Display Brightness

Sets main display brightness via CAN ID 0x273 byte 4 (0–127, factor 0.5).

- `maindisplay:N` — Set brightness (0–127)

### Power Control

Controls vehicle power states via CAN ID 0x273.

- `power:acc:on` / `power:acc:off` — Accessory power (bit 0)
- `power:off` — Power off (bit 31)
- `power:ready` — Drive-ready state (bit 62)

## Feature Availability by Variant

| Feature | HW4 | HW3 | Legacy |
| -------- | ---- | ---- | ------- |
| FSD Enable | ✅ | ✅ | ✅ |
| Nag Suppress | ✅ | ✅ | ✅ |
| Speed Profile | ✅ | ✅ | ✅ (limited) |
| Speed Offset | ❌ | ✅ | ❌ |
| ISA Chime | ✅ | ❌ | ❌ |
| Nag Killer (EPAS) | ✅ | ✅ | ❌ |
| Preconditioning | ✅ | ✅ | ❌ |
| Track Mode | ✅ | ✅ | ❌ |
| BMS Telemetry | ✅ | ✅ | ❌ |
| OTA Safety | ✅ | ✅ | ❌ |
| Auto HW Detect | ✅ | ✅ | ❌ |
| Summon | ✅ | ✅ | ❌ |
| Vehicle Commands | ✅ | ✅ | ❌ |
| Seat Heating | ✅ | ✅ | ❌ |
| Wiper Control | ✅ | ✅ | ❌ |
| Display Brightness | ✅ | ✅ | ❌ |
| Power Control | ✅ | ✅ | ❌ |
| CAN Error Monitor | ✅ | ✅ | ✅ |

## NVS Persistence

All toggle states (FSD, nag, ISA chime, nag killer, precondition, track mode, profile, offset, variant) are saved to NVS flash (ESP32) or EEPROM (Arduino) and persist across reboots.
