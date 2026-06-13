# DAS Drive — Gamepad CAN Injection

> **TL;DR** — DAS Drive lets a Bluetooth gamepad drive the car by _injecting
> openpilot-shaped DAS frames_ on the autopilot party CAN. It is **not**
> Autopilot, EAP, FSD or openpilot. There is no perception, no planning, no
> path. The sticks become the actuators.

## What it actually is

The firmware module `firmware/lib/vehicle/can/feature/das/das_drive.h` emits the
same three CAN frames that openpilot's Tesla port uses to command the car:

| ID      | Frame                 | Rate  | Purpose                         |
| ------- | --------------------- | ----- | ------------------------------- |
| `0x2B9` | `DAS_control`         | 25 Hz | Longitudinal accel/jerk request |
| `0x488` | `DAS_steeringControl` | 50 Hz | Lateral steering angle request  |
| `0x27D` | `APS_eacMonitor`      | 10 Hz | EPAS "allow" gate               |

Bus: `BUS_CHASSIS` (X179 pins 13–14, autopilot party CAN). The encoding
(byte layout, counter/checksum, units) is taken straight from
[`opendbc/car/tesla/teslacan.py`](../../legacy/commaai-openpilot/opendbc_repo/opendbc/car/tesla/teslacan.py).

The gamepad mapping (in `firmware/lib/client/gamepad/drive.h::gamepadDriveTick`):

| Input         | Output                                     |
| ------------- | ------------------------------------------ |
| Left stick X  | Steering angle (±60°, then safety-clamped) |
| Left trigger  | Brake (0..−3.48 m/s² ACCEL_MIN)            |
| Right trigger | Accel (0..+2.0 m/s² ACCEL_MAX)             |

## What it is NOT

- ❌ **Autopilot / FSD / EAP** — these are _Tesla's_ closed-loop driving
  stacks running on the AP ECU. DAS Drive does not engage them; in fact it
  _replaces_ their CAN output and uses `apgate` to make sure the AP isn't
  fighting the gamepad.
- ❌ **openpilot** — openpilot is a full driving stack (camera, model,
  controller). DAS Drive borrows only its _CAN protocol_; the openpilot
  controller is replaced by the human on the gamepad.
- ❌ **Lane keep / cruise / TACC** — there is no perception, so there is
  nothing to follow.

## Safety envelope (openpilot parity)

All limits below are enforced in firmware, in the gamepad → CAN tick, and
mirror the values in
[`opendbc/car/tesla/values.py::CarControllerParams`](../../legacy/commaai-openpilot/opendbc_repo/opendbc/car/tesla/values.py).

| Limit                | Value                | Source                        |
| -------------------- | -------------------- | ----------------------------- |
| `ACCEL_MIN`          | −3.48 m/s²           | openpilot CarControllerParams |
| `ACCEL_MAX`          | +2.0 m/s²            | openpilot CarControllerParams |
| `JERK_LIMIT_MIN/MAX` | ±4.9 m/s³            | openpilot CarControllerParams |
| `MAX_ANGLE_RATE`     | 5° per 20 ms tick    | openpilot CarControllerParams |
| `MAX_ANGLE_ABS`      | 360°                 | physical wheel                |
| `MAX_LATERAL_ACCEL`  | ≈3.0 m/s² (ISO+roll) | bicycle-model derived         |
| `WHEELBASE`          | 2.875 m              | Model 3/Y                     |
| `STANDSTILL_HOLD`    | −0.4 m/s² < 1.5 km/h | brake-hold to prevent creep   |
| `SPEED_CAP_DEFAULT`  | 25 km/h              | NVS `tcm_das/cap`             |
| `SPEED_CAP_MIN..MAX` | 1..200 km/h          | hard firmware bounds          |

The shaping happens in `dasTick()`:

1. Steering target is clamped to `dasMaxSteerAtSpeed(v_kph)` (bicycle model
   from `MAX_LATERAL_ACCEL` and `WHEELBASE`).
2. Then rate-limited by `dasRateLimitAngle(target, last)` to ±5°/frame.
3. Longitudinal request is clamped to `[ACCEL_MIN, ACCEL_MAX]` and jerk-shaped.
4. Below `STANDSTILL_KPH` (1.5), `dasApplyStandstillHold` injects a small
   negative accel so the car doesn't creep when the trigger is at rest.
5. On disable, `dasSendCancelBurst()` pre-syncs the rolling counter so the
   AP ECU sees a clean handover and doesn't fault.

## Persistence (NVS)

Namespace `tcm_das`:

| Key   | Type   | Default | Notes                            |
| ----- | ------ | ------- | -------------------------------- |
| `en`  | bool   | false   | Drive enabled at boot            |
| `cap` | uint16 | 25      | Hard speed cap, clamped 1..200   |
| `spd` | uint8  | 25      | User speed limit, clamped 1..cap |

## Wire commands

See [`docs/reference/commands.md` → DAS Drive](../reference/commands.md#das-drive-gamepad-can-injection).

| Command         | Notes                                                       |
| --------------- | ----------------------------------------------------------- |
| `drive:on`      | Arm drive mode (gamepad takes over actuators)               |
| `drive:off`     | Disarm + emit a 5-frame cancel burst, returns control to AP |
| `drive:speed:N` | Set user limit in km/h (clamped to current cap)             |
| `drive:cap:N`   | Set runtime hard cap in km/h (1..200, persisted)            |

Status payload fields:

- `dasDriveEnabled` — drive mode toggle
- `dasSpeedLimitKph` — current user speed limit
- `dasSpeedCapKph` — current runtime cap
- `dasSpeedCapMaxKph` — absolute compile-time ceiling (200)

## Debug & verification

Use the bundled debug CLI:

```pwsh
node tools/debug.js das-drive --port COM5 --action status
node tools/debug.js das-drive --port COM5 --action set-cap --kph 50
node tools/debug.js das-drive --port COM5 --action toggle
node tools/debug.js das-drive --port COM5 --action soak --duration 5000
```

`soak` puts the board into raw-CAN streaming and checks that the three DAS
frame IDs appear at their nominal cadence (±20%).

## Bench-only checklist

Before arming on a real vehicle:

- [ ] Vehicle on jack stands or in a closed private space.
- [ ] `drive:cap:N` set to a low value you are comfortable with.
- [ ] Gamepad pairing verified (`status:features` shows `gamepad: paired`).
- [ ] AP gate (`apgate:status`) shows the gate logic is sane for your test.
- [ ] An emergency cutoff is reachable (kill switch, disconnect, brake).
- [ ] `drive:off` confirmed to immediately stop frame emission (use `soak`).
