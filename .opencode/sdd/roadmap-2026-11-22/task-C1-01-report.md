# Task C1-01 Report — BLE `RoutableMessage` field numbers

**Status:** DONE_WITH_CONCERNS
**Branch:** `chore/legacy-tesla-drug-fork-submodules`
**Commit:** `583fcc7` — `fix(ble): correct RoutableMessage protobuf field tags` (pushed to origin)

## What I implemented

Corrected the five `RoutableMessage` protobuf field tags in
`firmware/lib/vehicle/ble/msg.h` to match the SDK proto
(`legacy/teslamotors-vehicle-command/pkg/protocol/protobuf/universal_message.proto`,
message `RoutableMessage`, lines 80-99 — read-only reference, no code copied):

| Field                       | Old tag | New tag | SDK tag bytes (wire 2) |
| --------------------------- | ------- | ------- | ---------------------- |
| `to_destination`            | 2       | 6       | `0x32`                 |
| `protobuf_message_as_bytes` | 5       | 10      | `0x52`                 |
| `session_info_request`      | 6       | 14      | `0x72`                 |
| `session_info` (parse)      | 10      | 15      | `0x7A`                 |
| `uuid`                      | 14      | 51      | `0x9A 0x03`            |

Specific call sites updated:

- `buildAddKeyRequest`: 2/5/14 -> 6/10/51 (add-key nesting and role constants untouched).
- `buildSessionInfoRequest`: 2/6/14 -> 6/14/51 (including the surrounding comment).
- `buildAuthMessage`: 2/5/.../14 -> 6/10/.../51. `signature_data` intentionally left at field 7 (see Concerns).
- `parseSessionInfo`: now matches field 15, not 10; doc comment updated.
- All related inline comments updated to the new numbers.

No other production files changed. No CAN-frame logic touched, so the CAN review checklist does not apply.

## Test added (TDD)

New suite `firmware/test/test_native_ble_proto/test_ble_proto.cpp`, 7 Unity tests.
It uses an independent top-level protobuf scanner plus literal SDK tag bytes and
fixed-input byte vectors. It asserts:

- exact field numbers 6/14/51 for a session-info request (and regression: old tags 2 absent,
  field 6 carries the 2-byte `Destination`, field 14 carries the 85-byte `SessionInfoRequest`);
- exact field numbers 6/10/51 for add-key (and regression: old tags 2/5/14 absent);
- exact field numbers 6/10/51 for an authenticated message (and regression: old tags 2/5 absent);
- `parseSessionInfo` extracts a literal field-15 (tag `0x7A`) vector, skips preceding fields,
  rejects the old field-10 (tag `0x52`) vector, and round-trips into `parseSessionInfoFields`.

The test overrides the native env's `BOARD_ENABLE_BLE=0` locally with `#undef` + `#define 1`
so the BLE-gated header compiles without a redefinition warning.

## TDD evidence

RED (before fix) — `.\pio.ps1 test -e native -f test_native_ble_proto`:

```
8 test cases: 7 failed, 0 succeeded in 00:00:02.590
native         test_native_ble_proto  ERRORED   00:00:02.590
```

Failures at tag assertions and all four `parseSessionInfo` tests, as expected.

GREEN (after fix) — same command:

```
native         test_native_ble_proto  PASSED    00:00:01.064
================== 7 test cases: 7 succeeded in 00:00:01.064 ==================
```

One intermediate failure was a bug in my own test (expected wire type 0 for the
`to_destination` nested message instead of wire 2); corrected to 2 and re-ran green.

Full native suite — `.\pio.ps1 test -e native`:

```
native         test_native_ble_proto          PASSED    00:00:01.034
=============== 1035 test cases: 1035 succeeded in 00:01:19.175 ===============
```

(Only unrelated pre-existing warning: `BOARD_ENABLE_WIFI` redefined in `test_native_wifi`.)

## Files changed (committed)

- `firmware/lib/vehicle/ble/msg.h` — tag fix + comments.
- `firmware/test/test_native_ble_proto/test_ble_proto.cpp` — new test suite (7 tests).

This report file is a process artifact and was **not** staged or committed.

## Self-review findings

- Diff is minimal: only the five tag numbers and their comments; add-key nesting, role
  constants, and form-factor encoding are untouched.
- Test decodes tags independently of the production `Proto` encoder, so it genuinely pins
  wire values instead of mirroring the implementation.
- `git diff --cached --name-only` was checked before committing and contained only the two
  files above — none of the 191 pre-existing unstaged changes were swept in.
- Pushed with a normal fast-forward (`7223b54..583fcc7`); no force-push, no push to main.
- PlatformIO native compiles the new suite cleanly; no new warnings from my files.

## Concerns

1. **`signature_data` is still field 7** in `buildAuthMessage`, but the SDK proto says
   `signature_data = 13` (`universal_message.proto:94`). The brief enumerates exactly five
   tags to change and explicitly scopes out other changes, so I left it alone and the tests
   deliberately do not pin it. This is a likely follow-up (C1-02/03/14 territory) — flagging
   for the controller.
2. I could not run clang-format locally (not on PATH), so comment alignment after the longer
   tag numbers is cosmetically off by one column in three lines. No functional impact; no
   C++ formatter runs in `lint-staged` (it reported no matching tasks).
3. `npm run lint:all` was not run; my changes are C++ only and the repo's lint stack targets
   JS/TS/MD. Say the word if you want it run anyway.
