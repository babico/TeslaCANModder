# Task C1-09c — Tools cleanup (PB-001..010)

Where this fits: Phase 1 tools hygiene, final piece of the split C1-09 batch. C1-09a (security) and C1-09b (session/args/replay) are done. This batch is small same-shape cleanup across tools files.

## Scope — 10 items (verify each against current code before editing)

- PB-001 `tools/debug.js:42-46` — usage on stderr; standardize stdout.
- PB-002 `tools/debug.js:106-110` — `variant:` not validated; accept `hw3|hw4|legacy|auto`.
- PB-003 `dump.js:75-77` + `replay.js:74-76` — duplicated `isSafePathInput`; move to `tools/lib/`.
- PB-004 `tools/commands/replay.js:78-84` — `isReplayFrameLike` exported nowhere/tested nowhere; move to lib or add test.
- PB-005 `tools/commands/das-drive.js:155-156` — obscure `idRaw` fallback; comment + test.
- PB-006 `tools/serial-http-bridge.js:19-67` — local `parseArgs` ignores `lib/args.js`; import it.
- PB-007 `tools/lib/output.js:60-62` — locale-dependent timestamp; ISO format.
- PB-008 `tools/lib/args.js:39-83` — mixed numeric styles; make consistent with the hardened `numericFlag()` from C1-09b.
- PB-009 `tools/commands/export.js:121-141` — standalone `if (process.argv[1]...)` entry; move to `tools/bin/export.js`.
- PB-010 `tools/commands/das-drive.js:181` — magic `10` for off-but-seeing-frames; named const + comment.

Current evidence: `.opencode/analysis/current-state-2026-11-22/tools-ci.md` §4.4. Read before starting.

## Acceptance

- Tools suites + lint green; any moved helper has a test; no duplicate path guard remains.
- Mark already-fixed/stale items in report and skip — do not force changes.
- Do not re-open C1-09a security behavior (bridge auth, Content-Type) or C1-09b validation semantics.

## Global constraints

- No `legacy/` code copied.
- Stage ONLY changed files; 191 unrelated unstaged files exist. Verify staged list before commit.
- Commit/push on current feature branch; never main, never force-push.
- Tools files only.
