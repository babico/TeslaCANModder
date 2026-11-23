# Task A2 report — SocketCAN reader with vcan tests

Branch: `spike/raspberry-pi-portability`
Base: `01641d8` (A1 tip).
Commit: `9e60626 feat(pi): SocketCAN reader with vcan tests` — pushed to `origin/spike/raspberry-pi-portability` (`01641d8..9e60626`).
Status: DONE — TDD RED/GREEN confirmed, 3 files committed with explicit staging, pushed, self-review clean. Post-commit re-verification: `npm test -w @teslacanmodder/pi` 15/15, `npm run typecheck:pi` clean.

## What was built

Read-only SocketCAN frame reader for `can0`/`can1`/`can2`, with a pure 16-byte
`struct can_frame` parser, strict literal-byte tests, bus mapping, reconnect-on-error,
chunk reassembly, and a `{t:"frame"}` wire serializer matching the firmware serial contract.
No CAN TX, no Sentry code, no native modules, no hardware access.

## Capture format decision (no ambiguity remained)

- Linux `struct can_frame`: `uint32 can_id` (LE) + `uint8 can_dlc` + 3 pad + `uint8 data[8]` = **16 bytes**.
- Evidence: `.opencode/reports/rpi-spike/can-hardware.md:36` ("Frame shape is unchanged:
  `Frame {uint32_t id; uint8_t dlc; uint8_t data[8]}` maps 1:1 to SocketCAN `struct can_frame`")
  and `firmware/lib/core/types.h:32-37`.
- Bus mapping taken verbatim from brief: `can0→0`, `can1→1`, `can2→2`.
- 11-bit only: EFF/RTR/ERR flag bits in `can_id` are rejected; ID masked with `0x7FF`.

## TDD evidence

### RED — tests written before the module existed

Files created first: `pi/test/support/fakeSocket.ts`, `pi/test/socketcan.test.ts`.
No `pi/src/can/socketcan.ts` existed. Command:

```text
npm test -w @teslacanmodder/pi -- test/socketcan.test.ts
```

Output (exit 1):

```text
FAIL test/socketcan.test.ts
  ● Test suite failed to run
    test/socketcan.test.ts:2:38 - error TS2307: Cannot find module '../src/can/socketcan.js' or its corresponding type declarations.
    test/socketcan.test.ts:10:8 - error TS2307: Cannot find module '../src/can/socketcan.js' or its corresponding type declarations.
Test Suites: 1 failed, 1 total
Tests:       0 total
```

### GREEN — after implementing `pi/src/can/socketcan.ts`

Command:

```text
npm test -w @teslacanmodder/pi -- test/socketcan.test.ts
```

Output (exit 0):

```text
PASS test/socketcan.test.ts
  socketcan frame parsing
    √ uses the 16-byte SocketCAN can_frame layout (2 ms)
    √ parses a literal 11-bit can_frame into a CanFrame (1 ms)
    √ accepts DLC 0 through 8 and rejects DLC 9+ or a short buffer (6 ms)
    √ rejects extended, RTR, and error frames (11-bit IDs only) (1 ms)
    √ maps can0/can1/can2 to bus 0/1/2 (1 ms)
    √ rejects interfaces outside can0..can2
    √ reconnects after a transport error and keeps yielding frames (1 ms)
    √ reassembles a frame split across chunks and two frames in one chunk
    √ passes the A1 CAN_TRANSPORT backend to the transport factory
  socketcan wire serialization
    √ emits the firmware {t:frame} JSON shape
    √ serializes a zero-length frame with an empty hex field
Test Suites: 1 passed, 1 total
Tests:       11 passed, 11 total
```

## Verification

| Command                                                            | Result                                                                                                                                                                                                                                                                        |
| ------------------------------------------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `npm test -w @teslacanmodder/pi -- test/socketcan.test.ts` (RED)   | FAIL as expected (TS2307)                                                                                                                                                                                                                                                     |
| `npm test -w @teslacanmodder/pi -- test/socketcan.test.ts` (GREEN) | 11/11 pass                                                                                                                                                                                                                                                                    |
| `npm test -w @teslacanmodder/pi`                                   | 15/15 pass (2 suites: smoke + socketcan)                                                                                                                                                                                                                                      |
| `npm run typecheck:pi`                                             | clean, no diagnostics                                                                                                                                                                                                                                                         |
| `npm run test:all`                                                 | All JS/TS suites green: serial-contract OK, protocol 580, client 305, tools 164, pi 15 tests. Hung at final `test:firmware` step (`.\pio.ps1 test -e native`) in this Windows environment; stuck process tree stopped. Pre-existing wrapper issue, no firmware files touched. |
| `npm run lint:all`                                                 | exit 0: eslint clean; prettier clean; markdownlint 0 issues in 44 files                                                                                                                                                                                                       |
| `npx prettier --check` on the 3 new files                          | clean                                                                                                                                                                                                                                                                         |
| `npx eslint` on the 3 new files                                    | exit 0, no warnings                                                                                                                                                                                                                                                           |

## Files

| File                            | Change                                                                                                                                                                                                                                                                            |
| ------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `pi/src/can/socketcan.ts`       | New: `CanFrame`, `BusId`, `FrameTransport`, `TransportFactory`, `WireFrame`, `CanReaderOptions`; `CAN_FRAME_BYTES`, `busFromIface`, `parseCanFrame`, `frameToWire`, `frameToWireJson`, `createNetTransport`, `createCanReader`. Consumes A1 `getCanTransport()` / `CanTransport`. |
| `pi/test/socketcan.test.ts`     | New: 11 tests — literal 16-byte vectors, DLC 0..8, invalid length/DLC/flags, bus mapping, invalid iface, reconnect-on-error, chunk split/coalesce, backend propagation, wire JSON.                                                                                                |
| `pi/test/support/fakeSocket.ts` | New: `canFrameBytes()` encoder (literal `can_frame` bytes) + scripted `createFakeFactory()` transport injector.                                                                                                                                                                   |

No other files changed. `package.json`, `jest.config.js`, `tsconfig.json`, and the root workspace
were already set up by A1 and needed no edits.

## Interface handed to A3/A4

```ts
export type BusId = 0 | 1 | 2;
export interface CanFrame {
    id: number;
    dlc: number;
    data: Uint8Array;
    bus: BusId;
    rxMs: number;
}
export function createCanReader(iface: string, options?: CanReaderOptions): AsyncIterable<CanFrame>;
export function parseCanFrame(raw: Uint8Array, bus: BusId, rxMs: number): CanFrame;
export function frameToWire(frame: CanFrame): WireFrame;
export function frameToWireJson(frame: CanFrame): string;
```

- `{t:"frame"}` wire shape: `{ t:"frame", dir:"rx", bus, id, dlc, d }` with lowercase hex `d`
  (empty string for DLC 0). Matches `docs/reference/serial-contract.md:112-115`.
- `CanReaderOptions`: `transport`, `now`, `sleep`, `reconnectDelayMs`, `maxReconnects`, `backend`.
  Tests inject a fake transport, a fixed clock and a no-op sleep for deterministic literal assertions.
- Transport errors → reconnect with backoff. Clean stream end → reader terminates.
- Default production transport `createNetTransport` reads raw frames from a local TCP gateway
  (`CAN_GATEWAY_HOST`, `CAN_GATEWAY_PORT` base + bus index) using `node:net`; a future native
  `socketcan` binding can replace this seam without touching the parser.

## Self-review

- Test-first ordering verified: RED command ran before any `pi/src/can/` file existed.
- ESM (`"type": "module"`), tabs, double quotes, trailing commas, always-parenthesized arrows,
  LF line endings — all match repo Prettier config; targeted `prettier --check` clean.
- Strict TS compiles clean (`npm run typecheck:pi`); no `any`, no `console.*`, `eqeqeq` respected.
- No production code imports `node:net` at test time via side effects; fake transport is injected,
  so tests never open a socket and install no native/hardware modules.
- Wire serializer produces the exact firmware JSON shape (key order asserted as a literal string).
- Staging used explicit paths only; verified `git diff --cached --name-only` = exactly the 3 files
  before commit. Commit stat confirms 3 files / 465 insertions, no collateral changes.
- ~202 pre-existing unstaged entries remain untouched; `git status --short -- pi/` is clean after commit.
  Husky lint-staged rewrote the 3 staged files (prettier/eslint) and post-commit re-verification passed.
- Left pre-existing `lint-staged automatic backup` stash refs alone (several predate this task);
  dropped none, as they may hold other agents' work.

## Concerns / notes for controller

1. **Hardware claims UNVERIFIED.** No Pi, no vcan, no MCP2515 bench capture available. All tests
   use the fake transport; the `createNetTransport` gateway protocol is a documented seam, not a
   measured SocketCAN integration. A vcan integration suite remains a B2/B3-style hardware task.
2. **Clean EOF terminates the reader.** Only transport _errors_ reconnect. A long-lived gateway
   that closes cleanly would end the stream; add `reconnectOnEnd` later if the Pi service needs it.
3. **11-bit only.** EFF/RTR/error frames are rejected (per brief). 29-bit support would need a
   separate parser path.
4. **"vcan-backed tests" are simulated**, not kernel-vcan: the fake transport feeds literal
   `can_frame` bytes. This is deliberate — no native modules / hardware allowed in this environment.
5. **`npm run test:all` cannot finish in this environment.** Every JS/TS suite passed, but the final
   `test:firmware` step (`cd firmware && .\pio.ps1 test -e native`) hung indefinitely (PlatformIO
   wrapper / Unicode path `Masaüstü` / `subst` drive logic). This is pre-existing and unrelated to A2;
   no firmware files were touched. Firmware green status is **UNVERIFIED** here.
6. **Report file is intentionally NOT committed** (`.opencode/sdd/**` is shared, untracked scratch).
