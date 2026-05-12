# Steering Torque Injection Logic Porting Guide

## 1. Overview

This logic reads Autopilot hands-on demand state and injects EPAS torque plus EPAS HandsOnLevel when the vehicle state allows it.

The intent is not to apply constant force. The logic deliberately creates rest periods, then applies natural-looking torque depending on the requested hands-on state. If the driver is actually steering, or if the steering wheel is already turned too far, the logic stops injection and passes through the original values.

Current behavior:

1) Torque injection is allowed only while Autopilot state is in the active range.
2) If HandsOnState is `1`, injection stops. This is a required element.
3) If HandsOnState changes to `2`, injection pauses for 2 seconds, then applies mild torque. This is a required element.
4) If HandsOnState changes to `3`, injection pauses for 1 second, then applies a stronger sweep torque.
5) If steering angle absolute value exceeds 5 degrees, injection stops.
6) If real incoming EPAS HandsOnLevel is non-zero, the driver is assumed to be steering and injection can stop.
7) Injected torque direction is opposite to the current steering angle direction.

Items 2 and 3 are mandatory behavior. Items 1 and 4 through 7 are recommended behavior and may be adapted to the target project after safety review.

## 2. Required Signals

### DAS Autopilot State

```text
CAN: Private CAN
Message ID: 923
Signal: DAS_autopilotState
Usage: Decide whether torque injection is allowed.
```

Torque injection is allowed only when `DAS_autopilotState` is one of:

```text
3, 4, 5, 6
```

For all other values, pass through original torque and HandsOnLevel.

### DAS Hands On Demand State

```text
CAN: Private CAN
Message ID: 923
Signal: DAS_autopilotHandsOnState
Usage: Select torque injection pattern.
```

This state drives the state 1, 2, and 3 behavior described below.

### EPAS Torsion Bar Torque

```text
CAN: Private CAN
Message ID: 880
Signal: EPAS3P_torsionBarTorque
Usage: Main torque injection target.
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
-0.5 Nm -> raw 1998
-2.0 Nm -> raw 1848
```

### EPAS HandsOnLevel

```text
CAN: Private CAN
Message ID: 880
Signal: EPAS3P_handsOnLevel
Usage: HandsOnLevel injection target.
```

Also read incoming `EPAS3P_handsOnLevel` to detect whether the driver is actually steering.

### Steering Angle

Current implementation uses:

```text
CAN: Private CAN test target
Message ID: 297
Signal: SCCM_steeringAngle
DBC: 16|14@1+ (0.1,-819.2)
Usage: Decide torque direction and safety pause.
```

Conversion:

```text
angleDeg = raw * 0.1 - 819.2
```

Rules:

```text
abs(angleDeg) > 5.0 -> stop injection
angleDeg > 0        -> use negative torque
angleDeg <= 0       -> use positive torque
```

## 3. Global Enable Conditions

Injection is allowed only when all conditions are true:

```text
Torque Override == ON
DAS_autopilotState in [3, 4, 5, 6]
abs(SCCM_steeringAngle) <= 5.0
```

If the optional `HANDS OVR` feature is ON, also require:

```text
incoming EPAS3P_handsOnLevel == 0
```

If incoming EPAS HandsOnLevel is non-zero, assume the driver is steering and pass through original torque and HandsOnLevel.

## 4. HandsOnState Logic

### State 0, 8, 15

Do not inject.

```text
torque = original
EPAS_handsOnLevel = original
```

### State 1

Do not inject.

```text
torque = original
EPAS_handsOnLevel = original
```

Intent:

```text
Create a period where no continuous steering force is applied.
```

### State 2

When entering state 2, pause injection for 2 seconds.

```text
if now - state2EnterTime < 2000ms:
    torque = original
    EPAS_handsOnLevel = original
```

If state 2 remains active after 2 seconds, apply mild organic torque.

Torque range:

```text
if steeringAngle > 0:
    torque range = -0.5 Nm to -2.0 Nm
else:
    torque range = +0.5 Nm to +2.0 Nm
```

Implementation style:

```text
Move with a small random walk inside the selected range.
Avoid sudden jumps by keeping the previous generated value and changing it gradually.
```

HandsOnLevel:

```text
if abs(torqueNm) >= 1.0:
    EPAS_handsOnLevel = 1
else:
    EPAS_handsOnLevel = 0
```

### State 3

When entering state 3, pause injection for 1 second.

```text
if now - state3EnterTime < 1000ms:
    torque = original
    EPAS_handsOnLevel = original
```

If state 3 remains active after 1 second, apply a strong sweep torque.

Torque pattern:

```text
-2.0 Nm -> +2.0 Nm: 0.5 seconds
+2.0 Nm -> -2.0 Nm: 0.5 seconds
```

This repeats as a 1 second cycle while state 3 remains active.

Example:

```text
activeMs = now - state3EnterTime - 1000
phase = activeMs % 1000

if phase < 500:
    torqueNm = -2.0 + (phase / 500.0) * 4.0
else:
    torqueNm = +2.0 - ((phase - 500) / 500.0) * 4.0
```

HandsOnLevel:

```text
if abs(torqueNm) > 1.5:
    EPAS_handsOnLevel = 2
elif abs(torqueNm) > 1.0:
    EPAS_handsOnLevel = 1
else:
    EPAS_handsOnLevel = 0
```

When state 3 ends, stop this sweep pattern immediately and return to the current state's rule.

## 5. State Transition Memory

Store at least these values:

```text
lastDasHandsOnState
state2EnterTime
state3EnterTime
lastGeneratedTorqueRaw
lastSpoofedHandsOnLevel
```

Transition rules:

```text
if previousHandsOnState != 2 and currentHandsOnState == 2:
    state2EnterTime = now

if currentHandsOnState != 2:
    state2EnterTime = 0

if previousHandsOnState != 3 and currentHandsOnState == 3:
    state3EnterTime = now

if currentHandsOnState != 3:
    state3EnterTime = 0
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

3. During each EPAS3P frame handling cycle, evaluate global injection conditions first:

```text
torqueOverrideEnabled
autopilotState in [3, 4, 5, 6]
abs(steeringAngleDeg) <= 5
handsOverrideBypass condition
```

4. If any condition fails, return original torque and original HandsOnLevel.

5. If conditions pass, apply the state 1, 2, or 3 logic based on `DAS_autopilotHandsOnState`.

6. Apply torque and HandsOnLevel changes to the same EPAS frame before transmission-layer finalization.

7. Let the transmission layer handle counter/checksum after all signal changes are applied.

## 7. Important Notes

- State 1 is not a mild injection state. It is a no-injection state.
- State 2 must wait 2 seconds before injection starts.
- State 3 must wait 1 second before injection starts.
- Do not inject when steering angle exceeds 5 degrees.
- Do not inject when the driver appears to be actively steering.
- Torque direction should oppose steering angle direction.
- If checksum validation fails on a frame, do not inject into that frame.
