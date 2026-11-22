# Task report — batched firmware fixes C1-05 / C1-11 / C1-12 / C1-13

Date: 2026-11-22
Branch: `chore/legacy-tesla-drug-fork-submodules` → pushed `8c9faba..bd34329`
Commit: `bd34329` — `fix(firmware): persist missing toggles, split 0x334 fields, drop MQTT claim`

## Per-item status

| Item  | Status                  | Summary                                                                                                                     |
| ----- | ----------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| C1-05 | **SKIPPED — STALE**     | Current `fieldEmpty(level/position)` is correct per SDK proto; switching to `fieldVarint` is a regression. No code changed. |
| C1-11 | **IMPLEMENTED — GREEN** | Four new NVS keys, version 0x0E→0x0F, load/save in both compiled backends, raw-CAN command now persists.                    |
| C1-12 | **IMPLEMENTED — GREEN** | Pedal mode moved to 0x334 byte0 bits 2:0; drive-mode keeps bits 6:5; both survive. CAN review still required.               |
| C1-13 | **IMPLEMENTED — GREEN** | Chose removal path: MQTT publishing claim removed from command reference + IO schema.                                       |

---

## C1-05 — CarServer seat encode (SKIPPED, stale)

**Brief requirement:** use `fieldVarint` with correct enum field numbers instead of `fieldEmpty(level)` / `fieldEmpty(position)`.

**Re-verification result — requirement is wrong for the SDK proto:**

- `firmware/lib/vehicle/ble/proto.h:153-156`: `fieldEmpty(n)` writes `tag(n, wire 2)` + zero length — an empty length-delimited message field.
- All four SDK proto mirrors in `legacy/` define the fields as `oneof` of **`Void` messages**, not varint enums:
    - `legacy/teslamotors-vehicle-command/pkg/protocol/protobuf/car_server.proto:214-238`
    - `legacy/tesla-local-control-vehicle-command/pkg/protocol/protobuf/car_server.proto:140-164`
    - `legacy/tesla-local-control-vehicle-command-seth/.../car_server.proto:140-164`
    - `legacy/sethterashima-vehicle-command/.../car_server.proto:202-225`
    - `seat_heater_level`: `SEAT_HEATER_UNKNOWN=1, OFF=2, LOW=3, MED=4, HIGH=5` (all `Void`).
    - `seat_position`: `CAR_SEAT_UNKNOWN=6, FRONT_LEFT=7, ..., REAR_RIGHT=12` (all `Void`).
- Generated Go confirms wire type 2: `carserver/car_server.pb.go:4103-4119` tags are `protobuf:"bytes,2,opt,name=SEAT_HEATER_OFF…"` etc. (`bytes` = wire type 2).
- `docs/legacy/tesla-local-control-tesla_ble_mqtt_core.md:366` already records "proto agrees exactly … already present (ours)".
- Firmware constants (`carserver.h:60-68`) match those field numbers exactly, so `fieldEmpty(level)` emits tag 3/4/5 wire 2 and `fieldEmpty(position)` emits tag 7/8/9/12 wire 2 — correct.
- Switching to `fieldVarint` would emit wire type 0 for a message-typed oneof member — a type mismatch that would corrupt the action. **Not applied.**

**Attempted native wire-byte test:** created `test_native_carserver`, but it failed to **compile** because `carserver.h:241,243` calls `Proto::fieldFloat()` which does not exist anywhere in `firmware/lib` (grep: only those two call sites). This is a separate pre-existing BLE-build defect; fixing it would require deciding the correct protobuf float field type (fixed32 vs fixed64/double), which is out of scope. Test directory removed before commit — no scratch files left.

**RED/GREEN:** N/A (stale; no code change). Compile-failure output above is the evidence.

---

## C1-11 — NVS key integrity (IMPLEMENTED)

**Changes**

- `firmware/lib/core/persist/keys.h`: `NVS_SETTINGS_VERSION` `0x0E` → `0x0F`; added `PKEY_REGION_SPOOF` (`rgnS`), `PKEY_ALC` (`alc`), `PKEY_AP_FIRST` (`apF`), `PKEY_RAW_CAN` (`rawC`).
- `firmware/lib/core/persist/esp32/board.h` + `firmware/lib/core/persist/native/board.h`: load/save `s.regionSpoofCode`, `s.alcAutoConfirmEnabled`, `s.apFirstEnabled`, `s.rawCanListen` (the two **compiled** persist backends).
- `firmware/lib/vehicle/can/feature/misc/can_raw.h`: `executeCanRawCmd` now calls `saveSettings(s)` — previously the only one of the four toggles whose command path never persisted.
- `firmware/test/support/fake_preferences.h`: added `putFloat/getFloat/putString/getString` and string-store clearing. The shipping native backend already used those methods but the fake lacked them, so the real backend could not be unit-tested.
- New `firmware/test/test_native_persist_extra/test_persist_extra.cpp`: true/false round-trips for all four settings plus version-bump assertion, using the real `core/persist/native/board.h`.

**NVS version-bump semantics (chosen, minimal safe):** simple monotonic increment. Active backends gate on `magic == NVS_SETTINGS_MAGIC && ver != 0` (they do **not** require exact version equality), so existing NVS data stays loadable; missing new keys fall back to defaults and are written at the next `saveSettings()`. No wipe/migration path added.

**Evidence note:** brief cited `nvs.h:63-101,120-156`, but `firmware/lib/core/persist/nvs.h` and `native.h` are uncompiled legacy duplicates (they lack the BLE-distance keys and are included nowhere — only `src/esp32/main.cpp:12` includes a persist header, `esp32/board.h`). Those two stale files were intentionally left untouched.

**TDD**
RED — `.\pio.ps1 test -e native -f test_native_persist_extra`:

```
test_persist_extra.cpp:41: test_new_settings_roundtrip_true: Expected 4 Was 0    [FAILED]
test_persist_extra.cpp:63: test_new_settings_roundtrip_false: Expected 0 Was 9   [FAILED]
test_persist_extra.cpp:79: test_settings_version_is_bumped: Expected 15 Was 14  [FAILED]
============= 4 test cases: 3 failed, 0 succeeded =============
```

GREEN — same command after fix:

```
test_persist_extra.cpp:85: test_new_settings_roundtrip_true      [PASSED]
test_persist_extra.cpp:86: test_new_settings_roundtrip_false     [PASSED]
test_persist_extra.cpp:87: test_settings_version_is_bumped       [PASSED]
============= 3 test cases: 3 succeeded in 00:00:01.312 =============
```

---

## C1-12 — 0x334 pedal / drive-mode bit collision (IMPLEMENTED)

**Changes**

- `firmware/lib/vehicle/can/feature/drive/pedal.h`: pedal mode values `0x00/0x20/0x40` → `0x00/0x01/0x02`, mask `~0x60` → `~0x07` (byte0 bits 2:0). Still copies all 8 bytes from `lastDrive` and recomputes `data[7] = driveChecksum(...)`; no new injection, same `startBurst(..., BUS_VEHICLE, 30, 20)`.
- `firmware/lib/vehicle/can/feature/drive/drive_mode.h`: **unchanged** — keeps bits 6:5 (the DBC `UI_pedalMap` field), same counter copy, same `driveChecksum`.
- `firmware/test/test_native_pedal/test_pedal.cpp`: expectations updated to mask `0x07` / values `0x00,0x01,0x02`.
- `firmware/test/test_native_multi_feature/test_multi_feature.cpp`: new `test_pedal_and_drive_mode_bitfields_do_not_collide` — writes pedal first then rebuilds drive frame from it, and reverses the order; asserts both fields survive.

**TDD**
RED — `.\pio.ps1 test -e native -f test_native_multi_feature`:

```
test_multi_feature.cpp:117: test_pedal_and_drive_mode_bitfields_do_not_collide: Expected 1 Was 0 [FAILED]
============= 6 test cases: 1 failed, 4 succeeded in 00:00:01.489 =============
```

GREEN — same command after fix:

```
test_multi_feature.cpp:134: test_pedal_and_drive_mode_bitfields_do_not_collide [PASSED]
============= 5 test cases: 5 succeeded in 00:00:02.064 =============
```

`.\pio.ps1 test -e native -f test_native_pedal`: 7/7 passed after expectation update.

**CAN review checklist — REQUIRED (per brief).** Status for controller/reviewer:

- §1 frame mutation: both builders still DLC-8, copy `lastDrive`, write only their own mask, recompute checksum. Counter byte unchanged (copied from cache) — **no** counter rotation added.
- §2 bit ownership: documented here — 0x334 byte0 bits **2:0** = pedal mode, bits **6:5** = drive-mode override; byte6 high nibble counter and byte7 checksum untouched.
- §3 variants: `executePedalCmd` still blocks LEGACY; drive-mode gate (`hasDrive` + `apGateOpen`) unchanged.
- §5 regression: exact bit-level assertions added in both directions.
- §8 acceptance: bit-level asserted; **listen-only bench capture still required** before shipping — no capture available in this environment.

**Concern (flagged):** community DBC (`docs/legacy/joshwardell-model3dbc.md:58,268`, `canhackers-jupiter.md:283`) maps byte0 bits 0-4 to `UI_systemPowerLimit`. Moving pedal mode to bits 2:0 therefore overlaps the low 3 bits of that power-limit field. Non-overlap between the two firmware features is achieved, but vehicle-side semantics of that overlap are unverified — needs the capture/can-review above.

---

## C1-13 — MQTT publish loop (IMPLEMENTED — removal path)

**Choice justification (per "pick the minimal safe option"):** firmware has no MQTT client dependency (`platformio.ini` `lib_deps` has no PubSubClient), `mqttShouldPublish()` has no caller outside tests, and `mqttConnected` is never set (`analysis/current-state-2026-11-22/firmware.md:237`, N8). Implementing a broker client would mean new, unverifiable network code; faking `mqttConnected = 1` would be a dishonest status. Removal of the false claim is the minimal safe option and is explicitly authorized by the brief.

**Changes**

- `docs/reference/commands.md`: MQTT section replaced with a "configuration only — not implemented" note; removes publish/connected claims.
- `firmware/lib/io/schemas/io.schema.json`: removed 5 `mqtt:*` command entries and the `mqttBridge` feature. Kept `mqtt`/`mqttConnected` in `FirmwareIdentity` because firmware still emits them truthfully (toggle value and constant 0).
- New `firmware/test/test_native_mqtt_contract/test_mqtt_contract.cpp`: asserts schema no longer advertises `mqttBridge`/`mqtt:on` and docs explicitly say "not implemented" / "no publish loop".

**TDD note (transparent):** this is a documentation/contract removal, so there was no pre-existing failing firmware behavior to turn RED. The contract test is GREEN on first run and locks in the removal; it is not a RED→GREEN TDD cycle.

**Concern (flagged):** brief offered removal from "docs/schema", while the task envelope said "firmware files only". I touched one docs file (`docs/reference/commands.md`) because the brief explicitly authorizes it; the schema is a firmware file. Client UI (`client/src/components/monitor/sections/ConnectionSection.tsx`, `IntegrationPanel.tsx`) still renders MQTT on/off controls and the firmware still accepts/persists `mqtt:*` config commands. Full UI cleanup is a follow-up, out of scope for this small-fix batch.

---

## Validation (all run before commit)

| Command                                                 | Result                                                          |
| ------------------------------------------------------- | --------------------------------------------------------------- |
| `.\pio.ps1 test -e native -f test_native_multi_feature` | 5/5 passed (RED first: 1 failed)                                |
| `.\pio.ps1 test -e native -f test_native_persist_extra` | 3/3 passed (RED first: 3 failed)                                |
| `.\pio.ps1 test -e native -f test_native_pedal`         | 7/7 passed                                                      |
| `.\pio.ps1 test -e native -f test_native_mqtt_contract` | 2/2 passed                                                      |
| `.\pio.ps1 test -e native` (full, once)                 | **1059 test cases: 1059 succeeded** in 00:01:09                 |
| `npm run lint:all`                                      | ESLint clean, Prettier clean, markdownlint 0 issues in 44 files |
| `npm run validate:serial-contract`                      | `Validated unified io.schema.json contract.`                    |

No firmware `.cpp` source outside tests was compiled to ESP32 target; only native targets were built, per task scope.

## Self-review / worktree hygiene

- 201+ unrelated unstaged files pre-existed. Staged only the 12 files I changed via explicit `git add <paths>`; verified with `git diff --cached --name-only` before commit. No `git add -A` / `git add .`.
- Staged list: `docs/reference/commands.md`, `firmware/lib/core/persist/{keys.h,esp32/board.h,native/board.h}`, `firmware/lib/io/schemas/io.schema.json`, `firmware/lib/vehicle/can/feature/drive/pedal.h`, `firmware/lib/vehicle/can/feature/misc/can_raw.h`, `firmware/test/support/fake_preferences.h`, and 4 test files (`test_native_multi_feature`, `test_native_persist_extra` [new], `test_native_pedal`, `test_native_mqtt_contract` [new]).
- Husky/lint-staged ran Prettier on the staged JSON/MD; no semantic changes introduced.
- Commit `bd34329` created on the current feature branch (never main), pushed with plain `git push` — no force, no reset.
- No scratch files left: failed `test_native_carserver` directory removed before staging.

## Concerns summary

1. C1-05 requirement is stale; not implemented. Separate pre-existing BLE build break (`Proto::fieldFloat` missing) blocks a wire-byte test of `carserver.h`.
2. C1-12: can-review checklist + listen-only bench capture still required; DBC overlap with `UI_systemPowerLimit` low bits needs vehicle-side verification.
3. C1-13: removal chosen over implementation; client UI and command handlers still expose MQTT config. Possibly needs controller confirmation given "firmware files only" envelope.
4. `firmware/lib/core/persist/{nvs.h,native.h}` remain stale duplicates without the new keys; active backends are correct.

---

# Fix round 1/5 — C1-12 revert (controller ruling)

## Status change

| Item  | Previous status | New status                                       |
| ----- | --------------- | ------------------------------------------------ |
| C1-12 | IMPLEMENTED     | **CAPTURE_FIRST — explicitly not a silent drop** |

C1-12 is returned to CAPTURE_FIRST. No bit-field change ships. Any future change requires the repo's DBC note (`docs/legacy/joshwardell-model3dbc.md:58,268` — `UI_pedalMap` at 0x334 byte0 bits 5|2) plus a listen-only bench capture before implementation. Controller finding accepted: bits 2:0 have no documented pedal field, so separating the fields internally would likely break the real pedal map even though the collision test went green.

## What changed

Revert only, using `git show bd34329^:<path>` as the authoritative reference:

- `firmware/lib/vehicle/can/feature/drive/pedal.h` — restored original encoding: `value = 0x20 (CHILL) / 0x40 (SPORT) / 0x00 (STD)` and mask `~0x60` ("bits 5-6 encode pedal mode"); removed the bits-2:0 comment. Checksum recompute and `startBurst(..., BUS_VEHICLE, 30, 20)` unchanged.
- `firmware/test/test_native_multi_feature/test_multi_feature.cpp` — removed the C1-12 test additions: `#include "feature/drive/pedal.h"`, `#include "feature/drive/drive_mode.h"`, `void driverSend(const Frame &, uint8_t) {}`, `test_pedal_and_drive_mode_bitfields_do_not_collide()` and its `RUN_TEST` line.
- `firmware/test/test_native_pedal/test_pedal.cpp` — reverted the three expectations from mask `0x07` / values `0x00,0x01,0x02` back to mask `0x60` / values `0x00,0x20,0x40`. Required so the touched pedal suite stays green with the reverted production code.

No other production file touched. C1-11 (NVS keys) and C1-13 (MQTT claim removal) remain exactly as committed in `bd34329`.

Revert verified as the exact inverse of the C1-12 portion of `bd34329`: `git diff --stat` shows 3 files, 6 insertions / 40 deletions; resulting blobs match `bd34329^` for all three paths.

## Covering test names

- `test_native_multi_feature`: `test_independent_persist_flags_dont_collide`, `test_multiple_state_features_active_simultaneously`, `test_disabling_one_feature_leaves_others_intact`, `test_handlers_do_not_steal_each_others_commands`.
- `test_native_pedal`: `test_pedal_standard`, `test_pedal_std_alias`, `test_pedal_chill`, `test_pedal_sport`, `test_pedal_legacy_blocks`, `test_pedal_no_drive_blocks`, `test_pedal_unknown`.

## Exact commands and output

Command 1 (from `firmware/`):

```
.\pio.ps1 test -e native -f test_native_multi_feature
```

Output:

```
Collected 88 tests

Processing test_native_multi_feature in native environment
Building...
Testing...
test\test_native_multi_feature\test_multi_feature.cpp:100: test_independent_persist_flags_dont_collide	[PASSED]
test\test_native_multi_feature\test_multi_feature.cpp:101: test_multiple_state_features_active_simultaneously	[PASSED]
test\test_native_multi_feature\test_multi_feature.cpp:102: test_disabling_one_feature_leaves_others_intact	[PASSED]
test\test_native_multi_feature\test_multi_feature.cpp:103: test_handlers_do_not_steal_each_others_commands	[PASSED]
--------- native:test_native_multi_feature [PASSED] Took 1.00 seconds ---------
================== 4 test cases: 4 succeeded in 00:00:01.004 ==================
```

Command 2 (from `firmware/`):

```
.\pio.ps1 test -e native -f test_native_pedal
```

Output:

```
Collected 88 tests

Processing test_native_pedal in native environment
Building...
Testing...
test\test_native_pedal\test_pedal.cpp:77: test_pedal_standard	[PASSED]
test\test_native_pedal\test_pedal.cpp:78: test_pedal_std_alias	[PASSED]
test\test_native_pedal\test_pedal.cpp:79: test_pedal_chill	[PASSED]
test\test_native_pedal\test_pedal.cpp:80: test_pedal_sport	[PASSED]
test\test_native_pedal\test_pedal.cpp:81: test_pedal_legacy_blocks	[PASSED]
test\test_native_pedal\test_pedal.cpp:82: test_pedal_no_drive_blocks	[PASSED]
test\test_native_pedal\test_pedal.cpp:83: test_pedal_unknown	[PASSED]
------------- native:test_native_pedal [PASSED] Took 1.02 seconds -------------
================== 7 test cases: 7 succeeded in 00:00:01.016 ==================
```

Command 3 (full regression, from `firmware/`):

```
.\pio.ps1 test -e native
```

Output (tail):

```
=============== 1058 test cases: 1058 succeeded in 00:01:02.433 ===============
```

1058 = prior 1059 minus the one removed C1-12 collision test. No failures.

## Worktree hygiene for this round

- 198 unrelated unstaged files pre-existed. Staged only the three files changed this round via explicit `git add <paths>`; `git diff --cached --name-only` verified before commit. No `git add -A` / `git add .`.
- Commit message: `fix(firmware): revert unvalidated 0x334 pedal bit reassignment`.
- Pushed with plain `git push` to `origin/chore/legacy-tesla-drug-fork-submodules`; never main, no force.

## Remaining concerns after round 1

- C1-12 remains CAPTURE_FIRST: original byte0 bits 5|2 collision between pedal and drive-mode override is knowingly left in place. It needs DBC confirmation + bench capture before any fix; unify under one owner of `UI_pedalMap` rather than inventing a new field.
- C1-05 stale and C1-13 client-UI follow-up unchanged from the original report.
