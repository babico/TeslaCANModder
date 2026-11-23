# Task C1-03 — BLE roles and form factor (report)

Status: COMPLETE (bench UNVERIFIED — no hardware/capture available in this environment).

Branch: `chore/legacy-tesla-drug-fork-submodules`
Commit: `a168d83` `fix(ble): align key roles and NFC form factor with SDK` (pushed to origin).

## Implemented

- `firmware/lib/vehicle/ble/feature/key.h`
    - `ROLE_OWNER = 2`, `ROLE_CHARGING_MANAGER = 6` (SDK `keys.proto:7-15`; quoted in `docs/legacy/tesla-local-control-vehicle-command.md:56,235`).
    - Added `roleValueFromName()` in namespace `Tesla`, outside `BOARD_ENABLE_BLE`, so the role-string → protocol-byte mapping is directly testable in native tests without ESP32/mbedTLS headers. `stdint.h`/`string.h` moved above the guard to serve it.
- `firmware/lib/io/ble/esp32/tesla_board.h`
    - `Esp32Board::roleValue()` now delegates to `roleValueFromName()`; SDK values documented in the doc comment. Same behavior as before for the two stored strings, with corrected byte values.
- `firmware/lib/vehicle/ble/msg.h`
    - `KEY_FORM_FACTOR_NFC_CARD = 3` → `1` (SDK `vcsec.proto KeyFormFactor.NFC_CARD=1`). This closes C1-02's residual: production `tesla:key:send` dispatch (`handler/dispatch.h:204`) passes this constant directly into `buildAddKeyRequest()`.
    - Updated `buildAddKeyRequest()` doc comment to new role values.
- `firmware/test/test_native_ble_proto/test_ble_proto.cpp`
    - New `test_role_value_mapping`: asserts `"owner"`→2, `"charging_manager"`→6, and the two role constants.
    - New `test_nfc_form_factor_value`: asserts `KEY_FORM_FACTOR_NFC_CARD == 1`.
    - Re-pinned `test_add_key_tags` and `test_add_key_exact_wire` from the old literal `keyRole=4` + test-only formFactor constant to production `ROLE_OWNER`=`0x02` + `KEY_FORM_FACTOR_NFC_CARD`=`0x01`. C1-02's 16→5→`PermissionChange{key=1,keyRole=4,metadataForKey=6}` nesting is unchanged.
    - Added `test_add_key_charging_manager_exact_wire`, pinning the full 105-byte wire with `keyRole=0x06`.
    - Removed the test-only `SDK_KEY_FORM_FACTOR_NFC_CARD` shim; tests now use the production constants.

Not changed (per brief): add-key nesting (C1-02), RoutableMessage tags (C1-01), `metadataForKey` placement (separate bench follow-up).

## TDD evidence

RED — new tests written first, production unchanged:

```
cd firmware; .\pio.ps1 test -e native -f test_native_ble_proto
test\test_native_ble_proto\test_ble_proto.cpp:58:36: error: 'roleValueFromName' was not declared in this scope
test\test_native_ble_proto\test_ble_proto.cpp:60:36: error: 'ROLE_OWNER' was not declared in this scope
test\test_native_ble_proto\test_ble_proto.cpp:61:36: error: 'ROLE_CHARGING_MANAGER' was not declared in this scope
... also ROLE_OWNER/ROLE_CHARGING_MANAGER undefined in add-key call sites ...
----------- native:test_native_ble_proto [ERRORED] -----------
Exit code: 1
```

Failure was the intended missing-feature failure (test references a mapping/constant API that did not yet exist), not a typo.

GREEN — after minimal production fix:

```
cd firmware; .\pio.ps1 test -e native -f test_native_ble_proto
test\test_native_ble_proto\test_ble_proto.cpp:655: test_role_value_mapping [PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:656: test_nfc_form_factor_value [PASSED]
...
test\test_native_ble_proto\test_ble_proto.cpp:669: test_add_key_charging_manager_exact_wire [PASSED]
----------- native:test_native_ble_proto [PASSED] Took 1.38 seconds -----------
================= 21 test cases: 21 succeeded in 00:00:01.376 =================
```

Full native suite (run once before commit):

```
cd firmware; .\pio.ps1 test -e native
=============== 1049 test cases: 1049 succeeded in 00:01:02.533 ===============
```

All suites PASSED, including `test_native_ble_proto` and `test_native_bluetooth`. No failures observed, no red test scrolled past.

## Wire-level proof (byte pins, from tests)

Add-key request, owner (`0x20,0x02` = PermissionChange.keyRole field 4 = 2; `0x32,0x02,0x08,0x01` = metadataForKey formFactor 1) — 105 bytes total, unchanged 16→5→5 nesting from C1-02. Charging-manager variant identical except `keyRole=0x06`. Values sourced from SDK proto quoted in legacy analysis doc; no legacy code copied into shipping code.

## Checklist / self-review

- `docs/checklists/can-review-checklist.md` walked. This is a BLE protobuf wire change only: no CAN ID, DLC, bit position, checksum, mux rule, profile mapping, variant behavior, or bus routing touched; no new TX path when disabled. Section 5 satisfied via exact byte-vector pins plus role-value assertions. Stream/status shape unaffected.
- Committed diff reviewed with `git show a168d83`: 4 files, +103/-28. Only intended files staged; `git diff --cached --name-only` verified before commit. The 191+ pre-existing unrelated unstaged files were not touched or committed (lint-staged reported no matching staged files; worktree still shows them modified).
- `roleValueFromName()` is `static` in a header, matching the repo's existing header-only-library pattern (internal linkage per TU); no ODR concern. ESP32 build still sees the same functions inside `BOARD_ENABLE_BLE`; native test sees only the mapping + constants.
- Command validation in `handler/dispatch.h:152` still accepts only `"owner"`/`"charging_manager"`, so the mapping's fallback remains consistent with the persisted role strings.

## Concerns / follow-ups

- **Bench verification: UNVERIFIED.** No vehicle, ESP32, or BLE capture available here. SDK values are pinned only from proto quotes in `docs/legacy/tesla-local-control-vehicle-command.md` + byte tests; a real `tesla:key:send` capture should confirm the vehicle accepts OWNER=2 / CHARGING_MANAGER=6 with NFC_CARD=1 before release.
- `metadataForKey` placement remains a separate bench follow-up from C1-02 and was intentionally left unchanged.
- Other known defective BLE wire vectors documented as CHARACTERIZATION pins in `test_ble_proto.cpp` (signature_data fields, Action wrapper, wake/charge/amps/climate fields) were deliberately out of scope and remain unchanged.
- No C++ build of an ESP32 BLE environment (`pio run -e esp32_ble_chassis_8mhz`) was attempted; changes are header-only constants/functions already compiled by native tests. If controller wants a hardware-target compile, request it explicitly.
