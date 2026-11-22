# SDD ledger — plan: .opencode/plans/roadmap-2026-11-22.md

Workspace: `.opencode/sdd/roadmap-2026-11-22/` (repo-local, do not create `docs/superpowers/`).
Spec: `.opencode/analysis/current-state-2026-11-22.md` + 4 component reports under `.opencode/analysis/current-state-2026-11-22/`.
Branch: `chore/legacy-tesla-drug-fork-submodules` (feature branch, already pushed).

Ruling: no separate git worktree — the current branch is already an isolated feature branch. The main worktree has 191 pre-existing unstaged files; every implementer must stage ONLY files it changed and verify `git diff --cached --name-only` before committing. Cost if wrong: unrelated user work could be swept into a task commit (visible and revertible).

Ruling: capture-gated tasks C1-06 (OTA), C1-07 (gear), C1-08 (ALC) require pulling CH-01 (golden-capture harness) forward. Existing offline corpora inside `legacy/` (uhi22 1048-frame `0x249`, amzoo 25 CSVs, SlipknotTN dumps) may serve as read-only evidence; never copy their code into shipping code. Cost if wrong: a gate flipped on code-only evidence, caught by can-review checklist.

## Pre-flight conflict scan (pairs sharing files/interfaces)

| Tasks                      | Shared surface                                               | Finding / ruling                                                                                                                             |
| -------------------------- | ------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------- |
| C1-01, C1-02, C1-03, C1-14 | `firmware/lib/vehicle/ble/msg.h`, `key.h`, proto constants   | Sequential order: C1-01 envelope tags → C1-14 byte-level proto suite → C1-02/03 add-key+roles. Never parallel: same headers, dependent tags. |
| C1-04                      | `dispatch.h`, `ble_client.h` scan filter                     | Independent of tag numbers; may run after C1-01.                                                                                             |
| C1-06, C1-07               | `core/types.h` (`txPaused`, gate state), `dispatch.h`        | C1-07 first (single gate authority), then C1-06 as one gate input. Both need CH-01.                                                          |
| C1-07, C1-08               | `das_drive.h` vs `auto_lane_change.h`, shared gate semantics | ALC injection must call the single gate API produced by C1-07; no second gate.                                                               |
| C1-07, C1-13               | `core/types.h`                                               | Sequence C1-07 then C1-13 to avoid overlapping edits.                                                                                        |
| C1-10, C1-07               | `dispatch.h:422-427` (`!s.hasCtrl` return), gate semantics   | C1-07 first; C1-10 then moves `log`/`platform`/`recorder:*` above the return.                                                                |
| C1-11                      | `keys.h`, persist backends                                   | Touches `types.h` only indirectly; after C1-07.                                                                                              |
| C1-09                      | `tools/**` only                                              | Independent; can be dispatched any time.                                                                                                     |
| C1-05, C1-12               | `carserver.h`, `pedal.h`/`drive_mode.h`                      | Independent of BLE/CAN gate work.                                                                                                            |
| Any task                   | 191 pre-existing unstaged files                              | All implementers: stage only own files, verify staged list before commit.                                                                    |

## Environment

- `pio`, `python`, `node`, `node_modules` present. Native tests: `cd firmware && ..\pio.ps1 test -e native` (or `pio test -e native`). JS tests: `npm test -w tools`, `npm test -w packages/protocol`.
- No build/test/capture was run in P0-00; every finding is static. CH-01 harness is the first capture-related deliverable.

## Task status

- Task C1-01: complete (commits 7223b54..583fcc7, review clean). 5 tags 6/10/14/15/51 pinned by independent byte decoder; 7/7 focused + 1035/1035 native green. Staging verified via `git show --stat`: only `msg.h` + new test file.
    - Ruling: reviewer's Important item `signature_data` field 7 vs SDK 13 is real but explicitly out of scope (brief lists exactly five tags) — register as C1-15 follow-up in the next task batch; do not reopen C1-01. Cost if wrong: one extra small task later.
    - Ruling: Minor test-scanner bounds issue is test-only with fixed vectors; register in intake, no fix loop. Cost if wrong: obscure failure if a future malformed vector is added.

- Task C1-14: fix round 1/5 started (reviewer Approved with 2 Important + 1 Minor; commits 46955fa..5fc7952).
    - Ruling: 7 exact-wire tests pin CURRENT DEFECTIVE bytes, not SDK vectors. Keep as explicitly labeled characterization baseline (controller acknowledges); follow-up production-fix tasks must flip those assertions to the recorded SDK vectors in the SAME commit. Cost if wrong: a correct production fix temporarily turns 7 tests red — intended signal.
    - Fix dispatched: add literal SDK vectors for add-key/Action envelope comments; label the 7 characterization tests; correct report count typo. No production change.
    - Minor (deferred): dead `feature/vcsec.h` included in test TU; revisit when that header is removed.
- Task C1-14: complete (commits 46955fa..5fc7952, fix 3ed48a5, scoped re-review clean). 18/18 focused + 1046/1046 native green; zero production churn. 7 defective-byte tests carry CHARACTERIZATION markers; SDK vectors recorded for the two previously missing cases. Reviewer confirmed all findings addressed, no new breakage.
    - New plan tasks C1-15..C1-18 added from the five wire defects found here; each must flip its matching CHARACTERIZATION assertion in the same commit as the production fix.
    - Ruling: AddKey SDK path is UnsignedMessage field 16 → WhitelistOperation field 5 (implementer refinement, :612 vector) — C1-02 brief must carry this exact nesting.

- Task C1-02: complete (commits c2f6eda..0767463, review clean). Nested 16→5→PermissionChange{key=1, role=4} encoded; add-key CHARACTERIZATION assertion flipped to the 105-byte SDK vector in the same commit; other characterization tests untouched. Focused 18/18, full native 1046/1046.
    - ⚠️ Bench capture of an accepted add-key remains UNVERIFIED (no hardware) — closed as environment limit, not a code defect.
    - Minor (deferred to C1-03/bench): `metadataForKey` placement matches the recorded vector but contradicts `vcsec.proto:135` (field 6 sibling vs nested f5). Real vehicle may reject; confirm at bench.
    - Minor (deferred to C1-03): byte pin uses formFactor=1 while production dispatch still passes 3.

- Task C1-03: complete (commits 85d1bd8..a168d83, review clean). OWNER=2, CM=6, NFC_CARD=1; `roleValue()` aligned; production formFactor=1. Focused 21/21, full native 1049/1049.
    - ⚠️ Bench capture of real `tesla:key:send` remains UNVERIFIED (environment), plus no ESP32-target build run — both closed as environment limits, not code defects.
    - Minor (deferred): `roleValueFromName()` is `static` in a header (key.h:28-32) — could warn under future `-Wall/-Werror`; `static inline` is the cleanup.
    - C1-02 residual closed here; `metadataForKey` placement still a bench follow-up.

- Task C1-04: complete (commits 9f3bec9..6a8bf74, review clean). Local Name `S+sha1(VIN)[:8]+C` with mbedTLS on device + injected Sha1Fn for native tests; two literal VIN vectors independently verified by reviewer with Python hashlib. Focused 4/4, native 1053/1053, `esp32_ble_chassis_8mhz` build success, lint clean.
    - ⚠️ Real-vehicle advert capture and on-device SHA-1 vector remain UNVERIFIED (environment), declared in report — not code defects.
    - Minor (deferred): VIN length gate 8→5 (`dispatch.h:45`) is an extra behavior change; documented and aligned with `tesla:vin` validation.
    - Minor (deferred): test-only SHA-1 is a two-vector conformance check, not a general suite.

Ruling: C1-09 (35 small tools items) split into C1-09a (P3-001..010 bridge security), C1-09b (P7-001..015 session/args/replay) and C1-09c (PB-001..010 cleanup) — one implementer + one scoped review per family. Rationale: each family has its own test surface; a single 35-item diff would be unreviewable. Cost if wrong: three commits instead of one.

- Task C1-09a: complete (commits f6c4e52..b56a315, review clean). 10/10 P3 items fixed with regression tests; tools 13 suites/131 tests green, lint clean. Auth fails closed when token unset; shared `resolveInsideCwd()` helper; stream drain/error handling.
    - Minor (deferred): `--allow-origin '*'` still accepted verbatim though default is restricted; report claimed otherwise.
    - Minor (deferred, P7-011): no Content-Type enforcement on POST /api/command.
    - Minor (deferred): `set-cap` hardcodes 200 instead of shared constant; no requestTimeout (inactivity timeout still active).
    - Residual (deferred, out of scope): static token over plain HTTP when `--host 0.0.0.0`; lexical-only path containment (symlink escape) — needs realpath + TLS decision.

- Task C1-09b: complete (commits 9cb76e3..b457e30, review clean). 14 fixed + 1 legitimately skipped (P7-005 already covered by C1-09a); tools 150/150 tests, lint clean.
    - Minor (deferred): `diagnosis.js:2` boot regex misses envelopes where `"t":"boot"` is the last field — low impact, one-line lookahead fix at final review.
    - Minor (deferred): `das-drive.js:122` still uses `Number(opts.duration) || 5000`; validated upstream, use `??` for consistency.

- Task C1-09c: complete (commits 4f44e09..a9ff277, review clean). 8 fixed + 2 genuine skips (PB-003/PB-008 pre-resolved). Tools 163/163 tests, lint clean. Bridge C1-09a security and C1-09b Content-Type verified untouched in review.
    - Minor (deferred): `parseFrameId` parses digit-only bare strings as decimal (`"488"` → 488), comment overstates bare-hex behavior.
    - Minor (deferred): variant validation also runs for portless `export` — harmless over-reach.
    - Informational: ISO timestamps change dump textual shape (intended); `export` invoked via `node tools/bin/export.js`, no npm bin alias.

- Task C1-10: complete (commits 13cb1c6..ca75ebd, review clean). Decision table covers all command families; aliases applied in builders + cross-check whitelist + docs; `log`/`platform`/`recorder:*` relocated above `!s.hasCtrl` as pure move; real Jest grep guard added; docs marked UNSUPPORTED. Protocol 501, client 305, native 1053, lint clean.
    - Ruling: reviewer's Important item (fresh-boot test is a source-order proxy, not a compiled `executeCommand` test) is accepted for this phase because no native dispatch harness exists; add C1-19 to build one. Cost if wrong: a new early return could slip past the proxy — mitigated by the grep guard and can-review checklist.
    - Minor (deferred): grep guard skips templated/bare commands not in its allowlist; "dispatched" detection too broad (`strcmp` anywhere under firmware).
    - Minor (deferred): report arithmetic 11 vs 10 families, 36 vs 35 markers; generated in-app docs bundle stale (policy excludes it); `KEEP` category should be relabeled IMPLEMENT-already-present.
    - Out of scope (deferred): client `ConnectionSection.tsx` still emits raw `gvret:*` / `espnow:*` that now hit UNSUPPORTED.

Ruling: batch C1-05, C1-11, C1-12, C1-13 into one implementer + one scoped review. Each is a small firmware fix with its own native test but no cross-task interface; separate commits would add four review seats for four near-identical shapes. Cost if wrong: one reviewer must check four small hunks instead of one.

- Firmware batch: fix round 1/5 started (reviewer Needs fixes; commits 8c9faba..bd34329).
    - Ruling: reviewer's Important finding on C1-12 is load-bearing — bits 2:0 have no DBC evidence, while the repo's own DBC note puts `pedalMap` at byte0 bits 5|2 (owned by drive mode). Shipping the reassignment would likely break pedal mode on hardware. Fix: REVERT C1-12 production change and its new assertion; keep C1-11/C1-13. C1-12 returns to CAPTURE_FIRST (needs DBC + bench) — mark in report, do not silently drop. Cost if wrong: internal collision remains as before, a pre-existing documented risk.
    - Fix round 1/5 complete (commit 6bf4801, scoped re-review clean): C1-12 reverted, C1-11/C1-13 untouched; multi_feature 4/4, pedal 7/7, native 1058/1058.
    - Batch closed: C1-05 STALE (fieldEmpty correct for Void oneofs; new C1-20 added for real BLE compile break), C1-11 done, C1-13 done by removal, C1-12 CAPTURE_FIRST.
    - Minor (deferred): restored `pedal.h:47` comment says bits 5-6; DBC says bits 5|2 (verbatim pre-bd34329 text).
    - Out of scope (deferred): client MQTT UI/commands still present after C1-13 removed the firmware claim.

- Task C1-08: complete (commits 907f7cf..462dffd, review clean). `buildStalkFrame` now DLC-4 with corpus-derived `MAGIC_0x249` (engine untouched), counter at data[1] low nibble, stalk enum at data[2] low nibble. 40 literal vectors in native CI; reviewer independently verified 7 diverse vectors + 870/870 corpus replay. esp32 chassis build green.
    - ⚠️ Real-vehicle acceptance UNVERIFIED; left/right enum mapping (0x08/0x04) disputed across legacy docs — mandatory bench before hardware ship.
    - Minor (deferred): report header cites pre-doc commit 28f029e; native CI pins 40/870 vectors (full corpus offline only).

Ruling: split C1-07 into C1-07a (safety hardening that needs no capture: gate `dasTick` on `apGateOpen()`, fail closed on gear 0/5, remove CAN-timeout force-open, bounded freshness) and C1-07b (capture-first ground truth for gear/pedal semantics). Rationale: fail-closed behavior is conservative and testable now; the exact gear-bit mapping still needs a real capture. Cost if wrong: C1-07b may later refine which values count as Park, but fail-closed stays safe.

- Task C1-07a: fix round 1/5 started (reviewer Approved with Important finding; commits e866da4..cbcab7e).
    - Ruling: requirement 4 explicitly names `hasCtrl` freshness, so the reviewer's Important item (dispatch.h:526 still gates one-shot commands on a bare `hasCtrl`) is plan-mandated, not deferrable. Fix: wire `hasFreshCtrl(now)` into that dispatch guard with a stale-timestamp test. Do NOT add `apGateOpen()` to one-shot commands without a separate decision — that changes user-facing command semantics. Cost if wrong: one-shot commands could be blocked more than intended until C1-07b settles the policy.
    - Fix round 1/5 complete (commit 73d25aa, scoped re-review clean): `!s.hasFreshCtrl(now)` guard, stale test RED `Expected 0 Was 30`→GREEN; focused 38/38, native 1071/1071, ESP32 build success. Arduino.h shim confirmed native-only.
    - Task C1-07a: complete (commits e866da4..73d25aa, review clean).
    - Minor (deferred): report RED block arithmetic and focused-count wording inconsistent (documentation only).
    - Residual (deferred): `apGateParked` default true pre-existing; cancel burst suppressed while gate closed; 1 s bound from repo analysis not vehicle capture; exact gear mapping → C1-07b.

- Task C1-06: complete (commits 6552f2a..23484b5, review clean). OTA guard claims removed from 8 canonical docs + io.schema.json (properties + required); raw `CAN_ID_GTW_CAR_STATE` kept; contract test RED 9→GREEN 38/38; CAPTURE_FIRST rationale with both disputed evidence sides in report.
    - Ruling: reviewer's Important item (live `client/src/docs/_generated-docs.ts` still advertises OTA pause via `catalog.ts:1`) is real but generated-file policy excludes it from this feature commit — register C1-22 for a separate regenerating commit, not a fix to this diff. Cost if wrong: in-app docs stay wrong until C1-22 lands.
    - Ruling: 4 `command-dispatch-coverage` failures are a genuine regression from C1-07a, not C1-06 — register C1-21; do not attribute to this task.
    - Minor (deferred): skills `can-protocol-safety` / `firmware-development` still assert txPaused → folded into C1-22.

- Task C1-21: complete (commits 86d63da..a480839, review clean). Test marker updated to `hasFreshCtrl(now)`; guard/stale-marker assertions byte-identical; protocol 28 suites/539 tests green, lint clean. No firmware change.
    - Minor (deferred): exact-source-string gate locator is by design (fails loudly on reformat), disclosed in report.
    - Minor (deferred): stale firmware comment `dispatch.h:423` says "hasCtrl gate" — firmware out of scope here; fold into C1-22 cleanup.

- BLE wire batch (C1-15..18) in progress — implementer raised two genuine ambiguities; rulings:
    - Ruling Q1: recorded C1-14 amps vector `9A 02` decodes to field 35 (HvacBioweaponModeAction), contradicting SDK `setChargingAmpsAction=43` (wire tag `DA 02`). SDK proto is binding authority → use `DA 02`, correct the test comment to record the fixed vector, and log a C1-14 transcription error in the report. Cost if wrong: none — production already emits the SDK-correct tag.
    - Ruling Q2: `test_wake_action_exact_wire` has NO recorded SDK vector; upstream says wake is VCSEC RKE, not CarServer.VehicleAction. Binding brief says flip only the 5 tests with recorded vectors → keep `buildWakeAction` on the legacy double-wrap so its test stays green; mark the test `LEGACY — no SDK vector; wake belongs to VCSEC RKE` and register C1-23 (capture-first migration). Cost if wrong: one non-SDK builder remains until C1-23.
    - BLE wire batch complete (commit 6bbec1e, review clean): signature_data f13, Action single wrap f2, charge/stop/amps/limit/climate SDK fields, dead carserver.h deleted; 5 characterization assertions flipped; focused 21/21, native 1071/1071, ESP32 BLE build success.
    - Minor (deferred): report RED header arithmetic (22 vs 21); now-unused COMPONENT\_\* constants disclosed.
    - ⚠️ Real-vehicle acceptance UNVERIFIED for all changed wire fields; SDK correctness is code-level only.

- Task C1-22: complete (commits 4a1be7b..b5f2b7f, review clean). Generated bundle in own commit (7c19de9); skills, dispatch comment, claim test + minimal lint ignores in b5f2b7f. Reviewer parsed 144 bundle entries: 0 canonical OTA claims; legacy exclusions real. Protocol 575, client 305, lint clean.
    - ⚠️ Commit-1 isolation rests on commit message/report (combined diff only) — low risk, no code impact.
    - Minor (deferred): lint-config edits outside brief's literal commit-2 scope but necessary and disclosed.

- Task C1-19: complete (commits bb5db63..2b7d7af, review clean). Real `executeCommand` with `hasCtrl=false` asserts JSON for log/platform/recorder:status + gate-error absence; contrast test proves `lock` still rejected; TS ordering assertion kept secondary; production untouched. Native 1075/1075, protocol 575/575, lint clean.
    - Minor (deferred): assertions are substring tokens, not full JSON parse (disclosed in report).

## Phase 1 closure

All executable tasks DONE. Parked until capture/DBC/bench (cannot be completed in this environment): C1-07b (gear/pedal bit ground truth), C1-12 (0x334 pedal field), C1-23 (wake → VCSEC RKE). These are explicitly listed as open in the plan with their required capture.
