# Task C1-09a — Tools security (P3-001..010)

Where this fits: Phase 1 security. The serial-HTTP bridge is an unauthenticated LAN command proxy; dump/replay CLI allows path traversal and unsafe writes. The fresh P0-00 review marked all 10 P3 items STILL_PRESENT.

## Scope — these 10 items (from `docs/superpowers/todo.md`, verify each against current code before editing)

- P3-001 `tools/serial-http-bridge.js:81-83` — CORS `*`, no auth, default `0.0.0.0`, arbitrary `cmd` proxy. Fix: bind 127.0.0.1 default, token auth, restrict CORS.
- P3-002 `serial-http-bridge.js:88-94` — unbounded request body. Fix: cap 1 MB, reject 413.
- P3-003 `serial-http-bridge.js:111` — no `server.timeout`/`headersTimeout`. Fix: 30s / 5s.
- P3-004 `tools/commands/dump.js:12-15,75` — `..` path traversal. Fix: resolve and assert inside cwd.
- P3-005 `tools/commands/replay.js:16,74` — same for `--input`.
- P3-006 `tools/commands/drive-context.js:370,378` — `writeFileSync` no path check. Fix: resolved path inside cwd.
- P3-007 `dump.js:23,71` — fire-and-forget stream write. Fix: await drain + error handler.
- P3-008 `tools/lib/session.js:124` — `write()` without callback. Fix: callback/rejection on error.
- P3-009 `tools/commands/das-drive.js:103-108` — `set-speed` no upper bound. Fix: cap at capMax (200).
- P3-010 `tools/commands/flash.js:21-25` — honors `PLATFORMIO_CORE_DIR` verbatim. Fix: ignore override, use `~/.platformio`.

Full current evidence: `.opencode/analysis/current-state-2026-11-22/tools-ci.md` §4.1, N-01, N-04, N-06. Read it before starting.

## Acceptance

- Regression tests for 401 (no token), 413 (oversized body), path traversal refusal, speed cap.
- All 9 tools test suites green; bridge help text matches new default.
- If any item is already fixed or stale in current code, mark it in the report and skip — do not force a change.

## Global constraints

- No code copied from `legacy/`.
- Stage ONLY files you change; 191 unrelated unstaged files exist. Verify `git diff --cached --name-only` before committing.
- Commit and push on the current feature branch; never main, never force-push.
- Do not change firmware or client code in this task.
