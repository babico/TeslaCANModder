# Final fix wave — complete findings list (base 5da646f)

One implementer, one scoped re-review. All findings must be addressed or explicitly parked with a ruling in the report.

## Important 1 — no runtime entrypoint

`pi/deploy/tcm-can.service:20` points to `pi/dist/main.js`; `pi/tsconfig.json:9` has `noEmit: true`; `pi/package.json` has no build script. `createCanReader`, `createBridgeListener`, `createRecorder`, `createGatedTx` are only referenced by their own sources/tests. No process binds `127.0.0.1:8090`, nginx `/tcm` proxies to a dead upstream, and the loopback pin in systemd has no code consumer.
Fix: add `pi/src/main.ts` that composes reader → bridge + recorder (gated TX remains UNMOUNTED), add an emit-capable build config and `build:pi` script, add a composition test. ADR §8 wording must match reality after the fix.

## Important 2 — production read path depends on an unshipped TCP gateway

`pi/src/can/socketcan.ts:115-142` `createNetTransport()` connects to `CAN_GATEWAY_HOST`/`CAN_GATEWAY_PORT + bus` expecting raw 16-byte `can_frame`s; no gateway implementation exists in the repo or deploy docs, yet `tcm-can.service:13` selects this path. All 79 tests use fakes.
Fix: ship a real SocketCAN read transport without native node modules — spawn `candump -L can0` (can-utils, already in the runbook) behind an injectable process factory, parse its log lines into the same `CanFrame`, add unit tests with a fake spawned process, and wire it into `main.ts`/systemd with a runbook note. Keep `createNetTransport` only if explicitly marked optional; do NOT call the old path "SocketCAN".

## Minors (fold into the same wave)

3. `pi/src/index.ts:58-66` — `bridge` early-returns so `fallback` is dead; fix stale `fallback` doc comment (`index.ts:19-22`) and dead `/health` in `bridge.ts:172-175`.
4. `pi/src/can/tx.ts:129-145` — gate evaluated once at construction; re-check per `send()`.
5. `pi/src/can/socketcan.ts:124` — validate `CAN_GATEWAY_PORT` integer 1..65535, fail fast (no silent reconnect storm).
6. `pi/src/can/socketcan.ts:166-179` — hoist `carry` outside reconnect loop (TCP can split mid-frame).
7. `pi/src/http/bridge.ts:28-29` — unused `timeoutMs`: use it or drop it and document mounted-only use.
8. `pi/src/record/recorder.ts:67-68` — sanitize filename (reject `/`, `\`, `..`), keep provenance.
9. `pi/src/sentry/connector.ts:165-173` — add `if (clip === drive) continue;` self-link guard.
10. `pi/src/rt/measure.ts:81-86,143-158` — validate `method.durationMin >= 60` in `isGateOpen()`.
11. `pi/deploy/nginx.conf:24-28,41-48` — add `proxy_set_header Connection "";` for `/` keepalive or drop keepalive.

## Constraints

- No CAN TX enabled, no gate artifact generated, no hardware claims — gated TX stays unmounted.
- No Sentry code copied; no code from the external guides copied.
- Stage ONLY files you change (~200 unrelated unstaged); verify staged lists before each commit.
- Feature branch `spike/raspberry-pi-portability`; never main, never force-push.
- Run `npm test -w @teslacanmodder/pi`, `npm run lint:all`, `npm run typecheck:pi`; full `test:all` is known to hang at Windows `pio.ps1` — note it, don't block on it.
- Full report at `.opencode/sdd/unified-pi-implementation-plan/final-fix-report.md`.
- No subagents.
