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
