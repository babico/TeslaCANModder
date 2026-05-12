---
title: linuchoicoegwangsu / soft-nag-bypass
description: Local research documents (no upstream remote) describing a full state-machine EPAS torque injection algorithm. Two guide versions (v1/v2) plus a C++ core snippet. The most detailed public specification of DAS-aware organic torque injection for Tesla nag suppression.
category: legacy
folder: legacy
tags: [legacy, community, external]
author: linuchoicoegwangsu
repo: soft-nag-bypass
---

# linuchoicoegwangsu / soft-nag-bypass

## Overview

Local research documents — no upstream git remote. Three files describing a full state-machine EPAS torque injection algorithm for Tesla nag suppression. This is the most detailed public specification of DAS-aware organic torque injection, including a complete C++ core snippet with all state variables, transition logic, and torque generation functions.

Files:
- `STEERING_TORQUE_INJECTION_GUIDE-v1.md` — original guide (3 states, simpler)
- `STEERING_TORQUE_INJECTION_GUIDE-v2.md` — revised guide (states 1/2/3-4-5, more nuanced)
- `STEERING_TORQUE_CORE_SNIPPET.md` — complete C++ implementation extract

## Signal Requirements

All three documents agree on the required CAN signals:

| Signal | CAN ID | DBC | Usage |
|--------|--------|-----|-------|
| `DAS_autopilotState` | 923 | — | Global injection gate (allow only states 3-6) |
| `DAS_autopilotHandsOnState` | 923 | — | State machine selector |
| `EPAS3P_torsionBarTorque` | 880 | 19\|12@0+ | Torque injection target |
| `EPAS3P_handsOnLevel` | 880 | 39\|2@0+ | HandsOnLevel injection + driver bypass |
| `SCCM_steeringAngle` | 297 | 16\|14@1+ (0.1,-819.2) | Torque direction |

Torque raw encoding: `raw = 2048 + torqueNm × 100` (center 2048, scale 0.01 Nm/LSB).

## State Machine (v2 — authoritative)

### Global enable conditions

```
torqueOverrideEnabled == true
DAS_autopilotState in {3, 4, 5, 6}
DAS_autopilotHandsOnState not in {0, 8, 15}
checksum/counter validation passes
[optional] incoming EPAS3P_handsOnLevel == 0  (driver bypass)
```

### State 0, 8, 15 — no injection

Pass through original torque and HandsOnLevel.

### State 1 — idle with grace hold

No injection. Exception: if transitioning down from state 2 or 3-5, hold the last generated torque and HandsOnLevel for **500 ms** to avoid abrupt cutoff.

```
if now - state1EnterTime < 500ms:
    torque = lastGeneratedTorque
    HandsOnLevel = lastSpoofedHandsOnLevel
else:
    torque = original
    HandsOnLevel = original
```

### State 2 — mild organic torque

**2-second initial pause** (pass through original), then random-walk torque:

```
Direction: opposite to steeringAngle sign
Range:     ±0.5 Nm to ±2.0 Nm
Method:    persistent random walk, step ±12 raw units per frame
```

HandsOnLevel from torque magnitude:
```
|torque| >= 2.0 Nm → level 2
|torque| >= 1.0 Nm → level 1
else              → level 0
```

When level first reaches 2: hold current torque and level=2 for **1000 ms**.

### States 3, 4, 5 — strong ramp-and-hold

Treated as one group. Entering the group (from outside) starts a **1-second pause**, then:

```
cycle = 1500 ms
phase = activeMs % 1500

if phase < 500ms:  magnitude ramps 0.0 → 2.1 Nm
else:              magnitude holds at 2.1 Nm

torque = ±magnitude (sign opposite to steeringAngle)
```

HandsOnLevel: same magnitude thresholds as state 2. Moving between 3/4/5 does not reset the timer.

## State Transition Memory

```cpp
uint8_t  lastDasHandsOnState
uint32_t lastGeneratedTorqueRaw   // center 2048
uint8_t  lastSpoofedHandsOnLevel

// State 1
unsigned long state1EnterTime
uint32_t      state1HoldTorqueRaw
uint8_t       state1HoldHandsLevel

// State 2
unsigned long state2EnterTime
unsigned long state2HoldUntilMs
uint32_t      state2HoldTorqueRaw
uint8_t       state2HoldHandsLevel
bool          state2Level2WasActive

// States 3/4/5
unsigned long strongStateEnterTime
```

## C++ Core Snippet Key Functions

From `STEERING_TORQUE_CORE_SNIPPET.md`:

| Function | Purpose |
|----------|---------|
| `onDasAutopilotState(val)` | Update global AP state gate |
| `onDasHandsOnState(val)` | State transition logic, timer resets |
| `patchHandsOnLevel(originalRaw)` | Compute spoofed HandsOnLevel for current state |
| `generateOrganicTorque(originalRaw)` | Compute synthetic torque raw value |
| `observeSccmSteeringAngle(val)` | Decode steering angle: `deg = val × 0.1 - 819.2` |
| `observeEpasHandsOnLevel*(val)` | Track real driver hands-on for bypass |

The snippet uses `FeaturesController::` class scope and `millis()` for timing — directly portable to our ESP32 firmware with minor adaptation.

## Comparison with Our nag.h

| Aspect | linuchoicoegwangsu v2 | Our nag.h natural mode |
|--------|----------------------|------------------------|
| State machine | Full 5-state (0,1,2,3-5,8,15) | 3-mode (legacy/safe/natural) |
| State 1 grace hold | 500 ms | Not implemented |
| State 2 initial pause | 2000 ms | Not implemented |
| State 2 torque range | ±0.5–2.0 Nm | 0.08–0.18 Nm |
| State 2 hold at level 2 | 1000 ms latch | Not implemented |
| States 3-5 initial pause | 1000 ms | Not implemented |
| States 3-5 ramp-and-hold | 0→2.1 Nm over 500 ms, hold 1000 ms | Not implemented |
| Torque direction | Opposite to steering angle | Opposite to steering angle ✓ |
| Driver bypass | Incoming HandsOnLevel != 0 | Not implemented |
| AP state gate | DAS_autopilotState in {3,4,5,6} | DAS_autopilotHandsOnState != 0,8 |
| Steering angle gate | |angle| > 5° stops injection (v1) / disabled (v2) | Proportional bias only |

## Relevance to Our Project

This is the most actionable research document for improving our nag suppression. The C++ snippet is nearly drop-in portable.

- **Reusability**: Very high — C++ snippet is directly portable
- **Key Takeaways**:
  - Our natural mode torque range (0.08–0.18 Nm) is far too small — effective range is 0.5–2.1 Nm
  - The state machine (pause → mild → strong) is the correct architecture for evading DAS heuristics
  - State 1 grace hold prevents abrupt torque cutoff that DAS can detect
  - State 2 level-2 hold prevents threshold chatter
  - States 3-5 ramp-and-hold is the pattern for escalated nag states
  - Driver bypass (real HandsOnLevel != 0) is a safety feature worth implementing
  - The `generateOrganicTorque` + `patchHandsOnLevel` split is the right separation of concerns
