# Task C1-21 — Protocol suite regression from C1-07a — Report

Status: DONE

## What was observed

`npm test -w packages/protocol` produced exactly the 4 expected failures, all in
`test/integration/command-dispatch-coverage.test.ts`, all inside the
`fresh-boot commands must work without 0x273 CAN traffic` describe block:

1. `locates the hasCtrl early return in dispatch.h` — `hasCtrlIndex` was `-1`
   because the marker `if (!s.hasCtrl)` no longer exists in `dispatch.h`.
2. `dispatches strcmp(cmd, "log") before the hasCtrl gate` — `beforeHasCtrl: false`
3. `dispatches strcmp(cmd, "platform") before the hasCtrl gate` — `beforeHasCtrl: false`
4. `dispatches strncmp(cmd, "recorder:", before the hasCtrl gate` — `beforeHasCtrl: false`

All other 27 suites (535 tests) passed.

## Root cause — stale test, NOT a firmware regression

C1-07a changed `firmware/lib/client/command/dispatch.h`:

- old gate: `if (!s.hasCtrl)` (bare "was a 0x273 frame ever seen?")
- new gate: `if (!s.hasFreshCtrl(now))` at `dispatch.h:529`

`State::hasFreshCtrl(now)` in `firmware/lib/core/types.h:869` is
`hasCtrl && (now - lastCtrlMs) < AP_GATE_FRESHNESS_MS`. It is a _stricter_
single freshness gate. The three fresh-boot diagnostics are still dispatched
before it, so the guard's underlying invariant is unchanged:

| Command marker                 | Source line | Gate line |
| ------------------------------ | ----------- | --------- |
| `strcmp(cmd, "log")`           | 425         | 529       |
| `strcmp(cmd, "platform")`      | 448         | 529       |
| `strncmp(cmd, "recorder:", 9)` | 484         | 529       |

The dispatch.h comment at line 423 still says "hasCtrl gate" (stale wording). It
is a comment only, not behavior; left untouched because this task is
protocol/test-files-only. Flagged for the controller.

Because the invariant held and only the marker string/API name was obsolete, no
controller question was needed and no firmware was reverted.

## Change made

`packages/protocol/test/integration/command-dispatch-coverage.test.ts`:

- gate marker `indexOf("if (!s.hasCtrl)")` -> `indexOf("if (!s.hasFreshCtrl(now))")`
- renamed `hasCtrlIndex` -> `hasFreshCtrlIndex` and `beforeHasCtrl` -> `beforeFreshCtrlGate`
- updated test titles to name the `hasFreshCtrl` gate
- added a comment explaining why the gate changed and that the guard semantics are preserved

Guard semantics preserved intentionally:

- still fails if the gate marker cannot be located in `dispatch.h`
- still fails if any of `log` / `platform` / `recorder:*` is dispatched _after_ the gate
- the separate repo-grep guard block (documented command must have a dispatch branch;
  stale `UNSUPPORTED` markers rejected) was not touched at all
- no tests removed, no assertions relaxed, no `if` guard weakened

## Verification

- `npm test -w packages/protocol`: **28 suites / 539 tests passed**, 0 failed
  (was 1 failed / 27 passed / 539 total before the fix).
- `npm run lint:all`: ESLint clean; Prettier "All matched files use Prettier code style!"
- Husky pre-commit (prettier + eslint --fix) ran clean on the staged file.

## Git hygiene

- Branch: `chore/legacy-tesla-drug-fork-submodules` (feature branch, not `main`).
- 191+ unrelated unstaged files left untouched. Staged with explicit path only;
  `git diff --cached --name-only` confirmed a single file before commit. No
  `git add -A` / `git add .`, no force-push.

## Commits

- `2a6f724` — test(protocol): align dispatch-coverage guard with hasFreshCtrl gate (C1-21)
- report commit follows (this file)

## Concerns / follow-ups for controller

1. `firmware/lib/client/command/dispatch.h:423` comment still says "0x273 hasCtrl
   gate" while the code now uses `hasFreshCtrl(now)`. Cosmetic only; out of scope
   for this protocol/test-only task.
2. The test locates the gate by exact source string (`"if (!s.hasFreshCtrl(now))"`).
   Any future whitespace/format change in that line will break the guard loudly
   (intended), but a regex-based locator would be more future-proof. Kept the
   exact-string approach to preserve parity with the original guard and avoid
   weakening it.
3. No other protocol regressions surfaced in this run; C1-06 OTA-claim removal
   is unrelated and its guard test passes.
