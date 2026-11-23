# Task A3 report — authenticated command bridge

Branch: `spike/raspberry-pi-portability`
Base: `9e60626` (A2 tip).
Commit: `5d7bc72 feat(pi): authenticated command bridge` — pushed to `origin/spike/raspberry-pi-portability` (`9e60626..5d7bc72`).
Status: DONE — TDD RED/GREEN confirmed, 3 files committed with explicit staging, pushed, self-review clean. Post-commit re-verification green.

## What was built

Authenticated JSON command bridge mounted into the A1 Pi server:

- `POST /api/command` — bearer auth, `Content-Type: application/json` only, 1 MiB body cap, JSON-RPC `{"cmd":...}` input.
- `GET /status` and `GET /api/status` (compat alias) — snapshot derived from the A2 read-only CAN reader.
- `GET /health` — served by the A1 skeleton first (bridge also handles it when used standalone).
- `OPTIONS` preflight; everything else 404.

Security posture copied from the hardened `tools/serial-http-bridge.js`: fail-closed when no token, constant-time `timingSafeEqual` with length guard, body drain past the cap (413), media-type normalization with `charset` allowed, `WWW-Authenticate: Bearer`, `Cache-Control: no-store`, 30 s server / 5 s header timeouts.

No CAN TX, no Sentry code, no hardware dependency, no native modules.

## Envelope decision (brief asked to ask if ambiguous — no ambiguity remained)

Reused the existing serial contract rather than inventing one:

- Command responses carry the contract `t` field (`docs/reference/serial-contract.md:64-115`):
    - status → `{"t":"status","frames":N,"perBus":[..],"lastFrame":{t:"frame",...}}`
    - ping → `{"t":"pong","v":1}`
    - unknown/disallowed → `{"t":"error","msg":"Unknown command"}` (HTTP 400)
- `lastFrame` uses A2's `frameToWire()` — byte-for-byte the documented `{t:"frame",dir:"rx",bus,id,dlc,d}` shape.
- HTTP-level errors (401/415/413/404/400 request errors) use the established tools-bridge `{ok:false,error}` pattern, not a new wire envelope.

## Command registry reuse / no-TX enforcement

Wire strings come from the shared registry, not a second hand-written list:

- `const STATUS_WIRE = commands.status()`, `const PING_WIRE = commands.ping()`.
- `ALLOWED_COMMANDS = new Set([STATUS_WIRE, PING_WIRE])`.
- Every TX/injection builder in the registry (`fsd`, `summon`, `drive`, `lock`, …) is deliberately absent. A test posts `commands.fsd(true)` (`"fsd:on"`) and asserts 400 + `{t:"error",msg:"Unknown command"}`. This bridge has no TX backend at all.

## TDD evidence

### RED — tests written before either implementation file existed

Created `pi/test/bridge.test.ts` first. No `pi/src/http/bridge.ts`, no `bridge` option on `PiServerOptions`.

Command:

```text
npm test -w @teslacanmodder/pi -- test/bridge.test.ts
```

Output (exit 1):

```text
FAIL test/bridge.test.ts
  ● Test suite failed to run
    test/bridge.test.ts:6:54 - error TS2307: Cannot find module '../src/http/bridge.js' or its corresponding type declarations.
    test/bridge.test.ts:49:29 - error TS2353: Object literal may only specify known properties, and 'bridge' does not exist in type 'PiServerOptions'.
Test Suites: 1 failed, 1 total
Tests:       0 total
```

### GREEN — after implementing `bridge.ts` + mounting in `index.ts`

Same command, exit 0:

```text
PASS test/bridge.test.ts
  authenticated command bridge
    √ rejects POST /api/command without a bearer token (401, fail-closed) (48 ms)
    √ rejects a wrong bearer token with constant-time comparison (401)
    √ fails closed when no token is configured (401 for every authenticated route)
    √ rejects POST /api/command without Content-Type: application/json (415)
    √ rejects an oversized body over 1 MiB (413)
    √ returns status JSON from the injected A2 reader for the status command
    √ serves GET /status (and the /api/status alias) from the same reader
    √ answers ping with the serial-contract pong envelope
    √ refuses CAN-TX commands from the registry (unknown command, no injection)
    √ rejects malformed or missing cmd bodies (400)
    √ keeps the A1 /health route and returns 404 for unknown routes
Test Suites: 1 passed, 1 total
Tests:       11 passed, 11 total
```

## Verification

| Command                                                         | Result                                                                                                                                                                                          |
| --------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `npm test -w @teslacanmodder/pi -- test/bridge.test.ts` (RED)   | FAIL as expected (TS2307 + TS2353)                                                                                                                                                              |
| `npm test -w @teslacanmodder/pi -- test/bridge.test.ts` (GREEN) | 11/11 pass                                                                                                                                                                                      |
| `npm test -w @teslacanmodder/pi`                                | 26/26 pass (3 suites: bridge + smoke + socketcan), before and after commit                                                                                                                      |
| `npm run typecheck:pi`                                          | clean, no diagnostics (before and after commit)                                                                                                                                                 |
| `npm run lint:all`                                              | exit 0: ESLint clean (one `prefer-const` caught and fixed before commit); Prettier "All matched files use Prettier code style!"; markdownlint 0 issues in 44 files. Re-run after commit: clean. |
| `git diff --cached --name-only` (pre-commit)                    | exactly `pi/src/http/bridge.ts`, `pi/src/index.ts`, `pi/test/bridge.test.ts`                                                                                                                    |
| `git status --short -- pi` (post-commit)                        | empty — no collateral Pi changes                                                                                                                                                                |

`npm run test:all` was not run: prior A2 report records the pre-existing Windows hang at `test:firmware` (`pio.ps1` wrapper). No firmware files touched; full-suite status remains UNVERIFIED for the same environment reason. The brief's exact gates (`npm test -w @teslacanmodder/pi` + `npm run lint:all`) both pass.

## Files

| File                     | Change                                                                                                                                                                                                                                                                               |
| ------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `pi/src/http/bridge.ts`  | New: `MAX_BODY_BYTES`, `CanStatus`, `BridgeOptions`, `createBridgeListener()`. Constant-time bearer auth, 415/413 guards, serial-contract response envelopes, read-only registry allowlist, background A2 reader→snapshot pump, OPTIONS/health/status/command routing, 404 fallback. |
| `pi/test/bridge.test.ts` | New: 11 HTTP-level tests using genuine A2 frames built from literal SocketCAN bytes + an injected finite async reader. Covers 401 (missing/wrong/no configured token), 415, 413, status command + GET aliases, ping, TX-command refusal, malformed bodies, health/404.               |
| `pi/src/index.ts`        | Modified: `PiServerOptions.bridge?: RequestListener` mount point; routed after `/health` and before generic `fallback`; server idle/header timeouts set to 30 s / 5 s. A1 `/health` payload unchanged.                                                                               |

No other files changed. `package.json`, `jest.config.js`, `tsconfig.json`, root workspace, protocol package, and tools were already set up by A1/A2 and needed no edits.

## Interface handed to A4/B1

```ts
export const MAX_BODY_BYTES: number; // 1024 * 1024
export interface CanStatus {
    t: "status";
    frames: number;
    perBus: [number, number, number];
    lastFrame?: WireFrame; // A2 frameToWire shape
}
export interface BridgeOptions {
    token?: string; // empty => fail closed; TCM_BRIDGE_TOKEN env fallback
    reader: AsyncIterable<CanFrame>;
    timeoutMs?: number; // reserved
}
export function createBridgeListener(options: BridgeOptions): RequestListener;
```

Mount example:

```ts
const server = createPiServer({
    bridge: createBridgeListener({
        token: process.env.TCM_BRIDGE_TOKEN,
        reader: createCanReader("can0"),
    }),
});
server.listen(8090, "127.0.0.1");
```

- The listener binds no socket; loopback bind and port 8090 stay the caller's decision (B1 nginx proxies `/tcm` there).
- `createPiServer()` still handles `GET /health` first, so A1's exact payload is preserved; the bridge's standalone `/health` is only reachable if the listener is used directly.

## Self-review

- Test-first ordering verified: RED command ran when neither `bridge.ts` nor the `bridge` option existed.
- Staging used explicit paths only; staged list verified before commit; commit stat = exactly 3 files / 448 insertions / 2 deletions. ~200 pre-existing unstaged files untouched. Husky lint-staged reformatted only the 3 staged files; post-commit tests + lint re-verified green.
- Auth is checked before Content-Type and before body read — unauthenticated callers cannot probe body-parsing behavior. No token ⇒ 401 on every protected route (verified by test).
- `timingSafeEqual` guarded by a length check; no early string comparison leak beyond length.
- Body cap drains instead of truncating; 413 surfaced through the shared error path.
- Command allowlist is derived from protocol registry builders; TX refusal is covered by a test, not just code inspection.
- No `any`, no `console.*`, `eqeqeq` respected, strict TS clean, ESM/tabs/double quotes/trailing commas/LF per repo config.
- No circular import introduced: `index.ts` does not import `bridge.ts`; the caller composes them. A2's `socketcan.ts` still imports only `getCanTransport` from `index.ts`.
- No background watcher or long-running server left running; all test servers closed in `afterEach`.
- Pre-existing `lint-staged automatic backup` stash refs left intact; none dropped.

## Concerns / notes for controller

1. **No hardware claim.** No Pi, vcan, or MCP2515 capture available. The reader is injected as a finite fake built from literal `can_frame` bytes; no SocketCAN socket is opened in tests. Integration with a live bus remains B2 work.
2. **Status is a lightweight frame counter, not a firmware board snapshot.** The Pi has no board serial session; `status` summarizes frames seen by the A2 reader (`frames`, `perBus`, `lastFrame`). If the controller expects the full firmware `StatusMessage` field set, that needs a separate design decision — no existing Pi-side source produces it.
3. **Only `status` and `ping` are exposed.** Deliberate no-TX scope. If A4/B1 needs more read-only commands (`bms`, `tpms`, `platform`, …), extend the registry-derived allowlist and add handlers in `dispatchCommand` — do not generalize to all registry commands without a TX gate.
4. **Minors (deferred, non-blocking):** `SERVER_TIMEOUT_MS`/`HEADERS_TIMEOUT_MS` are exported from `bridge.ts` for reuse but `index.ts` currently inlines the same values (no cycle introduced); `BridgeOptions.timeoutMs` is reserved and unused; the bridge's standalone `/health` shape differs from the A1 server payload by design (A1 wins when mounted).
5. **`npm run test:all` not run** — see Verification. JS/TS gates prescribed by the brief are green; firmware full-suite status UNVERIFIED in this environment.
6. **Report file intentionally NOT committed** — `.opencode/sdd/**` is shared, untracked scratch used by other task briefs/reports.
