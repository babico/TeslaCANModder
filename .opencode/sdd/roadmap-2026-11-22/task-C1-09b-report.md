# Task C1-09b — Session/args/replay (P7-001..015) — Report

Date: 2026-11-22. Branch: `chore/legacy-tesla-drug-fork-submodules`.
Re-verified all 15 items against current on-disk code before editing. C1-09a (P3 security) not redone.

## Per-item status

| ID     | Status                  | Change                                                                                                                                                                                                 | Verification                                                                             |
| ------ | ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------- | -------------------------- | ------------------------------------------------------------------------------------------ |
| P7-001 | FIXED                   | `BoardSession` stores `_rl`; `close()` calls `rl.close()` before port close                                                                                                                            | New test `close() also closes the readline interface` (RED → GREEN)                      |
| P7-002 | FIXED                   | `openSerialWith(SerialPort, port, baud)` uses `autoOpen:false` + `sp.open(cb)`; `openSerial` delegates                                                                                                 | New tests assert one ctor arg, no constructor callback, open error rejects (RED → GREEN) |
| P7-003 | FIXED                   | `send()` throws `Serial port is not writable; command dropped` instead of silently returning                                                                                                           | New test (RED → GREEN)                                                                   |
| P7-004 | FIXED                   | `csvField` exported from `lib/canExport.js`; formula-leading cells (`= + - @ tab CR`) prefixed with `'`; dump CSV uses it for all cells                                                                | New CSV-injection test (RED → GREEN)                                                     |
| P7-005 | ALREADY FIXED — SKIPPED | `dump.js` already uses `writeWithDrain`/`endStream` from `lib/streams.js` (C1-09a). No change.                                                                                                         | Existing `streams.test.js` green; current code re-read                                   |
| P7-006 | FIXED                   | `smoke` raw phase waits for up to 5 real `frame` messages via `waitForType` with bounded deadline; removed fixed `stream:on`/`stream:off`/`can:raw:off` sleeps; queued frames drained after stream off | New `smoke.test.js` asserts raw-phase frame waits (RED → GREEN)                          |
| P7-007 | FIXED                   | `das-drive soak` hard-fails when `can:raw:on` ack missing, sends `can:raw:off`, returns before `stream:on`                                                                                             | New test (RED → GREEN)                                                                   |
| P7-008 | FIXED                   | `resolveOptions` uses validated `numericFlag()`: safe integer + per-flag ranges, throws on bad input; no `Number(x)                                                                                    |                                                                                          | default` for numeric flags | New tests for `--baud`, `--timeout`, `--speed`, `--duration`, `--flash-baud` (RED → GREEN) |
| P7-009 | FIXED                   | Shared `duration` parsed once; precedence documented and tested: `--drive-duration` > `--duration` > 30000; `watchDurMs`/`duration` use same parsed value                                              | New precedence tests (RED → GREEN)                                                       |
| P7-010 | FIXED                   | Serial parse-error entries carry `error` detail; bridge `normalizeEntry` returns `{t:"parse-error",raw,error}` instead of `{t:"text",raw:""}`                                                          | New bridge test + session parse-error test (RED → GREEN)                                 |
| P7-011 | FIXED                   | POST `/api/command` requires `application/json` Content-Type (charset parameter allowed), returns 415 before reading body/sending command                                                              | New bridge tests 415 + charset acceptance (RED → GREEN)                                  |
| P7-012 | FIXED                   | `main()` tracks server, exports `closeBridgeServer()`, closes it in catch before rethrow; shutdown closes server/session as before                                                                     | New bridge helper test (RED → GREEN)                                                     |
| P7-013 | FIXED                   | Exported `parseReplayLines()` separates frames from skipped lines (CSV header, malformed JSON, wrong shape); `runReplay` reports `Skipped: N` and appends `skipped: N` to pass line                    | New `replay.test.js` (RED → GREEN)                                                       |
| P7-014 | FIXED                   | `--flash-baud` mapped and validated (9600..3000000, default 460800); esptool invocation and info line use it                                                                                           | New args tests (RED → GREEN)                                                             |
| P7-015 | FIXED                   | Anchored `BOOT_ENVELOPE_RE` replaces substring `includes()` for raw fallback; noisy embedded boot text no longer counts                                                                                | New diagnosis test (RED → GREEN)                                                         |

Result: 14 fixed, 1 skipped (already fixed by C1-09a). No stale items found.

## TDD evidence

- RED run before implementation: `Test Suites: 7 failed, 8 passed, 15 total; Tests: 6 failed, 109 passed, 115 total`. Failures covered readline close, numeric args, flash-baud, diagnosis substring, CSV injection, das-drive ack, plus suite-link errors for new exports (`openSerialWith`, `closeBridgeServer`, `parseReplayLines`).
- GREEN run after implementation: `npm test -w tools` → `Test Suites: 15 passed, 15 total; Tests: 150 passed, 150 total`.
- `npm run lint:all` → ESLint clean, Prettier clean, markdownlint 0 issues.

## Files changed

Source:

- `tools/lib/session.js`
- `tools/lib/args.js`
- `tools/lib/diagnosis.js`
- `tools/lib/canExport.js`
- `tools/commands/dump.js`
- `tools/commands/smoke.js`
- `tools/commands/das-drive.js`
- `tools/commands/replay.js`
- `tools/commands/flash.js`
- `tools/serial-http-bridge.js`

Tests:

- `tools/test/session.test.js`
- `tools/test/args.test.js`
- `tools/test/diagnosis.test.js`
- `tools/test/bridge.test.js`
- `tools/test/security-commands.test.js`
- `tools/test/replay.test.js` (new)
- `tools/test/smoke.test.js` (new)

No firmware, client, protocol, docs, or `legacy/` files touched.

## Self-review

- All 15 items re-checked against current source; no duplicate P3 work (auth, body cap, timeouts, path containment, write callbacks unchanged).
- `openSerialWith` is exported for injection; production `openSerial` still dynamic-imports `serialport` and preserves the friendly module-not-found message.
- Numeric validation is centralized in one helper; optional `kph` stays `null`, `test-profile` keeps `-1` sentinel via fallback.
- `csvField` is now formula-safe globally; decoder CSV export still passes its existing tests.
- Bridge Content-Type check happens after auth and before body read; 401 still wins over 415.
- Smoke no longer conflates "no frames" with "sleep elapsed"; bounds remain deadline-based so it cannot hang.
- `send()` throwing on a down port is a behavior change for callers that previously dropped commands silently; reviewed all call sites via handler flow — teardown always happens in `finally`/shutdown, so no normal path sends after close.

## Concerns

- CLI clients that POST to the bridge without `Content-Type: application/json` now get 415 (intended by brief). Callers using `fetch` with a JSON body string already send `text/plain` unless they set the header explicitly; client bridge callers should be checked separately if they exist outside `tools/`.
- `csvField` now prefixes `'` to decoder-export values starting with `-`, `+`, `=`, `@`, tab or CR. Existing CSV tests pass; this is intentional safety hardening but changes literal output for such values.
- P7-006 replaced sleeps with bounded frame waits using a 50 ms floor; on very slow buses smoke may report "No raw frames" sooner than before (bounded by `timeoutMs` when larger).
- P7-003 throw-on-down-port could surface as a hard failure in a command whose serial port drops mid-run; that is the intended fail-loud behavior.
