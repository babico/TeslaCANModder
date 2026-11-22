# Task C1-09a — Tools security (P3-001..010) — report

Date: 2026-11-22. Branch: `chore/legacy-tesla-drug-fork-submodules`.

## Overall status

**10/10 fixed.** Every item was re-verified against current on-disk code before editing. None were already fixed or stale; all matched the §4.1 / N-01 / N-04 / N-06 evidence. No firmware or client files touched. No code from `legacy/`.

## Auth / CORS contract (decision recorded in task brief)

- Token flag: `--token <secret>` on `tools/serial-http-bridge.js`; env fallback `TCM_BRIDGE_TOKEN` (flag wins). Consistent with existing tools CLI style (explicit flags; env only where needed for non-interactive use).
- Wire format: `Authorization: Bearer <secret>` on `/api/status` and `/api/command`. Comparison is length-checked then `crypto.timingSafeEqual`.
- If no token is configured, **all `/api/*` calls return 401** — the bridge is fail-closed. `/health` stays public for liveness.
- CORS: `--allow-origin <origin>` restricts `Access-Control-Allow-Origin` to one explicit origin (default `http://<host>:<listen>`). `--allow-origin none` omits CORS headers entirely. Wildcard `*` is never emitted.
- Default bind is `127.0.0.1`; help text now matches the default and documents token/CORS flags.

## Per-item status

| ID     | Status | Fix                                                                                                                                                                                                                      |
| ------ | ------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | --- | ------------------------------------------------------------- |
| P3-001 | FIXED  | `serial-http-bridge.js`: default host `127.0.0.1`; bearer token required on `/api/*`; single-origin CORS (no `*`); `BRIDGE_HELP` updated. Handler extracted to exported `createBridgeServer()` so auth is unit-testable. |
| P3-002 | FIXED  | `readBody(req, MAX_BODY_BYTES)` with `MAX_BODY_BYTES = 1 MiB`; excess bytes are drained then rejected with 413 (no retained memory).                                                                                     |
| P3-003 | FIXED  | `server.timeout = 30000`, `server.headersTimeout = 5000` inside `createBridgeServer()`.                                                                                                                                  |
| P3-004 | FIXED  | `dump.js` uses shared `resolveInsideCwd()`; rejects `..`, absolute-outside, NUL, empty, and cwd-as-file before creating a stream.                                                                                        |
| P3-005 | FIXED  | `replay.js` same shared helper for `--input`; duplicate local helper removed.                                                                                                                                            |
| P3-006 | FIXED  | `drive-context.js` validates `--output` and `--note-output` at function entry (fail before any serial query), reuses the resolved absolute paths for both writes.                                                        |
| P3-007 | FIXED  | New `tools/lib/streams.js`: `writeWithDrain()` awaits `drain` on backpressure; `endStream()` awaits `finish` and propagates errors captured by the persistent stream error listener in `dump.js`.                        |
| P3-008 | FIXED  | `BoardSession.send()` passes a write callback; errors go to optional `onWriteError` hook or `console.error`. Existing tests updated for the new callback argument.                                                       |
| P3-009 | FIXED  | `das-drive.js`: exported `resolveSpeedCapMax()` (status `dasSpeedCapMaxKph`, clamped to hard 200); `set-speed` rejects `kph < 1                                                                                          |     | kph > capMax`with message`--kph must be an integer 1..<cap>`. |
| P3-010 | FIXED  | `flash.js`: `getPlatformIoHomes()` no longer reads `PLATFORMIO_CORE_DIR`; only `~/.platformio` is trusted. Exported for regression test.                                                                                 |

## TDD evidence

RED — new tests written before implementation, `npm test -w tools`:

```text
Test Suites: 5 failed, 8 passed, 13 total
Tests:       1 failed, 105 passed, 106 total
```

Failures: `Cannot find module '../lib/paths.js'`; `Cannot find module '../lib/streams.js'`; `SyntaxError: The requested module '../commands/flash.js' does not provide an export named 'getPlatformIoHomes'`; `SyntaxError: ... serial-http-bridge.js does not provide an export named 'BRIDGE_HELP'`; `TypeError: cb is not a function` at `session.test.js:127` (no write callback). Existing 105 tests still passed at RED.

GREEN — after implementation, `npm test -w tools`:

```text
Test Suites: 13 passed, 13 total
Tests:       131 passed, 131 total
Time:        1.044 s
```

Lint — `npm run lint:all` (run once):

```text
eslint .            -> clean
prettier --check .  -> All matched files use Prettier code style!
markdownlint-cli2   -> Summary: 0 issues in 0 files (44 files linted)
```

Bridge help smoke: `node tools/serial-http-bridge.js --help` prints the loopback default, `--token`, `TCM_BRIDGE_TOKEN`, and `--allow-origin` lines; exits 0 without opening serial.

New/changed regression tests: `tools/test/bridge.test.js` (401 missing/wrong token, 200 with bearer, protected status vs public health, 413 oversized body, no wildcard CORS, 30s/5s timeouts, loopback default, help text), `tools/test/paths.test.js` (containment variants incl. sibling-prefix escape), `tools/test/streams.test.js` (drain + finish/error), `tools/test/security-commands.test.js` (dump/replay/drive-context traversal refusal, cap derivation and rejection, flash env override), `tools/test/session.test.js` (write callback + error hook).

## Files changed

Source: `tools/serial-http-bridge.js`, `tools/lib/paths.js` (new), `tools/lib/streams.js` (new), `tools/lib/session.js`, `tools/commands/{dump,replay,drive-context,das-drive,flash}.js`.
Tests: `tools/test/{bridge,paths,streams,security-commands}.test.js` (new), `tools/test/session.test.js`.

Report file `.opencode/sdd/roadmap-2026-11-22/task-C1-09a-report.md` kept unstaged so the commit stays code+tests only.

## Self-review

- Checked each fix against the original cited line ranges; no item silently skipped.
- `createBridgeServer()` is separated from `main()`, and `main()` now runs only when the module is invoked directly — importing the bridge in tests no longer opens serial or calls `process.exit`.
- Auth check precedes body read and command dispatch; 413 path never calls `session.send()`.
- Reused one `resolveInsideCwd()` helper in three commands instead of three copies (PB-003 partially resolved as a side effect).
- `endStream()` accepts pre-captured errors because a `createWriteStream` failure can occur before the final await; without this, `dump` could report success after a write error.
- No `-A` / `.` staging; explicit `git add <paths>` only, `git diff --cached --name-only` verified before commit.

## Concerns / residual (out of scope or follow-up)

1. Static shared bearer token over plain HTTP: safe on the new loopback default, but an operator using `--host 0.0.0.0` sends the token in cleartext on the LAN. Needs TLS or a reverse proxy for remote use; no rotation/revocation.
2. Path containment is lexical. A pre-existing symlink inside cwd pointing outside still escapes. A `realpath`-based check would need the target to exist; not done here (brief asked for resolve + assert).
3. `BoardSession.send()` still returns silently when `_sp.writable === false` (P7-003), so commands can be dropped before reaching the write callback. Left untouched as out of scope.
4. Bridge still does not enforce Content-Type (P7-011), normalize parse errors (P7-010), or close the server on fatal startup error (P7-012). Out of scope.
5. `session.close()` still leaks the readline interface (P7-001). Out of scope.
6. `set-cap` still uses a hardcoded 200 literal rather than the new exported constant; behavior identical. Cosmetic follow-up.
7. 191 pre-existing unrelated unstaged files were left untouched.
