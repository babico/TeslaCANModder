# Task C1-08 — ALC `0x249` frame shape — report

Status: **COMPLETE** (code + tests + checklist). Real-vehicle acceptance: **UNVERIFIED**.

- Branch: `chore/legacy-tesla-drug-fork-submodules`
- Commit: `28f029e` — `fix(firmware): rebuild ALC 0x249 stalk frame from capture corpus`
- Pushed: `907f7cf..28f029e chore/legacy-tesla-drug-fork-submodules -> origin/chore/legacy-tesla-drug-fork-submodules`
- Staging: exactly 3 files (`crc8.h`, `auto_lane_change.h`, new `test_native_alc_stalk/test_alc_stalk.cpp`); verified with `git diff --cached --name-only` before commit. No `-A`, no unrelated files swept.

## 1. Requirement

Re-derive DLC, stalk field, counter and CRC from the offline corpus; rebuild `buildStalkFrame`; walk `docs/checklists/can-review-checklist.md`. No new injection gate (C1-07 pending). No code copied from `legacy/`.

Acceptance from plan (roadmap line 70): "replay 1048-frame corpus; generated DLC-4 frame matches accepted frames; checklist signed."

Controller ruling (brief): no bench capture possible; use `legacy/uhi22-tesla-crc/249_sortedFile.txt` + `229_sortedFile.txt` as CAPTURE-grade evidence with provenance from `legacy/uhi22-tesla-crc/readme.md` and `docs/legacy/uhi22-tesla-crc.md`. Real-vehicle acceptance stays UNVERIFIED.

## 2. Capture evidence and provenance

| Source                                                     | Role                                                                                            | Grade                                        |
| ---------------------------------------------------------- | ----------------------------------------------------------------------------------------------- | -------------------------------------------- |
| `legacy/uhi22-tesla-crc/249_sortedFile.txt`                | 870 accepted 4-byte `0x249` frames                                                              | CAPTURE                                      |
| `legacy/uhi22-tesla-crc/229_sortedFile.txt`                | 178 accepted 3-byte `0x229` frames (different ID, **not modified by this task**)                | CAPTURE                                      |
| `legacy/uhi22-tesla-crc/readme.md:13-26`                   | Frame convention + magic-table method (prose, re-derived independently here)                    | SUMMARY-ONLY, corroborated by our derivation |
| `docs/legacy/uhi22-tesla-crc.md:44,97-109,170-185,209-225` | Prior full-read analysis; parity matrix P5-P8                                                   | ANALYSIS                                     |
| `docs/legacy/canhackers-jupiter.md:195,308,422`            | Working in-flight rewriter: stalk nibble `8=left full, 4=right full, 6=left half, 2=right half` | CODE-READ (direction cross-ref only)         |
| `docs/legacy/joshwardell-model3dbc.md:215,252,372`         | Community DBC: `SCCM_turnIndicatorStalkStatus` 16\|4, `UP_1=2`, `DOWN_1=6`                      | CODE-READ (direction cross-ref only)         |

1048 = 870 (0x249) + 178 (0x229). This task changes only `0x249`; the 870-frame corpus is the applicable oracle. `0x229` left untouched.

Provenance caveat (inherited from `docs/legacy/uhi22-tesla-crc.md:341`): which vehicle/bus/tap produced the corpus is UNVERIFIED. The corpus is accepted-frame evidence, not a capture of our own injected frame.

## 3. Independent derivation (no legacy code copied)

A standalone Python derivation was written from first principles (CRC-8/OPENSAFETY poly `0x2F`, init `0x00`, no reflect, no final XOR) and run against the raw corpus:

1. **DLC** — every corpus line has exactly 4 bytes ⇒ `dlc = 4`.
2. **Counter** — `b1[3:0]` takes all values 0..15 and wraps; masked out of the CRC input; used as magic-table index. Old builder put the counter nibble _inside_ the CRC window — defect.
3. **Stalk field** — distinct `b2` values are `0,1,2,3,4,5,6,7,8`; single-bit states `1/2/4/8` plus combos. `b3 = 0x00` in all 870 frames.
4. **Magic table** — the 16 zero-payload lines (1-16) have `crc8({0,0,0,0}) = 0`, so their `b0` values _are_ `magic249[counter]`: `9B E8 2A D3 D3 83 4C 5E 3F 5E E2 28 3A 13 AF CE`.
5. **CRC rule** — `b0 == crc8_opensafety({b1 & 0xF0, b2, b3, 0x00}) ^ magic249[b1 & 0x0F]`. Replayed **870/870 match, 0 fail**. Old shipped table fit **0/870**.

The CRC engine (`firmware/lib/vehicle/can/crc8.h:35-43`) was already correct and was not modified — only the `MAGIC_0x249` constant values and their comment were replaced with corpus-derived values.

## 4. Changes

`firmware/lib/vehicle/can/feature/fsd/auto_lane_change.h` — `buildStalkFrame`:

- `dlc 3 → 4`.
- `data[1]` = counter only (high nibble 0, idle); counter nibble excluded from CRC window.
- `data[2]` = turn-stalk enum at low nibble: `0x08` left (full-down), `0x04` right (full-up). Old `byte1[7:6]` stalk bits removed.
- `data[3] = 0x00`.
- CRC computed via `teslaCrc8({b1 & 0xF0, b2, b3}, 3, counter, MAGIC_0x249)` — virtual `0x00` appended by the helper.

`firmware/lib/vehicle/can/crc8.h` — `MAGIC_0x249` replaced with the corpus-derived 16-byte table; comment now records derivation, 870/870 evidence, and UNVERIFIED real-vehicle status (removed the previous bare "verified" claim).

`firmware/test/test_native_alc_stalk/test_alc_stalk.cpp` (new): 40 **literal** corpus vectors (all 16 counters at zero payload, stalk enums `1/2/4/6/8`, every observed `b1[7:4]` nibble, and stalk+high-nibble combos). Four tests:

- `test_alc_magic_table_matches_corpus` — shipped table == corpus-derived table.
- `test_alc_corpus_crc_replay` — every literal vector replayed through shipped `teslaCrc8` matches its captured `b0`.
- `test_alc_stalk_frame_shape` — DLC 4, ID `0x249`, `b3=0`, idle high nibble, stalk enum in `b2` low nibble, CRC consistency.
- `test_alc_stalk_left_right_distinct` — left `0x08`, right `0x04`, both corpus-observed.

No gate changes: the existing `s.apGateOpen()` call at `vehicle.h:140` and the `driverSend(confirm, BUS_VEHICLE)` site at `:154` are untouched. No new gate added (C1-07 still pending). No command surface, docs, or client changes.

## 5. TDD evidence

- **RED** (before fix), focused `pio test -e native -f test_native_alc_stalk`: 4 failures — magic slot `Expected 0x9B Was 0x44`, CRC replay `Expected 0x9B Was 0x44`, shape `Expected 4 Was 3`, stalk `Expected 0x08 Was 0x00`.
- **GREEN** (after fix), focused: `4 test cases: 4 succeeded`.
- **Full native suite** (once, after fix): `1062 test cases: 1062 succeeded`.
- **ESP32 build** `pio run -e esp32_chassis_8mhz`: `SUCCESS` (17.45 s), flash 26.2%, RAM 16.5%.

## 6. can-review checklist walk

Legend: `[x]` satisfied / verified, `[n/a]` not applicable to this change, `[!]` residual risk stated.

### 1. Frame mutation safety

- `[x]` Every direct `frame.data[...]` read has a matching DLC guard before access — no incoming-frame reads added; `readDasLaneChangeState` already guards `f.dlc < 5` (unchanged).
- `[x]` Every direct `frame.data[...]` write has a matching DLC guard before mutation — builder constructs a new frame at fixed DLC 4 and writes only `data[0..3]`; no incoming DLC dependency.
- `[n/a]` Shared helpers reject invalid bit indexes / out-of-DLC writes — no shared bit helper changed; `teslaCrc8` still clamps `payloadLen` to 7.
- `[x]` Bit writes only touch intended field — `b1[7:4]=0`, `b1[3:0]=counter`, `b2[7:4]=0`, `b2[3:0]=stalk enum`; no adjacent-bit leakage, asserted in tests.
- `[x]` Existing checksum logic recalculated when a changed byte requires it — CRC recomputed over the masked 3-byte window + virtual `0x00` and the corpus-derived table.

### 2. Bit and field ownership

- `[x]` Exact bit numbers documented — commit message, doc comment, and this report record bytes 0-3 and nibble positions.
- `[x]` Changed bits cross-checked against active behavior, not an old sketch — canonical oracle is the 870-frame corpus; parity rows P5-P8 in `docs/legacy/uhi22-tesla-crc.md` guided the re-derivation.
- `[x]` Legacy disagreement noted with canonical source — source choice: capture corpus for frame shape/CRC; direction enum from `canhackers-jupiter` + `joshwardell` cross-refs. Residual mapping conflict recorded in §8.
- `[n/a]` Mux-specific behavior preserved — `0x249` is not muxed; no mux path touched.
- `[x]` RX/TX ownership clear — same single TX site (`vehicle.h:154`), same ID, same bus; no RX handler added.

### 3. Variant behavior review

- `[x]` `legacy`, `hw3`, `hw4` considered — `buildStalkFrame` is the non-HW4 path; HW4 uses `buildPalladiumTurnFrame` (`0x3C2`) which is untouched.
- `[n/a]` Unsupported controls remain unsupported — no command surface changed.
- `[n/a]` `hw3` speed offset preserves profile/nag — untouched.
- `[n/a]` `hw4` ISA preserves mux/checksum — untouched.
- `[n/a]` Legacy behavior not accidentally changed — no variant-specific helper reused or altered.

### 4. Stream and I/O protocol review

- `[x]` Transmitted-frame change checked against `firmware/lib/io/serial/usb/esp32/board.h` — no I/O layer code changed; frame shape is carried as JSON `d` hex + `dlc`, and stream output still emits both.
- `[x]` Client-side transport/parsing reviewed — no parser keys off the `0x249` DLC; the `frame` envelope shape is unchanged, only the hex payload length changes 6→8 chars.
- `[n/a]` `boot`/`status` capability changes — none; no feature flag or advertised capability changed.
- `[x]` Stream output preserves `dir`, `id`, `dlc`, `d`, metadata — unchanged.
- `[x]` WiFi/BLE identical command semantics — no command routing changed.
- `[x]` Command routing checked against handler + serial/WiFi/BLE entry points — no dispatch edits; full native suite green.

### 5. Required regression tests

- `[x]` Exact bit-set / byte assertion for the changed field — `test_alc_stalk_frame_shape`, `test_alc_stalk_left_right_distinct` assert exact bytes/nibbles.
- `[n/a]` Short-frame regression — builder synthesizes its own DLC-4 frame; no incoming short-frame path exists for this helper.
- `[n/a]` Variant-specific regression — no variant behavior change; existing HW3/HW4/Legacy suites still green in full run.
- `[x]` Message-shape assertion when board output changes — DLC 4 and `b3=0` explicitly asserted.

### 6. Manual reviewer questions

- `[x]` Change which CAN IDs are intercepted or transmitted? **No** — same `0x249`, same TX site.
- `[x]` Change any bit position, checksum, mux rule, profile mapping? **Yes, documented** — stalk field `b1[7:6] → b2[3:0]`; CRC window mask + magic table; no mux/profile change.
- `[x]` Add any path that can transmit when feature disabled? **No** — `alcShouldConfirm` + existing `s.apGateOpen()` gate unchanged; no new gate added.
- `[x]` Change browser-visible meaning of streamed frames or status? **No** — raw frame bytes only; no status field added or removed.
- `[!]` If from a legacy sketch, is it the correct reference? **Canonical = corpus**; no legacy code copied. Direction enum cross-ref sources disagree — see §8.
- `[x]` Affect bus routing? **No** — still `BUS_VEHICLE`.
- `[x]` Change behavior for any of the 3 MCP2515 buses? **No**.
- `[x]` WiFi/BLE command path still consistent with serial? **Yes** — unchanged.

### 7. High-risk change types

- `[x]` Helper change in `firmware/lib/vehicle/can/feature/*` — this is exactly that; checklist walked in full.
- `[x]` Checksum change — magic table + CRC window; replayed against 870 literal capture frames (full offline) and 40 literal vectors (native).
- `[n/a]` Mux / profile / offset / ISA changes — none.
- `[n/a]` Handler filter ID changes — none.
- `[x]` Stream message shape change — DLC 3→4; reviewed against I/O layer in §4; no parser change needed.
- `[n/a]` Serial / WiFi / BLE entry changes affecting CAN mutation state — none.
- `[n/a]` MCP2515 driver changes — none.
- `[n/a]` Bus routing changes — none.
- `[n/a]` WiFi/BLE endpoint exposing new CAN mutation paths — none.

### 8. Minimum acceptance bar

- `[x]` Intended bit-level behavior asserted in tests — 4 focused tests pass; RED→GREEN demonstrated.
- `[n/a]` Short-frame behavior asserted where relevant — no incoming-frame path for this helper.
- `[x]` Active variant behavior still explicit — non-HW4 path, HW4 path untouched, all variant suites green.
- `[x]` Reviewer can explain exactly which CAN IDs and bits changed — ID `0x249` only; bytes 0-3 as documented above.

Checklist signed: **yes** (all applicable items satisfied; residual risks in §8 recorded, not silently dropped).

## 7. Scope discipline

- No code copied from `legacy/` — the magic table was derived independently from corpus lines 1-16; the CRC rule was re-derived and verified 870/870 with an independently written bitwise engine.
- No new injection gate — C1-07 remains pending; existing `s.apGateOpen()` call kept intact.
- No re-advertising of C1-10 UNSUPPORTED commands.
- No files created under `docs/superpowers/`.
- No unrelated working-tree changes staged (191+ pre-existing unstaged files untouched).

## 8. Concerns / residual risks (UNVERIFIED items)

1. **Real-vehicle acceptance UNVERIFIED.** No bench capture of an injected frame was possible. The corpus proves the _shape_ and _CRC_ are accepted frames; it does not prove our car accepts the injected frame. Required before ship on hardware: listen-only `can:raw:on` + recorder capture comparing injected `0x249` bytes to vehicle-emitted frames.
2. **Direction enum mapping conflict.** Corpus supplies stalk enum values but no left/right labels. Sources disagree:
    - `canhackers-jupiter.md:195`: `8=left full, 4=right full, 6=left half, 2=right half` (chosen: left `0x08`, right `0x04`).
    - `joshwardell-model3dbc.md:215`: `UP_1=2, DOWN_1=6` (would give left `6`, right `2`).
    - `LeeGaHyeon-...:191`: `1=UP_0_5, 2=UP_1, 6=DOWN_1`.
    - `bobmorane83-TeslaCam.md:316`: `1/2 left, 3/4 right`.
      A wrong choice would still produce a CRC-valid frame but could signal the wrong direction. Bench confirmation required; recorded here rather than silently resolved.
3. **Native test replays 40 literal vectors, not all 870.** The full 870/870 replay was performed by the independent offline derivation script (§3) and is documented; the native suite pins a representative subset covering all structural axes. Expanding to all 870 literal vectors is a possible follow-up if stricter CI replay is desired.
4. **Unprovenanced tables remain.** `MAGIC_0x229` and `MAGIC_0x370` still carry "verified" comments despite the prior analysis finding `MAGIC_0x229` fits 0/178 of the 0x229 corpus. Out of scope for C1-08; noted as pre-existing and left untouched.
5. **Corpus provenance UNVERIFIED** (vehicle/bus/tap unknown — inherited from `docs/legacy/uhi22-tesla-crc.md:341`); the corpus is accepted-frame evidence only.
6. CRC replay test exercises `teslaCrc8` directly for arbitrary payloads because `buildStalkFrame` exposes only left/right; `buildStalkFrame` itself is covered by shape + CRC-consistency tests. Acceptable separation, but noted.
