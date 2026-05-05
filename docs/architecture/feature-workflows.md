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
| `nagKillerShouldEcho()` | Returns false                                     |

### Burst Architecture (`infra/burst.h`)

Non-blocking frame repeater. `startBurst(s, f, bus, count, delayMs)` stores a
single frame in State. `burstTick()` in the main loop drains it one frame per
interval. Only one burst active at a time — new burst overrides previous.

Used by: pedal, regen, stop, lock, light, mirror, seat, wiper, display, power,
charge, climate, sentry, window, trunk, track_mode.

NOT used by: summon (dynamic frame per tick), precondition (indefinite heartbeat).

### Variant-Aware Feature Gating

`features()` returns a `Features` struct based on `s.variant`:

| Feature  | HW4     | HW3     | Legacy |
| -------- | ------- | ------- | ------ |
| fsd      | yes     | yes     | yes    |
| fsdForce | yes     | yes     | yes    |
| offset   | **yes** | **yes** | no     |
| profile  | yes     | yes     | yes    |
| nag      | yes     | yes     | yes    |
| isaChime | **yes** | no      | no     |
| summon   | yes     | yes     | **no** |

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

## 3. Nag Suppress (Toggle-Inject, Bus 0)

```
Command: "nag:on|off"
  │
  ├─ Gate: features().nag
  ├─ State: s.nagSuppress
  ├─ resetHandlerLogFlags()
  ├─ saveSettings(s)
  └─ applyFilters(s)

CAN Path (when ON):
  Bus 0 → CAN_ID_FSD_MUX, mux==1
    ├─ HW4: setBit(19,false) setBit(47,true) → driverSend(f, 0)
    ├─ HW3: setBit(19,false)                 → driverSend(f, 0)
    └─ Legacy: setBit(19,false)              → driverSend(f, 0)

Notes:
  - HW4 sets extra bit 47 (HW4-specific nag bit). HW3/Legacy do not.
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

## 5. Nag Killer (Echo-Inject, Bus 1)

```
Command: "nag:killer:on|off"
  │
  ├─ Gate: features().nag
  ├─ State: s.nagKillerEnabled
  ├─ resetHandlerLogFlags()
  └─ saveSettings(s)

Command: "nag:killer:mode:safe|legacy"
  │
  ├─ Gate: features().nag
  ├─ State: s.nagKillerMode = NAG_KILLER_SAFE / NAG_KILLER_LEGACY
  └─ saveSettings(s)

CAN Path (when ON):
  Bus 1 → CAN_ID_EPAS_TORQUE (0x370)
    ├─ Always: decode steeringMode from byte[0] bits[7:4]
    ├─ Gate: !txPaused && nagKillerShouldEcho(s)
    │   ├─ LEGACY mode: always echo
    │   └─ SAFE mode: only if dasSeen && dasHandsOnRequested
    ├─ Clone frame → nagKillerModify(echo):
    │   ├─ counter = (byte[1] & 0x0F) + 1
    │   ├─ byte[2]=0, byte[3]=0 (zero torque)
    │   └─ byte[7] = nagKillerChecksum
    └─ driverSend(echo, BUS_VEHICLE)

Notes:
  - No applyFilters() — EPAS_TORQUE (0x370) is in the static vehicle bus filter.
  - DAS hands-on state from CAN_ID_DAS_STATUS (0x39B), always in vehicle filter.
  - SAFE mode stays silent until DAS_STATUS frame first arrives (dasSeen=false).
```

---

## 6. Speed Profile (Config-Only, applied by handlers)

```
Command: "profile:N" / "sp:N" (0-4)
  │
  ├─ Gate: features().profile
  ├─ State: s.speedProfile = N, s.profileOverride = true
  └─ saveSettings(s)

Command: "profile:auto|lock|unlock" / "sp:auto|lock|unlock"
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
  - HW3 auto-track produces 0-2 only. Values 3-4 are settable manually but never auto-tracked on HW3.
```

---

## 7. Speed Offset (Config-Only, applied by handlers)

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

## 8. Summon (Tick-Inject, Bus 1)

```
Command: "summon-inject:on|off|toggle"
  │
  ├─ Gate: features().summon
  ├─ State: s.summonInject
  ├─ If OFF: s.summonMode=STOP, s.summonRemaining=0
  ├─ resetHandlerLogFlags()
  └─ saveSettings(s)

Command: "summon:forward|fwd|reverse|rev"
  │
  ├─ Gate: features().summon + summonInject + hasCtrl
  ├─ State: s.summonDirection, s.summonMode=START, s.summonRemaining=30
  └─ (no save — transient action)

Command: "summon:stop"
  │
  ├─ Gate: features().summon
  └─ State: s.summonMode=STOP, s.summonRemaining=0

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

## 9. Preconditioning (Tick-Inject, Bus 1)

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

## 10. Sentry Mode (Burst-Inject, Bus 2)

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

## 11. Window Vent (Burst-Inject, Bus 2)

```
Command: "window:vent:open|close" / "vent:open|close"
  │
  ├─ Gate: variant != LEGACY
  └─ controlWindowVent(action, s):
      ├─ Build frame: CAN_ID_WINDOW_VENT (0x119)
      └─ startBurst(s, f, BUS_BODY, 30, 20)
```

---

## 12. Trunk / Frunk / Glovebox (Burst-Inject)

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

## 13. Charge Control (Burst-Inject, Bus 1)

```
Command: "charge:start|stop" / "charge:port" / "chargeport"
  │
  ├─ Gate: variant != LEGACY + hasCharge
  └─ controlCharge(action, s.lastCharge, s):
      ├─ Uses cached charge frame (CAN_ID_CHARGE, 0x333)
      └─ startBurst(s, f, BUS_VEHICLE, 20, 20)
```

---

## 14. Climate Control (Burst-Inject, Bus 1)

```
Command: "climate:keep|off"
  │
  ├─ Gate: variant != LEGACY + hasClimate
  └─ controlClimate(action, s.lastClimate, s):
      ├─ Uses cached climate frame (CAN_ID_CLIMATE, 0x2F3)
      └─ startBurst(s, f, BUS_VEHICLE, 30, 20)
```

---

## 15. Pedal Mode (Burst-Inject, Bus 1)

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

## 16. Regen Level (Burst-Inject, Bus 1)

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

## 17. Stop Mode (Burst-Inject, Bus 1)

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
  - All three share driveChecksum() from infra/can.h.
  - Require hasDrive flag (cached drive frame received from bus 1).
```

---

## 18. Lock / Unlock / Horn (Burst-Inject, Bus 1)

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

## 19. Lights (Burst-Inject, Bus 1)

```
Command: "light:fog:front|rear" / "light:highbeam:auto" / "light:ambient"
         "light:home" / "light:dome:off|on|auto"
  │
  ├─ Gate: hasCtrl
  └─ Various control functions using cached ctrl frame
      └─ startBurst(s, f, BUS_VEHICLE, 30, 20)
```

---

## 20. Mirror Control (Burst-Inject, Bus 1)

```
Command: "mirror:fold|unfold|heat|autofold|dip"
  │
  ├─ Gate: hasCtrl
  └─ controlMirrorFold/controlMirrorHeat/etc(s):
      └─ startBurst(s, f, BUS_VEHICLE, 50|30, 20)
```

---

## 21. Seat Heat (Burst-Inject, Bus 1)

```
Command: "seat:fl|fr|rl|rr|rc:0|1|2|3"
  │
  ├─ Gate: hasCtrl
  └─ controlSeatHeat(seat, level, s):
      └─ startBurst(s, f, BUS_VEHICLE, 30, 20)
```

---

## 22. Wiper Control (Burst-Inject, Bus 1)

```
Command: "wiper:off|1|2|3"
  │
  ├─ Gate: hasCtrl
  └─ controlWiper(level, s):
      └─ startBurst(s, f, BUS_VEHICLE, 20, 20)
```

---

## 23. Display Brightness (Burst-Inject, Bus 1)

```
Command: "maindisplay:N" (0-127)
  │
  ├─ Gate: hasCtrl
  └─ controlDisplayBrightness(level, s):
      └─ startBurst(s, f, BUS_VEHICLE, 20, 20)
```

---

## 24. Power Control (Burst-Inject, Bus 1)

```
Command: "power:acc:on|off" / "power:ready" / "power:off"
  │
  ├─ Gate: hasCtrl
  └─ controlAccessoryPower/controlDriveState/controlPowerOff(s):
      └─ startBurst(s, f, BUS_VEHICLE, 30, 20)

Notes (hasCtrl-gated features 18-24):
  - All use CAN_ID_UI_VEHICLE_CTRL (0x273) cached in s.lastCtrl.
  - No explicit variant gate — hasCtrl serves as effective gate.
    Legacy variants never receive 0x273, so hasCtrl stays false.
  - All use non-blocking startBurst().
```

---

## 25. Track Mode (Burst-Inject + Persisted, Bus 1)

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

## 26. Ban Shield (Config-Only)

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
```

---

## 27. CAN Clock (Config-Only)

```
Command: "canclock:auto|8|12|16|20"
  │
  ├─ Gate: none
  ├─ State: s.canClockReqMHz = N (0 = auto)
  └─ saveSettings(s)
```

---

## 28. Variant Selection (Config + Filter)

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

## 29. Stream (Config-Only)

```
Command: "stream:on|off|toggle"
  │
  ├─ Gate: none
  ├─ State: s.streamEnabled, s.streamCount reset
  └─ (no save, no filters — enables serial output of CAN frames)
```

---

## 30. CAN Raw (Config + Filter)

```
Command: "can:raw:on|off|toggle"
  │
  ├─ Gate: none
  ├─ State: s.rawCanListen
  └─ applyFilters(s)   ← opens all filters (pass all CAN IDs)

Notes:
  - No saveSettings — intentionally transient (raw mode for debugging).
```

---

## 31. BMS Telemetry (Read-Only)

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

## 32. OTA Safety (Read-Only, Auto-Pause)

```
No command (automatic)

CAN Path:
  Bus 1 → CAN_ID_GTW_CAR_STATE (0x318)
    ├─ byte[0] bit 0 = GTW_updateInProgress
    ├─ If becomes true:  s.otaInProgress=true, s.txPaused=true
    └─ If becomes false: s.otaInProgress=false, s.txPaused=false

Effect: txPaused gates ALL transmission (see Transmission Safety table above).
```

---

## Pattern Compliance Matrix

| #   | Feature           | Gate                            | State | Save | Filters    | LogReset | Bus |
| --- | ----------------- | ------------------------------- | ----- | ---- | ---------- | -------- | --- |
| 1   | fsd               | features().fsd                  | ✅    | ✅   | ✅         | ✅       | 0   |
| 2   | fsd:force         | features().fsdForce             | ✅    | ✅   | —          | ✅       | —   |
| 3   | nag               | features().nag                  | ✅    | ✅   | ✅         | ✅       | 0   |
| 4   | isa-chime         | HW4 + features().isaChime       | ✅    | ✅   | ✅         | ✅       | 0   |
| 5   | nagkiller         | features().nag                  | ✅    | ✅   | — (static) | ✅       | 1   |
| 6   | profile           | features().profile              | ✅    | ✅   | —          | ✅       | —   |
| 7   | offset            | features().offset               | ✅    | ✅   | —          | ✅       | —   |
| 8   | summon-inject     | features().summon               | ✅    | ✅   | —          | ✅       | 1   |
| 8   | summon (trigger)  | summon + summonInject + hasCtrl | ✅    | —    | —          | —        | 1   |
| 9   | precondition      | variant != LEGACY               | ✅    | ✅   | —          | —        | 1   |
| 10  | sentry            | variant != LEGACY               | —     | —    | —          | —        | 2   |
| 11  | window            | variant != LEGACY               | —     | —    | —          | —        | 2   |
| 12  | trunk/frunk/glove | variant != LEGACY               | —     | —    | —          | —        | 1/2 |
| 13  | charge            | variant != LEGACY + hasCharge   | —     | —    | —          | —        | 1   |
| 14  | climate           | variant != LEGACY + hasClimate  | —     | —    | —          | —        | 1   |
| 15  | pedal             | variant != LEGACY + hasDrive    | —     | —    | —          | —        | 1   |
| 16  | regen             | variant != LEGACY + hasDrive    | —     | —    | —          | —        | 1   |
| 17  | stop              | variant != LEGACY + hasDrive    | —     | —    | —          | —        | 1   |
| 18  | lock/horn         | hasCtrl                         | —     | —    | —          | —        | 1   |
| 19  | light             | hasCtrl                         | —     | —    | —          | —        | 1   |
| 20  | mirror            | hasCtrl                         | —     | —    | —          | —        | 1   |
| 21  | seat              | hasCtrl                         | —     | —    | —          | —        | 1   |
| 22  | wiper             | hasCtrl                         | —     | —    | —          | —        | 1   |
| 23  | display           | hasCtrl                         | —     | —    | —          | —        | 1   |
| 24  | power             | hasCtrl                         | —     | —    | —          | —        | 1   |
| 25  | trackmode         | variant != LEGACY               | ✅    | ✅   | —          | —        | 1   |
| 26  | banshield         | none                            | ✅    | ✅   | —          | —        | —   |
| 27  | canclock          | none                            | ✅    | ✅   | —          | —        | —   |
| 28  | variant           | none                            | ✅    | ✅   | ✅         | —        | —   |
| 29  | stream            | none                            | ✅    | —    | —          | —        | —   |
| 30  | can:raw           | none                            | ✅    | —    | ✅         | —        | —   |

---

## File Map

All features live in `firmware/lib/feature/` as flat header-only files:

```
feature/
  ban_shield.h    bms.h          can_clock.h    can_raw.h
  charge.h        climate.h      display.h      fsd.h
  isa_chime.h     light.h        lock.h         mirror.h
  nag.h           offsets.h      pedal.h        power.h
  precondition.h  profile.h      regen.h        seat.h
  sentry.h        stop.h         stream.h       summon.h
  track_mode.h    trunk.h        variant.h      window.h
  wiper.h
```

Shared infrastructure:

```plaintext
infra/burst.h   — startBurst() non-blocking frame repeater
infra/can.h     — bus constants, CAN IDs, frame helpers, driveChecksum()
infra/parse.h   — parseBoolCmd() shared command parser

core/types.h    — State, Frame, Features structs
core/forward.h  — forward declares saveSettings, resetHandlerLogFlags, applyFilters, driverSend

handler/variant/hw4.h   — HW4 FSD mux handler (mux 0/1/2 + ISA speed + follow dist)
handler/variant/hw3.h   — HW3 FSD mux handler (mux 0/1/2 + follow dist)
handler/variant/legacy.h — Legacy FSD handler (mux 0/1 + stalk profile)
handler/dispatch/esp32.h — main loop dispatch (summonTick, preconditionTick, burstTick, nagKiller echo)
```
