# Task C1-09c — Tools cleanup (PB-001..010) — report

Branch: `chore/legacy-tesla-drug-fork-submodules`
Commit: included in this task's commit (code + this report).
Scope: tools files only. No `legacy/` code copied. No C1-09a security behavior
(bridge auth / Content-Type / body cap / timeouts / path containment) and no
C1-09b validation semantics (`numericFlag`) re-opened — verified by diff review.

## Re-verification result (per item)

| ID     | Status at start | Action                                                                                                                          |
| ------ | --------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| PB-001 | STILL_PRESENT   | FIXED — `debug.js printUsage()` now writes usage to stdout.                                                                     |
| PB-002 | STILL_PRESENT   | FIXED — variant validated against `hw3\|hw4\|legacy\|auto` before serial open.                                                  |
| PB-003 | ALREADY FIXED   | STALE — C1-09a replaced both `isSafePathInput` copies with shared `resolveInsideCwd` in `tools/lib/paths.js`. Skipped.          |
| PB-004 | STILL_PRESENT   | FIXED — `isReplayFrameLike` exported and unit-tested.                                                                           |
| PB-005 | STILL_PRESENT   | FIXED — extracted/documented `parseFrameId`, unit-tested; soak uses it.                                                         |
| PB-006 | STILL_PRESENT   | FIXED — bridge imports shared `parseArgs` from `tools/lib/args.js`; local tokenizing loop removed.                              |
| PB-007 | STILL_PRESENT   | FIXED — `ts()` returns ISO-8601 UTC (`toISOString()`); test updated.                                                            |
| PB-008 | ALREADY FIXED   | STALE — C1-09b already converted every numeric flag to `numericFlag()`; no `Number(x) \|\| default` left in `args.js`. Skipped. |
| PB-009 | STILL_PRESENT   | FIXED — standalone entry moved to `tools/bin/export.js`; smoke test added.                                                      |
| PB-010 | STILL_PRESENT   | FIXED — magic `10` replaced by `OFF_SOAK_NOISE_FRAME_MAX` with comment; soak tests added.                                       |

PB-003 evidence: `tools/commands/dump.js:7,14` and `tools/commands/replay.js:6,16`
both import/call `resolveInsideCwd`; `tools/lib/paths.js` is the single guard.
Grep for `isSafePathInput` over `tools/**` returns zero hits. No duplicate path
guard remains, so no change was forced.

PB-008 evidence: `tools/lib/args.js` uses `numericFlag()` for baud, timeout,
warmup, duration, test-profile, flash-baud, scan-duration, speed, bench-duration,
drive-duration, drive-min-samples and kph. No mixed style remains.

## TDD RED → GREEN

RED (tests written before implementation, `npm test -w tools`):

- `Test Suites: 5 failed, 10 passed, 15 total`
- `Tests: 2 failed, 119 passed, 121 total`
- Suite-load failures: `args.test.js` (`isValidVariant` missing), `replay.test.js`
  (`isReplayFrameLike` missing), `security-commands.test.js` (`parseFrameId` missing).
- Assertion failure: `output.test.js ts` expected ISO, received `18:28:58 GMT`.
- `export-command.test.js`: `Cannot find module .../tools/bin/export.js`.

GREEN (after implementation):

- `Test Suites: 15 passed, 15 total`
- `Tests: 163 passed, 163 total`
- `npm run lint:all` green: ESLint clean, Prettier "All matched files use
  Prettier code style", markdownlint `0 issues in 0 files` (44 files).

CLI smoke: `node tools/debug.js` prints usage on stdout only (verified by
redirecting stderr/stdout separately) and exits 1; `node tools/debug.js --port
COM3 --variant hw5` prints `ERROR: unknown variant "hw5". Valid:
hw3|hw4|legacy|auto` and exits 1 before opening serial.

## Files changed

Production:

- `tools/debug.js` (PB-001, PB-002)
- `tools/lib/args.js` (PB-002)
- `tools/lib/output.js` (PB-007)
- `tools/commands/replay.js` (PB-004)
- `tools/commands/das-drive.js` (PB-005, PB-010)
- `tools/serial-http-bridge.js` (PB-006)
- `tools/commands/export.js` (PB-009)
- `tools/bin/export.js` (new, PB-009)

Tests:

- `tools/test/args.test.js`
- `tools/test/replay.test.js`
- `tools/test/output.test.js`
- `tools/test/security-commands.test.js`
- `tools/test/export-command.test.js`
- `tools/test/bridge.test.js`

Report: this file.

## Self-review

- Diff reviewed end-to-end: only the 15 files above changed; no unrelated file
  touched. `git status --porcelain` for these paths confirmed before staging.
- Bridge refactor keeps all C1-09a behavior intact (loopback default, bearer
  token, explicit CORS origin, 1 MB body cap, 415 Content-Type, timeouts,
  server close on boot failure) and keeps the existing `Number(x) || default`
  numeric handling — no C1-09b numeric semantics re-opened.
- `parseFrameId` is the only intentional behavior change: digit-only strings
  are now parsed as decimal (old `parseInt(x, 16)` read `"69"` as `105`).
  Used solely in `das-drive soak` id matching; documented in code and tested.
- `ts()` consumers are `dump.js`, `scan.js`, `watch.js` and the observation
  logger — all display/timestamp uses, none parse the old local-time shape.
- All new helpers are exported and directly tested; the moved standalone entry
  has an end-to-end subprocess test.

## Concerns

- None blocking. Two stale items (PB-003, PB-008) were already closed by
  earlier batches and intentionally skipped, per acceptance criteria.
- `ts()` output grew from `HH:MM:SS GMT` to full ISO-8601 UTC; harmless for
  current consumers but changes the textual shape of dump CSV/JSONL timestamps.
- `tools/package.json` `bin` map still exposes only `tcm-debug`; the export
  entry is invoked as `node tools/bin/export.js` (as documented). Adding a
  separate npm bin alias was out of scope.
