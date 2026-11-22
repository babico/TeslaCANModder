# Task C1-14 report — BLE proto test suite (all builders)

Status: **DONE_WITH_CONCERNS**

## Implemented

Extended the existing single suite `firmware/test/test_native_ble_proto/test_ble_proto.cpp`
(no duplicate suite, no production code changed). Added 11 tests (7 -> 18):

| Test                                   | Builder pinned                                             |
| -------------------------------------- | ---------------------------------------------------------- |
| `test_enc_destination_exact_wire`      | `encDestination` (VS=2, Infotainment=3)                    |
| `test_session_info_request_exact_wire` | `buildSessionInfoRequest` full 110-byte wire               |
| `test_add_key_exact_wire`              | `buildAddKeyRequest` full 98-byte wire                     |
| `test_auth_message_exact_wire`         | `buildAuthMessage` full 59-byte wire incl. signature_data  |
| `test_action_envelope_exact_wire`      | `buildAction` double-wrap                                  |
| `test_wake_action_exact_wire`          | `buildWakeAction`                                          |
| `test_charge_actions_exact_wire`       | `buildChargeStartAction`, `buildChargeStopAction`          |
| `test_set_amps_limit_exact_wire`       | `buildSetAmpsAction`, `buildSetLimitAction`                |
| `test_climate_action_exact_wire`       | `buildClimateAction` on/off                                |
| `test_vcsec_rke_exact_wire`            | `buildVCSECRKEAction` lock/unlock + `buildVCSECWakeAction` |
| `test_vcsec_closure_exact_wire`        | `buildVCSECClosureAction` trunk-open / frunk-close         |

All tests use the existing independent literal-byte decoder style: hand-built expected
byte arrays and `TEST_ASSERT_EQUAL_HEX8_ARRAY`, no production `Proto` helper in assertions.

Notes:

- VCSEC builders live in `firmware/lib/vehicle/ble/feature/vcsec.h`, which is not
  included anywhere in production and collides with `msg.h`'s `buildWakeAction`.
  Included it in the same TU under a local macro rename (`buildVCSECWakeAction`);
  verified compiles and links. No new suite/test folder created.
- `firmware/lib/vehicle/ble/feature/carserver.h` could NOT be pinned: it is dead code
  (not included anywhere) and does not compile against `proto.h` (uses nonexistent
  `Proto::fieldFloat`). Its field numbers also conflict with `msg.h`. Documented below.
- Distance has no proto builder; it is RSSI math already covered by
  `test_native_ble_distance` and `executeBleKeyDistanceCmd` tests. "Distance/role
  encoding as applicable" is satisfied by role/form-factor in `test_add_key_exact_wire`.

## TDD RED / GREEN evidence

Baseline (pre-change): 7/7 passed.

RED run — expectations written from SDK proto vectors (`cd firmware; .\pio.ps1 test -e native -f test_native_ble_proto`):

```
18 test cases: 5 failed, 13 succeeded in 00:00:01.112
test_auth_message_exact_wire:   Expected 57 Was 59
test_wake_action_exact_wire:    Expected 9  Was 7
test_charge_actions_exact_wire: Expected 6  Was 10
test_set_amps_limit_exact_wire: Expected 7  Was 8
test_climate_action_exact_wire: Element 0 Expected 0x12 Was 0x0A
```

Each failure is a genuine SDK-vs-current wire mismatch (details below), not a test bug.
Per brief step 3, production was NOT changed. The committed assertions pin the exact
CURRENT bytes as a regression baseline, and each affected test carries a `DEFECT
(reported, not fixed)` comment with the SDK-correct vector and exact file:line refs.

GREEN run — focused suite:

```
18 test cases: 18 succeeded in 00:00:01.047
native:test_native_ble_proto PASSED
```

Full native suite (`cd firmware; .\pio.ps1 test -e native`):

```
1046 test cases: 1046 succeeded in 00:01:21.983
native:test_native_ble_proto PASSED
```

## Files changed

- `firmware/test/test_native_ble_proto/test_ble_proto.cpp` (only file)

## Self-review

- One suite, one file. No duplicate test runner, no production edits.
- All 11 new tests assert both exact length and exact byte array; SDK vectors for
  defective builders preserved as comments.
- VCSEC macro rename is local (`#define`/`#undef` around the include) and documented.
- Fixed a reversed line-range typo in a comment during review; focused suite re-run after.
- Staged only the one changed file; verified `git diff --cached --name-only`.

## Concerns — real wire defects found (reported, NOT fixed)

Reference SDK (read-only): `legacy/teslamotors-vehicle-command/pkg/protocol/protobuf/*.proto`.
All paths below are in `firmware/lib/vehicle/ble/msg.h` unless noted.

1. **Auth message signature placement — critical.** `msg.h:313` writes signature_data
   at field 7. SDK `universal_message.proto:85` defines field 7 = `from_destination`;
   `signature_data` is field 13 (`:94`). Vehicle will parse AES-GCM signature bytes as
   a Destination and reject/desync the command. Inner structure also wrong:
   `msg.h:294-301` uses SignedData field 2, AES_GCM field 1=tag/2=counter/3=component;
   SDK `signatures.proto:80` says field 5, `:46` counter=3, `:48` tag=5.
   Current 59 B vs SDK-correct 57 B. `feature/session.h:253` is the caller, so every
   authenticated BLE command is affected.

2. **CarServer Action envelope wrong.** `msg.h:150` and `:153` both wrap at field 1
   -> `Action{1:{1:VehicleAction}}`. SDK `car_server.proto:16` defines a single
   `Action{2: VehicleAction}`.

3. **CarServer action field numbers wrong** (all in `msg.h`):
    - `buildWakeAction` `:169` field 25; SDK `car_server.proto:42` field 25 =
      `vehicleControlCancelSoftwareUpdateAction` and there is NO wake field at all.
      Wake belongs to VCSEC RKE `RKE_ACTION_WAKE_VEHICLE=30` (`vcsec.proto:81`).
    - `buildChargeStartAction` `:191` field 11 -> SDK `:27` field 6; `:189` start field 1
      -> SDK `:186` field 2.
    - `buildChargeStopAction` `:213` field 11 -> SDK field 6; `:211` stop field 2
      -> SDK `:189` field 5.
    - `buildSetAmpsAction` `:231` field 13 -> SDK `:56` field 43.
    - `buildSetLimitAction` `:249` field 12 -> SDK `:26` field 5.
    - `buildClimateAction` `:267` field 14 -> SDK `:31` field 10; `manual_override`
      (SDK `:211` field 2) omitted.

4. **AddKey inner structure wrong.** `msg.h:78` writes add-key at UnsignedMessage field 2,
   but SDK `vcsec.proto:229` field 2 = RKEAction; correct is field 5 (`:127`).
   `msg.h:69-71`: raw key bytes at field 1 (SDK `:105` wraps `PublicKey{PublicKeyRaw=1}`),
   role at field 2 (SDK `:107` `keyRole=4`), form_factor at field 5 (SDK `:135`
   `metadataForKey=6` -> `KeyMetadata.keyFormFactor=1`). `KEY_FORM_FACTOR_NFC_CARD=3`
   at `msg.h:25` but SDK `vcsec.proto:32` says 1. Affects `tesla:key:send`.

5. **Dead/conflicting alternate builders.** `firmware/lib/vehicle/ble/feature/carserver.h`
   and `feature/vcsec.h` are included nowhere in production. `carserver.h` does not
   compile (`Proto::fieldFloat` absent from `proto.h`). Its field numbers match the SDK
   better than `msg.h` (e.g. charge start/stop=6, amps=43, limit=5, climate=10) and it
   appears to be an attempted fix that was never wired in. Controller should decide
   which tree is authoritative before any production change.

Session-info request, Destination, and all VCSEC RKE/closure builders match SDK vectors
and are pinned GREEN with SDK-correct bytes.

Recommendation: follow-up task to fix the above production field numbers, then flip the
`DEFECT` assertions in this suite to the SDK vectors already recorded in the comments.
This task deliberately did not change wire behavior.

## Round 1 fix (reviewer findings)

Controller ruling applied: the 7 exact-wire tests that pin the current defective bytes
are a **characterization baseline** (production fixes out of scope for this task).

What changed:

1. Added the marker `// CHARACTERIZATION — current defective wire; flip to SDK vector with
the production fix.` to all 7 defective tests: `test_add_key_exact_wire`,
   `test_auth_message_exact_wire`, `test_action_envelope_exact_wire`,
   `test_wake_action_exact_wire`, `test_charge_actions_exact_wire`,
   `test_set_amps_limit_exact_wire`, `test_climate_action_exact_wire`.
   The 4 SDK-correct pins (`test_enc_destination_exact_wire`,
   `test_session_info_request_exact_wire`, `test_vcsec_rke_exact_wire`,
   `test_vcsec_closure_exact_wire`) are intentionally unmarked.
2. Added literal SDK-correct byte arrays as comments where they were missing:
    - `test_action_envelope_exact_wire`: `{0x12, 0x02, 0x32, 0x00}` — single
      `Action{field 2: VehicleAction{field 6: empty}}` per `car_server.proto:16`
      (current pin is `{0x0A,0x04,0x0A,0x02,0x32,0x00}`).
    - `test_add_key_exact_wire`: full 105-byte vector derived from `vcsec.proto`
      (UnsignedMessage.WhitelistOperation field 16 at `:230`,
      WhitelistOperation.addKeyToWhitelistAndAddPermissions field 5 at `:127`,
      PermissionChange.key/PublicKey wrapping at `:105`, keyRole field 4 at `:107`,
      metadataForKey field 6 at `:135`, SDK NFC_CARD=1 at `:32`).
      Refinement to the original concern: the add-key message is not placed directly
      at UnsignedMessage field 5 — it is field 16 (WhitelistOperation) -> field 5.
      Comment also notes SDK `ROLE_OWNER=2` (`keys.proto:11`) vs 4 used by msg.h.
3. Corrected the report RED-line typo from `19 test cases` to `18 test cases`.
4. No production code changed; no assertions flipped.

Covering tests: `test_native_ble_proto` (18 tests, all passing).

Exact command run (from repo root):

```
cd firmware; .\pio.ps1 test -e native -f test_native_ble_proto
```

Output (verbatim):

```
Verbosity level can be increased via `-v, -vv, or -vvv` option
Collected 85 tests

Processing test_native_ble_proto in native environment
--------------------------------------------------------------------------------
Building...
Testing...
test\test_native_ble_proto\test_ble_proto.cpp:602: test_session_info_request_tags	[PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:603: test_add_key_tags	[PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:604: test_auth_message_tags	[PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:605: test_parse_session_info_uses_field_15	[PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:606: test_parse_session_info_skips_other_fields	[PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:607: test_parse_session_info_rejects_old_field_10	[PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:608: test_session_info_fields_round_trip_via_field_15	[PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:610: test_enc_destination_exact_wire	[PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:611: test_session_info_request_exact_wire	[PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:612: test_add_key_exact_wire	[PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:613: test_auth_message_exact_wire	[PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:614: test_action_envelope_exact_wire	[PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:615: test_wake_action_exact_wire	[PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:616: test_charge_actions_exact_wire	[PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:617: test_set_amps_limit_exact_wire	[PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:618: test_climate_action_exact_wire	[PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:619: test_vcsec_rke_exact_wire	[PASSED]
test\test_native_ble_proto\test_ble_proto.cpp:620: test_vcsec_closure_exact_wire	[PASSED]
----------- native:test_native_ble_proto [PASSED] Took 1.75 seconds -----------

=================================== SUMMARY ===================================
Environment    Test                   Status    Duration
-------------  ---------------------  --------  ------------
native         test_native_ble_proto  PASSED    00:00:01.752
================= 18 test cases: 18 succeeded in 00:00:01.752 =================
```

Concerns unchanged from the original report; all production wire defects remain
reported and unfixed by design.
