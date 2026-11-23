# Task A2 — SocketCAN reader with vcan tests

Where: second task of the unified Pi plan. Consumes the `pi/` workspace from A1.

## Requirement (verbatim from plan)

Files:

- Create: `pi/src/can/socketcan.ts`, `pi/test/socketcan.test.ts`
- Test support: `pi/test/support/fakeSocket.ts`

Interfaces:

- Consumes: `CAN_TRANSPORT` from A1.
- Produces: `createCanReader(iface: string): AsyncIterable<CanFrame>` where `CanFrame = {id:number; dlc:number; data:Uint8Array; bus:0|1|2; rxMs:number}`. Emits `{t:"frame"}`-shaped JSON for the bridge.

Steps:

1. Write vcan-backed tests for frame parse (11-bit ID, DLC 0–8, bus mapping can0→0/can1→1/can2→2) and reconnect-on-error. Assert literal frame bytes.
2. Run `npm test -w @teslacanmodder/pi` — expect FAIL.
3. Implement reader using Node `net`/`socketcan` with a fake transport injected in tests.
4. Run focused test + full `npm run test:all` — expect PASS.
5. Commit — `feat(pi): SocketCAN reader with vcan tests`.

## Global constraints

- No hardware here: use vcan/fake transport only. Every hardware claim UNVERIFIED.
- No CAN TX in this task — reader only.
- Stage ONLY files you change; ~200 unrelated unstaged files exist. Verify staged list before commit.
- Feature branch `spike/raspberry-pi-portability`; never main, never force-push.
