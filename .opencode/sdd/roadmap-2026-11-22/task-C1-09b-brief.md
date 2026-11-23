# Task C1-09b — Session/args/replay (P7-001..015)

Where this fits: Phase 1 tools lifecycle. The fresh P0-00 review marked 49 tools todo items STILL_PRESENT; this batch covers the 15 P7 items (session lifecycle, argument validation, replay diagnostics). C1-09a already handled P3 security; do not redo it.

## Scope — 15 items (verify each against current code before editing)

- P7-001 `tools/lib/session.js:32-34` — readline interface never stored/closed. Fix: store `_rl`, close in `close()`.
- P7-002 `session.js:9-13` — `cb` can fire synchronously in SerialPort constructor. Fix: `sp.open()` promise pattern.
- P7-003 `session.js:115` — silent no-op when port down. Fix: throw/reject.
- P7-004 `tools/commands/dump.js:47-49` — CSV formula injection from `m.dir`/`m.d`. Fix: prefix `= + - @ tab CR` cells with `'`.
- P7-005 `dump.js:34-63` — ignored `stream.write()` return. Fix: await `drain` on false.
- P7-006 `tools/commands/smoke.js:71-72,82-83` — fixed sleeps after stream on/off. Fix: wait for actual frame.
- P7-007 `tools/commands/das-drive.js:145-147` — `can:raw:on` ack timeout treated as success. Fix: hard-fail or reduce soak.
- P7-008 `tools/lib/args.js:34-83` — `Number(x) || default` hides invalid input. Fix: validate ranges, throw on bad numeric.
- P7-009 `args.js:75` — `duration` reused as `watchDurMs`. Fix: split/document precedence.
- P7-010 `tools/serial-http-bridge.js:96-104` — parse errors become `{t:"text",raw:""}`. Fix: preserve `parse-error` + error detail.
- P7-011 `serial-http-bridge.js` POST `/api/command` — no Content-Type check. Fix: require `application/json` (deferred from C1-09a).
- P7-012 `serial-http-bridge.js:218-222` — no `server.close()` on boot failure. Fix: chain close in catch.
- P7-013 `tools/commands/replay.js:31-39` — JSON parse errors / CSV headers silently skipped. Fix: report `skipped: N`.
- P7-014 `tools/commands/flash.js:14,113-130` — `FLASH_BAUD` hardcoded. Fix: add `--flash-baud` flag.
- P7-015 `tools/lib/diagnosis.js:7` — substring boot detection. Fix: anchored regex.

Current evidence: `.opencode/analysis/current-state-2026-11-22/tools-ci.md` §4.3/§4.4. Read before starting.

## Acceptance

- `close()` demonstrably closes readline (test).
- Invalid numeric args throw; Content-Type enforced on POST.
- All tools suites + lint green.
- Mark any already-fixed/stale item in the report and skip it.

## Global constraints

- No code copied from `legacy/`.
- Stage ONLY files you change; 191 unrelated unstaged files exist. Verify staged list before commit.
- Commit/push on current feature branch; never main, never force-push.
- Tools files only — no firmware/client changes.
