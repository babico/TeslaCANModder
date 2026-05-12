# Steering Torque Injection Logic Porting Guide

## 1. Overview

This document describes only the steering torque and EPAS HandsOnLevel injection strategy. It intentionally does not explain the full CAN transport, BLE dashboard, or unrelated feature toggles.

The current logic reads Autopilot demand state, EPAS hands-on feedback, and steering angle. When the vehicle state allows it, it injects a synthetic EPAS torsion-bar torque and a matching EPAS HandsOnLevel. The intent is not to apply a constant force. The logic creates pauses, mild assistance, stronger ramp-and-hold pulses, and driver-bypass behavior so the output is less mechanical.

Current behavior summary:

1. Torque injection is allowed only when torque override is ON and Autopilot state is in the active range.
2. HandsOnState `1` keeps the last injected value briefly, then stops injection.
3. HandsOnState `2` waits 2 seconds, then applies mild organic torque.
4. HandsOnState `3`, `4`, or `5` waits 1 second, then applies stronger ramp-and-hold torque.
5. If real incoming EPAS HandsOnLevel is non-zero and driver-feedback bypass is enabled, injection stops and original values pass through.
6. Torque direction is selected opposite to the current steering angle sign.
7. Counter/checksum handling is done after signal changes; checksum failure must prevent injection.

Items 2 and 3 are mandatory behavior for this strategy. The other items are recommended implementation details and may be adapted after safety review.

## 2. Required Signals

### DAS Autopilot State

```text
CAN: Private CAN
Message ID: 923
Signal: DAS_autopilotState
Usage: Global allow/deny gate for torque injection.
```

Injection is allowed only when `DAS_autopilotState` is one of:

```text
3, 4, 5, 6
```

For all other values, pass through original torque and original EPAS HandsOnLevel.

### DAS Hands-On Demand State

```text
CAN: Private CAN
Message ID: 923
Signal: DAS_autopilotHandsOnState
Usage: Select the injection pattern.
```

This value chooses the state `1`, `2`, or `3/4/5` behavior described below.

### EPAS Torsion Bar Torque

Default path:

```text
CAN: Private CAN
Message ID: 880
Signal: EPAS3P_torsionBarTorque
DBC: 19|12@0+
Usage: Main torque injection target.
```

Optional legacy/test path still exists in code:

```text
CAN: Chassis CAN
Message ID: 82
Signal: EPAS3P_torsionBarTorque
DBC: 19|12@0+
Usage: Optional alternate EPAS frame path.
```

Current raw center:

```text
2048
```

Conversion model:

```text
raw = 2048 + torqueNm * 100
torqueNm = (raw - 2048) * 0.01
```

Examples:

```text
+0.5 Nm -> raw 2098
+2.0 Nm -> raw 2248
+2.1 Nm -> raw 2258
-0.5 Nm -> raw 1998
-2.0 Nm -> raw 1848
-2.1 Nm -> raw 1838
```

### EPAS HandsOnLevel

Default path:

```text
CAN: Private CAN
Message ID: 880
Signal: EPAS3P_handsOnLevel
DBC: 39|2@0+
Usage: HandsOnLevel injection target and driver-bypass feedback.
```

Optional legacy/test path:

```text
CAN: Chassis CAN
Message ID: 82
Signal: EPAS3P_handsOnLevel
DBC: 39|2@0+
```

Incoming `EPAS3P_handsOnLevel` can also be observed. If a project implements driver-feedback bypass and the incoming value is non-zero, the driver should be assumed to be steering and injection should be bypassed.

### Steering Angle

Current implementation uses:

```text
CAN: Private CAN
Message ID: 297
Signal: SCCM_steeringAngle
DBC: 16|14@1+ (0.1,-819.2)
Usage: Decide injected torque direction.
```

Conversion:

```text
angleDeg = raw * 0.1 - 819.2
```

Direction rule:

```text
angleDeg > 0  -> inject negative torque
angleDeg <= 0 -> inject positive torque
```

There is a retained safety gate for pausing torque above a steering-angle threshold, but it is currently disabled in code. Do not assume angle-based pause is active unless explicitly re-enabled.

## 3. Global Enable Conditions

Injection is allowed only when all conditions are true:

```text
Torque Override == ON
DAS_autopilotState in [3, 4, 5, 6]
HandsOnState is not 0, 8, or 15
Checksum/counter validation allows patching on the target frame
```

If the target project implements driver-feedback bypass, also require:

```text
incoming EPAS3P_handsOnLevel == 0
```

If this condition fails, pass through original torque and original EPAS HandsOnLevel.

## 4. HandsOnState Logic

### State 0, 8, 15

Do not inject.

```text
torque = original
EPAS_handsOnLevel = original
```

### State 1

When entering state `1`, the current implementation keeps the most recent generated torque and spoofed HandsOnLevel for 500 ms. After that short grace period, injection stops.

State `1` itself is intended to be idle. The 500 ms grace period exists only to avoid an abrupt cutoff when torque was already being injected in state `2` or a stronger state and the demand drops to `1`.

```text
if now - state1EnterTime < 500ms:
    torque = lastGeneratedTorque
    EPAS_handsOnLevel = lastSpoofedHandsOnLevel
else:
    torque = original
    EPAS_handsOnLevel = original
```

Intent:

```text
When no hands-on demand is active, remove injected torque and create an idle period.
```

### State 2

When entering state `2`, pause injection for 2 seconds.

```text
if now - state2EnterTime < 2000ms:
    torque = original
    EPAS_handsOnLevel = original
```

After the 2 second delay, apply mild organic torque.

Torque range:

```text
if steeringAngle > 0:
    torque range = -0.5 Nm to -2.0 Nm
else:
    torque range = +0.5 Nm to +2.0 Nm
```

Implementation style:

```text
Keep a persistent raw torque value.
Move it by a small random-walk step.
Clamp it inside the selected range.
```

HandsOnLevel:

```text
if abs(torqueNm) >= 2.0:
    EPAS_handsOnLevel = 2
elif abs(torqueNm) >= 1.0:
    EPAS_handsOnLevel = 1
else:
    EPAS_handsOnLevel = 0
```

Additional state-2 hold behavior:

```text
When HandsOnLevel first reaches 2:
    hold the current torque and HandsOnLevel=2 for 1000ms
```

### States 3, 4, 5

States `3`, `4`, and `5` are treated as the same strong hands-on demand group.

When entering this group from outside the group, pause injection for 1 second.

```text
if now - strongStateEnterTime < 1000ms:
    torque = original
    EPAS_handsOnLevel = original
```

Moving between `3`, `4`, and `5` does not reset this timer. Leaving the group and re-entering starts a new cycle.

After the 1 second delay, apply a stronger ramp-and-hold torque in the direction opposite the steering angle.

Torque pattern:

```text
cycle = 1500ms
phase = activeMs % 1500

if phase < 500ms:
    magnitude ramps from 0.0 Nm to 2.1 Nm
else:
    magnitude holds at 2.1 Nm

if steeringAngle > 0:
    torque = -magnitude
else:
    torque = +magnitude
```

HandsOnLevel:

```text
if abs(torqueNm) >= 2.0:
    EPAS_handsOnLevel = 2
elif abs(torqueNm) >= 1.0:
    EPAS_handsOnLevel = 1
else:
    EPAS_handsOnLevel = 0
```

When the state leaves `3/4/5`, stop this strong pattern immediately and return to the current state's rule.

## 5. State Transition Memory

Store at least these values:

```text
lastDasHandsOnState
state1EnterTime
state2EnterTime
strongStateEnterTime
lastGeneratedTorqueRaw
lastSpoofedHandsOnLevel
state2HoldUntilTime
state2HoldTorqueRaw
state2HoldHandsLevel
```

Transition rules:

```text
if previousHandsOnState != 1 and currentHandsOnState == 1:
    state1EnterTime = now
    state1HoldTorque = lastGeneratedTorqueRaw
    state1HoldHandsLevel = lastSpoofedHandsOnLevel

if currentHandsOnState != 1:
    clear state1 memory

if previousHandsOnState != 2 and currentHandsOnState == 2:
    state2EnterTime = now

if currentHandsOnState != 2:
    clear state2 delay and hold memory

if previousHandsOnState not in [3,4,5] and currentHandsOnState in [3,4,5]:
    strongStateEnterTime = now

if currentHandsOnState not in [3,4,5]:
    clear strong-state memory
```

## 6. Implementation Direction For Coding Agents

1. Extract these signals from the CAN receive layer:

```text
DAS_autopilotState
DAS_autopilotHandsOnState
EPAS3P_torsionBarTorque
EPAS3P_handsOnLevel
SCCM_steeringAngle
```

2. Convert steering angle into a physical value and keep it as internal state:

```text
steeringAngleDeg = raw * 0.1 - 819.2
```

3. During each EPAS frame handling cycle, evaluate global injection conditions first:

```text
torqueOverrideEnabled
autopilotState in [3, 4, 5, 6]
handsOverrideBypass condition
checksum/counter validation
```

4. If any condition fails, return original torque and original HandsOnLevel.

5. If conditions pass, apply state `1`, `2`, or `3/4/5` logic based on `DAS_autopilotHandsOnState`.

6. Apply torque and HandsOnLevel changes to the same EPAS frame before transmission-layer finalization.

7. Let the transmission layer update counter/checksum after all signal changes are applied.

## 7. Important Notes

- State `1` is an idle/no-injection state. The 500 ms hold is only a transition cushion after state `2` or stronger states.
- State `2` must wait 2 seconds before injection starts.
- State `2` uses mild random-walk torque in the direction opposite the steering angle.
- States `3`, `4`, and `5` share the strong ramp-and-hold pattern.
- Strong demand waits 1 second, then ramps to 2.1 Nm over 0.5 seconds and holds for 1 second.
- If the target project observes real driver hands-on feedback, do not inject when the driver appears to be actively steering.
- Torque direction should oppose steering angle direction.
- If checksum validation fails on a frame, do not inject into that frame.
