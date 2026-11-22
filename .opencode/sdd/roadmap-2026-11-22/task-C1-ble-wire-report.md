# Task C1-ble-wire report — C1-15, C1-16, C1-17, C1-18

Status: **DONE**

Branch: `chore/legacy-tesla-drug-fork-submodules` (current feature branch; no main, no force-push).

## Summary

Four BLE wire defects from C1-14 fixed in one commit, with the five CHARACTERIZATION
assertions that carry recorded SDK vectors flipped in the same commit. Dead
`feature/carserver.h` deleted. No C1-01..04 behavior changed; `metadataForKey` placement
untouched; no code copied from `legacy/` — all field numbers derived from the SDK
`.proto` files read as reference only.

## Per-item status

### C1-15 — `signature_data` field — DONE

- `firmware/lib/vehicle/ble/msg.h` `buildAuthMessage`:
    - `signature_data` moved field 7 → **field 13** (`universal_message.proto:94`;
      field 7 is `from_destination` at `:85`).
    - `SignedData.AES_GCM_personalized_data` field 2 → **field 5**
      (`signatures.proto:80`).
    - AES-GCM inner: `counter` field 2 → **field 3** (`signatures.proto:46`);
      `tag` field 1 → **field 5** (`signatures.proto:48`); removed non-SDK
      `component_id` field 3.
    - Wire: 59 B → SDK-correct 57 B.
- Test flipped: `test_auth_message_exact_wire` now pins
  `32 02 08 03 | 52 08 30*8 | 6A 16 2A 14 18 07 2A 10 04*16 | 9A 03 10 03*16`.
- **Real-vehicle acceptance: UNVERIFIED.** No bench capture available; correctness
  argued from SDK field numbers and exact-wire tests only.

### C1-16 — CarServer Action envelope — DONE

- `buildAction` replaced the double wrap (`fieldMsg(1, va)` + `fieldMsg(1, vehicleAction)`)
  with a **single wrap at field 2** per `car_server.proto:16`.
- Test flipped: `test_action_envelope_exact_wire` now pins `{0x12, 0x02, 0x32, 0x00}`.
- **Real-vehicle acceptance: UNVERIFIED.** No bench capture available.

### C1-17 — CarServer action field numbers — DONE

- `buildChargeStartAction`: `chargingStartStopAction` 11 → **6**; `start` 1 → **2**;
  flattened the redundant `ChargeNow` empty-message wrapper.
- `buildChargeStopAction`: `chargingStartStopAction` 11 → **6**; `stop` 2 → **5**;
  same flattening.
- `buildSetAmpsAction`: `setChargingAmpsAction` 13 → **43** (`car_server.proto:56`).
- `buildSetLimitAction`: `chargingSetLimitAction` 12 → **5** (`car_server.proto:26`).
- `buildClimateAction`: `hvacAutoAction` 14 → **10** (`car_server.proto:31`);
  added `manual_override = field 2, false` (`car_server.proto:211`).
- Tests flipped: `test_charge_actions_exact_wire`, `test_set_amps_limit_exact_wire`,
  `test_climate_action_exact_wire` now pin the recorded SDK vectors.
- **Real-vehicle acceptance: UNVERIFIED.** Bench capture of accepted
  charge/amps/limit/climate commands was not possible in this environment.

### C1-18 — dead `feature/carserver.h` — DONE

- Deleted `firmware/lib/vehicle/ble/feature/carserver.h`. Verified zero includes before
  deletion. It was non-compiling (`Proto::fieldFloat` absent from `proto.h`, formerly
  at `:241,243`) and held a conflicting parallel SDK-number proto definition.
- Acceptance: no stale parallel proto definition remains; ESP32 BLE target builds.
- **Real-vehicle acceptance: UNVERIFIED / not applicable** (no wire behavior; dead
  header only).

## Controller rulings applied

1. **Q1 — amps vector transcription error.** C1-14's recorded vector
   `12 05 9A 02 02 08 20` was wrong: `9A 02` decodes as tag varint 282 → field 35
   (`HvacBioweaponModeAction`), not field 43. SDK `car_server.proto:56` is binding:
   `setChargingAmpsAction = 43` → wire tag `DA 02` (346 → `DA 02`). Production already
   emitted `DA 02`, so it was kept; the assertion was flipped to
   `12 05 DA 02 02 08 20` and the test comment corrected to record the SDK-derived
   vector and the transcription error. No code was copied from `legacy/`.
2. **Q2 — wake test has no recorded SDK vector.** `test_wake_action_exact_wire` was
   marked CHARACTERIZATION by C1-14 but carries no literal SDK vector; upstream
   states wake belongs to VCSEC RKE, not `CarServer.VehicleAction`. Per ruling,
   `buildWakeAction` was kept on the legacy double-wrap so its pin stays green
   (7-byte `{0A 05 0A 03 CA 01 00}`, field 25 unchanged); no 5-byte single-wrap
   bytes were introduced without SDK evidence. Both the builder and the test now
   carry the comment `LEGACY — no SDK vector; wake belongs to VCSEC RKE; see follow-up C1-23`.
   The wake wire remains a known non-SDK defect by design.

## TDD RED / GREEN evidence

Baseline before any edit: focused suite 21/21 green (current defective bytes pinned).

RED — after flipping the five SDK-vector assertions, before production fixes
(`cd firmware; .\pio.ps1 test -e native -f test_native_ble_proto`):

```
22 test cases: 5 failed, 16 succeeded
test_auth_message_exact_wire:   Expected 57 Was 59
test_action_envelope_exact_wire: Expected 4  Was 6
test_charge_actions_exact_wire: Expected 6  Was 10
test_set_amps_limit_exact_wire: Expected 7  Was 8
test_climate_action_exact_wire: Element 0 Expected 0x12 Was 0x0A
```

Exactly the five intended tests failed; no collateral failures.

GREEN — after production fixes and controller rulings:

```
cd firmware; .\pio.ps1 test -e native -f test_native_ble_proto
21 test cases: 21 succeeded in 00:00:00.977

cd firmware; .\pio.ps1 test -e native
1071 test cases: 1071 succeeded in 00:01:20.194

cd firmware; .\pio.ps1 run -e esp32_ble_chassis_8mhz
[SUCCESS] 1 succeeded in 00:00:33.981
RAM 22.1%, Flash 56.2%; build/firmware/esp32_ble_chassis_8mhz.bin produced
```

## Files changed

- `firmware/lib/vehicle/ble/msg.h` — production fixes (C1-15/16/17; C1-17 wake kept legacy)
- `firmware/test/test_native_ble_proto/test_ble_proto.cpp` — five SDK-vector flips
  plus wake LEGACY comment
- `firmware/lib/vehicle/ble/feature/carserver.h` — deleted (C1-18)

Staging: only these three paths staged via explicit `git add` / `git rm`;
`git diff --cached --name-only` verified before commit. 191 unrelated unstaged files
left untouched.

## Follow-up registered

- **C1-23 (new, not implemented here):** capture-first migration of wake from the
  legacy CarServer double-wrap to VCSEC RKE `RKE_ACTION_WAKE_VEHICLE` (30). Requires a
  real-vehicle capture of the accepted wake command before any wire change. Current
  wake wire is deliberately left as-is per controller ruling.

## Concerns

- Real-vehicle acceptance for C1-15, C1-16 and C1-17 remains **UNVERIFIED**; these are
  SDK-field-number-derived fixes validated only by exact-wire unit tests. Authenticated
  BLE commands (auth signature) and charge/climate actions should be bench-tested on a
  vehicle before release.
- `COMPONENT_INFOTAINMENT` / `COMPONENT_VEHICLE_SECURITY` constants remain in `msg.h`
  after `component_id` removal (the second was already unused); left in place to avoid
  unrelated churn.
- Wake remains a known non-SDK wire pending C1-23; do not build new UI on it.
