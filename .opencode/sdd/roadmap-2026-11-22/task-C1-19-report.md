# Task C1-19 — native dispatch harness for fresh-boot commands — Report

Status: DONE

## Requirement

Replace the fresh-boot source-order proxy with a native test that really calls
`executeCommand` with no CAN state (`hasCtrl=false`) and asserts `log` /
`platform` / `recorder:status` emit JSON. Keep the ordering assertion as a
secondary check. Preserve C1-07a's single `hasFreshCtrl(now)` gate API and
C1-10's gate placement.

## Before-you-begin check

No new stubs were required and no production header was touched. The existing
`firmware/test/test_native_cmd_freshness/test_cmd_freshness.cpp` harness (added
by C1-07a) already links `executeCommand` with the full stub set: fake `Serial`
capturing into `capturedOutput`, fake `millis()`, `driverSend`/filter stubs,
`Preferences` shim, and every feature header `dispatch.h` references. All stubs
live under `firmware/test/**`. Production code untouched in the final state.

## Change made

1. `firmware/test/test_native_cmd_freshness/test_cmd_freshness.cpp` — added four
   native Unity tests on the existing harness:
    - `test_fresh_boot_log_emits_json_without_ctrl` — `State s = {};`
      (`hasCtrl=false`) → `executeCommand("log", …)` → asserts `"t":"log"` in
      captured serial output and no `"Waiting for 0x273"`.
    - `test_fresh_boot_platform_emits_json_without_ctrl` — same with `platform` →
      `"t":"platform"`.
    - `test_fresh_boot_recorder_status_emits_json_without_ctrl` — same with
      `recorder:status` → `"t":"recorder"`.
    - `test_gated_cmd_errors_without_ctrl` — contrast: `lock` with `hasCtrl=false`
      still sends nothing and errors with `"Waiting for 0x273"`.
      All four registered in `main()`. File `@brief` updated to C1-07a/C1-19.

2. `packages/protocol/test/integration/command-dispatch-coverage.test.ts` —
   added a comment inside the existing `fresh-boot commands` describe block
   naming the native test as the primary guard and the source-order assertions
   as a secondary check. No assertion removed, changed, or weakened; the
   `hasFreshCtrl(now)` marker/ordering guard from C1-21 stays intact.

## TDD evidence

- GREEN first: focused suite passed 6/6 against current (correct) production.
- RED demonstrated meaningfully: temporarily changed the `log` branch literal
  in `firmware/lib/client/command/dispatch.h` from `"log"` to
  `"log_C1_19_RED_CHECK"`, which pushes `log` through the freshness gate. Focused
  run then failed exactly on `test_fresh_boot_log_emits_json_without_ctrl`
  (1 failed / 5 passed; runner reported ERRORED on the failed case). Restored the
  literal immediately; `git diff -- firmware/lib/client/command/dispatch.h`
  returned 0 lines — production tree clean.

## Verification

- Focused: `.\pio.ps1 test -e native -f test_native_cmd_freshness` — 6/6 passed.
- Full native: `.\pio.ps1 test -e native` — **1075/1075 test cases passed**
  across all 90 test suites (including the modified suite).
- `npm test -w packages/protocol` — **28 suites / 575 tests passed**, 0 failed.
- `npm run lint:all` — ESLint clean; Prettier "All matched files use Prettier
  code style!"; markdownlint "0 issues in 0 files" (44 files linted).

## Git hygiene

Branch `chore/legacy-tesla-drug-fork-submodules` (feature branch, not `main`).
191+ unrelated unstaged files left untouched. Staged only the two changed test
files (plus this report in a separate commit) with explicit `git add <path>`;
verified `git diff --cached --name-only` before each commit. No `git add -A` /
`git add .`, no force-push, no history rewrite.

## Commits

- test commit — native fresh-boot dispatch tests + TS secondary-guard comment
- docs(sdd) commit — this report

## Concerns / follow-ups for controller

1. The native tests assert the `t` type token and absence of the gate error
   string, not full JSON schema equality. This matches the acceptance criterion
   ("produce JSON") and keeps the tests robust to added fields. If byte-exact
   envelope validation is desired later, extend with a JSON-parse assertion.
2. The primary guard now lives in the firmware test suite, so a protocol-only
   CI run no longer exercises it. Full `npm run test:all` / native CI does. Kept
   the TS source-order assertion so protocol CI still has a cheap proxy.
3. `test_native_cmd_freshness` now covers both C1-07a gate behavior and C1-19
   fresh-boot dispatch; if the controller prefers a dedicated one-purpose suite,
   the four new tests can be split into `test_native_fresh_boot_dispatch`
   (would duplicate the harness). Chose reuse to avoid stub drift.
