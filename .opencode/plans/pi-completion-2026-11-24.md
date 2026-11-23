# Pi completion plan — 2026-11-24

Goal: close the remaining software gaps so the Pi stack is internally complete; hardware verification stays a separate gate.

Scope: the parked Medium finding, remaining minors from the final review, and the `npm run test:all` hang root cause.

## Tasks

- **P1 — candump clean-exit reconnect.** `pi/src/can/candump.ts:24-30,104-116,154-161`: `candump` child exiting cleanly currently ends the stdout iterable with no error, so `createCanReader()` returns and the service silently stops seeing CAN. Add an `exit`/`exitCode` seam to `CandumpProcess`, populate it in `spawnCandumpProcess`, and treat clean exit as a reconnectable error after the stdout loop. Test with a fake process that ends cleanly and a second open that succeeds.
- **P2 — bound fanout queues.** `pi/src/main.ts:72-94`: each subscriber queue is unbounded; a stalled recorder can grow memory without limit. Cap at 256 frames per subscriber, drop oldest, and document/count drops in the report. Test that the pump never exceeds the cap and that bridge/recorder consumers stay independent.
- **P3 — loopback-only bind.** `pi/src/main.ts:34-55`: `parseBindConfig()` rejects wildcards but accepts arbitrary unicast hosts. Accept only `127.0.0.1`, `localhost`, and `::1`; reject everything else. Boundary tests.
- **P4 — absolute gate artifact path.** `pi/src/rt/measure.ts:74` and `pi/src/can/tx.ts:52` use cwd-relative `pi/gate/rt-result.json`. Resolve to an absolute path rooted at the `pi/` package (env override for tests); keep injected artifact paths working. Test that a service launched from another cwd still finds the same file.
- **P5 — fix `npm run test:all` hang.** Root cause: root `package.json:26` runs `cd firmware && .\pio.ps1 test -e native` under `cmd.exe`, which opens `.ps1` with Notepad (observed PID) and hangs. Add `scripts/run-pio.mjs`: on win32 spawn `powershell -NoProfile -ExecutionPolicy Bypass -File firmware/pio.ps1 <args>` (wrapper needed for non-ASCII path via `subst`), elsewhere spawn `pio <args>` with cwd `firmware`. Update `test:firmware` and `regen:firmware:compiledb` to use it. Verify `npm run test:firmware` exits 0 and `npm run test:all` completes.
- **P6 — verification + ADR.** Run protocol/client/tools/pi suites, lint, typecheck, build:pi, and `npm run test:all`; update `.opencode/reports/rpi-spike/ADR-2026-11-22.md` §8 with the closed items. Hardware claims remain UNVERIFIED.

## Constraints

- No CAN TX enablement, no gate artifact, no hardware claims.
- No Sentry code copied; stage ONLY changed files (~200 unrelated unstaged exist); verify staged lists before commits.
- Feature branch only; never main directly, never force-push; user standing instruction: commit + push.
- Markdown/TS hygiene per repo config.
