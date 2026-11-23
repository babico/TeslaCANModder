# Batched BLE wire fixes — C1-15, C1-16, C1-17, C1-18

Four related production fixes found by C1-14's SDK proto audit. Each has a `CHARACTERIZATION` byte test in `firmware/test/test_native_ble_proto/test_ble_proto.cpp` that currently pins the DEFECTIVE bytes and carries the SDK-correct literal vector in a comment. Implement all four in ONE commit and flip the matching assertions to the recorded SDK vectors in the SAME commit.

## C1-15 — `signature_data` field

SDK says 13, code uses 7 (`msg.h:313`). Acceptance: auth message byte test flipped to SDK vector, green.

## C1-16 — CarServer Action envelope

Single wrap at field 2, not double wrap at field 1 (`msg.h:150,153`). Acceptance: action envelope test flipped to literal `{0x12,0x02,0x32,0x00}` vector.

## C1-17 — CarServer action field numbers

charge/amps/limit/climate/wake actions (`msg.h:169,189-249`) vs SDK `car_server.proto`. Acceptance: all four affected tests flipped to their recorded SDK vectors.

## C1-18 — dead `feature/carserver.h`

Actual path `firmware/lib/vehicle/ble/feature/carserver.h`: non-compiling (calls `Proto::fieldFloat`, absent from `proto.h`, at `:241,243`) and holds conflicting SDK-correct numbers. Delete it or align it; acceptance: no stale parallel proto definition remains, ESP32 BLE target builds.

## Controller rulings (binding)

- Only flip the 5 characterization tests whose SDK vectors were recorded by C1-14. Do NOT touch the other 2 SDK-correct pins or any C1-01..04 behavior (tags, add-key nesting, roles, scan filter).
- C1-02 residual: `metadataForKey` placement is a bench follow-up — do not change it here.
- Bench capture of accepted commands is impossible here → each report item must state UNVERIFIED for real-vehicle acceptance.
- No code copied from `legacy/`; derive from proto field numbers.

## Global constraints

- Stage ONLY files you change; 191 unrelated unstaged files exist. Verify staged list before commit.
- Commit/push on current feature branch; never main, never force-push.
- Run focused `test_native_ble_proto`, full native suite, and the ESP32 BLE build before committing.
