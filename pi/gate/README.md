# CAN TX gate contract

> **STATUS: DISABLED — INJECTION BLOCKED UNTIL BENCH CAPTURE.**
> No Raspberry Pi, CAN adapter, `cyclictest` binary or signed gate artifact was
> available when this backend was written. No frame in this directory has been
> transmitted on a vehicle bus. Do not treat this code as authorisation to inject.

## Purpose

`pi/src/can/tx.ts` is the Linux-side CAN transmit backend. It is intentionally
unreachable by default. Two independent conditions must both hold before a
single frame can be encoded and handed to a transport:

1. The `PI_CAN_TX` feature flag is exactly the string `1`. Any other value —
   including unset, `0`, `true` or `yes` — leaves TX off. The flag lives in
   `pi/src/index.ts` as `isCanTxEnabled()` and is consumed by `createGatedTx()`.
2. B3's `isGateOpen()` from `pi/src/rt/measure.ts` returns `true`. It verifies
   the HMAC-SHA256 signature over canonical JSON, schema version 1, the binding
   thresholds, the `verdict.pass` flag and an independent re-decision of the raw
   measurements. This is the **only** gate check; no second gate is invented.

Missing file, wrong key, tampered numbers, stale schema, a failing verdict or a
failing measurement all fail closed. The gate artifact is `pi/gate/rt-result.json`;
its contract and the bench procedure are documented in `pi/deploy/rt-gate.md`.
**The file does not exist in this repo.** A unit test asserts its absence.

Even with both conditions satisfied, a transport must be injected. The backend
has no default socket: importing it opens nothing, reads nothing and sends
nothing. Real injection also needs a signed `docs/checklists/can-review-checklist.md`
review backed by a listen-only capture reference — neither exists for this task.

## What is implemented

- `buildDriveModeFrame()` — mirrors the firmware's `buildDriveModeFrame()` and
  `driveChecksum()` conventions from
  `firmware/lib/vehicle/can/feature/drive/drive_mode.h` and
  `firmware/lib/vehicle/can/ids.h`: copy the cached 8-byte 0x334 payload, write
  the 2-bit mode into byte[0] bits[6:5], recompute the additive checksum in
  byte[7]. A cached frame shorter than 8 bytes produces no frame.
- `driveChecksum()` / `encodeCanFrame()` — pure helpers with DLC and 11-bit ID
  guards. `encodeCanFrame()` emits the 16-byte native-endian SocketCAN
  `struct can_frame` used by the read path.
- `createGatedTx()` — evaluates the hard gate once at construction and returns
  an inert backend when it is shut. `send()` returns `false` without touching
  the transport in that case.

The frame builder is a structural port only. Its exact byte layout is verified
against an independent test oracle in `pi/test/tx.test.ts`, not against a
vehicle capture, so it is **UNVERIFIED** for real-bus acceptance.

No Sentry code was copied, read or adapted to produce this backend.

## Release blockers (not delivered here)

- Generate `pi/gate/rt-result.json` on the exact target board by walking
  `pi/deploy/rt-gate.md` with a real PREEMPT_RT kernel, injected probe runner
  and out-of-band key. Do not commit the key or a real artifact to this repo.
- Obtain a listen-only capture of the target frame from the ESP32 bench oracle
  and walk `docs/checklists/can-review-checklist.md` against it. The report for
  this task records every item as capture-first / UNVERIFIED where no evidence
  exists.
- Wire a real transport (SocketCAN gateway or native binding) and an on-board
  entrypoint in a later task. Until then the only transport is the fake used in
  unit tests.

Until all three are closed, enabling `PI_CAN_TX` on a bench is unsupported.
