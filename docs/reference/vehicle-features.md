---
title: Vehicle Features
title_tr: Araç Özellikleri
description: All controllable vehicle features and their CAN commands
category: reference
folder: reference
tags: [vehicle, features, tesla]
order: 7
icon: 🚗
---

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
fsd:force:on   # Override UI FSD selection gate
fsd:force:off  # Require UI FSD selection gate
```

When `fsd:force:on` is enabled, mux-0 FSD edits are applied even if the UI FSD-selected bit is not currently set.

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

| Profile | Description           |
| ------- | --------------------- |
| 0       | Most aggressive       |
| 1       | Moderate-aggressive   |
| 2       | Moderate-conservative |
| 3       | Most conservative     |

```bash
profile:0      # Set to most aggressive
profile:auto   # Auto-track from follow distance stalk
```

In auto mode, the profile follows the stalk position. When pinned, it stays at the set value regardless of stalk changes.

## Speed Offset

Applies a speed offset to the FSD mux frame (mux ID 2). Auto-routes by detected hardware:

- **HW3:** Sets offset in mux-2 (range 0–100). `auto` tracks UI offset steps.
- **HW4:** Overrides mux-2 `data[1]` low 6 bits (range 0–63). `off`/`0` disables.
- **Legacy:** Not supported.

```bash
offset:5       # Set speed offset to 5
offset:auto    # Auto-track from UI offset steps (HW3)
offset:off     # Disable offset override
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

## DAS Drive (Gamepad CAN Injection)

Openpilot-style autopilot CAN injection on `BUS_CHASSIS` (X179 pins 13–14). Emits three Tesla DAS frames at the same rates the AP computer would:

| ID    | Name                  | Rate  | Purpose                 |
| ----- | --------------------- | ----- | ----------------------- |
| 0x2B9 | `DAS_control`         | 25 Hz | Longitudinal accel/jerk |
| 0x488 | `DAS_steeringControl` | 50 Hz | Steering angle request  |
| 0x27D | `APS_eacMonitor`      | 10 Hz | EPAS steer-allow gate   |

Safety envelope (mirrors `opendbc/car/tesla` CarControllerParams):

- Accel: −3.48 … +2.0 m/s²
- Jerk: ±4.9 m/s³ (ACC faults at ±5.0)
- Angle rate: ≤ 5° per 20 ms steering frame (EPS faults at 12)
- Speed cap: 1 … 200 km/h (NVS-persisted, default 25)
- Hardware variant aware: HW3/LEGACY → ANGLE_CONTROL (1), HW4 → LANE_KEEP_ASSIST (2)
- Dead-man timer: 150 ms without `dasSetControl()` auto-cancels
- Standstill brake-hold: −0.4 m/s² when no input near zero speed
- Disable emits 5 cancel frames before going silent

```bash
drive:on            # Enable DAS drive (gamepad takes over actuators)
drive:off           # Disable + cancel burst
drive:speed:N       # User speed limit km/h (1..current cap), persisted
drive:cap:N         # Hard safety cap km/h (1..200), persisted
```

> ⚠️ Bench rig or jack stands only. See [DAS Drive guide](../guides/das-drive.md) for full safety model and bench checklist.

## Gamepad Input (BLE HID)

Pairs a BLE HID controller (Xbox/PS, service UUID `0x1812`) and routes its analog axes into DAS Drive plus its 16 buttons into bound serial commands. Default bindings: `A → drive:on`, `B → drive:off`, `X → horn`, button hold (≥500 ms) can be bound separately. Pairing, axis tuning, and button bindings persist to NVS namespaces `tcm_gpad` / `tcm_gbnd`. Battery % and RSSI are captured when the device exposes them.

## Nag Killer (EPAS Torque Spoofing)

More aggressive than basic nag suppression — intercepts CAN ID 0x370 (EPAS torque sensor) and echoes it back with zeroed torque values. This eliminates the "hands on wheel" nag entirely by spoofing zero steering torque.

- Increments counter byte, zeros torque bytes 2–3
- Recalculates checksum (sum bytes 0–6 + 0x70 + 0x03)
- Setting persisted to NVS/EEPROM

```bash
nag:killer:on      # Enable EPAS torque spoofing
nag:killer:off     # Disable EPAS torque spoofing
nag:killer:mode:legacy  # Always echo when nag killer is enabled
nag:killer:mode:safe    # Echo only when DAS hands-on is requested
```

### Safe Mode

Safe mode uses `DAS_autopilotHandsOnState` from CAN ID `0x39B` to decide whether to emit spoof frames.

- Lower background spoof traffic in normal driving.
- Keeps legacy behavior available for compatibility.
- Current mode is exposed as `nagKillerMode` in status output.

> **Warning:** This is more aggressive than `nag:on` and modifies a safety-related CAN frame. Use with caution.

## Battery Preconditioning

Triggers battery preconditioning for supercharging by injecting CAN ID 0x082 (UI_tripPlanning) on the Vehicle bus. Useful before arriving at a Supercharger to warm the battery for faster charging.

- Injects `data[0] = 0x05` to activate, `0x00` to deactivate
- Periodic injection at 500ms intervals while active
- Setting persisted to NVS/EEPROM

```bash
precondition:on      # Start battery preconditioning
precondition:off     # Stop battery preconditioning
```

## Track Mode

Enables Track Mode by injecting CAN ID 0x313 (UI_trackModeSettings) on the Vehicle bus. Sends a 20-frame burst to activate.

- Sets byte[0] bits 1:0 = 0x01 to enable
- Sends 20 frames at 20ms intervals
- Setting persisted to NVS/EEPROM

```bash
trackmode:on      # Enable Track Mode
trackmode:off     # Disable Track Mode
```

## BMS Battery Telemetry

Reads real-time battery management system data from the CAN bus. The dashboard displays:

| Metric          | CAN ID | Description                          |
| --------------- | ------ | ------------------------------------ |
| Voltage         | 0x132  | Pack voltage (raw × 0.01V)           |
| Current         | 0x132  | Pack current (signed, raw × 0.1A)    |
| Power           | —      | Calculated (V × A × 0.001 kW)        |
| State of Charge | 0x292  | SoC (10-bit, raw × 0.1%)             |
| Temp Min/Max    | 0x312  | Cell temperatures (byte − 40 °C)     |
| Wh/km           | 0x33A  | Energy consumption (raw × 0.1 Wh/km) |

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

## Vehicle Platform Identity

Provides a unified vehicle identity hierarchy: **Model → Hardware Generation → Software Version**. This system replaces ad-hoc variant checks with a structured platform object that drives feature availability and protocol selection.

### Model Detection

Decoded from CAN ID 0x398 (GTW_carConfig):

| Value | Model      |
| ----- | ---------- |
| 0     | UNKNOWN    |
| 1     | Model 3    |
| 2     | Model Y    |
| 3     | Model S    |
| 4     | Model X    |
| 5     | Cybertruck |

### Hardware Generation

Mapped from the existing `Variant` enum:

| Value | Generation | Description                   |
| ----- | ---------- | ----------------------------- |
| 0     | HW_UNKNOWN | Not yet detected              |
| 1     | HW_LEGACY  | Pre-HW3 (RP2040-based builds) |
| 2     | HW_3       | Autopilot HW 3.0              |
| 3     | HW_4       | Autopilot HW 4.0              |

### Software Version

Parsed from CAN ID 0x392 (GTW_fwVersion) as `YYYY.WW.release[.patch]`. The firmware extracts year, week, release, and patch fields.

### FSD Protocol Detection

The FSD protocol version is automatically determined from hardware generation and software version:

| Protocol | Condition                            |
| -------- | ------------------------------------ |
| V12      | HW3 or Legacy (any software version) |
| V14      | HW4 with software ≥ 2026.2.9         |
| V13      | HW4 with software < 2026.2.9         |

### Software Compatibility

Checks the detected software version against known-good ranges:

| Level    | Meaning                          |
| -------- | -------------------------------- |
| OK       | Within tested range              |
| UNTESTED | Newer than latest tested version |
| TOO_OLD  | Older than minimum supported     |
| TOO_NEW  | Reserved                         |

### Platform Capabilities

Each resolved platform exposes capability flags:

- `hasMux0Fsd` — FSD mux-0 frame available
- `hasMux1Nag` — Nag suppression mux-1 available
- `hasMux2Speed` — Speed profile mux-2 available
- `hasIsaChime` — ISA chime suppression supported (HW4 only)
- `hasSummon` — Summon supported (HW4/HW3)
- `hasNagKiller` — EPAS torque spoofing supported (HW4/HW3)

## MCP2515 CAN Bus Health Check

At boot, the firmware probes each MCP2515 SPI CAN controller to verify hardware connectivity. This detects missing or malfunctioning CAN transceivers before any vehicle communication begins.

### Bus Mapping

| Index | Bus     | Function                    |
| ----- | ------- | --------------------------- |
| 0     | Chassis | EPAS, suspension (via X179) |
| 1     | Vehicle | Powertrain, drive systems   |
| 2     | Body    | Comfort, lighting, HVAC     |

### Health Report

Each bus reports:

- **configured** — Bus is enabled in the current build
- **detected** — MCP2515 responded to SPI probe
- **receiving** — Frames have been received (chassis bus checks `chassisOnline`)

The health report is included in `boot`, `status`, and `platform` command responses as a `canHealth` object. Missing buses trigger a serial warning at startup.

## GTW Autopilot Tier Diagnostics

Reads GTW autopilot entitlement tier from CAN ID `0x7FF` mux `2` (`byte[5] bits 4:2`) and exposes it in status as `gtwAutopilotTier`.

- `0`: NONE
- `1`: HIGHWAY
- `2`: ENHANCED
- `3`: SELF_DRIVING
- `4`: BASIC
- `-1`: unknown/not seen yet

This is read-only diagnostics and helps confirm whether the vehicle reports an autopilot tier compatible with your expected feature set.

## CAN Error Monitoring

Tracks CAN bus error flags and timestamps for diagnostic purposes. Error state is visible in the dashboard status panel.

## Runtime MCP2515 Clock Profile

Switches MCP2515 bitrate timing profile at runtime and reapplies filters after reinit.

## Enhanced BMS Telemetry

In addition to the basic BMS telemetry (voltage, current, SoC, temps, Wh/km), the firmware now decodes enhanced battery data from additional CAN frames:

| Metric              | CAN ID | Mux | Description                                              |
| ------------------- | ------ | --- | -------------------------------------------------------- |
| Nominal Full Pack   | 0x352  | 0   | Full pack capacity (bytes 2–3, raw × 0.02 kWh)           |
| Nominal Remaining   | 0x352  | 0   | Remaining energy (bytes 4–5, raw × 0.02 kWh)             |
| Ideal Remaining     | 0x352  | 0   | Ideal remaining energy (bytes 6–7, raw × 0.02 kWh)       |
| Cell Voltage Max    | 0x332  | 1   | Highest cell voltage (12-bit, raw × 0.002 V)             |
| Cell Voltage Min    | 0x332  | 1   | Lowest cell voltage (12-bit, raw × 0.002 V)              |
| Max Regen Power     | 0x252  | —   | Maximum regen power limit (bytes 0–1, raw × 0.01 kW)     |
| Max Discharge Power | 0x252  | —   | Maximum discharge power limit (bytes 2–3, raw × 0.01 kW) |

Enhanced data appears automatically once these CAN frames are received. The `hasEnhancedBms` flag in the status payload indicates availability.

## Steering Mode Monitoring

Reads the current EPAS steering tune mode from CAN ID 0x370 (EPAS_sysStatus), byte[0] bits [7:4]:

| Value | Mode        |
| ----- | ----------- |
| 0     | FAIL_SAFE   |
| 1     | COMFORT     |
| 2     | STANDARD    |
| 3     | SPORT       |
| 4     | RWD_COMFORT |
| 5     | RWD_SPORT   |
| 6     | RWD_SPORT+  |

The steering mode is extracted from the same 0x370 frame used by the Nag Killer feature. The `hasSteeringMode` flag indicates when a steering mode value has been received.

> **Note:** This is a read-only monitoring feature. Changing steering mode requires writing to CAN ID 0x101 on the **Chassis CAN bus**, which is not accessible from the standard X179 connector.

```bash
canclock:auto
canclock:8
canclock:12
canclock:16
canclock:20
```

- `8`, `16`, and `20` map directly to MCP2515 clock profiles.
- `12` is accepted as a compatibility request and resolves via safe fallback to a supported profile.
- Status exposes `canClockReqMHz` (requested) and `canClockMHz` (active).

## Vehicle Commands (3 CAN required)

## Ban Shield (Experimental Telemetry Monitoring)

Monitors incoming CAN telemetry for patterns that might indicate Tesla anti-modification detection attempts.

```bash
banshield:on
banshield:off
```

**Status fields:**

- `banShield` — Monitoring enabled (0=off, 1=on)
- `banThreat` — Current threat level (0=none, 1-5=escalating)
- `banDetectCount` — Cumulative ban threat events

**Detection patterns:**

- Negative UDS responses (0x7FE)
- Security access requests (UDS 0x27XX)
- Unusual OTA/telemetry frame burst rate

The threat level automatically decreases when no threats are detected for 10+ seconds.

## Drive Mode Override (Ghost Mode)

Overrides the vehicle's drive mode by continuously injecting CAN ID 0x334 at 20 Hz (50ms interval). This allows forcing Chill, Standard, or Performance mode regardless of the vehicle's software settings.

```bash
drivemode:off          # Disable override
drivemode:chill        # Force Chill mode
drivemode:standard     # Force Standard mode
drivemode:performance  # Force Performance mode
```

**Status fields:**

- `driveMode` — Current override setting (0=off, 1=chill, 2=standard, 3=performance)
- `currentDriveMode` — Readback from vehicle DI_steer frame

The override is persisted to NVS and survives reboots. The vehicle's actual drive mode is read back from CAN for confirmation.

## ECE R79 Bypass

European vehicles enforcing ECE R79 have a steering speed limit that restricts autopilot lane changes above certain speeds. This feature bypasses that restriction.

```bash
ecer79:on    # Enable ECE R79 bypass
ecer79:off   # Disable ECE R79 bypass
```

**How it works:**

- Clears bit 20 in mux 1 of the FSD frame (CAN ID 1021)
- Only active when the vehicle is detected as European market (region code 2)
- Region is auto-detected from CAN ID 0x398 (GTW_carConfig)

**Status fields:**

- `eceR79` — Bypass enabled (0=off, 1=on)
- `regionCode` — Detected region (0=unknown, 1=NA, 2=EU, 3=CN, 4=APAC, 5=ME)
- `hasRegion` — Whether region has been detected
- `cnLocked` — Chinese gateway lock detected

> ⚠️ This modifies safety-related steering behavior. Use with caution.

## Region Detection

Automatically detects the vehicle's market region from CAN ID 0x398 (GTW_carConfig). The low nibble of byte 0 contains the region code:

| Code | Region              |
| ---- | ------------------- |
| 0    | Unknown             |
| 1    | North America (NA)  |
| 2    | Europe (EU)         |
| 3    | China (CN)          |
| 4    | Asia-Pacific (APAC) |
| 5    | Middle East (ME)    |

Chinese market vehicles may have gateway restrictions that prevent certain CAN modifications.

## Turn Signals (3-Blink Lane Change)

Triggers a 3-blink lane change signal by injecting CAN ID 0x3F5 (VCFRONT_vehicleLights). Sends a burst of 3 frames at 100ms intervals.

**Byte[0] bits[1:0]:**

| Value | Signal |
| ----- | ------ |
| 0     | Off    |
| 1     | Left   |
| 2     | Right  |
| 3     | Hazard |

```bash
turn:left3    # 3-blink left
turn:right3   # 3-blink right
turn:hazard   # Hazard lights
turn:off      # Cancel
```

## Seatbelt Emulation

Suppresses rear seatbelt warnings by periodically injecting CAN ID 0x3F3 (VCRIGHT_seatbeltStatus) with all 3 rear seats reported as buckled.

- Injects `data[0] = 0x07` (bits 2:0 = all buckled) every 500ms
- Automatically pauses during OTA updates
- Setting persisted to NVS

```bash
seatbelt:on    # Enable rear seatbelt emulation
seatbelt:off   # Disable rear seatbelt emulation
```

## Air Recirculation Control

Toggles cabin air recirculation by injecting CAN ID 0x2AA. Uses the last-received climate frame as a base and sends a burst of 20 frames at 20ms intervals.

```bash
airecirc:on    # Switch to recirculation mode
airecirc:off   # Switch to fresh air mode
```

> This is a momentary action and is not persisted across reboots.

## Wiper Speed Persistence

Tesla resets wiper speed to auto on each drive cycle. This feature saves the last-set wiper speed to NVS and automatically re-injects it on boot or wake.

- Hooks into the existing wiper control module
- Saves speed when `wiperpersist:on` is active and a wiper command is sent
- Restores saved speed when `hasCtrl` becomes true (vehicle bus active)

```bash
wiperpersist:on    # Enable wiper speed persistence
wiperpersist:off   # Disable wiper speed persistence
```

## Mirror Auto-Fold on Lock

Automatically folds side mirrors when the vehicle locks and unfolds them when it unlocks. Monitors vehicle lock state transitions from CAN bus frames.

- Uses the existing mirror fold/unfold control module
- Only acts on state transitions (lock → fold, unlock → unfold)
- Setting persisted to NVS

```bash
mirror:autofold:on    # Enable mirror auto-fold
mirror:autofold:off   # Disable mirror auto-fold
```

## Powertrain Telemetry

Read-only decode of motor and drivetrain CAN signals for performance monitoring:

| Signal            | CAN ID | Decoding                                   |
| ----------------- | ------ | ------------------------------------------ |
| Vehicle Speed     | 0x257  | Byte[2:3] signed int16, ÷100 → km/h        |
| Gear State        | 0x118  | Byte[0] bits[3:1] → 1=P, 2=R, 3=N, 4=D     |
| Accelerator Pedal | 0x118  | Byte[1] → 0–100%                           |
| Steering Angle    | 0x129  | Byte[0:1] signed int16, ÷10 → degrees      |
| Rear Motor RPM    | 0x106  | Byte[4:5] signed int16                     |
| Front Motor RPM   | 0x115  | Byte[4:5] signed int16 (0 if single motor) |

```bash
powertrain   # Query current powertrain telemetry
```

Data automatically populates in the web and mobile dashboards when frames are received.

## CAN Simulation Mode

Generates synthetic CAN frames for testing without a real vehicle. When enabled, periodically injects test data into the decode pipeline at 200ms intervals.

**Simulated signals:**

| Signal             | Value          |
| ------------------ | -------------- |
| BMS Voltage        | ~375V          |
| BMS Current        | ~5A            |
| BMS SoC            | 70%            |
| TPMS (all 4 tires) | 2.5 bar / 25°C |
| Vehicle Speed      | 60–61 km/h     |
| Gear               | D (Drive)      |
| Accelerator Pedal  | 15%            |

Frames are **never transmitted on the physical CAN bus** — they only feed the internal `handleMessage()` decode pipeline. Safe for bench testing.

```bash
simu:start   # Start CAN simulation
simu:stop    # Stop CAN simulation
```

## TPMS (Tire Pressure Monitoring)

Decodes real-time tire pressure and temperature from CAN ID 0x219:

| Byte | Field          | Formula         |
| ---- | -------------- | --------------- |
| 0    | FL pressure    | raw × 0.025 bar |
| 1    | FR pressure    | raw × 0.025 bar |
| 2    | RL pressure    | raw × 0.025 bar |
| 3    | RR pressure    | raw × 0.025 bar |
| 4    | FL temperature | raw − 40 °C     |
| 5    | FR temperature | raw − 40 °C     |
| 6    | RL temperature | raw − 40 °C     |
| 7    | RR temperature | raw − 40 °C     |

```bash
tpms   # Query current TPMS data
```

Data is displayed in the web and mobile dashboards when available. The `hasTpms` flag indicates when TPMS data has been received.

## Single-Shot TX Mode

Configures the MCP2515 CAN controllers to use one-shot transmission mode, disabling automatic retransmission on TX errors. Uses the CANCTRL OSM (One-Shot Mode) bit.

- Applied to all three MCP2515 buses simultaneously
- Useful on noisy buses where retransmits cause frame storms
- Setting persisted to NVS and restored on boot

```bash
singleshot:on    # Enable one-shot TX mode
singleshot:off   # Disable one-shot TX (normal retransmit)
```

## Firmware Version Compatibility

Read-only decode of the gateway firmware version from CAN ID 0x392 (multiplexed). Evaluates whether the detected firmware is compatible with the current mod features.

| Compat Level | Meaning                                        |
| ------------ | ---------------------------------------------- |
| UNKNOWN      | Not yet decoded                                |
| OK           | Firmware ≥ 2024, fully compatible              |
| WARN         | Older firmware, may have reduced compatibility |
| FAIL         | Known incompatible firmware                    |

```bash
fwcompat    # Query firmware version and compatibility
```

The web and mobile dashboards show firmware version and compatibility status with color coding (green=OK, yellow=WARN, red=FAIL).

## MQTT Telemetry Bridge

Publishes JSON telemetry snapshots to a configurable MQTT broker over WiFi. Requires `BOARD_ENABLE_WIFI` to be active.

- Configurable broker host, port, and publish interval
- All MQTT settings persisted to NVS
- Auto-reconnects on connection loss

```bash
mqtt:on                    # Enable MQTT publishing
mqtt:off                   # Disable MQTT publishing
mqtt:broker:192.168.1.10   # Set broker host (1–63 chars)
mqtt:port:1883             # Set broker port (default 1883)
mqtt:interval:2000         # Set publish interval in ms (100–60000)
```

**Status fields:**

- `mqtt` — Bridge enabled (0=off, 1=on)
- `mqttConnected` — Currently connected to broker

## Vehicle Identification

Automatically identifies the vehicle model and year from CAN ID 0x398 (GTW_carConfig). Decoded platform ID maps to vehicle model.

| Model ID | Vehicle    |
| -------- | ---------- |
| 0        | Unknown    |
| 1        | Model 3    |
| 2        | Model Y    |
| 3        | Model S    |
| 4        | Model X    |
| 5        | Cybertruck |

```bash
vehicle    # Query vehicle model and year
```

Vehicle capabilities (FSD support, motor configuration) are inferred from the model ID.

## Ring Buffer Frame Distribution

All received CAN frames are pushed into a 256-entry lock-free ring buffer with monotonic sequence numbers. Multiple consumers (serial output, MQTT, WiFi streaming) independently track their read position. Overflow is detected per consumer.

## Debug Log Ring Buffer

A 256-entry circular buffer captures timestamped debug events from all firmware subsystems. Useful for diagnosing issues without a serial connection.

```bash
log    # Dump ring buffer contents
```

## Vehicle Commands ( 3 CAN required )

These commands require a build with `_vehicle` (e.g. `esp32_wifi_ble_chassis_vehicle_body_8mhz`) and send frames on the Vehicle bus:

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

| Feature                | HW4       | HW3        | Legacy       |
| ---------------------- | --------- | ---------- | ------------ |
| FSD Enable             | ✅        | ✅         | ✅           |
| Nag Suppress           | ✅        | ✅         | ✅           |
| Speed Profile          | ✅        | ✅         | ✅ (limited) |
| Speed Offset           | ✅ (0–63) | ✅ (0–100) | ❌           |
| ISA Chime              | ✅        | ❌         | ❌           |
| Nag Killer (EPAS)      | ✅        | ✅         | ❌           |
| Preconditioning        | ✅        | ✅         | ❌           |
| Track Mode             | ✅        | ✅         | ❌           |
| BMS Telemetry          | ✅        | ✅         | ❌           |
| OTA Safety             | ✅        | ✅         | ❌           |
| Auto HW Detect         | ✅        | ✅         | ❌           |
| Summon                 | ✅        | ✅         | ❌           |
| Vehicle Commands       | ✅        | ✅         | ❌           |
| Seat Heating           | ✅        | ✅         | ❌           |
| Wiper Control          | ✅        | ✅         | ❌           |
| Display Brightness     | ✅        | ✅         | ❌           |
| Power Control          | ✅        | ✅         | ❌           |
| CAN Error Monitor      | ✅        | ✅         | ✅           |
| Drive Mode Override    | ✅        | ✅         | ❌           |
| ECE R79 Bypass         | ✅        | ✅         | ❌           |
| TPMS Monitoring        | ✅        | ✅         | ❌           |
| Region Detection       | ✅        | ✅         | ❌           |
| Rate Limiting          | ✅        | ✅         | ✅           |
| Single-Shot TX         | ✅        | ✅         | ✅           |
| FW Version Compat      | ✅        | ✅         | ❌           |
| MQTT Bridge            | ✅        | ✅         | ❌           |
| Vehicle Identification | ✅        | ✅         | ❌           |
| Ring Buffer            | ✅        | ✅         | ❌           |

## NVS Persistence

All toggle states (FSD, nag, ISA chime, nag killer, precondition, track mode, profile, offset, variant, drive mode, ECE R79, single-shot TX, MQTT settings) are saved to NVS flash (ESP32) or EEPROM (Arduino) and persist across reboots. NVS version is `0x0C`.
