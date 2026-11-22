# Current-State Synthesis — 2026-11-22 (authoritative)

Integration output for task **P0-00**. This file replaces the provisional synthesis. It consolidates four fresh, read-only component reports:

- `current-state-2026-11-22/firmware.md` (Firmware)
- `current-state-2026-11-22/client-protocol.md` (Client + Protocol)
- `current-state-2026-11-22/tools-ci.md` (Tools + CI)
- `current-state-2026-11-22/cross-cutting.md` (Cross-cutting drift)

All four reports cite `file:line` in the working tree at branch `chore/legacy-tesla-drug-fork-submodules`, HEAD `58da72b3` (cross-cutting.md:5). The tree carries 191 pre-existing unstaged changes; this pass changed no code and staged only this file plus `.opencode/plans/roadmap-2026-11-22.md`.

Evidence grades are inherited from the component reports: **CODE-READ** (source line read), **TEST-READ** (test source read), **CAPTURE** (real frame/wire data), **SUMMARY-ONLY** (inherited from docs or repo-state), **UNVERIFIED** (not checked). A claim's grade is stated where it matters.

---

## 0. Executive verdict

The repo is **not** in a shippable correctness state. Static code review found three classes of defect that are invisible to the current test suite:

1. **Transport-visible wire contracts are wrong.** BLE `RoutableMessage` field numbers, add-key schema, role/form-factor constants, and CarServer seat encoding disagree with the upstream proto checked out under `legacy/` (firmware.md §3.2, §7.1#2–4, §7.1#7). No BLE wire test exists (firmware.md §5, §7.2#10).
2. **Injection safety gates are fail-open or bypassed.** The advertised `0x318` OTA TX guard has no implementation; `apGateOpen()` is true when the gate is disabled, treats gear 0/5 as Park, and `dasTick` never consults the gate; the documented `das:arm` has no handler (firmware.md §2.3, §7.1#1, §7.1#5; cross-cutting.md §2.2).
3. **The advertised command surface is partly fictional.** 144 builders are implemented, 21 have no firmware handler, 29 are build-gated (BLE), and ~31 dead wire strings reach `sendError("Unknown command")` (cross-cutting.md §2, §8; client-protocol.md §5). The cross-check test that claims to be "single source of truth" hardcodes its list and never opens a firmware file (client-protocol.md §3, §7 N2).

Everything corrective is **blocked on evidence discipline**: 20 of 24 CAN contradictions are CAPTURE_FIRST (cross-cutting.md §4, §8), no build/test/capture was run in this pass, and GitHub Actions runtime status is unknown (tools-ci.md §7 UNVERIFIED). The roadmap therefore front-loads correctness/security, then CI truth + capture harness, then hardware spikes, then the UI rewrite on frozen contracts.

**No claim in this synthesis is a runtime proof.** See §8.

---

## 1. Verified tree state

### 1.1 Firmware

- **CODE-READ** `git ls-files firmware` = 239 tracked files; layout matches the post-2026-06-20 reorg; legacy `lib/interface/`, `lib/transport/`, `lib/feature/`, `lib/handler/`, `lib/infra/` trees do not exist (firmware.md §1.1).
- **CODE-READ** `platformio.ini` = 571 lines, 30+ env sections; `default_envs = esp32_chassis_8mhz`; all buses opt-in via `BUS_*_ACTIVE`; `upload_port`/`monitor_port = COM4` still hardcoded (firmware.md §1.2; `platformio.ini:99-100`).
- **CODE-READ** CAN RX path `src/esp32/main.cpp:199-213` → `ringPush`/`sendFrame`/`handleMessage`; dispatch by bus at `handler/dispatch.h:27-46` (firmware.md §2.1).
- **CODE-READ** 84 tracked test suites (6 hardware, 78 native); only one BLE-named suite exists and it tests mode-name round trips, not wire/proto bytes (firmware.md §5).

### 1.2 Client + Protocol

- **CODE-READ** `packages/protocol/src` = 11 files, `client/src` = 102 files, `client/tests` = 45 files, `packages/protocol/test` = 27 files (client-protocol.md, Scope). Protocol is a real shared library consumed via `@teslacanmodder/protocol`, with one deliberate cross-workspace import exception at `ControlsScreen.tsx:9` (client-protocol.md §1, P2-008).
- **CODE-READ** Provider tree `BoardConnectionContext.tsx:78-115` composes Transport → BoardState → CommandProvider; compat hook at `:119-173` (client-protocol.md §2).
- **CODE-READ** God components: `AppExperience.tsx` 1148 lines with duplicate local state at `:125-152` and its own `runCommand` at `:385-444`; `ControlsBlocks.tsx` 1189; `DriveScreen.tsx` 855 with 1 test (client-protocol.md §2).
- **CODE-READ (grep-counted)** ~302 client test cases, ~455 protocol test cases; none executed (client-protocol.md §3, §7 UNVERIFIED).

### 1.3 Tools + CI

- **CODE-READ** `tools/debug.js` wires 10 session commands plus `flash` and `export`; `tools/README.md:24-35` omits `das-drive` and `export` (tools-ci.md §1.1, N-08).
- **CODE-READ** `tools/serial-http-bridge.js:24,81-83,147-186` — bind `0.0.0.0`, CORS `*`, no auth, arbitrary `cmd` proxy, unbounded body, no timeouts (tools-ci.md §1.3, P3-001..003).
- **CAPTURE (local)** `npm test -w tools` → 9 suites, 105 tests passing; `npm run validate:serial-contract` green (tools-ci.md §1.5, §2.2).
- **CODE-READ** 11 CI jobs in `.github/workflows/ci.yml:17-264`, each repeating the same legacy-folder exception block (tools-ci.md §2.1).

### 1.4 Cross-cutting

- **CODE-READ** `docs/reference/commands.md:57` points at removed `firmware/lib/interface/dispatch.h`; real dispatcher is `firmware/lib/client/command/dispatch.h:25` (cross-cutting.md §1 D1).
- **CODE-READ** `docs/reference/serial-contract.md:62` says 768-byte serial buffer; both headers define `SERIAL_CMD_BUFFER_SIZE 64` (`io/serial/usb/esp32/state.h:13-14`, `common.h:23-24`) (cross-cutting.md §1 D5).
- **CODE-READ** `package.json:40-42` requires Node ≥22 while `README.md:83`, `AGENTS.md:79`, `CONTRIBUTING.md:20` say ≥18 (cross-cutting.md §1 D6).
- **CODE-READ** 20 doc/code drift rows D1–D20: 17 actionable, 3 info/duplicate, 0 confirmed fixed (cross-cutting.md §1, §8).

---

## 2. Old-todo recheck — deduplicated

### 2.1 Raw tallies as reported

| Report            | Item set               | FIXED | STILL_PRESENT | STALE | Total | Citation                        |
| ----------------- | ---------------------- | ----- | ------------- | ----- | ----- | ------------------------------- |
| Firmware          | P1 + P5 + P9           | 7     | 31            | 3     | 41    | firmware.md §6, `:221`          |
| Client + Protocol | P2 + P6 + PA           | 5     | 34            | 4     | 43    | client-protocol.md §4, `:156`   |
| Tools + CI        | P3 + P4 + P7 + PB + PC | 1     | 49            | 3     | 53    | tools-ci.md §4, `:107`          |
| Cross-cutting     | P4 + P8 + PC           | 1     | 21            | 3     | 25    | cross-cutting.md §5, `:218`, §8 |

Naive sum = **162 items**. This is wrong: two reports re-check the same IDs.

### 2.2 Deduplication by ID

- **Firmware** IDs P1/P5/P9 appear only in firmware.md → 41 unique.
- **Client** IDs P2/P6/PA appear only in client-protocol.md → 43 unique.
- **Tools** IDs P3/P7/PB appear only in tools-ci.md; **P4-001..010** and **PC-001..008** appear in **both** tools-ci.md §4.2/§4.5 and cross-cutting.md §5 → 18 IDs are double-counted.
- **Cross-cutting** unique contribution is only **P8-001..005** (5 IDs).

Deduplicated union = **41 + 43 + 53 + 5 = 142 unique todo IDs**, not 162.

### 2.3 Deduplicated status roll-up

| Status        | Count   | Basis                                                |
| ------------- | ------- | ---------------------------------------------------- |
| FIXED         | ≈13     | 7 firmware + 5 client + 1 tools; no P8 item is FIXED |
| STILL_PRESENT | ≈118    | 31 firmware + 34 client + 49 tools + 4 P8            |
| STALE         | ≈11     | 3 firmware + 4 client + 3 tools + 1 P8               |
| **Total**     | **142** | 41 + 43 + 53 + 5 unique IDs                          |

Caveats on this roll-up (do not treat as exact):

- One duplicate ID has **conflicting status**: **P4-008** is STALE in tools-ci.md §4.2 (no `.husky/` tracked or on disk) but STILL_PRESENT in cross-cutting.md §5 (lint-staged `.md` → Prettier only, `package.json:31-39`). Both sub-claims are valid; resolved here as PRESENT with a STALE sub-claim. ±1 sensitivity.
- Cross-cutting.md states 25 items (§5, §8) but its §5 table enumerates 23 rows (P4×10, P8×5, PC×8). The P8-only union arithmetic is unaffected; the reported +2 is an internal discrepancy. Marked UNVERIFIED.
- Client-protocol.md PA-004 row (`:146`) has a corrupted pipe in its evidence cell; the STALE verdict is still legible.

### 2.4 Subsystem highlights

- Firmware P1 criticals still live: seat encode `fieldEmpty` (`carserver.h:222-225`), `rawCanListen` no NVS key (`dispatch.h:138-144`, `nvs.h:63-101`), 0x334 pedal/drive-mode collision (`pedal.h:38-48` vs `drive_mode.h:94-110`), gear 0/5 as Park (`vehicle.h:392-393`) (firmware.md §6.1).
- Client all 11 P2 items STILL_PRESENT: dead `commandLifecycle` (`CommandContext.tsx:100-101,132-180`), four self-referential gates (`gating.ts:762-767,778-783,794-813,816-821`) (client-protocol.md §4 P2).
- Tools: 0 FIXED in P3/P4/P7/PB; only PC-008 FIXED (`firmware/README.md:72-84`) (tools-ci.md §4).
- Cross-cutting: 3 save-without-key fields — `region:spoof:*`, `alc`, `ap-first` call `saveSettings` with no NVS key (cross-cutting.md §3, N4).

---

## 3. Advertised vs implemented command verdict

Counts are by **builder function** in `packages/protocol/src/commands.ts:108-540` (194 functions), compared against `firmware/lib/client/command/dispatch.h:25-1086` and `docs/reference/commands.md` (cross-cutting.md §2; client-protocol.md §5).

| Verdict                      | Count | Meaning                                                                | Citation                  |
| ---------------------------- | ----- | ---------------------------------------------------------------------- | ------------------------- |
| IMPLEMENTED                  | 144   | firmware handler exists                                                | cross-cutting.md §2.1, §8 |
| NO_HANDLER                   | 21    | falls through to `sendError("Unknown command")` at `dispatch.h:1085`   | cross-cutting.md §2.2, §8 |
| UNCERTAIN                    | 29    | implemented only inside `#if BOARD_ENABLE_BLE` (`dispatch.h:909-1083`) | cross-cutting.md §2.3, §8 |
| Dead wire strings / patterns | ~31   | distinct strings advertised with no handler                            | cross-cutting.md §8       |

Notes:

- client-protocol.md §5 gives "roughly 29 advertised strings across 12 feature groups" derived from builder arity in `cross-check.test.ts:417-647`, and flags it as approximate; cross-cutting.md §8 gives 31 patterns. Treat **29–31** as the honest range; the authoritative figure needs a machine-deduped literal scan.
- NO_HANDLER families: `smt:*`, `elm327:*`, `gvret:*`, `ha:*`, `espnow:*`, `speedalert:*`, `btnmap:*`, `teslable:*`, `bleencrypt:*`, `lanegraph:*`, `apfirst:*` (cross-cutting.md §2.2 table). `apfirst`/`lanegraph` are spelling drift — firmware accepts `ap-first:*`/`lane-graph:*` at `dispatch.h:579,588,619,628` (cross-cutting.md §1 D13).
- Four gaps beyond the known 11: `teslable:*`, `bleencrypt:*`, naming drift, and firmware-only `wifi:status`/`wifi:config` (`io/wifi/esp32/cmd.h:50-58`) absent from protocol/docs (cross-cutting.md §2.2).
- Non-command ABSENT surfaces: 0x318 OTA guard, MQTT publish loop (`mqttShouldPublish` has no caller; `mqttConnected` never true), 0x401 per-cell decode, 0-100 timer (cross-cutting.md §2.2; repo-state `§C`). The timer is SUMMARY-ONLY/UNVERIFIED scope, not a dispatch gap.
- Reverse drift: `log`, `platform`, `recorder:*` are implemented but silently early-return on `!s.hasCtrl` at `dispatch.h:422-427` while their handlers sit at `:679`, `:836`, `:872` (cross-cutting.md §1 D14, N2).
- Test blind spot: `cross-check.test.ts:35-361` hardcodes `FIRMWARE_WIRE_COMMANDS` and `:708` whitelists the `apfirst`/`lanegraph` mismatch, so no test opens a firmware file (client-protocol.md §3, §7 N2).

---

## 4. Top critical findings by subsystem

### 4.1 Firmware

| #   | Finding                                                                                                                                                                                                       | Evidence                                                                                                             | Grade                            |
| --- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------- | -------------------------------- |
| F1  | No OTA TX guard: `0x318` defined but unhandled; `otaInProgress`/`txPaused` absent from all `.h` files and `State`, though advertised in schema                                                                | `vehicle/can/ids.h:52`; `io/schemas/io.schema.json:1205-1206`; firmware.md §2.3, §7.1#1                              | CODE-READ (negative grep)        |
| F2  | BLE `RoutableMessage` field numbers wrong: `to_destination` 2 vs 6, bytes 5 vs 10, session req 6 vs 14, info 10 vs 15, uuid 14 vs 51                                                                          | `msg.h:90-92,131-133,312-314,364-368` vs `universal_message.proto:84-99`; firmware.md §3.2, §7.1#2                   | CODE-READ                        |
| F3  | BLE add-key encoded against non-existent schema: flat `UnsignedMessage.field 2` instead of `WhitelistOperation=16 → addKeyToWhitelistAndAddPermissions=5 → PermissionChange{key=1 PublicKeyRaw=1, keyRole=4}` | `msg.h:63-95`; `vcsec.proto:104-108,121-127,43-45`; firmware.md §7.1#3                                               | CODE-READ                        |
| F4  | BLE roles/form-factor wrong: OWNER=4, CHARGING_MANAGER=5, NFC_CARD=3 vs upstream 2/6/1                                                                                                                        | `key.h:22-23`, `msg.h:25`, `tesla_board.h:124-127`; firmware.md §7.1#4                                               | CODE-READ                        |
| F5  | AP gate fail-open + DAS bypass: true when disabled, gear 0/5 counted Park, forced true on CAN timeout, `dasTick` never checks gate, `das:arm` unimplemented                                                   | `core/types.h:837-842`; `vehicle.h:392-393`; `main.cpp:149`; `das_drive.h:414-498,515-519`; firmware.md §2.3, §7.1#5 | CODE-READ                        |
| F6  | ALC `0x249` frame shape wrong: DLC-3 additive magic CRC vs capture DLC-4                                                                                                                                      | `auto_lane_change.h:48-59`; firmware.md §7.2 / repo-state `:29`; cross-cutting.md §4 C7                              | CODE-READ + SUMMARY-ONLY CAPTURE |
| F7  | CarServer seat encode emits malformed protobuf (enum used as field number with wire type 2, not varint)                                                                                                       | `carserver.h:222-225`; firmware.md §7.1#7                                                                            | CODE-READ                        |
| F8  | Scan filter cannot match real advert: last-8-VIN suffix vs `S+sha1(VIN)[:8]C`                                                                                                                                 | `dispatch.h:36-43`, `ble_client.h:116-120`; repo-state `:28`; firmware.md §7.2#9                                     | CODE-READ + SUMMARY-ONLY         |

### 4.2 Client + Protocol

| #   | Finding                                                                                | Evidence                                                                                      | Grade                |
| --- | -------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------- | -------------------- |
| C1  | 12 ghost groups / ~29–31 wire strings have no firmware handler                         | `commands.ts:408-468,522-524`; `dispatch.h:1085`; client-protocol.md §5, §7 N1                | CODE-READ (negative) |
| C2  | Cross-check test fakes firmware truth with a hardcoded list and explicit whitelist     | `cross-check.test.ts:35-361,708`; client-protocol.md §7 N2                                    | TEST-READ            |
| C3  | Provider path drops `dispatchActions` → permanently empty command lifecycle            | `CommandContext.tsx:132-180` vs `AppExperience.tsx:418-421`; client-protocol.md §7 N3, P2-001 | CODE-READ            |
| C4  | Unreachable `nag` gate: registry keys are `nagMode`/`nagBypass`, gate defined on `nag` | `gating.ts:52-57`; `commands.ts:129,133`; client-protocol.md §7 N4                            | CODE-READ            |
| C5  | `board.ts` drops `eap`/`evd`/`tlssc` and casts wire payload without validation         | `board.ts:88-104,145-146`; `types.ts:13-15`; client-protocol.md §7 N6/N9                      | CODE-READ            |

### 4.3 Tools + CI

| #   | Finding                                                                                                                                | Evidence                                                                                        | Grade                       |
| --- | -------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------- | --------------------------- |
| T1  | `serial-http-bridge.js` is an unauthenticated LAN command proxy                                                                        | `:24,81-83,88-94,111-193,164`; tools-ci.md §6 N-01                                              | CODE-READ                   |
| T2  | Release can publish with no firmware binaries and no firmware tests: `fail_on_unmatched_files: false`, `quality-gate` omits `pio test` | `release.yml:159,47-62`; tools-ci.md §6 N-02                                                    | CODE-READ                   |
| T3  | `release-matrix-smoke.mjs` dead: live dry-run exits 1 because `release.yml` moved to dynamic env discovery                             | `scripts/release-matrix-smoke.mjs:109-128`; `release.yml:68-94,103-106`; tools-ci.md §2.2, N-03 | CODE-READ + CAPTURE (local) |
| T4  | Serial-contract gate self-disables if schema absent                                                                                    | `scripts/validate-serial-contract.mjs:181-188`; tools-ci.md §6 N-05                             | CODE-READ                   |
| T5  | All 7 disputed CAN IDs have zero tool-side decode or golden-capture support; `0x118` only as export fixture                            | tools-ci.md §5.2, N-07                                                                          | CODE-READ (negative grep)   |

### 4.4 Cross-cutting

| #   | Finding                                                                                                    | Evidence                                                                                                    | Grade                            |
| --- | ---------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------- | -------------------------------- |
| X1  | `region:spoof`, `alc`, `ap-first` call `saveSettings` with no NVS key → DATA LOSS despite "persisted" docs | `region.h:175-185`, `auto_lane_change.h:126-134`, `dispatch.h:619-635`, `keys.h:11-58`; cross-cutting.md §3 | CODE-READ                        |
| X2  | Serial buffer doc 768 vs code 64                                                                           | `serial-contract.md:62`, `state.h:13-14`, `common.h:23-24`; cross-cutting.md §1 D5                          | CODE-READ                        |
| X3  | Node floor ≥22 vs ≥18 across docs                                                                          | `package.json:40-42`, `README.md:83`, `AGENTS.md:79`, `CONTRIBUTING.md:20`; cross-cutting.md §1 D6          | CODE-READ                        |
| X4  | `0x3F5` turn requests field mismatch (ours 2-bit enum vs captured separate bits with source enum)          | `turn_signal.h:15-31`, `joshwardell:370`; cross-cutting.md §4 C19                                           | CODE-READ + SUMMARY-ONLY CAPTURE |

**Top five across all subsystems** (for triage ordering): F1 (OTA guard), F2 (BLE envelope), F3+F4 (add-key/roles), F5 (AP gate/DAS bypass), F6 (ALC 0x249, S0 capture-first).

---

## 5. CAN decode contradiction register

From cross-cutting.md §4 (rows C1–C24) and repo-state §A/§B. Classification: **DOC_ONLY** = docs wrong, code fine; **SETTLED_BY_CAPTURE** = upstream corpus + our code agree; **CAPTURE_FIRST** = genuine conflict, do not flip on code-only evidence.

| Class              | Count  | Rows                                                                        |
| ------------------ | ------ | --------------------------------------------------------------------------- |
| DOC_ONLY           | 3      | C1 region nibble, C2 0x334 byte, C3 DAS status field names                  |
| SETTLED_BY_CAPTURE | 1      | C4 0x370 checksum (`nagChecksum` matched 25,288 frames; `MAGIC_0x370` dead) |
| CAPTURE_FIRST      | **20** | C5–C24                                                                      |

The 20 CAPTURE_FIRST rows (all capture evidence inherited from `docs/legacy/*.md` §11 tables, **not re-run this session** — cross-cutting.md §0, §4 closing note):

| Row | Topic               | Ours                                             | Upstream claim                                                                       | Cite                                                     |
| --- | ------------------- | ------------------------------------------------ | ------------------------------------------------------------------------------------ | -------------------------------------------------------- |
| C5  | 0x370 counter byte  | `data[1] & 0x0F`                                 | AP1 byte6 low nibble; our modes split byte6/byte1                                    | `amzoo:262-263`, `ev-open-can-tools.md:515`, `nag.h:462` |
| C6  | 0x229 role/CRC      | "EPAS_harness", CRC high nibble `data[0]`        | gear-stalk, CRC full byte0; our magic fits 0/178                                     | `uhi22-tesla-crc.md:218-219`                             |
| C7  | 0x249 ALC frame     | DLC 3, stalk bits `data[1][7:6]`, additive magic | DLC 4, stalk enum `b2[3:0]`, masked counter; magic fits 870/870 upstream, 0/870 ours | `auto_lane_change.h:51,54,58`, `uhi22:220-222`           |
| C8  | Parked gate gear    | gear 1 \|\| 0 \|\| 5                             | fail-closed gear==1 only (upstream capture-fixed)                                    | `vehicle.h:393`, `ev-open-can-tools.md:510`              |
| C9  | 0x118 gear/pedal    | byte0 bits3:1 / byte1 percent                    | byte2 bits[7:5] / byte2 ×0.4                                                         | `ev-open-can-tools.md:509`, `joshwardell:374-375`        |
| C10 | 0x257 speed         | signed16 bytes2-3 /100                           | 12-bit @bit12 ×0.08 −40                                                              | `hanswolff:346`, `joshwardell:377`                       |
| C11 | 0x106/0x115 RPM     | big-endian                                       | LE signed16                                                                          | `hanswolff:347`                                          |
| C12 | 0x219 TPMS          | DLC 8 grouped `[p×4,t×4]`                        | DLC 5 wheel-muxed; `0x31F` DLC 8 interleaved                                         | `joshwardell:380`, `amzoo:270`                           |
| C13 | 0x392 meaning       | GTW firmware version                             | BMS_packConfig m0/m1                                                                 | `joshwardell:386`                                        |
| C14 | 0x318 meaning       | GTW_carState / OTA guard                         | RTC timestamp                                                                        | `hanswolff:351`, `joshwardell:387`                       |
| C15 | 0x3F3 meaning       | VCRIGHT seatbelt                                 | UI_odo DLC3 ChassisBus                                                               | `joshwardell:388`                                        |
| C16 | 0x292 SoC labels    | bits0-9=soc, bits10-19=socUI                     | UI=bits0-9, min=bits10-19                                                            | `hanswolff:344`, `joshwardell:381`                       |
| C17 | 0x352 energy        | muxed 16-bit ×0.02                               | unmuxed six 10-bit ×0.1                                                              | `hanswolff:349`, `joshwardell:384`                       |
| C18 | 0x132 current       | bytes2-3 signed ×0.1                             | smooth bits16-30 ×−0.01; raw ×−0.05+500                                              | `hanswolff:342`                                          |
| C19 | 0x3F5 turn requests | one 2-bit field LEFT=1/RIGHT=2/HAZARD=3          | separate bits + source enum                                                          | `joshwardell:370`, `turn_signal.h:15-31`                 |
| C20 | 0x273 summon bits   | bit4 active, bit5 dir, bit0 start                | bit5 frunk request, bit0 accessory power                                             | `joshwardell:362`                                        |
| C21 | 0x334 regen         | byte2 raw 0-200                                  | byte3 ×0.5                                                                           | `joshwardell:369`, `regen.h:31`                          |
| C22 | 0x3FD m0 bits       | set 38,39,46,59,60                               | 2021 DBC defines 46 at m1 only                                                       | `joshwardell:343`                                        |
| C23 | 0x3F8 drivingSide   | clears bit41 only                                | 2-bit field, we clear 1 of 2 bits                                                    | `joshwardell:350`, `hw4.h:96-98`                         |
| C24 | 0x2B9 jerk scale    | 0.018 / −9.1 / 0.034                             | DBC 0.03 / −15.232 / 0.059; own safety comment ±5.0                                  | `joshwardell:359`                                        |

**Rule:** no edit to C5–C24 lands without `docs/checklists/can-review-checklist.md` and a listen-only X179 bench capture (firmware.md §7 severity note; cross-cutting.md §4 closing). Note C8 is already SETTLED upstream but our code is not aligned — still needs bench confirmation before flip (cross-cutting.md §4 C8). C14 means the very ID used for the advertised OTA guard may not be GTW_carState — the guard decision must be capture-first, not doc-first.

---

## 6. CI status

- **CODE-READ/SUMMARY-ONLY** Last observed CI: firmware job **red** on "Firmware size regression check"; all other jobs green; the failure predates the current branch (repo-state `:16`). Not re-observed this session.
- **CAPTURE (local)** `npm test -w tools` = 9/9 suites, 105/105 tests green; `npm run validate:serial-contract` prints "Validated unified io.schema.json contract." (tools-ci.md §1.5, §2.2).
- **UNVERIFIED** GitHub Actions runtime status: no API calls made; local file reading and two local commands only (tools-ci.md §7).
- **CODE-READ** 11 CI jobs each repeat the legacy-folder exception block — drift risk (tools-ci.md §2.1).
- **CODE-READ** `release.yml:159` `fail_on_unmatched_files: false` plus no firmware native tests in `quality-gate` (`:47-62`) means a release can publish empty of binaries (tools-ci.md §6 N-02).
- **CODE-READ + CAPTURE (local)** `scripts/release-matrix-smoke.mjs:109-128` is dead against dynamic env discovery; dry-run exits 1 (tools-ci.md §2.2, N-03).
- **CODE-READ** `scripts/validate-serial-contract.mjs:181-188` skips with exit 0 when schema missing (tools-ci.md §6 N-05).
- **CODE-READ** `.husky/` is not tracked or on disk; markdownlint is CI-only (`ci.yml:195-209`) and lint-staged `.md` → Prettier only (`package.json:31-39`) (tools-ci.md §2.4, P4-008).

---

## 7. Effort / risk re-baseline

Estimates are inherited from the component reports, not independently timed.

| Work stream                                                | Rough effort                | Risk        | Source                                 |
| ---------------------------------------------------------- | --------------------------- | ----------- | -------------------------------------- |
| OTA guard (0x318 → txPaused + central TX check)            | 3–5 h                       | High        | firmware.md §8                         |
| BLE envelope field numbers                                 | 2–4 h                       | High        | firmware.md §8                         |
| BLE add-key rebuild + roles/form-factor                    | 4–6 h                       | High        | firmware.md §8                         |
| AP gate gear/fail-open + DAS gate wiring                   | 3–5 h                       | High        | firmware.md §8                         |
| ALC 0x249 DLC/CRC (capture-first)                          | 3–5 h                       | High        | firmware.md §8                         |
| CarServer seat `fieldVarint`                               | 0.5 h                       | Medium      | firmware.md §8                         |
| `can:raw` persistence / 0x334 split / freshness timestamps | 1 / 1–2 / 2–3 h             | Low–Medium  | firmware.md §8                         |
| Implement-or-remove advertised surfaces                    | 6–20 h                      | Medium      | firmware.md §8                         |
| Native BLE proto test suite                                | 3–4 h                       | Low         | firmware.md §8                         |
| Tools batch (N-01..N-09)                                   | ~3–5 h + bench              | Low–Medium  | tools-ci.md Effort table               |
| Golden-capture harness                                     | 2–4 h + bench               | Medium      | tools-ci.md Effort table               |
| Web UI rewrite total                                       | 60–90 h (12–16 h high-risk) | High        | client-protocol.md §6                  |
| Hardware spikes A–D                                        | Large / measurement-driven  | Medium–High | repo-state §Hardware; road-map Phase 3 |

**Re-baseline verdict:** correctness/security is the small, high-risk front (≈25–40 h firmware + ≈3–5 h tools). The large cost is the UI rewrite (≈60–90 h), which must not start until the command contract is frozen (client-protocol.md §6 ordering). Hardware work is parallel measurement, not a shipping dependency (repo-state §Hardware).

---

## 8. Explicitly UNVERIFIED — no build, test run, or capture in this pass

- **No firmware build, no `pio test`, no hardware run.** All firmware verdicts are CODE-READ of a dirty working tree (firmware.md §0.2).
- **No client/protocol build or test execution.** Test counts are grep-counted; pass/fail unknown (client-protocol.md §3, §7).
- **No tools tests run by the integration agent.** The 9/9, 105/105 green result is inherited CAPTURE from tools-ci.md §1.5, not re-executed here.
- **No `npm run test:all`, `npm run lint:all`, or `npm run format:check` baseline** — P0-03 remains open.
- **No CAN capture of any kind.** Every CAPTURE grade above (25,288-frame 0x370 checksum, 1048-frame 0x249, all 20 CAPTURE_FIRST rows) is inherited from `docs/legacy/*.md` §11 tables or repo-state; raw corpora were not read or re-run (cross-cutting.md §0; firmware.md §0).
- **No GitHub Actions API call** — current red/green on GitHub unknown (tools-ci.md §7).
- **"0-100 timer" scope** — SUMMARY-ONLY/UNVERIFIED; no `client/src` or firmware hit, only legacy docs (cross-cutting.md §2.2, SC-015).
- **Exact ghost-string count** — 29 (client-protocol.md) vs 31 (cross-cutting.md); no machine-deduped scan performed.
- **Cross-cutting 25-vs-23 row discrepancy** — §2.3 above; not resolved.
- **Feature-handler internals** outside the cited files, all `can-decoder/*.json` blobs, and the 11754-line generated docs bundle were not read (client-protocol.md §7).

---

## 9. Evidence index

All factual claims above cite one of:

1. `.opencode/analysis/current-state-2026-11-22/firmware.md`
2. `.opencode/analysis/current-state-2026-11-22/client-protocol.md`
3. `.opencode/analysis/current-state-2026-11-22/tools-ci.md`
4. `.opencode/analysis/current-state-2026-11-22/cross-cutting.md`

plus `file:line` citations read this session in `.opencode/analysis/repo-state-2026-11-22.md` and `.opencode/plans/roadmap-2026-11-22.md` (the latter only for Phase-3 spike naming and Definition-of-Done). No new files were created under `docs/superpowers/`.
