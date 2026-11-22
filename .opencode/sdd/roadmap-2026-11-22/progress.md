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
