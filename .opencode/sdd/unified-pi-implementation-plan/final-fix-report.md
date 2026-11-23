# Final fix wave — implementation report

- Branch: `spike/raspberry-pi-portability`
- Base: `5da646f` (11 task commits) + this wave
- Date: 2026-11-23
- Scope: all 2 Important + 9 minor findings from `final-fix-brief.md`. No finding parked.

## Outcome

Both Important gaps are closed in code: a real runtime entrypoint exists and is emitted to the
systemd target, and the production read path is a `candump -L` process with an injectable seam —
not the unshipped TCP gateway. Gated TX remains UNMOUNTED; no gate artifact was generated; no
hardware verification is claimed. Pi suite grew 79 -> 102 passing tests (8 -> 10 suites).

## Per-finding status

| #   | Finding                                               | Status | Evidence                                                                                                                                                                                                                                                                                                                                                                               |
| --- | ----------------------------------------------------- | ------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --- | --------------------------------------------------------------------------- |
| 1   | No runtime entrypoint                                 | DONE   | `pi/src/main.ts` composes reader -> bridge + recorder, honours `TCM_BIND_HOST`/`TCM_BIND_PORT`, rejects wildcard binds. `pi/tsconfig.build.json` + `build:pi` emit `pi/dist/main.js`. Composition tests in `pi/test/main.test.ts` (5 tests). ADR §8 updated.                                                                                                                           |
| 2   | Production read path depends on unshipped TCP gateway | DONE   | `pi/src/can/candump.ts` spawns `candump -L <iface>` via `CandumpProcessFactory`, parses `-L` (and classic bracketed) lines into `CanFrame`, reuses `createCanReader` reconnect/parse. Fake-process tests in `pi/test/candump.test.ts` (7 tests). Wired into `main.ts` + systemd + README. `createNetTransport` kept but documented OPTIONAL/UNSHIPPED and never called by the service. |
| 3   | Stale `fallback` doc + dead bridge `/health`          | DONE   | Removed `PiServerOptions.fallback` and its branch (`pi/src/index.ts`), fixed comment, removed `/health` block from `pi/src/http/bridge.ts`. `smoke.test.ts` now asserts bridge mounting.                                                                                                                                                                                               |
| 4   | TX gate evaluated once                                | DONE   | `createGatedTx()` re-checks flag + gate on every `send()` (`pi/src/can/tx.ts`). 2 new tests incl. gate closing between sends + flag flip.                                                                                                                                                                                                                                              |
| 5   | Gateway port validation                               | DONE   | `parseGatewayPort()` strict integer 1..65535 + `port+bus <= 65535`; `createNetTransport` fails fast at construction. 2 new tests.                                                                                                                                                                                                                                                      |
| 6   | Carry reset across reconnect                          | DONE   | `carry` hoisted outside the reconnect loop in `createCanReader`. New split-frame-across-error test.                                                                                                                                                                                                                                                                                    |
| 7   | Unused `timeoutMs`                                    | DONE   | Dropped from `BridgeOptions`; `createBridgeListener` doc now states explicit MOUNTED-ONLY lifecycle (no socket, no timeout).                                                                                                                                                                                                                                                           |
| 8   | Unsanitized recorder filename                         | DONE   | `sanitizeFileName()` rejects `/`, `\`, `..`, NUL, empty; provenance still written as first JSONL line. 2 new tests.                                                                                                                                                                                                                                                                    |
| 9   | Sentry self-link                                      | DONE   | `if (!drive                                                                                                                                                                                                                                                                                                                                                                            |     | clip === drive) continue;`in`correlateClipsToDrives`. New self-record test. |
| 10  | RT duration not checked                               | DONE   | `isGateOpen()` now requires finite `method.durationMin >= 60`. New boundary/NaN tests.                                                                                                                                                                                                                                                                                                 |
| 11  | nginx keepalive `/`                                   | DONE   | Added `proxy_set_header Connection "";` to `/` location. `deploy.test.ts` asserts it.                                                                                                                                                                                                                                                                                                  |

## Constraints check

- CAN TX: never imported by `main.ts`; `src/can/tx.ts` unchanged in reachability (still `PI_CAN_TX=0` default, still needs a signed artifact). `txFactory` in the composition options is deliberately ignored and the test asserts it is not called.
- Gate artifact: none generated; `pi/gate/rt-result.json` still absent (existing tests still assert this).
- No Sentry code copied; no code copied from external guides.
- Staging: only explicitly changed paths staged (see commit bodies); ~219 pre-existing unstaged files untouched. No `-A`, no `.`, no force-push, no main.
- `npm run test:all` not run: pre-existing Windows `pio.ps1` hang documented in ADR §8; per-workspace commands used instead.

## Test commands and output

```
npm test -w @teslacanmodder/pi
PASS test/candump.test.ts / main / bridge / socketcan / record / rt / sentry / smoke / deploy / tx
Test Suites: 10 passed, 10 total
Tests:       102 passed, 102 total
Time:        ~1.8 s
```

RED confirmation before implementation (same command, before fixes):

```
Test Suites: 8 failed, 2 passed, 10 total
Tests:       8 failed, 63 passed, 71 total
```

Failures spanned `candump`, `main`, `socketcan`, `tx`, `record`, `sentry`, `rt`, `deploy`; the two
passing suites (`bridge`, `smoke`) were updated for the removed fallback seam.

```
npm run typecheck:pi
> tsc -p pi/tsconfig.json --noEmit
(exit 0, no output)

npm run build:pi
> npm run build -w @teslacanmodder/pi
> tsc -p tsconfig.build.json
(exit 0; emitted pi/dist/main.js plus 8 modules and source maps)

npm run lint:all
> eslint .            (0 errors, 0 warnings)
> prettier --check .  All matched files use Prettier code style!
> lint:md             Summary: 0 issues in 0 files (48 files)
```

## UNVERIFIED / honest limits

- **No Raspberry Pi, MCP2515 HAT, vcan, can-utils binary, bench capture or `nginx -t` run.** Every
  runtime claim is UNVERIFIED. The candump `-L` format (`(epoch) iface ID#DATA`) is parsed from
  documented can-utils output, not from a captured Pi log; the classic `[dlc]` form is a fallback.
- Fake spawned processes and fake HTTP listeners cover composition/parsing only. No real child
  process was spawned by the tests, no port was bound by the composition test, and no CAN frame
  was read from hardware.
- `pi/dist/main.js` was emitted and typechecked on Windows/Node 25; running it on Node 22 arm64
  with real can-utils is UNVERIFIED.
- CAN frame layout/semantics were not changed; no `can-review-checklist.md` walk was triggered
  (read-only path, no frame mutation/routing/checksum change).
- Full `npm run test:all` remains unrun due to the known Windows `pio.ps1` hang.

## Concerns for review

1. `fanoutFrames()` is backpressure-free (per-subscriber queues) so a stalled recorder grows memory
   on a high-rate bus. Chosen deliberately to keep the status snapshot non-blocking; a bounded
   policy with drop counters is a follow-up if bench data shows sustained rate.
2. `mergeFrames()` uses a microtask-driven drain; correctness is covered only by the fanout test
   and the single-interface composition test. Multi-bus (`TCM_CAN_IFACES=can0,can1`) aggregation is
   implemented but not bench-verified.
3. `createNetTransport()` is retained for bench compatibility. It is now explicitly documented as
   optional/unshipped; if the maintainer wants zero dead paths, deleting it and its tests is a clean
   follow-up with no impact on the service.
