# Pi completion — implementation report

Branch `chore/pi-completion`, base `7bab10b`, 2026-11-24. Plan: `.opencode/plans/pi-completion-2026-11-24.md`. Software-only closeout of P1–P6; no CAN TX enablement, no gate artifact, no hardware claims, no Sentry code copied.

## TDD evidence (P1–P4)

RED written first, then implementation, then GREEN. All runs via `npm test -w @teslacanmodder/pi` (Jest ESM, Pi workspace).

| Phase                              | Result                                               | Detail                                                                                                                                         |
| ---------------------------------- | ---------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| Baseline (before any edit)         | 10 suites / 102 tests PASS                           | pre-change reference                                                                                                                           |
| RED (new tests, no implementation) | 4 suites FAIL, **7 failed / 103 passed / 110 total** | `test/candump.test.ts`, `test/main.test.ts` (bind hosts + fanout cap), `test/rt.test.ts` (path resolver ×3), `test/tx.test.ts` (path resolver) |
| GREEN (after P1–P4)                | 10 suites / **110 tests PASS**                       | no skipped tests                                                                                                                               |

Representative RED assertions:

- candump clean exit: only one `candump` open observed → expected 2 (`fake.calls().length`).
- loopback bind: `parseBindConfig({ TCM_BIND_HOST: "192.168.1.10" })` did not throw.
- fanout cap: stalled subscriber received 300 frames instead of 256.
- RT/TX path: `resolveGateArtifactPath` was `undefined` (missing export) and default write/load path was cwd-relative.

After GREEN, one test needed a legitimate refinement: the shared reader intentionally rethrows after `maxReconnects` is exhausted, so the clean-exit test now catches that final error and asserts it mentions `candump exited unexpectedly` while still verifying two process opens and both frames. No implementation behaviour was weakened.

## Implemented changes

- **P1** `pi/src/can/candump.ts`: added `CandumpExit` + `CandumpProcess.exit()` seam, populated from the child `exit` event in `spawnCandumpProcess`. After stdout ends, a spawn error wins; otherwise any recorded exit status becomes an error (`candump exited unexpectedly (code=…, signal=…)`) so `createCanReader()` reconnects. Fakes that end stdout with no exit event remain clean ends.
- **P2** `pi/src/main.ts`: exported `FANOUT_QUEUE_LIMIT = 256`; each subscriber queue drops its oldest frame at the bound. Documented that drops are per-consumer, never block peers, and are **not yet exported as a metric** (counting was not requested by the brief; deliberately not invented).
- **P3** `pi/src/main.ts`: `LOOPBACK_HOSTS = {127.0.0.1, localhost, ::1}` exact-match allowlist; all other hosts (wildcards, unicast, `localhost.evil.test`, `::ffff:127.0.0.1`, `[::1]`, `127.0.0.2`) rejected before composition.
- **P4** `pi/src/rt/measure.ts` + `pi/src/can/tx.ts`: new `resolveGateArtifactPath(env)` returns an absolute path anchored to `pi/` via `import.meta.url`; `PI_RT_GATE_ARTIFACT_PATH` env override, `artifactPath` and `gatePath` injections still win. The old `RT_GATE_ARTIFACT_PATH` string remains the documented package-relative path. Writer and TX reader now agree regardless of cwd.
- **P5** `scripts/run-pio.mjs` (new) + root `package.json`: win32 spawns `powershell -NoProfile -ExecutionPolicy Bypass -File <abs>/firmware/pio.ps1 <args>` (avoids `cmd.exe` associating `.ps1` with Notepad); other platforms spawn `pio <args>` with cwd `firmware`. Exit code/signal is forwarded; the launcher never kills processes. `test:firmware` and `regen:firmware:compiledb` now use it.
- **P5 wrinkle (extra file, disclosed)** `firmware/pio.ps1`: the wrapper's ASCII `subst` mapped `firmware/` to the drive root, so the native test `test_native_mqtt_contract` could not resolve `../docs/reference/commands.md` (1 failed / 1077, exit 1 on first run). The wrapper now maps the **repo root** and pushes to `<short-root>/firmware`, keeping `PLATFORMIO_CORE_DIR` under `firmware/.pio-home`. Direct `platformio test -e native -f test_native_mqtt_contract` from the real firmware dir passed, confirming the test itself was sound. After the wrapper fix, full firmware suite 1076/1076, exit 0.
- **P6** ADR §8 updated: `.opencode/reports/rpi-spike/ADR-2026-11-22.md` now records all closed items and the final verification numbers; the old "test:all hangs" bullet is marked fixed.

## Verification commands and results

All run from repo root on Windows (non-ASCII workspace path), 2026-11-24.

| Command                                                     | Result                                                                                                                                   |
| ----------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------- |
| `npm test -w @teslacanmodder/pi`                            | **110/110 PASS**, 10 suites, exit 0                                                                                                      |
| `npm run typecheck:pi`                                      | PASS, exit 0                                                                                                                             |
| `npm run build:pi` (`tsc -p pi/tsconfig.build.json`)        | PASS, exit 0                                                                                                                             |
| `npm run lint:all` (eslint + prettier check + markdownlint) | PASS, 0 issues, exit 0                                                                                                                   |
| `npm run test:firmware` (via `scripts/run-pio.mjs`)         | **1076/1076 PASS**, exit 0, ~1 m 26 s, no hang                                                                                           |
| `npm run test:all`                                          | **exit 0**, completed end-to-end: serial-contract validation, protocol build, protocol 581, client 305, tools 164, Pi 110, firmware 1076 |

`npm run test:all` previously hung indefinitely at the `.ps1`/Notepad association; it now finishes. No processes were killed by this work; no stray PlatformIO/powershell process was left by the completed runs.

## Worktree / commit discipline

~200 unrelated unstaged files were reported in the brief; at execution time `git status` showed only my edits plus the pre-existing untracked plan file. Every `git add` used explicit paths from the list below; no `git add -A` / `git add .`, no main branch, no force-push. Staged files:

```text
.opencode/reports/rpi-spike/ADR-2026-11-22.md
.opencode/sdd/pi-completion/report.md
firmware/pio.ps1
package.json
pi/src/can/candump.ts
pi/src/can/tx.ts
pi/src/main.ts
pi/src/rt/measure.ts
pi/test/candump.test.ts
pi/test/main.test.ts
pi/test/rt.test.ts
pi/test/support/fakeProcess.ts
pi/test/tx.test.ts
scripts/run-pio.mjs
```

The untracked `.opencode/plans/pi-completion-2026-11-24.md` was present before this session and was deliberately **not** staged. `pi/dist/` emitted by `build:pi` produced no tracked diffs.

## Honest UNVERIFIED / concerns

- No Pi, MCP2515, vcan, `candump -L` capture, `cyclictest`, RT artifact or vehicle acceptance. All hardware claims stay UNVERIFIED; the `candump -L` line format is still parsed from documented output, not a Pi log.
- `resolveGateArtifactPath()` is covered by unit tests including a real `process.chdir(tmpdir())`; no real signed artifact was written or read (none exists in this repo).
- Fanout drops are bounded and documented but not surfaced as a count/metric; a stalled consumer loses the oldest frames by design.
- The real `candump` `exit` seam is wired to Node's `child.on("exit")` but only exercised via fake processes; real child exit timing was not observed on hardware.
- `firmware/pio.ps1` change is Windows-only in effect (subst/ASCII mapping). Non-Windows launcher path calls `pio` directly and was not exercised on Linux/macOS in this workspace.
- CAN TX remains unmounted by `pi/src/main.ts` and disabled by default; nothing here opens a TX socket or generates a gate artifact.
