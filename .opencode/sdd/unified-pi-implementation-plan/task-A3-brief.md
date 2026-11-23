# Task A3 — Command bridge

Where: third task of the unified Pi plan. Mounts an authenticated JSON command API into the Pi server from A1, using CAN frames from A2.

## Requirement (verbatim from plan)

Files:

- Create: `pi/src/http/bridge.ts`, `pi/test/bridge.test.ts`
- Modify: `pi/src/index.ts` (mount bridge)

Interfaces:

- Consumes: `createCanReader()` (A2), command registry from `@teslacanmodder/protocol`.
- Produces: `POST /api/command` with bearer token, 1 MiB body cap, 415 on non-JSON, `GET /health`, `GET /status`.

Steps:

1. Write failing tests for 401 without token, 415 without Content-Type, 413 oversized body, and a `status` command returning JSON from a fake reader.
2. Run focused tests — expect RED.
3. Implement bridge following the hardened `tools/serial-http-bridge.js` pattern (constant-time token compare, default loopback bind, timeouts).
4. Run `npm test -w @teslacanmodder/pi` + `npm run lint:all` — PASS.
5. Commit — `feat(pi): authenticated command bridge`.

## Global constraints

- No CAN TX, no Sentry code, no hardware dependency; use injected fake reader.
- Reuse protocol command registry; do not invent a second command list.
- Stage ONLY files you change (~200 unrelated unstaged exist); verify staged list before commit.
- Feature branch `spike/raspberry-pi-portability`; never main, never force-push.
