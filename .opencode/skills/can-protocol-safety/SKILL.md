---
name: can-protocol-safety
description: Safety guidelines for modifying Tesla CAN bus frame behavior including DLC guards, checksums, variant-specific logic, and bus routing
license: GPL-3.0
compatibility: opencode
metadata:
    area: firmware
    stack: cpp
    safety: critical
---

## What I do

Enforce safety discipline for any change that touches Tesla CAN bus frame behavior. I cover:

- DLC guards, checksum recalculation, and bit-field isolation
- Variant-specific behavior (HW3, HW4, Legacy)
- Bus routing and I/O protocol consistency
- Required regression tests for CAN mutations
- High-risk change detection

## When to use me

Use this skill when:

- Adding or modifying CAN feature handlers in `firmware/lib/vehicle/can/feature/`
- Changing frame mutation, routing, or checksum logic
- Modifying MCP2515 filters or dispatch behavior
- Changing stream output format or protocol-visible message shape
- Working with variant-specific behavior (HW3, HW4, Legacy)

## Scope of CAN review

Changes that MUST follow `docs/checklists/can-review-checklist.md`:

- `firmware/lib/vehicle/can/feature/*` - feature handlers and frame mutation
- `firmware/lib/vehicle/can/handler/*` - variant handlers and dispatch
- `firmware/lib/core/can/*` - MCP2515 driver, CAN bus init, frame TX/RX
- `firmware/lib/core/util/*` - shared CAN helpers, burst helpers, parsing, bit logic
- `firmware/lib/io/*` - serial, WiFi REST API, BLE I/O layers

## Frame mutation safety

Every change MUST ensure:

- Every direct `frame.data[...]` read has a matching DLC guard before access.
- Every direct `frame.data[...]` write has a matching DLC guard before mutation.
- Shared helpers still reject invalid bit indexes and writes outside the frame DLC.
- Bit writes only touch the intended field and do not leak into adjacent bits.
- Existing checksum logic is recalculated when a changed byte requires it.

## Bit and field ownership

- The exact bit numbers changed by the patch must be documented in the PR or commit notes.
- Changed bits must be cross-checked against the active TeslaCANModder behavior, not only an old legacy sketch.
- If legacy references disagree, note which source is canonical for the active repo.
- Mux-specific behavior must be preserved: a mux-only mutation must not leak into other mux paths.
- RX and TX ownership must be clear: the code only transmits frames on intended paths.

## Variant behavior review

- `legacy`, `hw3`, and `hw4` must all be considered, even if only one variant changed.
- Unsupported controls must still remain unsupported on the wrong variant.
- `hw3` speed offset changes must still preserve profile and nag behavior.
- `hw4` ISA speed-chime changes must still preserve mux and checksum behavior.
- Legacy behavior must not be changed accidentally by reusing HW3/HW4 helper logic.

## Bus assignments

| Bus     | Index | MCP2515 | X179 Pins | Function                | Speed   |
| ------- | ----- | ------- | --------- | ----------------------- | ------- |
| Chassis | 0     | #1      | 13-14     | Chassis / Autopilot CAN | 500kbps |
| Vehicle | 1     | #2      | 9-10      | Vehicle Control CAN     | 500kbps |
| Body    | 2     | #3      | 2-3       | Body Control CAN        | 500kbps |

All buses are opt-in via build flags. DAS injection only works when Chassis CAN is active.

## Transmission safety

There is **no OTA TX guard**. `CAN_ID_GTW_CAR_STATE` (`0x318`) is defined in
`firmware/lib/vehicle/can/ids.h` as a documented ID for capture-first research only;
no handler pauses TX and no `otaInProgress` / `txPaused` state field exists (C1-06).
Do not reintroduce claims that OTA automatically pauses transmission.

Actual gating today:

- One-shot vehicle commands use the single freshness gate `s.hasFreshCtrl(now)`
  (`lastCtrlMs + AP_GATE_FRESHNESS_MS`); a stale `0x273` must not template injections.
  Fresh-boot diagnostics are dispatched before that gate.
- Continuous injection features use the runtime AP injection gate (`s.apGateOpen()`,
  `apgate:on`) plus their own variant/toggle checks. One-shot commands intentionally
  skip the AP gate.
- `startBurst()` / `burstTick()` / `summonTick()` / `preconditionTick()` and the nag
  echo path each keep their own DLC, variant and freshness guards. There is no global
  OTA kill switch to rely on.

## Feature categories

| Category      | Pattern                                                            | Typical Bus |
| ------------- | ------------------------------------------------------------------ | ----------- |
| Toggle-Inject | ON -> add filter + intercept + modify + send, OFF -> remove filter | 0 (FSD)     |
| Echo-Inject   | Read frame -> clone -> modify -> send back on same bus             | 1 (Vehicle) |
| Burst-Inject  | Build frame -> `startBurst(count, delayMs)` non-blocking           | 1 or 2      |
| Tick-Inject   | Dedicated timer loop sends frames indefinitely or with countdown   | 1 (Vehicle) |
| Read-Only     | Decode frame -> update state (no send)                             | 1 (Vehicle) |
| Config-Only   | Update state + persist (no CAN interaction)                        |             |

## I/O protocol consistency

- Any change to transmitted frames must be checked against `firmware/lib/io/serial/board.h`.
- If frame message shape changed, the client-side transport and parsing layers must be reviewed at the same time.
- Stream output must preserve `dir`, `id`, `dlc`, `d`, and any required metadata fields.
- WiFi REST API and BLE NUS must produce identical command semantics to serial.

## Required regression tests

At least one of these must be added or updated when CAN-control behavior changes:

- exact bit-set / bit-clear assertion for the changed field
- short-frame regression test proving no send happens when DLC is too small
- variant-specific regression test proving unsupported variants do not drift
- stream/message-shape assertion when board output changes

## High-risk change types

Treat these as review-sensitive even if the diff looks small:

- helper changes in `firmware/lib/core/util/`
- mux / profile / offset / ISA-related changes in feature handlers such as `profile.h`, `offsets.h`, `isa_chime.h`, or `nag.h`
- checksum changes
- handler filter ID changes
- stream message shape changes
- serial / WiFi / BLE command entry changes that affect CAN mutation state
- MCP2515 driver changes
- bus routing changes (BUS_CHASSIS / BUS_VEHICLE / BUS_BODY assignments)
- WiFi/BLE endpoint changes that expose new CAN mutation paths

## Manual reviewer questions

- Does this patch change which CAN IDs are intercepted or transmitted?
- Does this patch change any bit position, checksum, mux rule, or profile mapping?
- Does this patch add any path that can transmit when the feature is disabled?
- Does this patch change the browser-visible meaning of streamed frames or status?
- If this behavior came from a legacy sketch, is the old sketch actually the correct reference?
- Does this patch affect bus routing (BUS_CHASSIS=0, BUS_VEHICLE=1, BUS_BODY=2)?
- Does this patch change behavior for any of the 3 MCP2515 buses on ESP32?
- Is the WiFi/BLE command path still consistent with the serial command path?

## Minimum acceptance bar

A CAN-control change is not complete until:

- the intended bit-level behavior is asserted in tests
- short-frame behavior is asserted where relevant
- the active variant behavior is still explicit
- the reviewer can explain exactly which CAN IDs and bits changed
