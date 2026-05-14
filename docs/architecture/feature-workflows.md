---
title: Feature Workflows
title_tr: Özellik İş Akışları
description: Command-to-CAN path reference for every firmware feature
category: architecture
folder: architecture
tags: [features, workflows, design]
order: 6
icon: 🔄
---

# Feature Workflow Reference

Canonical reference for all firmware features.
Each entry shows: command → gate → state → CAN path → bus target.

---

## Architecture

### Bus Layout (Tesla X179 Connector)

| Bus | ID          | Name                | Direction            |
| --- | ----------- | ------------------- | -------------------- |
| 0   | BUS_CHASSIS | Chassis CAN         | Intercept + modify   |
| 1   | BUS_VEHICLE | Vehicle Control CAN | Inject / echo / read |
| 2   | BUS_BODY    | Body Control CAN    | Inject               |

### Feature Categories

| Category      | Pattern                                                          | Typical Bus |
| ------------- | ---------------------------------------------------------------- | ----------- |
| Toggle-Inject | ON → add filter + intercept + modify + send, OFF → remove filter | 0 (FSD)     |
| Echo-Inject   | Read frame → clone → modify → send back on same bus              | 1 (Vehicle) |
| Burst-Inject  | Build frame → `startBurst(count, delayMs)` non-blocking          | 1 or 2      |
| Tick-Inject   | Dedicated timer loop sends frames indefinitely or with countdown | 1 (Vehicle) |
| Read-Only     | Decode frame → update state (no send)                            | 1 (Vehicle) |
| Config-Only   | Update state + persist (no CAN interaction)                      | —           |

### Transmission Safety

`txPaused` gates **all** transmission paths when OTA update is detected:

| Layer                   | Effect                                            |
| ----------------------- | ------------------------------------------------- |
| HW4/HW3/Legacy handlers | Pass frames through unmodified (no FSD injection) |
| `startBurst()`          | Refuses to start new burst                        |
| `burstTick()`           | Cancels active burst (`burstRemaining = 0`)       |
| `summonTick()`          | Cancels summon (`summonRemaining = 0`)            |
| `preconditionTick()`    | Returns immediately                               |
| `nagShouldEcho()`       | Returns false                                     |

### Burst Architecture (`vehicle/can/burst.h`)

Non-blocking frame repeater. `startBurst(s, f, bus, count, delayMs)` stores a
single frame in State. `burstTick()` in the main loop drains it one frame per
interval. Only one burst active at a time — new burst overrides previous.

Used by: pedal, regen, stop, lock, light, mirror, seat, wiper, display, power,
charge, climate, sentry, window, trunk, track_mode, air_recirc, turn_signal,
glovebox, mirror_autofold.

NOT used by: summon (dynamic frame per tick), precondition (indefinite heartbeat),
das_drive (dedicated tick at 50 Hz).

### Variant-Aware Feature Gating

`features()` returns a `Features` struct based on `s.variant`:

| Feature        | HW4     | HW3     | Legacy |
| -------------- | ------- | ------- | ------ |
| fsd            | yes     | yes     | yes    |
| fsdForce       | yes     | yes     | yes    |
| offset         | **yes** | **yes** | no     |
| profile        | yes     | yes     | yes    |
| nag            | yes     | yes     | yes    |
| isaChime       | **yes** | no      | no     |
| summon         | yes     | yes     | **no** |
| regionSpoof    | yes     | yes     | no     |
| eceR79         | yes     | yes     | **no** |
| autoLaneChange | yes     | yes     | no     |
| dasDrive       | **yes** | yes     | yes    |

Features not in this table use explicit variant checks or have no gate.

---

## 1. FSD Enable (Toggle-Inject, Bus 0)

```
Command: "fsd:on|off"
  │
  ├─ Gate: features().fsd
  ├─ State: s.fsdEnabled
  ├─ resetHandlerLogFlags()
  ├─ saveSettings(s)
  └─ applyFilters(s)

CAN Path (when ON):
  Bus 0 → CAN_ID_FSD_MUX (0x3FD / 1021)
    │
    ├─ HW4 mux==0 + fsdAllowed:
    │   ├─ setBit(38,true) setBit(46,true) setBit(60,true)
    │   └─ driverSend(f, 0)
    │
    ├─ HW3 mux==0 + fsdAllowed:
    │   ├─ readHW3UiOffsetSteps → auto-track profile + offset
    │   ├─ setBit(38,true) setBit(46,true)
    │   ├─ setSpeedProfileV12V13(f, profile)
    │   └─ driverSend(f, 0)
    │
    └─ Legacy mux==0 + fsdAllowed (CAN_ID 1006):
        ├─ readHW3UiOffsetSteps → auto-track profile
        ├─ setBit(46,true)
        ├─ setSpeedProfileV12V13(f, profile)
        └─ driverSend(f, 0)

Notes:
  - HW4 sets 3 bits (38,46,60). HW3 sets 2 (38,46). Legacy sets 1 (46).
    Bit 60 is HW4-only. Bit 38 exists on HW3 but not Legacy.
  - HW4 delegates profile to mux==2 (writeHW4SpeedProfile).
    HW3/Legacy write profile in mux==0 via setSpeedProfileV12V13.
  - fsdAllowed = fsdEnabled && (fsdForceEnabled || isFSDSelectedInUI)
```

---

## 2. FSD Force (Config-Only)

```
Command: "fsd:force:on|off"
  │
  ├─ Gate: features().fsdForce
  ├─ State: s.fsdForceEnabled
  ├─ resetHandlerLogFlags()
  └─ saveSettings(s)

Notes:
  - No applyFilters() — correct: doesn't change which frames to intercept.
  - Modifies fsdAllowed calculation: bypasses isFSDSelectedInUI check.
  - Has no effect unless fsdEnabled=true (no CAN path without FSD filter).
```

---

## 3. Nag Suppression — Unified (Toggle-Inject + Echo-Inject, Bus 0 + Bus 1)

Tesla's autopilot nag ("Apply pressure to steering wheel") is suppressed via
multiple strategies selectable through a unified command interface.

```
Command: "nag:mode:off|bit19|legacy|safe|natural|organic|full"
  │
  ├─ Gate: features().nag
  ├─ State: s.nagMode = NAG_MODE_OFF / BIT19 / LEGACY / SAFE / NATURAL / ORGANIC / FULL
  ├─ resetHandlerLogFlags()
  ├─ saveSettings(s)
  └─ applyFilters(s)  (bit19 and FULL need FSD filter on Bus 0)

Command: "nag:bypass:on|off"  (organic mode only)
  │
  ├─ State: s.nagOrganicDriverBypass
  └─ saveSettings(s)
```

### Strategy: bit19 (Toggle-Inject, Bus 0)

Clears a single bit on CAN-UI frame to suppress nag. No torque spoofing.

```
CAN Path:
  Bus 0 → CAN_ID_FSD_MUX (0x3FD), mux==1
    ├─ HW4: setBit(19,false) setBit(47,true) → driverSend(f, 0)
    ├─ HW3: setBit(19,false)                 → driverSend(f, 0)
    └─ Legacy: setBit(19,false)              → driverSend(f, 0)
```

### Strategies: legacy / safe / natural / organic (Echo-Inject, Bus 1)

All torque-based strategies intercept 0x370 (EPAS_sysStatus) on Bus 1,
modify the torque bytes, and echo the frame back:

```
CAN Path:
  Bus 1 → CAN_ID_EPAS_TORQUE (0x370)
    ├─ Gate: !txPaused && nagShouldEcho(s, mode)
    ├─ Clone frame → apply strategy-specific torque
    ├─ Counter in byte[1] low nibble incremented
    ├─ byte[7] = nagChecksum(f.data)
    └─ driverSend(echo, BUS_VEHICLE)

Strategy differences:
  legacy  — always echo, zero torque, handsOnLevel=1
  safe    — echo only when DAS requests hands-on (dasHandsOnRequested)
  natural — Gaussian-jittered torque 0.08–0.18 Nm, 150–350 ms intervals,
            steering feedback, simulates hand tremor
  organic — full state machine: mandatory pause windows,
            random-walk torque ±0.5–2.0 Nm, grip excursions ±3.1–3.3 Nm,
            requires AP active (dasApState in {3,4,5,6})
```

### Strategy: full

Applies both `bit19` and whichever echo mode was last selected simultaneously
for maximum suppression on newer firmware.

```
Status payload:
  "nagMode":  "<off|bit19|legacy|safe|natural|organic|full>"
  "nagOrgBypass": <bool>
  "dasHandsOn":   <uint>
```

---

## 4. ISA Chime Suppress (Toggle-Inject, Bus 0, HW4-only)

```
Command: "isa-chime:on|off"
  │
  ├─ Gate: variant == HW4 + features().isaChime (double-gated)
  ├─ State: s.isaChimeSuppress
  ├─ resetHandlerLogFlags()
  ├─ saveSettings(s)
  └─ applyFilters(s)

CAN Path (when ON, HW4 handler only):
  Bus 0 → CAN_ID_ISA_SPEED (921)
    ├─ f.data[1] |= 0x20
    ├─ f.data[7] = computeHW4IsaChecksum(f)
    └─ driverSend(f, 0)

Notes:
  - Command returns false on non-HW4 (explicit variant check).
  - features().isaChime is also HW4-only in the Features table.
```

---

## 5. Speed Profile (Config-Only, applied by handlers)

```
Command: "profile:N" (0-3)
  │
  ├─ Gate: features().profile
  ├─ State: s.speedProfile = N, s.profileOverride = true
  └─ saveSettings(s)

Command: "profile:auto|lock|unlock"
  │
  ├─ Gate: features().profile
  ├─ State: s.profileOverride = true/false
  └─ saveSettings(s)

Applied in handlers (passive — waits for next matching frame):
  ├─ HW4 mux==2: writeHW4SpeedProfile(f, speedProfile)
  ├─ HW3 mux==0: setSpeedProfileV12V13(f, speedProfile)
  └─ Legacy mux==0: setSpeedProfileV12V13(f, speedProfile)

Auto-track (when profileOverride=false):
  ├─ HW4:   CAN_ID_FOLLOW_DIST → mapHW4FollowDistToProfile (6 values: 0-4 + no-follow)
  ├─ HW3:   CAN_ID_FOLLOW_DIST → mapHW3FollowDistToProfile (4 values: 0-2 + no-follow)
  │          + mux==0: readHW3UiOffsetSteps → clamp to 0-2
  └─ Legacy: CAN_ID_LEGACY_STALK → byte[1]>>5 → profile mapping

Notes:
  - No applyFilters() — correct: profile is written INTO already-intercepted FSD frames.
  - Profile only takes effect when FSD handler is active (fsdEnabled=true).
  - HW3 auto-track produces 0-2 only. Values 3 are settable manually but never auto-tracked on HW3.
```

---

## 6. Speed Offset (Config-Only, applied by handlers)

```
Command: "offset:N"
  │
  ├─ Routes by detected HW:
  │
  ├─ HW4 path (detectedHW==3 || variant==HW4):
  │   ├─ Gate: features().offset
  │   ├─ State: s.speedOffset = N (0-63)
  │   └─ "offset:auto|off|0" → s.speedOffset = 0
  │
  └─ Non-HW4 path:
      ├─ Gate: features().offset (Legacy returns false)
      ├─ State: s.speedOffset = N (0-100), s.offsetOverride = true
      └─ "offset:auto" → s.offsetOverride = false

Applied in handlers:
  ├─ HW4 mux==2: f.data[1] = (f.data[1] & 0xC0) | (speedOffset & 0x3F)
  │   (only when speedOffset > 0 && fsdEnabled)
  ├─ HW3 mux==2: writeHW3SpeedOffset(f, speedOffset)
  │   (gated by fsdAllowed)
  └─ Legacy: no offset handler at all

Auto-track (HW3, when offsetOverride=false):
  HW3 mux==0: readHW3UiOffsetSteps → calculateHW3SpeedOffset → s.speedOffset

Notes:
  - Offset only applies when FSD is enabled.
  - Legacy returns false immediately from offset command (features().speedOffset = false).
  - HW4 gated by fsdEnabled only. HW3 gated by fsdAllowed (fsdEnabled + force/UI).
```

---

## 7. ECE R79 Bypass (Toggle-Inject, Bus 0)

```
Command: "ecer79:on|off"
  │
  ├─ Gate: features().eceR79 + isEuropeanMarket(regionCode)
  ├─ State: s.eceR79Bypass
  ├─ resetHandlerLogFlags()
  ├─ saveSettings(s)
  └─ applyFilters(s)

CAN Path (when ON, HW4/HW3 only):
  Bus 0 → CAN_ID_FSD_MUX (0x3FD), mux==1
    ├─ f.data[2] &= ~0x10  (clear bit 20)
    └─ driverSend(f, 0)

Notes:
  - Only active when vehicle region is detected as European market.
  - Removes the 57 km/h steering speed limit for EU vehicles.
  - Legacy variant returns false from features().eceR79.
```

---

## 8. Region Detection & Spoofing (Config + Toggle-Inject, Bus 0)

```
No explicit command for detection (automatic):
  Bus 1 → CAN_ID_GTW_CAR_CFG (0x398)
    ├─ s.regionCode = decodeRegionCode(f.data)  // byte[2] bits[7:4]
    └─ s.hasRegion = true

Command: "region:spoof:na|eu|cn|apac|me|off"
  │
  ├─ Gate: features().regionSpoof
  ├─ State: s.regionSpoofCode = 1|2|3|4|5, or 0 for off
  ├─ saveSettings(s)
  └─ applyFilters(s)

CAN Path (when spoof active):
  Bus 0 → CAN_ID_GTW_CAR_CFG (0x398) — overwrites region nibble in-frame
    ├─ f.data[2] = (f.data[2] & 0x0F) | (spoofCode << 4)
    └─ driverSend(f, 0)

Region codes: 0=unknown, 1=NA, 2=EU, 3=CN, 4=APAC, 5=ME
```

---

## 9. Auto Lane Change — ALC (Toggle-Inject, Bus 0)

```
Command: "alc:on|off"
  │
  ├─ Gate: features().autoLaneChange
  ├─ State: s.alcEnabled
  ├─ resetHandlerLogFlags()
  └─ saveSettings(s)

CAN Path (when ON):
  Monitors DAS_laneChangeState on 0x39B. When AP requests lane change:
  Bus 0:
    ├─ Model 3/Y (stalk): injects 0x249 SCCM_leftStalk with CRC-8
    └─ Palladium/Yoke (HW4): injects 0x3C2 VCLEFT_switchStatus button frame

Notes:
  - 2-second cooldown prevents duplicate injections.
  - Direction derived from active turn signal state.
  - Legacy variant returns false from features().autoLaneChange.
```

---

## 10. Summon (Tick-Inject, Bus 1)

```
Command: "summon"
  │
  ├─ Gate: features().summon + summonInject + hasCtrl
  ├─ State: s.summonMode=START, s.summonDirection=FORWARD, s.summonRemaining=30
  └─ (no save — transient action)

Command: "summon:forward|reverse|stop"
  │
  ├─ Gate: features().summon + summonInject + hasCtrl
  ├─ State: s.summonDirection, s.summonMode, s.summonRemaining
  └─ (no save — transient action)

CAN Path (summonTick, called from main loop):
  Every 20ms while summonRemaining > 0:
    ├─ Gate: hasCtrl && summonInject && !txPaused
    ├─ Build frame from s.lastCtrl (CAN_ID_UI_VEHICLE_CTRL)
    ├─ setSummonActive(f, true)    → bit 4
    ├─ setSummonDirection(f, dir)  → bit 5
    ├─ setSummonMode(f, mode)      → bit 0
    ├─ driverSend(f, BUS_VEHICLE)
    └─ summonRemaining--

Notes:
  - Uses dedicated tick (not startBurst) because each frame copies latest lastCtrl.
  - Requires hasCtrl (cached UI_VEHICLE_CTRL frame from bus 1).
  - features().summon = false on Legacy (no summon support).
```

---

## 11. Preconditioning (Tick-Inject, Bus 1)

```
Command: "precondition:on|off"
  │
  ├─ Gate: variant != LEGACY
  ├─ State: s.preconditionEnabled
  ├─ If ON: controlPrecondition(true, s) + s.precondLastMs = millis()
  ├─ If OFF: controlPrecondition(false, s)
  └─ saveSettings(s)

CAN Path (preconditionTick, called from main loop):
  Every 500ms while preconditionEnabled:
    ├─ Gate: !txPaused
    ├─ Build frame: CAN_ID_PRECONDITION (0x082), data[0]=0x05
    └─ driverSend(f, BUS_VEHICLE)

Notes:
  - Uses dedicated tick (not startBurst) — indefinite heartbeat while enabled.
  - controlPrecondition() also sends one frame on command execution (initial kick).
  - No features() gate — uses explicit variant check only.
```

---

## 12. Sentry Mode (Burst-Inject, Bus 2)

```
Command: "sentry:on|off"
  │
  ├─ Gate: variant != LEGACY
  └─ controlSentry(enable, s):
      ├─ Build frame: CAN_ID_SENTRY (0x284), dlc=5
      ├─ data[0] = enable ? 0x20 : 0x00
      └─ startBurst(s, f, BUS_BODY, 30, 20)
```

---

## 13. Window Vent (Burst-Inject, Bus 2)

```
Command: "window:vent:open|close" / "window:vent:N" (0-100) / "vent:open|close"
  │
  ├─ Gate: variant != LEGACY
  └─ controlWindowVent(action, s):
      ├─ Build frame: CAN_ID_WINDOW_VENT (0x119)
      └─ startBurst(s, f, BUS_BODY, 30, 20)
```

---

## 14. Trunk / Frunk / Glovebox (Burst-Inject)

```
Command: "frunk:open|close"
  │
  ├─ Gate: variant != LEGACY
  └─ startBurst(s, f, BUS_VEHICLE, 20, 20)

Command: "trunk:open|close"
  │
  ├─ Gate: variant != LEGACY
  └─ startBurst(s, f, BUS_BODY, 20, 100)

Command: "glovebox"
  │
  ├─ Gate: variant != LEGACY
  └─ startBurst(s, f, BUS_BODY, 20, 100)

All use CAN_ID_TRUNK_CTRL (0x3B3).
```

---

## 15. Charge Control (Burst-Inject, Bus 1)

```
Command: "charge:start|stop" / "chargeport"
  │
  ├─ Gate: variant != LEGACY + hasCharge
  └─ controlCharge(action, s.lastCharge, s):
      ├─ Uses cached charge frame (CAN_ID_CHARGE, 0x333)
      └─ startBurst(s, f, BUS_VEHICLE, 20, 20)
```

---

## 16. Climate Control (Burst-Inject, Bus 1)

```
Command: "climate:keep|off"
  │
  ├─ Gate: variant != LEGACY + hasClimate
  └─ controlClimate(action, s.lastClimate, s):
      ├─ Uses cached climate frame (CAN_ID_CLIMATE, 0x2F3)
      └─ startBurst(s, f, BUS_VEHICLE, 30, 20)
```

---

## 17. Pedal Mode (Burst-Inject, Bus 1)

```
Command: "pedal:standard|std|chill|sport"
  │
  ├─ Gate: variant != LEGACY + hasDrive
  └─ controlPedalMode(mode, s.lastDrive, s):
      ├─ Uses cached drive frame (CAN_ID_DRIVE_CONFIG, 0x334)
      ├─ Modifies byte[0] bits 5-6
      ├─ byte[7] = driveChecksum(f.data, 8)
      └─ startBurst(s, f, BUS_VEHICLE, 30, 20)
```

---

## 18. Regen Level (Burst-Inject, Bus 1)

```
Command: "regen:off|low|standard|std|max"
  │
  ├─ Gate: variant != LEGACY + hasDrive
  └─ controlRegenLevel(level, s.lastDrive, s):
      ├─ Uses cached drive frame (CAN_ID_DRIVE_CONFIG, 0x334)
      ├─ Modifies byte[2] (0-200)
      ├─ byte[7] = driveChecksum(f.data, 8)
      └─ startBurst(s, f, BUS_VEHICLE, 30, 20)
```

---

## 19. Stop Mode (Burst-Inject, Bus 1)

```
Command: "stop:creep|roll|hold"
  │
  ├─ Gate: variant != LEGACY + hasDrive
  └─ controlStopMode(mode, s.lastDrive, s):
      ├─ Uses cached drive frame (CAN_ID_DRIVE_CONFIG, 0x334)
      ├─ Modifies byte[5] bits 0-1
      ├─ byte[7] = driveChecksum(f.data, 8)
      └─ startBurst(s, f, BUS_VEHICLE, 30, 20)

Notes (pedal/regen/stop):
  - All three share CAN_ID_DRIVE_CONFIG (0x334) and s.lastDrive cache.
  - All three share driveChecksum().
  - Require hasDrive flag (cached drive frame received from bus 1).
```

---

## 20. Drive Mode Override — Ghost Mode (Tick-Inject, Bus 1)

```
Command: "drivemode:off|chill|standard|performance"
  │
  ├─ Gate: variant != LEGACY
  ├─ State: s.driveModeOverride = 0|1|2|3
  └─ saveSettings(s)

CAN Path (driveModeTick, called from main loop):
  Every 50ms while driveModeOverride > 0:
    ├─ Gate: !txPaused
    ├─ Build frame: CAN_ID_DRIVE_CONFIG (0x334)
    ├─ data[1] = driveModeOverride
    ├─ byte[7] = driveChecksum(f.data, 8)
    └─ driverSend(f, BUS_VEHICLE)

Drive mode readback from CAN_ID_DI_STEER (0x249):
  s.currentDriveMode decoded from DI_steer byte[0] bits[5:4]
```

---

## 21. Drive Context (Read-Only + Tick, Bus 1)

Decodes door/closure states and speed-limit signals for DAS drive safety.
Exposes `readBitsLE()` helper for little-endian bit field extraction.

```
No explicit command (always active on Bus 1)
Decodes: VCLEFT/VCRIGHT door status, frunk/trunk latch, UI speed limit,
         DAS ACC speed limit, GPS vehicle speed.
Sets: s.doorOpen*, s.speedLimitKph, s.accSpeedLimitKph, s.gpsSpeedKph.
Used by: DAS drive safety gates, mirror auto-fold.
```

---

## 22. Lock / Unlock / Horn (Burst-Inject, Bus 1)

```
Command: "lock" / "unlock" / "lock:child" / "horn"
  │
  ├─ Gate: hasCtrl
  └─ controlLock/controlChildLock/controlHorn(s):
      ├─ Uses cached ctrl frame (CAN_ID_UI_VEHICLE_CTRL, 0x273)
      └─ startBurst(s, f, BUS_VEHICLE, 30, 20)

Notes:
  - hasCtrl = cached UI_VEHICLE_CTRL frame. Legacy variants won't receive
    this CAN ID, so hasCtrl stays false — effective variant gate.
```

---

## 23. Lights (Burst-Inject, Bus 1)

```
Command: "light:fog:front|rear" / "light:highbeam:auto" / "light:ambient"
         "light:home" / "light:dome:off|on|auto"
  │
  ├─ Gate: hasCtrl
  └─ Various control functions using cached ctrl frame
      └─ startBurst(s, f, BUS_VEHICLE, 30, 20)
```

---

## 24. Mirror Control (Burst-Inject, Bus 1)

```
Command: "mirror:fold|unfold|heat|autofold|dip"
  │
  ├─ Gate: hasCtrl
  └─ controlMirrorFold/controlMirrorHeat/etc(s):
      └─ startBurst(s, f, BUS_VEHICLE, 50|30, 20)
```

---

## 25. Mirror Auto-Fold on Lock (Tick + Persist)

```
Command: "mirror:autofold:on|off"
  │
  ├─ State: s.mirrorAutoFoldEnabled
  └─ saveSettings(s)

Tick logic (auto):
  Monitors lock state transitions on Bus 1:
    lock detected → inject mirror:fold burst
    unlock detected → inject mirror:unfold burst
```

---

## 26. Seat Heat (Burst-Inject, Bus 1)

```
Command: "seat:fl|fr|rl|rr|rc:0|1|2|3"
  │
  ├─ Gate: hasCtrl
  └─ controlSeatHeat(seat, level, s):
      └─ startBurst(s, f, BUS_VEHICLE, 30, 20)
```

---

## 27. Wiper Control (Burst-Inject, Bus 1)

```
Command: "wiper:off|1|2|3"
  │
  ├─ Gate: hasCtrl
  └─ controlWiper(level, s):
      └─ startBurst(s, f, BUS_VEHICLE, 20, 20)
```

---

## 28. Wiper Speed Persistence (Config + Tick, Bus 1)

```
Command: "wiperpersist:on|off"
  │
  ├─ State: s.wiperPersistEnabled
  └─ saveSettings(s)

Tick logic (auto):
  Saves last-set wiper speed to NVS (s.savedWiperSpeed).
  On boot/wake: re-injects saved speed if wiperPersistEnabled.
  Tesla resets wiper to auto on each drive cycle; this overrides.
```

---

## 29. Display Brightness (Burst-Inject, Bus 1)

```
Command: "maindisplay:N" (0-127)
  │
  ├─ Gate: hasCtrl
  └─ controlDisplayBrightness(level, s):
      └─ startBurst(s, f, BUS_VEHICLE, 20, 20)
```

---

## 30. Power Control (Burst-Inject, Bus 1)

```
Command: "power:acc:on|off" / "power:ready" / "power:off"
  │
  ├─ Gate: hasCtrl
  └─ controlAccessoryPower/controlDriveState/controlPowerOff(s):
      └─ startBurst(s, f, BUS_VEHICLE, 30, 20)

Notes (hasCtrl-gated features 22-30):
  - All use CAN_ID_UI_VEHICLE_CTRL (0x273) cached in s.lastCtrl.
  - No explicit variant gate — hasCtrl serves as effective gate.
    Legacy variants never receive 0x273, so hasCtrl stays false.
  - All use non-blocking startBurst().
```

---

## 31. Track Mode (Burst-Inject + Persisted, Bus 1)

```
Command: "trackmode:on|off"
  │
  ├─ Gate: variant != LEGACY
  ├─ State: s.trackModeEnabled
  ├─ controlTrackMode(enabled, s) → startBurst(s, f, BUS_VEHICLE, 20, 20)
  └─ saveSettings(s)

Notes:
  - Uses CAN_ID_TRACK_MODE (0x313), zeroed frame (not cached).
  - Persists across reboot but does NOT auto-send on boot.
```

---

## 32. Turn Signals (Burst-Inject, Bus 1)

```
Command: "turn:left3|right3|hazard|off"
  │
  ├─ Gate: variant != LEGACY
  └─ controlTurnSignal(action, s):
      ├─ Build frame: CAN_ID_VCFRONT_LIGHTS (0x3F5)
      ├─ 3-blink: sends 3-frame burst at 100ms intervals
      └─ startBurst(s, f, BUS_VEHICLE, 20, 100)
```

---

## 33. Seatbelt Emulation (Tick-Inject, Bus 1)

```
Command: "seatbelt:on|off"
  │
  ├─ State: s.seatbeltEmulation
  └─ saveSettings(s)

CAN Path (seatbeltTick):
  Periodically injects CAN_ID_VCRIGHT_SEATBELT (0x3F3) with all 3 rear
  seatbelts reported as buckled. Suppresses rear seatbelt warnings.
```

---

## 34. Air Recirculation (Burst-Inject, Bus 1)

```
Command: "airecirc:on|off"
  │
  ├─ Gate: hasClimate
  └─ controlAirRecirc(enable, s.lastCharge, s):
      ├─ Build frame: CAN_ID_AIR_RECIRC (0x2AA)
      ├─ setAirRecircRequest(f, enable)  // byte[0] bit 0
      └─ startBurst(s, f, BUS_VEHICLE, 20, 20)

Notes:
  - Momentary action — not persisted.
  - Requires hasClimate (cached climate frame from bus 1).
```

---

## 35. Ban Shield (Config-Only)

```
Command: "banshield:on|off"
  │
  ├─ Gate: none (always available, any variant)
  ├─ State: s.banShieldEnabled
  ├─ If ON: reset s.banThreatLevel=0, s.banDetectionCount=0
  └─ saveSettings(s)

Notes:
  - Telemetry monitoring feature — intentionally ungated.
  - No CAN interaction; only affects state analysis.
  - Monitors for UDS negative responses (0x7FE), security access (0x27XX),
    unusual OTA/telemetry frame bursts.
```

---

## 36. Ban Detect — GTW Autopilot Tier Monitor (Read-Only, Bus 1)

```
No explicit command (automatic, feeds Ban Shield)

Decodes:
  Bus 1 → CAN_ID_GTW_AUTOPILOT_TIER (0x7FF), mux==2
    ├─ byte[5] bits[4:2] = autopilot tier
    ├─ 0=NONE, 1=HIGHWAY, 2=ENHANCED, 3=SELF_DRIVING, 4=BASIC (downgrade)
    └─ s.gtwAutopilotTier = tier

Notes:
  - BASIC tier detected as potential ban/downgrade event.
  - Exposed in status payload as "gtwAutopilotTier" (-1 when unknown).
```

---

## 37. CAN Clock (Config-Only)

```
Command: "canclock:auto|8|12|16|20"
  │
  ├─ Gate: none
  ├─ State: s.canClockReqMHz = N (0 = auto)
  └─ saveSettings(s)
```

---

## 38. Variant Selection (Config + Filter)

```
Command: "variant:hw3|hw4|legacy"
  │
  ├─ State: s.variant = V, s.variantAutoDetect = false
  ├─ saveSettings(s)
  └─ applyFilters(s)

Command: "variant:auto"
  │
  ├─ State: s.variantAutoDetect = true
  ├─ saveSettings(s)
  └─ applyFilters(s)

Auto-detection (in handleMessage):
  Bus 1 → CAN_ID_GTW_CAR_CFG (0x398)
    ├─ hw = byte[0] >> 6
    ├─ hw==3 → HW4, hw==2 → HW3
    ├─ If variantAutoDetect && detected != current:
    │   ├─ s.variant = detected
    │   ├─ applyFilters(s)
    │   └─ resetHandlerLogFlags()
    └─ Always stores s.detectedHW, s.hwAutoDetected
```

---

## 39. Stream (Config-Only)

```
Command: "stream:on|off"
  │
  ├─ Gate: none
  ├─ State: s.streamEnabled, s.streamCount reset
  └─ (no save, no filters — enables serial output of CAN frames)
```

---

## 40. CAN Raw (Config + Filter)

```
Command: "can:raw:on|off"
  │
  ├─ Gate: none
  ├─ State: s.rawCanListen
  └─ applyFilters(s)   ← opens all filters (pass all CAN IDs)

Notes:
  - No saveSettings — intentionally transient (raw mode for debugging).
```

---

## 41. CAN Simulation (Config-Only)

```
Command: "simu:start|stop"
  │
  ├─ Gate: none
  ├─ State: s.canSimEnabled
  └─ (no save, no filters)

CAN Path (when ON):
  Generates synthetic CAN frames for testing without a real vehicle:
    ├─ BMS: 375V, 5A, 70% SoC
    ├─ TPMS: 2.5 bar all tires
    ├─ Speed: 60 km/h
    └─ DI state: gear=D, pedal=15%

Notes:
  - Frames are NEVER transmitted on physical bus — internal decode pipeline only.
  - Does not require any active bus.
```

---

## 42. BMS Telemetry (Read-Only)

```
No command (always active when vehicle bus has frames)

CAN IDs decoded on Bus 1:
  ├─ 0x132: voltage, current, power, chargeTimeToFull
  ├─ 0x212: precondAllowed, heatingWorthwhile, contactorState, hvState
  ├─ 0x252: maxRegenPower, maxDischargePower, stationaryHeatPower, hvacPowerBudget
  ├─ 0x292: SOC, SOC_UI, SOC_max, SOC_avg, initialFullPack
  ├─ 0x2D2: minBusVoltage, maxBusVoltage, maxChargeCurrent, maxDischargeCurrent
  ├─ 0x312: tempMin, tempMax, powerDissipation, packTMin, packTMax
  ├─ 0x332: mux=0 thermistorT, modelT; mux=1 cellVoltageMax/Min
  ├─ 0x33A: whPerKm, expectedRange, idealRange, ratedConsumption
  ├─ 0x352: mux=0 nominalFull/remaining, idealRemaining; mux=1 energyBuffer
  ├─ 0x3D2: kwhDischargeTotal, kwhChargeTotal
  └─ 0x3F2: mux 0-3 acCharge/dcCharge/regen/driveDischarge totals

Sets s.hasBms and s.hasEnhancedBms flags. No injection.
```

---

## 43. TPMS (Read-Only)

```
Command: "tpms"  (query triggers response)

Decoded from:
  Bus 1 → CAN_ID_TPMS (0x219)
    ├─ data[0-3]: FL/FR/RL/RR pressure × 0.025 bar
    ├─ data[4-7]: FL/FR/RL/RR temperature − 40°C
    └─ Sets s.hasTpms, s.tpmsPressure[], s.tpmsTemp[]
```

---

## 44. Powertrain Telemetry (Read-Only)

```
Command: "powertrain"  (query triggers response)

Decoded from multiple Bus 1 CAN IDs:
  ├─ 0x257: vehicle speed (km/h × 100, signed)
  ├─ 0x118: gear (P/R/N/D), accelerator pedal (0-100%)
  ├─ 0x129: steering angle (degrees × 10, signed)
  ├─ 0x106: rear motor RPM
  └─ 0x115: front motor RPM (0 if single motor)
```

---

## 45. Motor Temperatures (Read-Only)

```
No explicit command (decoded from Bus 1 when available)

Decoded from:
  ├─ 0x315: rear inverter temp, rear stator temp
  └─ 0x376: front inverter temp, front stator temp (dual motor only)

Range: -40 to +215 °C (raw byte − 40 offset).
```

---

## 46. Wheel Speeds (Read-Only)

```
No explicit command (decoded from Bus 0 when available)

Decoded from:
  Bus 0 → CAN_ID_WHEEL_SPEED (0x175)
    ├─ FL/FR/RL/RR: 13-bit little-endian signals, 0.04 km/h per count
    └─ 8191 (0x1FFF) = signal not available

Used as safety input for DAS drive steering angle limiter.
```

---

## 47. Vehicle Config (Read-Only)

```
Command: "vehicle"  (query triggers response)

Decoded from:
  Bus 1 → CAN_ID_GTW_CAR_CFG (0x398)
    ├─ byte[1] bits[7:4]: platform ID (1=M3, 2=MY, 3=MS, 4=MX, 5=CT)
    └─ byte[2]: model year offset
```

---

## 48. Firmware Version Compatibility (Read-Only)

```
Command: "fwcompat"  (query triggers response)

Decoded from:
  Bus 1 → CAN_ID_GTW_VERSION (0x392), multiplexed:
    ├─ Mux 0: year (bytes 1-2), release (byte 3), minor (byte 4)
    ├─ Mux 1: build number (bytes 1-4, uint32)
    └─ Compatibility: OK (year ≥ 2024), WARN (older), FAIL (incompatible)
```

---

## 49. MQTT Telemetry Bridge (Config + WiFi)

```
Command: "mqtt:on|off"
  │
  ├─ State: s.mqttEnabled
  └─ saveSettings(s)

Command: "mqtt:broker:<host>"  (1-63 chars)
  │
  ├─ State: s.mqttHost
  └─ saveSettings(s)

Command: "mqtt:port:N"  (1-65535)
  │
  ├─ State: s.mqttPort = N
  └─ saveSettings(s)

Command: "mqtt:interval:N"  (500-60000 ms)
  │
  ├─ State: s.mqttInterval = N
  └─ saveSettings(s)

CAN Path: None (publishes telemetry snapshots via WiFi to MQTT broker).
Requires WiFi to be enabled (BOARD_ENABLE_WIFI).
```

---

## 50. Single-Shot TX Mode (Config-Only)

```
Command: "singleshot:on|off"
  │
  ├─ State: s.singleShotTx
  └─ saveSettings(s)

Notes:
  - Uses MCP2515 CANCTRL OSM bit to disable auto-retransmit.
  - Useful on noisy buses where retransmits cause frame storms.
  - No CAN interaction — changes MCP2515 register config.
```

---

## 51. DAS Drive (Tick-Inject, Bus 0)

```
Command: "drive:on|off"
  │
  ├─ Gate: features().dasDrive
  ├─ State: s.dasDriveEnabled
  ├─ If OFF: dasSendCancelBurst() — 5× DAS_ACC_CANCEL frames
  └─ saveSettings(s)

Command: "drive:speed:N" / "drive:cap:N"
  │
  ├─ State: s.dasSpeedLimitKph / s.dasSpeedCapKph
  └─ dasSaveNvs() (separate NVS namespace "tcm_das")

CAN Path (dasTick, called from main loop):
  Every 20ms while dasDriveEnabled:
    ├─ Gate: !txPaused && dasDriveEnabled
    ├─ Bus 0:
    │   ├─ 0x2B9 @ 25 Hz — DAS_control (longitudinal ACC, signed speed)
    │   ├─ 0x488 @ 50 Hz — DAS_steeringControl (Motorola mixed-endian angle)
    │   └─ 0x27D @ 10 Hz — APS_eacMonitor (EPAS steer-allow gate, CRC-8)
    ├─ Safety gates:
    │   ├─ Accel clamped to [-3.48, +2.0] m/s²
    │   ├─ Jerk limit ±4.9 m/s³
    │   ├─ Steering angle rate ≤ 5°/20ms, speed-aware lateral clamp
    │   └─ Standstill: brake hold at -0.4 m/s²
    ├─ Dead-man: frames stop if dasSetControl() not called within 150ms
    └─ Cancel: 5× DAS_ACC_CANCEL burst on disable

Notes:
  - HW4 uses LANE_KEEP_ASSIST (2), HW3/Legacy use ANGLE_CONTROL (1).
  - Gamepad sticks mapped via gamepad state → dasSetControl().
  - Default speed cap 25 km/h, absolute max 200 km/h.
```

---

## 52. TLSSC Restore (Toggle-Inject, Bus 0)

```
Command: "tlssc:on|off"
  │
  ├─ State: s.tlsscRestore
  └─ saveSettings(s)

CAN Path (when ON):
  Spoofs DAS_autopilotConfig (0x331) on Bus 0 to report SELF_DRIVING tier,
  restoring Tesla Licensed Self-Steering Capability.
```

---

## 53. OTA Safety (Read-Only, Auto-Pause)

```
No command (automatic)

CAN Path:
  Bus 1 → CAN_ID_GTW_CAR_STATE (0x318)
    ├─ byte[6] bits[1:0] = GTW_updateInProgress
    ├─ If becomes true:  s.otaInProgress=true, s.txPaused=true
    └─ If becomes false: s.otaInProgress=false, s.txPaused=false

Effect: txPaused gates ALL transmission (see Transmission Safety table above).
```

---

## Pattern Compliance Matrix

| #   | Feature           | Gate                           | State | Save | Filters    | LogReset | Bus   |
| --- | ----------------- | ------------------------------ | ----- | ---- | ---------- | -------- | ----- |
| 1   | fsd               | features().fsd                 | ✅    | ✅   | ✅         | ✅       | 0     |
| 2   | fsd:force         | features().fsdForce            | ✅    | ✅   | —          | ✅       | —     |
| 3   | nag (all modes)   | features().nag                 | ✅    | ✅   | ✅ (bit19) | ✅       | 0 / 1 |
| 4   | isa-chime         | HW4 + features().isaChime      | ✅    | ✅   | ✅         | ✅       | 0     |
| 5   | profile           | features().profile             | ✅    | ✅   | —          | ✅       | —     |
| 6   | offset            | features().offset              | ✅    | ✅   | —          | ✅       | —     |
| 7   | eceR79            | features().eceR79 + isEU       | ✅    | ✅   | ✅         | ✅       | 0     |
| 8   | region spoof      | features().regionSpoof         | ✅    | ✅   | ✅         | —        | 0     |
| 9   | auto lane change  | features().autoLaneChange      | ✅    | ✅   | —          | ✅       | 0     |
| 10  | summon            | features().summon + hasCtrl    | ✅    | —    | —          | —        | 1     |
| 11  | precondition      | variant != LEGACY              | ✅    | ✅   | —          | —        | 1     |
| 12  | sentry            | variant != LEGACY              | —     | —    | —          | —        | 2     |
| 13  | window            | variant != LEGACY              | —     | —    | —          | —        | 2     |
| 14  | trunk/frunk/glove | variant != LEGACY              | —     | —    | —          | —        | 1 / 2 |
| 15  | charge            | variant != LEGACY + hasCharge  | —     | —    | —          | —        | 1     |
| 16  | climate           | variant != LEGACY + hasClimate | —     | —    | —          | —        | 1     |
| 17  | pedal             | variant != LEGACY + hasDrive   | —     | —    | —          | —        | 1     |
| 18  | regen             | variant != LEGACY + hasDrive   | —     | —    | —          | —        | 1     |
| 19  | stop              | variant != LEGACY + hasDrive   | —     | —    | —          | —        | 1     |
| 20  | drive mode        | variant != LEGACY              | ✅    | ✅   | —          | —        | 1     |
| 21  | drive context     | none (always active)           | —     | —    | —          | —        | 0 / 1 |
| 22  | lock/horn         | hasCtrl                        | —     | —    | —          | —        | 1     |
| 23  | light             | hasCtrl                        | —     | —    | —          | —        | 1     |
| 24  | mirror            | hasCtrl                        | —     | —    | —          | —        | 1     |
| 25  | mirror autofold   | none                           | ✅    | ✅   | —          | —        | 1     |
| 26  | seat              | hasCtrl                        | —     | —    | —          | —        | 1     |
| 27  | wiper             | hasCtrl                        | —     | —    | —          | —        | 1     |
| 28  | wiper persist     | none                           | ✅    | ✅   | —          | —        | 1     |
| 29  | display           | hasCtrl                        | —     | —    | —          | —        | 1     |
| 30  | power             | hasCtrl                        | —     | —    | —          | —        | 1     |
| 31  | trackmode         | variant != LEGACY              | ✅    | ✅   | —          | —        | 1     |
| 32  | turn signals      | variant != LEGACY              | —     | —    | —          | —        | 1     |
| 33  | seatbelt          | none                           | ✅    | ✅   | —          | —        | 1     |
| 34  | air recirc        | hasClimate                     | —     | —    | —          | —        | 1     |
| 35  | banshield         | none                           | ✅    | ✅   | —          | —        | —     |
| 36  | ban detect        | none (automatic)               | —     | —    | —          | —        | 1     |
| 37  | canclock          | none                           | ✅    | ✅   | —          | —        | —     |
| 38  | variant           | none                           | ✅    | ✅   | ✅         | —        | —     |
| 39  | stream            | none                           | ✅    | —    | —          | —        | —     |
| 40  | can:raw           | none                           | ✅    | —    | ✅         | —        | —     |
| 41  | can sim           | none                           | ✅    | —    | —          | —        | —     |
| 42  | bms telemetry     | none (automatic)               | —     | —    | —          | —        | 1     |
| 43  | tpms              | none (automatic + query)       | —     | —    | —          | —        | 1     |
| 44  | powertrain        | none (query)                   | —     | —    | —          | —        | 1     |
| 45  | motor temps       | none (automatic)               | —     | —    | —          | —        | 1     |
| 46  | wheel speeds      | none (automatic)               | —     | —    | —          | —        | 0     |
| 47  | vehicle config    | none (query)                   | —     | —    | —          | —        | 1     |
| 48  | fw compat         | none (query)                   | —     | —    | —          | —        | 1     |
| 49  | mqtt bridge       | none                           | ✅    | ✅   | —          | —        | —     |
| 50  | single-shot       | none                           | ✅    | ✅   | —          | —        | —     |
| 51  | das drive         | features().dasDrive            | ✅    | ✅   | —          | —        | 0     |
| 52  | tlssc             | none                           | ✅    | ✅   | —          | —        | 0     |
| 53  | ota safety        | none (automatic)               | —     | —    | —          | —        | 1     |

---

## File Map

All features live in `firmware/lib/vehicle/can/feature/` as flat header-only files.
After restructuring (Phase 2), features will be grouped into subdirectories:

```
feature/
  fsd/        fsd.h, nag.h, offsets.h, profile.h, isa_chime.h,
              region.h, ece_r79.h, auto_lane_change.h
  comfort/    climate.h, seat.h, wiper.h, light.h, display.h,
              air_recirc.h, seatbelt.h, precondition.h, wiper_persist.h
  drive/      pedal.h, regen.h, stop.h, drive_mode.h, drive_context.h
  body/       lock.h, mirror.h, window.h, trunk.h, power.h,
              sentry.h, track_mode.h, horn.h, turn_signal.h,
              mirror_autofold.h, charge.h, glovebox.h
  telemetry/  bms.h, tpms.h, powertrain.h, motor_temps.h,
              wheel_speeds.h, vehicle_config.h, fw_compat.h, platform.h
  safety/     ban_shield.h, ban_detect.h, ap_gate.h, gtw_shield.h, single_shot.h
  das/        das_drive.h, tlssc.h
  misc/       can_clock.h, can_raw.h, can_sim.h, stream.h, variant.h, mqtt_bridge.h
```

Shared infrastructure:

```plaintext
vehicle/can/burst.h      — startBurst() non-blocking frame repeater
vehicle/can/ids.h        — bus constants, CAN IDs, frame helpers, driveChecksum()
vehicle/can/crc8.h       — CRC-8/OPENSAFETY with per-ID magic tables
core/util/parse.h        — parseBoolCmd() shared command parser

core/types.h             — State, Frame, Features structs
core/forward.h           — forward declares saveSettings, resetHandlerLogFlags, applyFilters, driverSend

vehicle/can/handler/variant/hw4.h    — HW4 FSD mux handler (mux 0/1/2 + ISA speed + follow dist)
vehicle/can/handler/variant/hw3.h    — HW3 FSD mux handler (mux 0/1/2 + follow dist)
vehicle/can/handler/variant/legacy.h — Legacy FSD handler (mux 0/1 + stalk profile)
vehicle/can/handler/dispatch.h       — main loop dispatch (summonTick, preconditionTick, burstTick, dasTick, nag echo)
```
