---
title: Command Reference
title_tr: Komut Referansı
description: Complete list of serial, BLE, and WiFi commands
category: reference
folder: reference
tags: [commands, serial, ble, wifi]
order: 4
icon: ⌨️
---

# Command Reference

All commands work over USB Serial, BLE (Nordic UART), and WiFi REST API. Commands are case-sensitive, lowercase, colon-separated.

## System Commands

| Command  | Description                                               |
| -------- | --------------------------------------------------------- |
| `ping`   | Health check — returns `{"t":"pong","v":1}`               |
| `status` | Full board state with all features, hardware info, uptime |

## Status Queries

| Command           | Description                                                           |
| ----------------- | --------------------------------------------------------------------- |
| `status:live:on`  | Enable the high-rate periodic status stream (250 ms interval)         |
| `status:live:off` | Disable the high-rate status stream and return to the normal interval |
| `status:live`     | Query whether high-rate status streaming is enabled                   |
| `status:compact`  | Return a compact snapshot with meta, connectivity, state, and CAN     |
| `status:meta`     | Return only the hardware, transport, and uptime metadata block        |
| `status:state`    | Return only the mutable control-state block                           |
| `status:features` | Return only the advertised feature-capability block                   |
| `status:can`      | Return only CAN clock and per-bus health information                  |

Split status commands use the same message sections as the full `boot` and `status` payloads:

- `status:meta` → `{"t":"status_meta",...}`
- `status:state` → `{"t":"status_state","state":{...}}`
- `status:features` → `{"t":"status_features","features":{...}}`
- `status:can` → `{"t":"status_can","clock":{...},"health":{...}}`
- `status:compact` → `{"t":"status_compact",...}`
- `status:live` → `{"t":"statusLive","on":0|1,"intervalMs":250}`

## MCP2515 Clock Profile

| Command         | Description                                                               |
| --------------- | ------------------------------------------------------------------------- |
| `canclock:auto` | Auto-detect best supported MCP2515 crystal profile (8/16/20 MHz fallback) |
| `canclock:8`    | Prefer 8 MHz profile with safe fallback                                   |
| `canclock:12`   | Compatibility request; resolves to nearest supported profile via fallback |
| `canclock:16`   | Prefer 16 MHz profile with safe fallback                                  |
| `canclock:20`   | Prefer 20 MHz profile with safe fallback                                  |

Status payload includes:

- `canClockReqMHz` — requested profile (`0` means auto)
- `canClockMHz` — active profile after fallback

## Ban Shield (Experimental Telemetry Monitoring)

| Command         | Description                                          |
| --------------- | ---------------------------------------------------- |
| `banshield:on`  | Enable ban threat detection and telemetry monitoring |
| `banshield:off` | Disable ban threat monitoring                        |

Status payload includes:

- `banShield` — monitoring enabled (0=off, 1=on)
- `banThreat` — current threat level (0=none, 1-5=escalating)
- `banDetectCount` — cumulative ban threat events detected

Ban Shield monitors for telemetry patterns that might indicate Tesla anti-modification detection, such as:

- UDS negative responses (0x7FE frames)
- Security access requests (0x27XX UDS services)
- Unusual OTA/telemetry frame bursts

Threat level automatically decreases when no patterns detected for 10+ seconds.

## GTW Shield (Snapshot-Based Frame Defense)

Active defense layer for the gateway 0x7FF frame. Snapshots a known-good frame, then re-injects it whenever a foreign payload appears.

| Command            | Description                                    |
| ------------------ | ---------------------------------------------- |
| `gtwshield:arm`    | Activate snapshot-based frame defense on 0x7FF |
| `gtwshield:disarm` | Disable defense                                |
| `gtwshield:reset`  | Clear snapshot and block counter               |

Status payload includes:

- `gtwShieldArmed` — defense enabled (0=off, 1=on)
- `gtwShieldBlocks` — count of foreign frames replaced since arm

## AP Injection Gate

Safety gate for write/injection commands. Gate opens only when AP is inactive and either Park or Summon is active.

| Command         | Description                             |
| --------------- | --------------------------------------- |
| `apgate:on`     | Enable AP injection gate enforcement    |
| `apgate:off`    | Disable AP injection gate enforcement   |
| `apgate:status` | Query AP gate status and open-condition |

## Enhanced Autopilot (Hidden AP Setting)

- `eap:on` — Enable enhanced autopilot bit (unlock EAP/Summon path)
- `eap:off` — Disable enhanced autopilot bit

Status payload includes:

- `enhancedAutopilot` — hidden AP setting state (0=off, 1=on)

## Driver Assist Toggles

| Command          | Description                        |
| ---------------- | ---------------------------------- |
| `lhd:on`         | Left-hand drive mode               |
| `lhd:off`        | Right-hand drive mode              |
| `apfirst:on`     | Enable AP-First injection gate     |
| `apfirst:off`    | Disable AP-First gate              |
| `lanegraph:on`   | Enable lane graph visualization    |
| `lanegraph:off`  | Disable lane graph visualization   |
| `assist-dev:on`  | Enable developer mode              |
| `assist-dev:off` | Disable developer mode             |
| `assist-nav:on`  | Enable drive-on-navigation assist  |
| `assist-nav:off` | Disable drive-on-navigation assist |
| `assist-hof:on`  | Enable hands-off mode              |
| `assist-hof:off` | Disable hands-off mode             |
| `assist-tel:on`  | Enable trip telemetry              |
| `assist-tel:off` | Disable trip telemetry             |

## DAS Drive (Gamepad CAN Injection)

Manual remote-control of the car's actuators by injecting openpilot-protocol
DAS frames (`DAS_control 0x2B9 @25 Hz`, `DAS_steeringControl 0x488 @50 Hz`,
`APS_eacMonitor 0x27D @10 Hz`) on the chassis CAN bus. Gamepad sticks become
steering and pedals. **Not** Autopilot/FSD — there is no perception or
planning. See [DAS Drive guide](../guides/das-drive.md) for safety model.

| Command         | Description                                                                  |
| --------------- | ---------------------------------------------------------------------------- |
| `drive:on`      | Enable DAS drive (gamepad takes over actuators)                              |
| `drive:off`     | Disable DAS drive and emit a 5-frame cancel burst                            |
| `drive:speed:N` | User speed limit in km/h (1..current cap), persisted to NVS                  |
| `drive:cap:N`   | Hard safety cap in km/h (1..200), persisted to NVS                           |
| `drive:status`  | Returns drive telemetry (enabled, active, steerDeg, accel range, speed, cap) |

Status payload includes:

- `dasDriveEnabled` — drive mode toggle
- `dasSpeedLimitKph` — current user speed limit
- `dasSpeedCapKph` — current runtime safety cap
- `dasSpeedCapMaxKph` — absolute compile-time ceiling (200)

## Gamepad (BLE HID)

Pair a BLE HID gamepad and route its sticks into [DAS Drive](#das-drive-gamepad-can-injection)
plus its 16 buttons into bound serial commands. Pairing, button bindings
(tap + hold) and per-axis tuning all persist to NVS namespaces `tcm_gpad`
/ `tcm_gbnd`. See [vehicle features](./vehicle-features.md#gamepad-input-ble-hid).

| Command                                | Description                                               |
| -------------------------------------- | --------------------------------------------------------- |
| `gamepad:scan`                         | Start a 6-second BLE HID scan                             |
| `gamepad:rescan`                       | Alias for `gamepad:scan`                                  |
| `gamepad:pair:<addr>`                  | Pair with the scanned MAC address (`AA:BB:CC:DD:EE:FF`)   |
| `gamepad:unpair`                       | Forget the paired device                                  |
| `gamepad:on`                           | Enable gamepad input (saved to NVS)                       |
| `gamepad:off`                          | Disable gamepad input (saved to NVS)                      |
| `gamepad:status`                       | Emit a JSON status (battery, RSSI, axes, mode)            |
| `gamepad:cancel`                       | Send a one-shot DAS cancel burst                          |
| `das:arm`                              | Arm DAS safety gate before injection (pre-drive check)    |
| `das:disarm`                           | Disarm DAS safety gate and block all injection            |
| `das:status`                           | Query DAS gate status and open-condition                  |
| `gamepad:bind:<n>:<cmd>`               | Bind tap of button `n` (0..15) to a serial command        |
| `gamepad:hold:<n>:<cmd>`               | Bind ≥500 ms long-press of button `n` to a serial command |
| `gamepad:axis:<n>:<dz\|expo\|inv>:<v>` | Per-axis tuning, axes 0..5                                |

Status payload (`gamepad` field) includes:

- `enabled` / `connected` / `scanning`
- `pairedAddr`, `pairedName`
- `rssi`, `battery`
- `devices[]` — last scan result
- `bindings[]` — current tap and hold bindings per button

## BLE Radio

Low-level control of the BLE radio. Useful to disable BLE when only WiFi or serial transports are in use. BLE builds only.

| Command           | Description                                 |
| ----------------- | ------------------------------------------- |
| `ble:on`          | Enable BLE radio (BLE builds only)          |
| `ble:off`         | Disable BLE radio                           |
| `ble:status`      | Returns enabled/connected/deviceName        |
| `ble:name:<name>` | Set BLE advertised device name (1-32 chars) |

## FSD & Autopilot

| Command         | Description                                               |
| --------------- | --------------------------------------------------------- |
| `fsd:on`        | Enable FSD CAN modification                               |
| `fsd:off`       | Disable FSD CAN modification                              |
| `fsd:force:on`  | Force FSD CAN edits even when UI FSD selection bit is off |
| `fsd:force:off` | Require UI FSD selection bit before FSD edits             |

## TLSSC Restore (Autopilot Tier)

| Command     | Description                                    |
| ----------- | ---------------------------------------------- |
| `tlssc:on`  | Enable TLSSC restore (spoof SELF_DRIVING tier) |
| `tlssc:off` | Disable TLSSC restore                          |

When enabled, rewrites DAS_autopilotConfig (0x331) byte[0] lower 6 bits to 0x1B (SELF_DRIVING tier) while preserving the upper counter bits. Setting is persisted to NVS.

## Emergency Vehicle Detection (HW4)

| Command   | Description                                  |
| --------- | -------------------------------------------- |
| `evd:on`  | Enable emergency vehicle detection bit (HW4) |
| `evd:off` | Disable emergency vehicle detection bit      |

When enabled, sets bit 59 in the FSD mux-0 frame so Autopilot can respond to emergency vehicles. HW4 only. Setting is persisted to NVS.

## Speed Profile

| Command          | Description                                               |
| ---------------- | --------------------------------------------------------- |
| `profile:0`      | Set speed profile to 0 (most aggressive)                  |
| `profile:1`      | Set speed profile to 1                                    |
| `profile:2`      | Set speed profile to 2                                    |
| `profile:3`      | Set speed profile to 3 (most conservative)                |
| `profile:auto`   | Auto-track from follow distance stalk                     |
| `profile:lock`   | Pin current profile (ignore follow-distance auto updates) |
| `profile:unlock` | Unpin profile and resume auto updates                     |

## Speed Offset

Auto-routes to HW4 (0–63) or HW3 (0–100) based on detected hardware variant. Legacy returns error.

| Command       | Description                                |
| ------------- | ------------------------------------------ |
| `offset:N`    | Set speed offset (HW3: 0–100, HW4: 0–63)   |
| `offset:auto` | Auto-track from UI offset steps (HW3 only) |
| `offset:off`  | Disable offset override                    |

## ISA Speed Chime (HW4 only)

| Command         | Description                |
| --------------- | -------------------------- |
| `isa-chime:on`  | Suppress ISA speed chime   |
| `isa-chime:off` | Restore original ISA chime |

## Variant Selection

| Command          | Description                        |
| ---------------- | ---------------------------------- |
| `variant:hw4`    | Set vehicle variant to HW4         |
| `variant:hw3`    | Set vehicle variant to HW3         |
| `variant:legacy` | Set vehicle variant to Legacy      |
| `variant:auto`   | Enable automatic variant detection |

## Nag Alert Suppression (unified)

Tesla's autopilot nag ("Apply pressure to steering wheel") can be suppressed
with different strategies, ranging from a cheap CAN-UI bit clear to a full
DAS-aware torque echo. All strategies are selected through one unified
command interface.

### Unified command

| Command            | Description                                                            |
| ------------------ | ---------------------------------------------------------------------- |
| `nag:mode:off`     | Disable all nag suppression                                            |
| `nag:mode:bit19`   | Clear ECE R79 hands-on bit on UI_autopilotControl mux=1 (cheapest)     |
| `nag:mode:legacy`  | 0x370 EPAS echo, fixed zero torque, always-on                          |
| `nag:mode:safe`    | 0x370 EPAS echo, only when DAS actively requests hands-on              |
| `nag:mode:natural` | 0x370 EPAS echo, Gaussian-jittered 0.08–0.18 Nm with steering feedback |
| `nag:mode:organic` | 0x370 EPAS echo, full DAS state machine + grip excursions ±1.0–3.3 Nm  |
| `nag:mode:full`    | `bit19` + whichever echo mode was last selected (max suppression)      |
| `nag:bypass:on`    | Organic mode only: stop injection when real driver hands-on detected   |
| `nag:bypass:off`   | Organic mode only: keep injecting regardless of driver input           |

### Strategy summary

- `bit19` — clears a single bit on a CAN-UI frame. No torque spoofing.
  Works on older firmware where DAS honours the UI bit. Matches the
  historical `nag:on` behaviour.
- `legacy` — echoes 0x370 with zeroed torque and handsOnLevel=1 on every
  real frame. Simple and loud; visible in every CAN trace.
- `safe` — same as legacy but only when `DAS_autopilotHandsOnState` is in
  a requesting state. Roughly 90 % less CAN traffic during normal AP
  driving.
- `natural` — Gaussian-jittered torque (0.08–0.18 Nm) with steering angle
  feedback and non-linear 150–350 ms timing. Simulates hand tremor.
- `organic` — most effective on modern Tesla firmware. Full state machine
  with mandatory pause windows, random-walk torque ±0.5–2.0 Nm, and
  periodic grip excursions ±3.1–3.3 Nm. Ported from linuchoicoegwangsu v2
  spec + zdenekbouresh grip-excursion pattern. Requires AP active
  (`dasApState in {3,4,5,6}`).
- `full` — applies both `bit19` and the last-selected echo mode
  simultaneously for maximum suppression on newer firmware that may
  ignore the bit-19 clear on its own.

### Status payload

```
"nagMode":      "<off|bit19|legacy|safe|natural|organic|full>"  — active mode
"nagOrgBypass": <bool>    — organic-mode driver-feedback bypass
"dasHandsOn":   <uint>    — decoded DAS hands-on-state
```

## Battery Preconditioning

| Command            | Description                                     |
| ------------------ | ----------------------------------------------- |
| `precondition:on`  | Start battery preconditioning for supercharging |
| `precondition:off` | Stop battery preconditioning                    |

## Track Mode

| Command         | Description        |
| --------------- | ------------------ |
| `trackmode:on`  | Enable Track Mode  |
| `trackmode:off` | Disable Track Mode |

## BMS Battery Telemetry

| Command | Description                                                               |
| ------- | ------------------------------------------------------------------------- |
| `bms`   | Query current BMS telemetry (voltage, current, SoC, temps, enhanced data) |

The `bms` response now includes enhanced fields when available:

| Field          | Description                                        |
| -------------- | -------------------------------------------------- |
| `nomFull`      | Nominal full pack capacity (kWh, integer ×1000)    |
| `nomRemain`    | Nominal energy remaining (kWh, integer ×1000)      |
| `idealRemain`  | Ideal energy remaining (kWh, integer ×1000)        |
| `cellVMax`     | Highest individual cell voltage (V, integer ×1000) |
| `cellVMin`     | Lowest individual cell voltage (V, integer ×1000)  |
| `maxRegen`     | Maximum regen power limit (kW, integer ×100)       |
| `maxDischarge` | Maximum discharge power limit (kW, integer ×100)   |
| `enhanced`     | `1` if enhanced BMS data is available              |

## Summon (HW3/HW4, requires 3 CAN buses)

| Command          | Description                   |
| ---------------- | ----------------------------- |
| `summon`         | Start summon (30-frame burst) |
| `summon:forward` | Summon forward                |
| `summon:reverse` | Summon reverse                |
| `summon:stop`    | Stop summon                   |

> Summon requires a cached 0x273 frame. Console shows "Waiting for 0x273" if not yet received.

## Summon Injection Gate

| Command             | Description                                         |
| ------------------- | --------------------------------------------------- |
| `summon-inject:on`  | Enable summon frame injection (required for summon) |
| `summon-inject:off` | Disable summon frame injection                      |

When disabled, any in-progress summon burst is stopped immediately. Setting is persisted to NVS.

## Streaming & Raw CAN

| Command       | Description                                    |
| ------------- | ---------------------------------------------- |
| `stream:on`   | Start frame streaming (JSON per frame)         |
| `stream:off`  | Stop frame streaming                           |
| `can:raw:on`  | Listen to all CAN IDs (unfiltered)             |
| `can:raw:off` | Return to filtered mode (variant-specific IDs) |

## CAN Frame Recorder

Captures incoming CAN frames into an on-device ring buffer for later retrieval and analysis. Useful for bench work and post-mortem debugging.

| Command           | Description                                                   |
| ----------------- | ------------------------------------------------------------- |
| `recorder:on`     | Start recording CAN frames                                    |
| `recorder:off`    | Stop recording                                                |
| `recorder:clear`  | Clear the recording buffer                                    |
| `recorder:status` | Returns enabled/count/capacity/captured/dropped/lastCaptureMs |

## Vehicle Commands (3 CAN buses required)

### Lock & Security

| Command      | Description    |
| ------------ | -------------- |
| `lock`       | Lock vehicle   |
| `unlock`     | Unlock vehicle |
| `lock:child` | Child lock     |
| `horn`       | Horn           |

### Trunk & Frunk

| Command                | Description     |
| ---------------------- | --------------- |
| `frunk` / `frunk:open` | Open frunk      |
| `frunk:close`          | Close frunk     |
| `trunk:open`           | Open trunk      |
| `trunk:close`          | Close trunk     |
| `glovebox`             | Toggle glovebox |

### Windows

| Command             | Description                                                       |
| ------------------- | ----------------------------------------------------------------- |
| `window:vent:N`     | Set vent position percentage (N=0..100, 0=closed, 100=fully open) |
| `window:vent:open`  | Fully open vent (same as `window:vent:100`)                       |
| `window:vent:close` | Fully close vent (same as `window:vent:0`)                        |
| `vent:open`         | Vent all windows                                                  |
| `vent:close`        | Close all windows                                                 |

### Mirrors

| Command           | Description       |
| ----------------- | ----------------- |
| `mirror:fold`     | Fold mirrors      |
| `mirror:unfold`   | Unfold mirrors    |
| `mirror:heat`     | Heat mirrors      |
| `mirror:autofold` | Auto-fold mirrors |
| `mirror:dip`      | Dip mirrors       |

### Lights

| Command               | Description             |
| --------------------- | ----------------------- |
| `light:fog:front`     | Toggle front fog lights |
| `light:fog:rear`      | Toggle rear fog lights  |
| `light:highbeam:auto` | Auto highbeam           |
| `light:ambient`       | Toggle ambient lighting |
| `light:home`          | Home lights             |
| `light:dome:on`       | Dome light on           |
| `light:dome:off`      | Dome light off          |
| `light:dome:auto`     | Dome light auto         |

### Sentry

| Command      | Description         |
| ------------ | ------------------- |
| `sentry:on`  | Enable sentry mode  |
| `sentry:off` | Disable sentry mode |

### Climate

| Command        | Description      |
| -------------- | ---------------- |
| `climate:keep` | Keep climate on  |
| `climate:off`  | Turn climate off |

### Charge

| Command        | Description      |
| -------------- | ---------------- |
| `charge:start` | Start charging   |
| `charge:stop`  | Stop charging    |
| `chargeport`   | Open charge port |

### Wiper

| Command     | Description            |
| ----------- | ---------------------- |
| `wiper:off` | Wiper off              |
| `wiper:1`   | Wiper speed 1 (low)    |
| `wiper:2`   | Wiper speed 2 (medium) |
| `wiper:3`   | Wiper speed 3 (high)   |

### Seat Heating

| Command                   | Description                                        |
| ------------------------- | -------------------------------------------------- |
| `seat:fl:0` – `seat:fl:3` | Front-left seat heat (0=off, 1=low, 2=med, 3=high) |
| `seat:fr:0` – `seat:fr:3` | Front-right seat heat                              |
| `seat:rl:0` – `seat:rl:3` | Rear-left seat heat                                |
| `seat:rr:0` – `seat:rr:3` | Rear-right seat heat                               |
| `seat:rc:0` – `seat:rc:3` | Rear-center seat heat                              |

### Display Brightness

| Command         | Description                         |
| --------------- | ----------------------------------- |
| `maindisplay:N` | Set main display brightness (0–127) |

### Power Control

| Command         | Description         |
| --------------- | ------------------- |
| `power:acc:on`  | Accessory power on  |
| `power:acc:off` | Accessory power off |
| `power:ready`   | Drive-ready state   |
| `power:off`     | Power off vehicle   |

### Drive Config

| Command                        | Description            |
| ------------------------------ | ---------------------- |
| `pedal:standard` / `pedal:std` | Standard pedal mode    |
| `pedal:chill`                  | Chill pedal mode       |
| `pedal:sport`                  | Sport pedal mode       |
| `regen:off`                    | Regen braking off      |
| `regen:low`                    | Regen braking low      |
| `regen:standard` / `regen:std` | Regen braking standard |
| `regen:max`                    | Regen braking max      |
| `stop:creep`                   | Stopping mode: creep   |
| `stop:roll`                    | Stopping mode: roll    |
| `stop:hold`                    | Stopping mode: hold    |

## Drive Mode Override (Ghost Mode)

| Command                 | Description                  |
| ----------------------- | ---------------------------- |
| `drivemode:off`         | Disable drive mode override  |
| `drivemode:chill`       | Force Chill drive mode       |
| `drivemode:standard`    | Force Standard drive mode    |
| `drivemode:performance` | Force Performance drive mode |

Status payload includes:

- `driveMode` — current override setting (0=off, 1=chill, 2=standard, 3=performance)
- `currentDriveMode` — readback from vehicle CAN bus (DI_steer)

The drive mode override injects CAN ID 0x334 at 20 Hz (50ms interval) to continuously enforce the selected mode.

## ECE R79 Bypass (European Steering Limit)

| Command      | Description                                |
| ------------ | ------------------------------------------ |
| `ecer79:on`  | Enable ECE R79 steering speed limit bypass |
| `ecer79:off` | Disable ECE R79 bypass                     |

Status payload includes:

- `eceR79` — bypass enabled (0=off, 1=on)
- `regionCode` — detected vehicle region (0=unknown, 1=NA, 2=EU, 3=CN, 4=APAC, 5=ME)
- `hasRegion` — whether region has been detected from CAN
- `cnLocked` — Chinese gateway lock detected

The bypass clears bit 20 in mux 1 of the FSD frame. Only active when the vehicle is detected as European market.

## Region Spoofing

| Command             | Description                               |
| ------------------- | ----------------------------------------- |
| `region:spoof:na`   | Spoof region to North America             |
| `region:spoof:eu`   | Spoof region to Europe                    |
| `region:spoof:cn`   | Spoof region to China                     |
| `region:spoof:apac` | Spoof region to Asia-Pacific              |
| `region:spoof:me`   | Spoof region to Middle East               |
| `region:spoof:off`  | Disable region spoofing (use real region) |

Overwrites the region nibble in 0x398 byte[2] bits[7:4] in-frame before other modules decode it. This enables FSD features in geographically restricted regions. Setting is persisted via NVS.

## Auto Lane Change (ALC) Auto-Confirm

| Command   | Description                                |
| --------- | ------------------------------------------ |
| `alc:on`  | Enable automatic lane change confirmation  |
| `alc:off` | Disable automatic lane change confirmation |

When enabled, monitors `DAS_laneChangeState` on 0x39B. When Autopilot requests a lane change, automatically injects a turn signal confirmation:

- **Model 3/Y (stalk vehicles):** Injects 0x249 SCCM_leftStalk with CRC-8
- **Palladium/Yoke (HW4):** Injects 0x3C2 VCLEFT_switchStatus button frame

A 2-second cooldown prevents duplicate injections. Direction is derived from the active turn signal state.

## TPMS (Tire Pressure Monitoring)

| Command | Description                                                        |
| ------- | ------------------------------------------------------------------ |
| `tpms`  | Query current TPMS data (pressure and temperature for all 4 tires) |

Response includes pressure (bar) and temperature (°C) for FL, FR, RL, RR tires. Data is decoded from CAN ID 0x219.

## Turn Signals (3-Blink Lane Change)

| Command       | Description                      |
| ------------- | -------------------------------- |
| `turn:left3`  | 3-blink left lane change signal  |
| `turn:right3` | 3-blink right lane change signal |
| `turn:hazard` | Activate hazard lights           |
| `turn:off`    | Cancel turn signal               |

Sends a burst of 3 frames at 100ms intervals to CAN ID 0x3F5 (VCFRONT_vehicleLights).

## Seatbelt Emulation

| Command        | Description                                               |
| -------------- | --------------------------------------------------------- |
| `seatbelt:on`  | Enable rear seatbelt buckle emulation (suppress warnings) |
| `seatbelt:off` | Disable rear seatbelt emulation                           |

When enabled, periodically injects CAN ID 0x3F3 with all 3 rear seatbelts reported as buckled. Setting is persisted across reboots via NVS.

## Air Recirculation

| Command        | Description                            |
| -------------- | -------------------------------------- |
| `airecirc:on`  | Switch cabin air to recirculation mode |
| `airecirc:off` | Switch cabin air to fresh air mode     |

Sends a burst of 20 frames at 20ms intervals to CAN ID 0x2AA. Momentary action (not persisted).

## Wiper Speed Persistence

| Command            | Description                                          |
| ------------------ | ---------------------------------------------------- |
| `wiperpersist:on`  | Enable wiper speed persistence across drive sessions |
| `wiperpersist:off` | Disable wiper speed persistence                      |

Tesla resets wiper speed to auto on each drive cycle. When enabled, the last-set wiper speed is saved to NVS and automatically re-injected on boot/wake.

## Mirror Auto-Fold on Lock

| Command               | Description                                     |
| --------------------- | ----------------------------------------------- |
| `mirror:autofold:on`  | Enable automatic mirror folding on vehicle lock |
| `mirror:autofold:off` | Disable automatic mirror folding                |

Monitors vehicle lock state transitions. Automatically folds mirrors when vehicle locks and unfolds when vehicle unlocks. Setting is persisted via NVS.

## Powertrain Telemetry

| Command      | Description                             |
| ------------ | --------------------------------------- |
| `powertrain` | Query current powertrain telemetry data |

Response includes:

| Field   | Description                                      |
| ------- | ------------------------------------------------ |
| `speed` | Vehicle speed (km/h × 100, signed)               |
| `gear`  | Gear state: 0=invalid, 1=P, 2=R, 3=N, 4=D        |
| `pedal` | Accelerator pedal position (0–100%)              |
| `steer` | Steering angle (degrees × 10, signed, + = right) |
| `rpmR`  | Rear motor RPM (signed)                          |
| `rpmF`  | Front motor RPM (signed, 0 if single motor)      |

Read-only telemetry decoded from CAN IDs 0x257, 0x118, 0x129, 0x106, 0x115.

## CAN Simulation Mode

| Command      | Description                                           |
| ------------ | ----------------------------------------------------- |
| `simu:start` | Start CAN simulation (generate synthetic test frames) |
| `simu:stop`  | Stop CAN simulation                                   |

Generates synthetic CAN frames for testing without a real vehicle. Simulates BMS (375V/5A, 70% SoC), TPMS (2.5 bar), vehicle speed (60 km/h), and DI state (gear=D, pedal=15%). Frames are **never transmitted on the physical bus** — they only feed the internal decode pipeline.

## Single-Shot TX Mode

| Command          | Description                                       |
| ---------------- | ------------------------------------------------- |
| `singleshot:on`  | Enable single-shot CAN TX (MCP2515 one-shot mode) |
| `singleshot:off` | Disable single-shot TX (normal retransmit)        |

Uses the MCP2515 CANCTRL OSM bit to disable automatic retransmission on TX failure. Useful on noisy buses where retransmits cause frame storms.

## Firmware Version Compatibility

| Command    | Description                                                    |
| ---------- | -------------------------------------------------------------- |
| `fwcompat` | Query decoded gateway firmware version and compatibility level |

Response includes:

| Field     | Description                                          |
| --------- | ---------------------------------------------------- |
| `year`    | Firmware year (e.g. 2024)                            |
| `release` | Release number (e.g. 44)                             |
| `minor`   | Minor version                                        |
| `build`   | Build number                                         |
| `compat`  | Compatibility level: 0=UNKNOWN, 1=OK, 2=WARN, 3=FAIL |

Decoded from CAN ID 0x392 (GTW firmware version), multiplexed:

- Mux 0: year, release, minor
- Mux 1: build number

## MQTT Telemetry Bridge

| Command              | Description                                                       |
| -------------------- | ----------------------------------------------------------------- |
| `mqtt:on`            | Enable MQTT telemetry publishing                                  |
| `mqtt:off`           | Disable MQTT telemetry publishing                                 |
| `mqtt:broker:<host>` | Set MQTT broker hostname/IP (1–63 chars)                          |
| `mqtt:port:N`        | Set MQTT broker port (N=1..65535, default 1883)                   |
| `mqtt:interval:N`    | Set publish interval in milliseconds (N=100..60000, default 2000) |

Publishes JSON telemetry snapshots to the configured MQTT broker at the specified interval. Requires WiFi. Settings are persisted to NVS.

Status payload includes:

- `mqtt` — MQTT bridge enabled (0=off, 1=on)
- `mqttConnected` — Currently connected to broker (0=no, 1=yes)

## Vehicle Config Query

| Command   | Description                          |
| --------- | ------------------------------------ |
| `vehicle` | Query decoded vehicle model and year |

Response includes:

| Field   | Description                                                                        |
| ------- | ---------------------------------------------------------------------------------- |
| `model` | Vehicle model: 0=UNKNOWN, 1=Model 3, 2=Model Y, 3=Model S, 4=Model X, 5=Cybertruck |
| `year`  | Model year                                                                         |

Decoded from CAN ID 0x398 (GTW_carConfig).

## Vehicle Platform Query

| Command    | Description                                                 |
| ---------- | ----------------------------------------------------------- |
| `platform` | Query the full vehicle platform identity and CAN bus health |

Response includes:

| Field       | Type   | Description                                                                      |
| ----------- | ------ | -------------------------------------------------------------------------------- |
| `model`     | number | Tesla model: 0=UNKNOWN, 1=MODEL_3, 2=MODEL_Y, 3=MODEL_S, 4=MODEL_X, 5=CYBERTRUCK |
| `hwGen`     | number | Hardware generation: 0=HW_UNKNOWN, 1=HW_LEGACY, 2=HW_3, 3=HW_4                   |
| `swYear`    | number | Software version year (e.g. 2026)                                                |
| `swWeek`    | number | Software version week (e.g. 14)                                                  |
| `swRelease` | number | Software version release number                                                  |
| `fsdProto`  | number | FSD protocol: 0=UNKNOWN, 1=V12, 2=V13, 3=V14                                     |
| `swCompat`  | number | Software compatibility: 0=OK, 1=UNTESTED, 2=TOO_OLD, 3=TOO_NEW                   |
| `resolved`  | number | 1 if platform identity has been resolved, 0 otherwise                            |
| `canHealth` | object | Per-bus MCP2515 detection: `{chassis:{on,det}, vehicle:{on,det}, body:{on,det}}` |

Example response:

<!-- markdownlint-disable MD010 -->

```json
{
    "t": "platform",
    "model": 1,
    "hwGen": 3,
    "swYear": 2026,
    "swWeek": 14,
    "swRelease": 1,
    "fsdProto": 3,
    "swCompat": 0,
    "resolved": 1,
    "canHealth": {
        "chassis": { "on": 1, "det": 1 },
        "vehicle": { "on": 1, "det": 1 },
        "body": { "on": 1, "det": 1 }
    }
}
```

<!-- markdownlint-enable MD010 -->

The platform is automatically resolved from CAN data (0x392 GTW_fwVersion, 0x398 GTW_carConfig) and re-evaluated whenever new vehicle information is decoded. Platform fields are also included in `boot` and `status` messages.

## Debug Log

| Command | Description                                         |
| ------- | --------------------------------------------------- |
| `log`   | Dump the debug event ring buffer (last 256 entries) |

Returns timestamped debug events useful for diagnostics.

## Button Remapping

| Command                        | Description                                      |
| ------------------------------ | ------------------------------------------------ |
| `btnmap:lamp:short:trunk`      | Map lamp button short press to trunk action      |
| `btnmap:lamp:long:sentry`      | Map lamp button long press to sentry action      |
| `btnmap:lamp:double:horn`      | Map lamp button double press to horn action      |
| `btnmap:parking:short:fold`    | Map parking button short press to mirror fold    |
| `btnmap:parking:long:hazard`   | Map parking button long press to hazard action   |
| `btnmap:parking:double:recirc` | Map parking button double press to recirc action |
| `btnmap:query`                 | Query current button mappings                    |
| `btnmap:reset`                 | Reset all button mappings to defaults            |

Button names: `lamp`, `parking`. Press types: `short`, `long`, `double`. Actions: `none`, `trunk`, `frunk`, `sentry`, `horn`, `fold`, `hazard`, `recirc`.

## Speed Camera Alert

| Command          | Description                            |
| ---------------- | -------------------------------------- |
| `speedalert:on`  | Enable speed camera alert BLE service  |
| `speedalert:off` | Disable speed camera alert BLE service |

## GVRET TCP Gateway

| Command         | Description                                   |
| --------------- | --------------------------------------------- |
| `gvret:on`      | Enable GVRET TCP server (SavvyCAN compatible) |
| `gvret:off`     | Disable GVRET TCP server                      |
| `gvret:port:23` | Set GVRET TCP port (1–65535)                  |

## ESP-NOW

| Command            | Description                           |
| ------------------ | ------------------------------------- |
| `espnow:on`        | Enable ESP-NOW multi-device broadcast |
| `espnow:off`       | Disable ESP-NOW broadcast             |
| `espnow:channel:1` | Set ESP-NOW WiFi channel (1–13)       |

## ScanMyTesla BT Bridge

| Command   | Description                             |
| --------- | --------------------------------------- |
| `smt:on`  | Enable ScanMyTesla Bluetooth SPP bridge |
| `smt:off` | Disable ScanMyTesla Bluetooth bridge    |

## ELM327 Emulation

| Command      | Description                        |
| ------------ | ---------------------------------- |
| `elm327:on`  | Enable ELM327 AT command emulation |
| `elm327:off` | Disable ELM327 emulation           |

## Tesla BLE Key Protocol

Low-level Tesla BLE key management and vehicle commands over the Tesla BLE protocol.
Distinct from the high-level VCSEC control below.

| Command                           | Description                                   |
| --------------------------------- | --------------------------------------------- |
| `tesla:key:gen`                   | Generate a new Tesla BLE key pair             |
| `tesla:key:show`                  | Show the current key's public key hash        |
| `tesla:key:role:owner`            | Set key role to owner                         |
| `tesla:key:role:charging_manager` | Set key role to charging manager              |
| `tesla:key:send`                  | Send the generated key to the vehicle via BLE |
| `tesla:vin:<VIN>`                 | Set vehicle VIN for BLE connection            |
| `tesla:wake`                      | Wake vehicle via BLE                          |
| `tesla:charge:start`              | Start charging via BLE                        |
| `tesla:charge:stop`               | Stop charging via BLE                         |
| `tesla:charge:amps:<1-32>`        | Set charging amperage (1–32 A) via BLE        |
| `tesla:charge:limit:<50-100>`     | Set charge limit percentage (50–100%) via BLE |
| `tesla:climate:on`                | Turn climate on via BLE                       |
| `tesla:climate:off`               | Turn climate off via BLE                      |

## Tesla BLE Vehicle Control

| Command           | Description                              |
| ----------------- | ---------------------------------------- |
| `teslable:on`     | Enable Tesla BLE vehicle control (VCSEC) |
| `teslable:off`    | Disable Tesla BLE control                |
| `teslable:auth`   | Authenticate with vehicle via BLE        |
| `teslable:forget` | Forget BLE vehicle pairing               |

## Home Assistant / ESPHome

| Command            | Description                           |
| ------------------ | ------------------------------------- |
| `ha:on`            | Enable Home Assistant integration     |
| `ha:off`           | Disable Home Assistant integration    |
| `ha:discovery`     | Trigger MQTT auto-discovery for HA    |
| `ha:interval:<ms>` | Set reporting interval (500–60000 ms) |

## Encrypted BLE Multi-Device

| Command             | Description                      |
| ------------------- | -------------------------------- |
| `bleencrypt:on`     | Enable AES-256-GCM encrypted BLE |
| `bleencrypt:off`    | Disable encrypted BLE            |
| `bleencrypt:pair`   | Start BLE pairing mode           |
| `bleencrypt:unpair` | Remove all paired devices        |

## Response Format

All responses are JSON with a `t` (type) field:

```json
{"t":"pong","v":1}
{"t":"ack","cmd":"fsd:on"}
{"t":"status","hw":"ESP32S_DevKit","variant":"hw4",...}
{"t":"error","msg":"Unknown command"}
{"t":"log","msg":"FSD enabled - saved to NVS"}
{"t":"frame","dir":"rx","bus":0,"id":1021,"dlc":8,"d":"0102030405060708"}
```
