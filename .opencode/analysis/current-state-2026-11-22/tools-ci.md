# Current-State Extraction — Tools + CI (2026-11-22)

Subagent P0-00. Read-only on codebase; sole written file is this report.

## Scope and evidence grades

- **CODE-READ** — source file read directly from the on-disk tree this session.
- **TEST-READ** — test file read directly.
- **CAPTURE** — live local command output captured this session (`node`, `npm test`, `npm run validate`, `git ls-files`, `git status`).
- **SUMMARY-ONLY** — claim taken from the old `docs/superpowers/todo.md` (untracked file, `git status` CAPTURE) without independent historical verification.
- **UNVERIFIED** — not checked; listed explicitly at the end.

Working-tree caveat: `git status` (CAPTURE) shows extensive **uncommitted** modifications across firmware, client, legacy, and config files. None of the `tools/**`, `.github/**`, or `scripts/**` files cited below appear in that modified list, so tools/CI evidence reflects committed state. The old todo is untracked (`?? docs/superpowers/todo.md`, CAPTURE); it was used only as an ID checklist, never as evidence.

## 1. `tools/**` — what actually exists

### 1.1 CLI entry point and command surface

`tools/debug.js` is the `tcm-debug` bin (`tools/package.json:7-9`, CODE-READ). It registers 10 session commands plus `flash` and portless `export`: `SESSION_COMMAND_HANDLERS` at `tools/debug.js:26-37`, `PORTLESS_COMMANDS`/`COMMANDS` at `tools/debug.js:39-40`, export dispatch at `tools/debug.js:69-71`, flash dispatch at `tools/debug.js:79-82`, session dispatch at `tools/debug.js:87-117`, diagnosis/JSON report at `tools/debug.js:119-155`.

| Command         | File                                      | Function                                                                                      |
| --------------- | ----------------------------------------- | --------------------------------------------------------------------------------------------- |
| `smoke`         | `tools/commands/smoke.js:6-113`           | boot/ping/status handshake, filtered + raw stream check, variant switch to hw4 and restore    |
| `watch`         | `tools/commands/watch.js:33-153`          | live frame/status monitor, hex bit-diff (`parseHexData`/`compareBits` at `:6-31`), ID filter  |
| `test`          | `tools/commands/test.js:5-121`            | FSD/profile round-trip, optional FSD activate-and-listen                                      |
| `flash`         | `tools/commands/flash.js:132-225`         | esptool flash, merged-image vs PlatformIO 4-segment plan (`:54-91`), post-flash serial verify |
| `scan`          | `tools/commands/scan.js:6-79`             | unique CAN ID discovery with count/dir/DLC                                                    |
| `dump`          | `tools/commands/dump.js:8-73`             | JSONL/CSV frame capture to file                                                               |
| `replay`        | `tools/commands/replay.js:8-72`           | JSONL frame playback as `{"cmd":"raw",...}` (`:52-58`)                                        |
| `benchmark`     | `tools/commands/benchmark.js:5-70`        | throughput, interval min/max, top-5 IDs                                                       |
| `vehicle`       | `tools/commands/vehicle.js:3-80`          | 39-name map to high-level vehicle commands                                                    |
| `drive-context` | `tools/commands/drive-context.js:241-385` | status-based D-05/D-11/D-13 evidence capture + closure gates                                  |
| `das-drive`     | `tools/commands/das-drive.js:57-191`      | DAS drive status/set-cap/set-speed/toggle/soak                                                |
| `export`        | `tools/commands/export.js:20-118`         | decoder dataset -> DBC/CSV/summary JSON                                                       |

`debug.js` usage goes to stderr (`tools/debug.js:42-46`, CODE-READ); `tools/README.md:24-35` core-command table omits both `das-drive` and `export` while the CLI wires them (new finding N-08).

### 1.2 Serial session layer

`tools/lib/session.js` (CODE-READ): `openSerial` dynamic-imports `serialport` and wraps the callback constructor in a Promise (`:5-22`); `BoardSession` keeps `_queue`, `_waiters`, `_allMessages` and a `readline` interface created at `:32-34` but never stored or closed; `_onRawLine` extracts JSON between first `{` and last `}` (`:36-57`); `waitFor`/`waitForType`/`waitForAck`/`drainType` at `:59-99`; `send` auto-wraps plain names into `{"cmd":...}`, passes `{`-prefixed envelopes through, supports `{raw:true}` (`:101-125`); `send` silently returns when `writable === false` (`:115`); `close` only closes the port (`:131-133`). No write callback, no readline close, no reject on down port.

`tools/lib/args.js` (CODE-READ): `parseArgs` at `:3-32`; `resolveOptions` at `:34-85` uses `Number(x) || default` for most numeric flags (`:38-45`, `:63`, `:69`, `:71`) but `!== undefined ? Number(x) : null` for `kph`/`duration` (`:82-83`). `driveCtxDurMs` reuses `args.duration` (`:75`), which is also `watchDurMs` (`:45`) and dash `duration` (`:83`).

`tools/lib/output.js` (CODE-READ): colors/counters at `:3-58`; `ts()` uses `toTimeString().slice(0,12)` (`:60-62`).
`tools/lib/diagnosis.js` (CODE-READ): health matrix at `:3-46`, verdict strings at `:48-68`, `tagPhase` at `:70-74`; boot fallback uses `raw.includes('"t":"boot"')` (`:7`).

### 1.3 Serial-to-HTTP bridge

`tools/serial-http-bridge.js` (CODE-READ, full file). Local `parseArgs` (`:19-67`) defaults `host: "0.0.0.0"` (`:24`), while its own help text suggests `--host 127.0.0.1` (`:71`). `json()` sets `Access-Control-Allow-Origin: *` (`:76-86`). `readBody` is unbounded (`:88-94`). Endpoints: `GET /health` (`:120-131`), `GET /api/status` (`:133-145`), `POST /api/command` (`:147-186`) with no auth, no Content-Type check, and arbitrary `cmd` proxied to firmware via `session.send(cmd)` (`:164`). No `server.timeout`/`headersTimeout` (`:111-193`). `main().catch` exits without `server.close()` (`:218-222`). SIGINT/SIGTERM shutdown at `:200-215`.

### 1.4 Capture, replay, export, decoder libraries

- `tools/lib/canExport.js` (CODE-READ, `:1-224`): `toDbcString` (`:25-83`, placeholder `0|8@1+` layout, only VAL\_ labels accurate — explicitly documented at `:17-20,38-39`), `toCsvString` (`:87-150`, proper `csvField` quoting at `:217-224`), `toSummaryJson` (`:164-203`). This is decoder-dataset export, **not** live frame decoding.
- `tools/commands/export.js` (CODE-READ): 25 MB input cap (`:18`, `:42-47`), overwrite guard (`:100-103`), standalone entry (`:120-141`).
- No `tools/io*.json` schema and no `tools/canExport.js` (git ls-files CAPTURE); the real schema is `firmware/lib/io/schemas/io.schema.json` (git ls-files CAPTURE).
- No golden-capture directory, no decoder fixture, no per-ID assertion library anywhere under `tools/**` (grep CAPTURE).

### 1.5 Tests

`tools/test/jest.config.js:1-6` (TEST-READ). Nine suites, 105 tests, all passing locally: `npm test -w tools` -> `Test Suites: 9 passed, 9 total; Tests: 105 passed, 105 total` (CAPTURE). Files: `args.test.js`, `canExport.test.js`, `diagnosis.test.js`, `drive-context.test.js`, `export-command.test.js`, `flash.test.js`, `output.test.js`, `session.test.js`, `watch.test.js` (git ls-files CAPTURE). Coverage is pure unit-level on libs/commands; `session.test.js:25-29` calls `_onRawLine` directly because readline cannot be mocked, so no end-to-end serial path is exercised. No test touches `serial-http-bridge.js`, `dump.js`, `replay.js`, `smoke.js`, `das-drive.js`, or `flash.js` esptool invocation.

## 2. CI, scripts, Docker, Husky — real local behavior

### 2.1 `.github/workflows/ci.yml` (CODE-READ, full file)

| Job              | Lines      | What runs                                                                                  | Local assessment                                                                                         |
| ---------------- | ---------- | ------------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------- |
| `firmware`       | `:17-43`   | `pio test -e native` (`:41`), `check_firmware_size.py --env esp32_chassis_8mhz` (`:42-43`) | Size check covers smallest env only; 25 `esp32*` envs exist (`platformio.ini` grep CAPTURE)              |
| `protocol`       | `:45-66`   | `npm test -w packages/protocol`                                                            | —                                                                                                        |
| `e2e-smoke`      | `:68-92`   | protocol build + `jest --testPathPattern="e2e-smoke"`                                      | —                                                                                                        |
| `client`         | `:94-118`  | protocol build, typecheck, tests                                                           | —                                                                                                        |
| `tools`          | `:120-141` | `npm test -w tools` (`:141`)                                                               | Local equivalent green (CAPTURE)                                                                         |
| `docker`         | `:143-155` | `docker compose build --no-cache`                                                          | —                                                                                                        |
| `lint`           | `:157-180` | serial-contract validate + `lint:all` + `lint:md`                                          | Local validate green (CAPTURE); `lint:md` duplicated inside `lint:all` (`package.json:16,18`, CODE-READ) |
| `workflow-lint`  | `:182-193` | `rhysd/actionlint@v1.7.12`                                                                 | —                                                                                                        |
| `markdown-lint`  | `:195-209` | `DavidAnson/markdownlint-cli2-action@v23`                                                  | `.opencode/**` ignored by `.markdownlint-cli2.jsonc:15`                                                  |
| `security-audit` | `:211-236` | `npm run audit:ci`, `license-checker`                                                      | —                                                                                                        |
| `docker-smoke`   | `:238-264` | `docker compose up -d client-prod` + curl loop on `127.0.0.1:8080`                         | —                                                                                                        |

Every job includes an identical "CI-only custom legacy folder exceptions" block (e.g. `:27-32`, `:52-55`) that `git update-index --force-remove`s two submodule paths and `rm -rf`s four legacy dirs — repeated 11 times (drift risk).

### 2.2 Scripts

- `scripts/release-matrix-smoke.mjs` (CODE-READ, `:1-332`): still parses a hardcoded `strategy.matrix.env` / `strategy.matrix.profile` from `release.yml` (`:109-128`). `release.yml` now uses `discover-envs` + dynamic `fromJSON` (`:68-94`, `:103-106`). Live proof: `node scripts/release-matrix-smoke.mjs --dry-run` -> `[release-matrix-smoke] ERROR: Could not find strategy.matrix.env in release workflow` (CAPTURE, exit 1). **Confirmed broken.**
- `scripts/validate-serial-contract.mjs` (CODE-READ, `:1-261`): schema path `firmware/lib/io/schemas/io.schema.json` (`:7-9`); if absent, prints "Skipping serial contract validation" and `exit(0)` (`:181-188`) — self-disabling gate. Schema is present today, and local run prints `Validated unified io.schema.json contract.` (CAPTURE).
- `scripts/npm-audit-actionable.cjs` (CODE-READ, `:1-110`): runs `npm audit --json`, filters at/above threshold (`:70-77`), treats `fixAvailable === true` as non-actionable (`:37-45`), exits 1 only for concrete non-major fixes (`:82-91`).
- `firmware/scripts/check_firmware_size.py` (CODE-READ, `:1-146`): thresholds for 4 envs (`:26-51`); unknown env prints "skipping" and returns `True` (`:56-58`); parses Flash/RAM from `pio run -v` output (`:65-90`).

### 2.3 Docker

`docker-compose.yml:1-50` (CODE-READ): `client` dev target on 4173 with healthcheck (`:2-29`), `client-prod` production target on 8080, read-only rootfs, tmpfs, `no-new-privileges` (`:31-50`). `client/Dockerfile:1-24` (CODE-READ): node:22 base, `npm install` after deleting root lockfile (`:9`), nginx:1.27-alpine production serving `client/dist-web` (`:21-24`).

### 2.4 Husky / lint-staged

`.husky/` is **not tracked and not present on disk** (git ls-files + glob CAPTURE). `package.json` still has `"prepare": "husky"` (`:19`) and lint-staged config (`:31-39`) that sends `*.md` through Prettier only, not markdownlint. CI enforces markdownlint independently (`ci.yml:195-209`). Net: markdown lint is CI-only; no repo-tracked pre-commit hook to fix.

## 3. Build, release, artifact publishing, flasher consumption

- Trigger: tags `v*` or `workflow_dispatch`; push to `main` auto-tags only when `package.json` version is stable, `CHANGELOG.md` has a `## [X.Y.Z]` section, and the tag does not already exist (`.github/workflows/auto-tag-release.yml:3-7,38-69`), then dispatches `release.yml` via `gh-script` (`:71-83`).
- `release.yml` gate (`validate-ref` `:13-21`, `quality-gate` `:24-62`): protocol tests, serial contract, client build/typecheck/tests/web-build, audit, license. **No firmware native tests** and no dependency on `ci.yml` success.
- Env discovery (`release.yml:68-94`): grep `^\[env:esp32[^]]*\]` from `firmware/platformio.ini` -> jq array. Actual count: 25 `esp32*` envs (grep CAPTURE); `native` and 6 `test_esp32_*` envs correctly excluded. Build matrix (`:97-139`) runs `pio run -e ${{ matrix.env }}`, copies `build/firmware/<env>.bin` into `release-artifacts/`, uploads artifact named `firmware-<env>.bin`.
- Release job (`:142-159`): downloads all artifacts with `merge-multiple: true`, publishes via `softprops/action-gh-release@v3` with `generate_release_notes: true`, `files: artifacts/*`, **`fail_on_unmatched_files: false`** — a missing artifact set still produces a green release job.
- Flasher consumption (`client/src/screens/FlasherScreen.tsx`, CODE-READ): env composed by `resolveEnvironment` from wifi/ble/chassis/vehicle/body toggles + 8/16 MHz (`:53-66`), asset name `${env}.bin` (`:68-70`), fetched from `https://api.github.com/repos/babico/TeslaCANModder/releases/latest` (`:46-47`, `:72-93`), matched by `assets[].name` (`:83`), flashed from `browser_download_url` through `flashMergedEspReleaseImage` (`:189-222`). `handleBuild` triggers a plain download (`:151-163`). Naming contract with `release.yml:131-138` matches. Edge case: if all buses are off, `resolveEnvironment` yields `esp32_8mhz`, which is not a defined env (platformio.ini grep CAPTURE) -> "asset not attached" error.

## 4. Old todo re-check (P3, P4, P7, PB, PC)

Statuses: **FIXED 1 / STILL_PRESENT 49 / STALE 3** out of 53 enumerated items. All "STILL_PRESENT" claims below are CODE-READ of current files unless noted.

### 4.1 P3 — tools security/data (10 items: 0 fixed, 10 still)

| ID     | Status        | Current evidence                                                                                         |
| ------ | ------------- | -------------------------------------------------------------------------------------------------------- |
| P3-001 | STILL_PRESENT | `tools/serial-http-bridge.js:24,81-83,147-186` — 0.0.0.0 default, CORS `*`, no auth, arbitrary cmd proxy |
| P3-002 | STILL_PRESENT | `tools/serial-http-bridge.js:88-94` — unbounded body                                                     |
| P3-003 | STILL_PRESENT | `tools/serial-http-bridge.js:111-193` — no server/headers timeouts                                       |
| P3-004 | STILL_PRESENT | `tools/commands/dump.js:12-17,75-77` — only `\0` rejected, `resolve()` no containment                    |
| P3-005 | STILL_PRESENT | `tools/commands/replay.js:16-21,74-76` — same helper                                                     |
| P3-006 | STILL_PRESENT | `tools/commands/drive-context.js:369-379` — `path.resolve` + `writeFileSync`, no containment             |
| P3-007 | STILL_PRESENT | `tools/commands/dump.js:23,71` — createWriteStream + end, no drain/error await                           |
| P3-008 | STILL_PRESENT | `tools/lib/session.js:124` — `this._sp.write(...)` with no callback                                      |
| P3-009 | STILL_PRESENT | `tools/commands/das-drive.js:103-108` — `kph >= 1` only; contrast `:86` set-cap 1..200                   |
| P3-010 | STILL_PRESENT | `tools/commands/flash.js:19-25` — `process.env.PLATFORMIO_CORE_DIR` first candidate                      |

### 4.2 P4 — CI drift (10 items: 0 fixed, 9 still, 1 stale)

| ID     | Status        | Current evidence                                                                                                                                                                                                              |
| ------ | ------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| P4-001 | STILL_PRESENT | `scripts/release-matrix-smoke.mjs:109-128` vs `.github/workflows/release.yml:68-94,103-106`; live dry-run error (CAPTURE)                                                                                                     |
| P4-002 | STILL_PRESENT | `docs/reference/commands.md:57` still labels node `firmware/lib/interface/dispatch.h`                                                                                                                                         |
| P4-003 | STILL_PRESENT | `docs/reference/serial-contract.md:121,130,131` still reference `lib/interface/common/json.h` and `lib/ble/nus/esp32/common.h`                                                                                                |
| P4-004 | STILL_PRESENT | `firmware/README.md:12,19-22,28,35,42-43` still uses nonexistent `esp32`/`esp32_wifi`/`esp32_ble`/`esp32_wifi_ble`                                                                                                            |
| P4-005 | STILL_PRESENT | `.github/dependabot.yml:27-37` still scans `/hardware` and `/web`                                                                                                                                                             |
| P4-006 | STILL_PRESENT | `.github/workflows/OPENCODE.md:40` says `ANTHROPIC_API_KEY` required; workflows use `OPENCODE_GO_API_KEY` (e.g. `opencode.yml:29-34`)                                                                                         |
| P4-007 | STILL_PRESENT | `OPENCODE.md:46` claims reviewer read-only; `opencode-review.yml:11-15` grants `pull-requests: write`                                                                                                                         |
| P4-008 | STALE         | No `.husky/` tracked or on disk (git ls-files/glob CAPTURE); the cited hook does not exist in this tree. Related residual: lint-staged `.md` -> Prettier only (`package.json:31-39`); markdownlint CI-only (`ci.yml:195-209`) |
| P4-009 | STILL_PRESENT | `firmware/scripts/check_firmware_size.py:56-58` — unknown env returns True                                                                                                                                                    |
| P4-010 | STILL_PRESENT | `.github/workflows/ci.yml:42-43` — only `esp32_chassis_8mhz`                                                                                                                                                                  |

### 4.3 P7 — tools medium (15 items: 0 fixed, 15 still)

| ID     | Status        | Current evidence                                                                                                             |
| ------ | ------------- | ---------------------------------------------------------------------------------------------------------------------------- | --- | ------------------------------ |
| P7-001 | STILL_PRESENT | `tools/lib/session.js:32-34` createInterface not stored; `:131-133` close() skips it                                         |
| P7-002 | STILL_PRESENT | `tools/lib/session.js:8-13` callback inside `new SerialPort`                                                                 |
| P7-003 | STILL_PRESENT | `tools/lib/session.js:115` silent return on `writable === false`                                                             |
| P7-004 | STILL_PRESENT | `tools/commands/dump.js:45-49` raw CSV interpolation (safe `csvField` exists in `tools/lib/canExport.js:217-224` but unused) |
| P7-005 | STILL_PRESENT | `tools/commands/dump.js:47,51` write return ignored                                                                          |
| P7-006 | STILL_PRESENT | `tools/commands/smoke.js:71-72,81-82,91` fixed sleeps                                                                        |
| P7-007 | STILL_PRESENT | `tools/commands/das-drive.js:145-146` `waitForAck` result ignored, soak continues                                            |
| P7-008 | STILL_PRESENT | `tools/lib/args.js:38-45,63,69,71` `Number(x)                                                                                |     | default` still hides bad input |
| P7-009 | STILL_PRESENT | `tools/lib/args.js:45,75,83` `duration` reused for watch/drive/dash                                                          |
| P7-010 | STILL_PRESENT | `tools/serial-http-bridge.js:96-104` parse errors normalized to `{t:"text",raw:""}`                                          |
| P7-011 | STILL_PRESENT | `tools/serial-http-bridge.js:147-157` no Content-Type enforcement                                                            |
| P7-012 | STILL_PRESENT | `tools/serial-http-bridge.js:218-222` catch logs + exits, no server close                                                    |
| P7-013 | STILL_PRESENT | `tools/commands/replay.js:31,37-39` CSV header and JSON errors silently skipped                                              |
| P7-014 | STILL_PRESENT | `tools/commands/flash.js:13,123,160` `FLASH_BAUD = "460800"` hardcoded                                                       |
| P7-015 | STILL_PRESENT | `tools/lib/diagnosis.js:5-7` substring boot detection                                                                        |

### 4.4 PB — tools low (10 items: 0 fixed, 10 still)

| ID     | Status        | Current evidence                                                                        |
| ------ | ------------- | --------------------------------------------------------------------------------------- |
| PB-001 | STILL_PRESENT | `tools/debug.js:42-46` `console.error` in `printUsage`                                  |
| PB-002 | STILL_PRESENT | `tools/debug.js:106-110` variant unvalidated                                            |
| PB-003 | STILL_PRESENT | `tools/commands/dump.js:75-77` + `tools/commands/replay.js:74-76` duplicate helper      |
| PB-004 | STILL_PRESENT | `tools/commands/replay.js:78-84` `isReplayFrameLike` unexported, untested               |
| PB-005 | STILL_PRESENT | `tools/commands/das-drive.js:155-156` `id ?? canId` + dual parse                        |
| PB-006 | STILL_PRESENT | `tools/serial-http-bridge.js:19-67` local parseArgs duplicates `tools/lib/args.js:3-32` |
| PB-007 | STILL_PRESENT | `tools/lib/output.js:60-62` locale-dependent `toTimeString().slice(0,12)`               |
| PB-008 | STILL_PRESENT | `tools/lib/args.js:38-45` vs `:82-83` mixed numeric style                               |
| PB-009 | STILL_PRESENT | `tools/commands/export.js:120-141` standalone entry in same file                        |
| PB-010 | STILL_PRESENT | `tools/commands/das-drive.js:178-181` magic `10` threshold                              |

### 4.5 PC — cross-cutting low (8 items: 1 fixed, 5 still, 2 stale)

| ID     | Status        | Current evidence                                                                                                                                                                           |
| ------ | ------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| PC-001 | STILL_PRESENT | `package.json:18` `npx --yes markdownlint-cli2` unpinned locally (CI pins `@v23` at `ci.yml:206`)                                                                                          |
| PC-002 | STILL_PRESENT | `firmware/platformio.ini:99-100` `upload_port = COM4`, `monitor_port = COM4`                                                                                                               |
| PC-003 | STILL_PRESENT | Both `docs/architecture/client-guide.md` and `docs/architecture/unified-client-guide.md` tracked (git ls-files CAPTURE)                                                                    |
| PC-004 | STALE         | `tools/io*.json` absent, but real schema `firmware/lib/io/schemas/io.schema.json` tracked and used by `scripts/validate-serial-contract.mjs:7-9,181-188` (CAPTURE); local validation green |
| PC-005 | STALE         | No `tools/canExport.js`; actual `tools/lib/canExport.js` is tested and docs reference the correct path (`docs/reference/can-export.md:115`; TEST-READ `tools/test/canExport.test.js`)      |
| PC-006 | STILL_PRESENT | `CHANGELOG.md:327` still lists the 5 removed envs (historical entry)                                                                                                                       |
| PC-007 | STILL_PRESENT | Only `firmware/scripts/check_firmware_size.py` tracked; no matching test (git ls-files CAPTURE)                                                                                            |
| PC-008 | FIXED         | `firmware/README.md:72-84` now documents real paths (`lib/core/`, `lib/core/can/`, `lib/vehicle/can/feature/`, `lib/vehicle/can/handler/`, `lib/io/`, `lib/client/`)                       |

## 5. Capture/replay readiness for disputed CAN IDs

Disputed IDs: `0x118` (DI_state), `0x257` (vehicle speed), `0x219` (TPMS), `0x3F5` (VCFRONT lights), `0x249` (DI_steer), `0x370` (EPAS torque), `0x318` (GTW state / OTA).

### 5.1 What tools can already do

- **Generic raw capture of any ID**: `dump` enables `can:raw:on` when `--raw` is set (`tools/commands/dump.js:28-31`), then writes every streamed `frame` to JSONL/CSV with `ts,id,dir,dlc,data,bus,seq` (`:33-63`). No ID-specific logic exists, so all seven IDs are capturable in principle.
- **Generic filtering/observation**: `watch --filter` accepts comma IDs via `tools/lib/args.js:50-53` and prints hex ID, DLC, seq, data (`tools/commands/watch.js:64-101`); `--bit-diff` compares RX/TX bytes per ID byte-by-byte (`:15-31,71-96`) — directly usable for mutation checks once golden RX bytes exist.
- **ID discovery/cadence**: `scan` (`:19-44`), `benchmark` (`tools/commands/benchmark.js:18-37,57-67`).
- **Generic replay**: `replay` reads JSONL and emits `{"cmd":"raw","id":...,"data":...,"bus":...}` at fixed `10/speed` ms spacing (`tools/commands/replay.js:27-69`).
- **Status-side evidence harness (pattern to copy)**: `drive-context` already implements coverage -> readiness -> checklist -> optional strict failure (`tools/commands/drive-context.js:133-230,301-367`) with clean unit tests (`tools/test/drive-context.test.js`, TEST-READ).
- **Decoder dataset export**: `tools/lib/canExport.js:25-83,164-203` converts decoder JSON to DBC/CSV/summary; `0x118` appears only as a _metadata fixture_ in `tools/test/canExport.test.js:11,142,206` and `tools/test/export-command.test.js:32` (TEST-READ) — not as live capture logic.

### 5.2 Exact gaps

1. **Zero tool-side handling of the seven IDs** — grep over `tools/**` finds only the three `0x118` fixture hits above; no `0x257`, `0x219`, `0x3F5`, `0x249`, `0x370`, `0x318` anywhere (grep CAPTURE). The only hardcoded IDs in tools are the DAS trio `0x2B9/0x488/0x27D` (`tools/commands/das-drive.js:20-22`).
2. **No golden-capture store or harness** — no `tools/test/fixtures/**`, no golden JSONL, no per-ID expected bytes/DLC/checksum (git ls-files + grep CAPTURE).
3. **No decoder in the tools path** — `canExport` emits DBC with placeholder bit layout (`tools/lib/canExport.js:38-39,61-62`); it cannot decode a captured frame back into signals, so captured bytes cannot be asserted semantically.
4. **`dump` has no `--filter`** although `resolveOptions` already produces `filterIds` (`tools/lib/args.js:50-53`) and `watch` consumes them (`tools/commands/watch.js:34,46,66`).
5. **No replay fidelity** — recorded `ts` is never used; replay always delays `10/speed` ms (`tools/commands/replay.js:65-68`). Bus defaults to 0 (`:56`) with no cross-check against recorded `bus`.
6. **No validation on ingest** — CSV header lines and unparseable lines are silently skipped (`tools/commands/replay.js:31,37-39`); no skipped-count reporting.
7. **No checksum verification** on capture or replay; raw bytes are preserved verbatim (good for fidelity, absent for assertion).
8. **No test wiring for a future harness** beyond the generic `tools/test/jest.config.js:1-6`; a new fixture-based test would need its own directory and npm wiring (currently `tools/package.json:10-13` only has `test` and `bridge`).

### 5.3 Minimal path to a golden-capture harness

1. Add `tools/lib/frameFixtures.js`: table keyed by the seven IDs with expected DLC, field bit masks/shifts, and checksum function name (source values to be CODE-READ from the firmware deciders and confirmed by capture — not inventable from tools alone).
2. Add `--filter` to `tools/commands/dump.js` by reusing `opts.filterIds` and the `watch.js:66` ID check; keep the existing JSONL `ts/id/dir/dlc/data/bus/seq` schema (no format change needed).
3. Capture one known-good JSONL per bus/variant under `tools/test/fixtures/golden/<env>/<variant>.jsonl` using `dump --raw --filter 280,599,537,1013,585,880,792` (decimal equivalents).
4. Add an offline `verify` mode (no serial needed) that reads a JSONL, decodes via `frameFixtures.js`, and asserts DLC, field ranges and checksums; mirror the `drive-context` coverage/readiness/checklist shape (`tools/commands/drive-context.js:133-230`) so it can gate with a non-zero exit.
5. Add `tools/test/golden-capture.test.js` (TEST-READ target) and a `times` fidelity check; `npm test -w tools` already discovers `test/**/*.test.js` (`tools/test/jest.config.js:3`), so no new runner wiring is required.
6. Extend `watch --bit-diff` to take the golden file as the RX reference so mutation diffs (`0x370` torque/counter, `0x3F5` turn bits, `0x249` stalk bits) become one-line observations.

## 6. New findings

| ID   | Severity | Finding                                                                                                                                                                                                          | One-line fix                                                                                                                      |
| ---- | -------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------- |
| N-01 | HIGH     | `serial-http-bridge.js` is an unauthenticated LAN command proxy: 0.0.0.0 default (`:24`), CORS `*` (`:81`), arbitrary `cmd` (`:164`), unbounded body (`:88-94`), no timeouts (`:111-193`). P3-001..003 all live. | Default to `127.0.0.1`, require a bearer token on `/api/command`, cap body at 1 MB and set `server.timeout`/`headersTimeout`.     |
| N-02 | HIGH     | Release can publish with no firmware binaries and without firmware tests: `fail_on_unmatched_files: false` (`release.yml:159`) and `quality-gate` omits `pio test -e native` (`:47-62`).                         | Set `fail_on_unmatched_files: true` and add a firmware native-test step to `quality-gate`.                                        |
| N-03 | MEDIUM   | `npm run smoke:firmware:release-matrix` is dead: parser expects a matrix that `release.yml` no longer has; live dry-run exits 1 (CAPTURE).                                                                       | Rewrite `parseWorkflowMatrix()` to read `firmware/platformio.ini` with the same regex as `release.yml:90-92`.                     |
| N-04 | MEDIUM   | Unconstrained file writes in `dump.js:17,23`, `replay.js:21`, `drive-context.js:370-379` plus fire-and-forget stream and CSV formula injection.                                                                  | Add one shared `resolveInsideCwd()` in `tools/lib/`, await stream finish/drain, and reuse `csvField` from `canExport.js:217-224`. |
| N-05 | MEDIUM   | Serial-contract gate silently passes when its schema is missing (`scripts/validate-serial-contract.mjs:181-188`); currently green only because the file exists.                                                  | Replace the skip with `fail(...)` so CI cannot lose the contract unnoticed.                                                       |
| N-06 | MEDIUM   | `BoardSession` can silently drop commands and leak its readline interface (`tools/lib/session.js:32-34,115,124,131-133`).                                                                                        | Store `this._rl`, close it in `close()`, and pass a write callback that surfaces errors.                                          |
| N-07 | MEDIUM   | All seven disputed IDs have zero tool-side decode or golden-capture support; `0x118` appears only as a decoder-export fixture.                                                                                   | Implement the 6-step harness in section 5.3, starting with `frameFixtures.js` + a `dump --filter`.                                |
| N-08 | LOW      | `tools/README.md:24-35` omits `das-drive` and `export`, both wired in `tools/debug.js:26-40,69-71`.                                                                                                              | Add both rows to the core-command table.                                                                                          |
| N-09 | LOW      | Only 4 of 25 shipping envs have size thresholds (`check_firmware_size.py:26-51`) and CI checks the smallest (`ci.yml:42-43`); unknown envs pass (`:56-58`).                                                      | Generate thresholds from `platformio.ini` and exit 2 on unknown env.                                                              |
| N-10 | LOW      | Flasher can compose nonexistent env `esp32_8mhz` when all buses are off (`FlasherScreen.tsx:53-66`), producing a false "asset not attached" error.                                                               | Disable no-bus selection or validate against a generated env allowlist.                                                           |
| N-11 | LOW      | Dependabot scans two deleted directories (`dependabot.yml:27-37`); duplicate markdownlint invocation (`ci.yml:180` inside `lint:all` at `package.json:16`).                                                      | Remove the two docker blocks and the extra `lint:md` step.                                                                        |
| N-12 | LOW      | Bridge help text advertises `--host 127.0.0.1` (`serial-http-bridge.js:71`) while runtime defaults to `0.0.0.0` (`:24`).                                                                                         | Make default and documented value agree (see N-01).                                                                               |

### Effort / risk table

| Fix                             | Effort                   | Risk   | Notes                                                                                         |
| ------------------------------- | ------------------------ | ------ | --------------------------------------------------------------------------------------------- |
| N-01 bridge hardening           | 30-60 min                | Medium | May require Expo Go clients to supply token; coordinate with client bridge URL config.        |
| N-02 release gate               | 15 min + one full CI run | Low    | Strict artifact matching may expose an existing missing-asset problem — desirable.            |
| N-03 matrix-smoke rewrite       | 30-45 min                | Low    | Script is already dead; no regression surface.                                                |
| N-04 path/stream hygiene        | 45-60 min                | Low    | Pure tooling; add tests using temp dirs as in `export-command.test.js:25-45`.                 |
| N-05 contract validator         | 5 min                    | Low    | Immediately makes a currently-skippable gate hard.                                            |
| N-06 session lifecycle          | 20-30 min                | Medium | Touches every CLI command's teardown; requires re-running all 9 tools suites.                 |
| N-07 golden harness             | 2-4 h + bench time       | Medium | Needs real captures for byte-level truth; decoder bits must be CODE-READ from firmware first. |
| N-08/N-10/N-11/N-12 docs/config | 5-15 min each            | Low    | Doc-only except flasher validation (N-10).                                                    |
| N-09 size thresholds            | 30 min + builds          | Medium | 25-env build matrix is slow; wrong thresholds create false CI failures.                       |

## Files read

- Tools: `tools/README.md`, `tools/package.json`, `tools/debug.js`, `tools/serial-http-bridge.js`, all of `tools/lib/{session,args,output,diagnosis,canExport}.js`, all of `tools/commands/{dump,replay,export,drive-context,das-drive,flash,smoke,watch,benchmark,scan,test,vehicle}.js`, `tools/test/jest.config.js` and all 9 `tools/test/*.test.js`.
- CI/scripts/config: `.github/workflows/{ci,release,auto-tag-release,opencode,opencode-review,opencode-audit,opencode-triage}.yml`, `.github/workflows/OPENCODE.md`, `.github/dependabot.yml`, `scripts/{release-matrix-smoke.mjs,validate-serial-contract.mjs,npm-audit-actionable.cjs}`, `firmware/scripts/check_firmware_size.py`, `package.json`, `tools/package.json`, `docker-compose.yml`, `client/Dockerfile`, `.prettierrc`, `.markdownlint.json`, `.markdownlint-cli2.jsonc`, `firmware/platformio.ini` (env list + `:90-107`), `firmware/README.md`, `CHANGELOG.md` (`:1-24`, `:320-332`), `docs/reference/commands.md`, `docs/reference/serial-contract.md`, `client/src/screens/FlasherScreen.tsx`.
- Old todo used only as ID list: `docs/superpowers/todo.md` (untracked).

## UNVERIFIED (explicitly not read / not checked)

- **GitHub Actions runtime status** — no GitHub API calls per instruction; "green/red" above comes from local file reading plus local `npm test -w tools` and `npm run validate:serial-contract` CAPTURE only. Whether any CI job is currently failing on GitHub is unknown.
- **Husky/local git hooks** — no `.husky/` tracked or on disk; `core.hooksPath`, global hooks, and whether `npm ci` regenerates a pre-commit hook were not inspected.
- **Firmware implementation** — no decider/handler/driver source read (other agents own firmware); byte/bit layouts for the seven disputed IDs were not derived.
- **packages/protocol source** — no decoder cross-check; protocol-side decode of the disputed IDs is unverified.
- **Client source** except `FlasherScreen.tsx` (and generated docs strings); no flasher end-to-end browser run.
- **Hardware-in-the-loop** — no serial device, no real captures generated, no golden files created; capture readiness is CODE-READ analysis only.
- **`npm run lint:all`, `npm run test:all`, `pio` builds/tests** — not run; no claim made about their current result.
- **History** — no `git log`/`git blame` on the todo items; FIXED/STILL/STALE reflects current tree only (hence SUMMARY-ONLY for the old todo's original claims).
