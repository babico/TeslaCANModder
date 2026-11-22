# Unified Pi + Sentry-Six Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `subagent-driven-development` (recommended) or `executing-plans` to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax.

**Goal:** Put the whole TeslaCANModder tooling stack on a Raspberry Pi beside Sentry USB, with one nginx entry point and a measured safety gate before any CAN injection.

**Architecture:** New `pi/` Node workspace reads SocketCAN `can0/1/2` from the existing MCP2515 modules, exposes our JSON-RPC command bridge, records captures, and is served at `/tcm` behind nginx while Sentry USB stays the `/` service. CAN TX stays disabled until the PREEMPT_RT measurement gate passes and the can-review checklist is signed.

**Tech Stack:** TypeScript ESM (Node 22), `@teslacanmodder/protocol`, SocketCAN (kernel `mcp2515` + `vcan` for tests), nginx, systemd, Rust apps Sentry Studio/Drive used as-is.

**Spec:** `.opencode/plans/unified-pi-sentry-design.md` and `.opencode/reports/rpi-spike/ADR-2026-11-22.md` (+ three component reports in `.opencode/reports/rpi-spike/`).

## Global Constraints

- No Sentry Six source code copied into our repo. Sentry USB is PolyForm Noncommercial 1.0 — integration is HTTP/API/iframe only. Studio/Drive are MIT and used as separate apps.
- No CAN TX before: PREEMPT_RT + isolated core, `cyclictest <100 µs`, RX→TX p99.99 <2 ms, worst <5 ms under load, listen-only bench capture, and signed `docs/checklists/can-review-checklist.md`.
- MCP2515 modules are 5 V; Pi GPIO is 3.3 V → level shifter mandatory. 8/16 MHz crystal must match the device-tree overlay. 500 kbps.
- TJA1050 transceiver needs 4.75–5.25 V: either cut/jumper module Vdd to separate 5 V while MCP2515 stays 3.3 V, or use a 3.3 V transceiver module. Common ground + 120 Ω termination mandatory. References in `.opencode/plans/unified-pi-sentry-design.md`; guides are read-only, no code copied.
- Pi 4B or Pi 5 only (Pi 3B/3B+ cannot be the dashcam USB drive).
- Stage ONLY files each task changes; the repo has ~200 pre-existing unstaged files. Explicit `git add <paths>` only, never `-A`/`.`.
- Commit/push on a feature branch; never main directly, never force-push.
- No new files under `docs/superpowers/`; plans/analysis/reports stay in `.opencode/{plans,analysis,reports}/`.

## Review Focus

- Level-shift/CS/INT wiring mistake → 5 V into Pi GPIO or no CAN frames; pin table + overlay check in the wiring task.
- Bridge auth regression → `tcm-can` must fail closed like the tools bridge (401/415/body cap).
- Service collision with Sentry USB → two distinct ports/paths; no shared static assets or session.
- TX gate accidentally enabled by default → feature flag defaults off; TX path unreachable without the gate artifact.
- Tests that pass on `vcan` being mistaken for bench proof → every hardware claim marked UNVERIFIED until measured.

---

## Phase A — Service skeleton (no Pi hardware required)

### Task A1 — `pi/` workspace scaffold

**Files:**

- Create: `pi/package.json`, `pi/tsconfig.json`, `pi/src/index.ts`, `pi/test/smoke.test.ts`
- Modify: root `package.json` (workspace + scripts `test:pi`, `typecheck:pi`)

**Interfaces:**

- Produces: `@teslacanmodder/pi` workspace; `createPiServer()` signature used by A3; `CAN_TRANSPORT` env switch (`vcan|socketcan`).

- [ ] **Step 1: Write the failing smoke test** asserting the workspace resolves `@teslacanmodder/protocol` and `createPiServer()` exists.
- [ ] **Step 2: Run `npm test -w @teslacanmodder/pi`** — expect FAIL (module missing).
- [ ] **Step 3: Add workspace, scripts and minimal `createPiServer()`** returning an HTTP server with `/health`.
- [ ] **Step 4: Run test + `npm run typecheck:pi`** — expect PASS.
- [ ] **Step 5: Commit** — `feat(pi): add Pi service workspace scaffold`.

### Task A2 — SocketCAN reader with vcan tests

**Files:**

- Create: `pi/src/can/socketcan.ts`, `pi/test/socketcan.test.ts`
- Test support: `pi/test/support/fakeSocket.ts`

**Interfaces:**

- Consumes: `CAN_TRANSPORT` from A1.
- Produces: `createCanReader(iface: string): AsyncIterable<CanFrame>` where `CanFrame = {id:number; dlc:number; data:Uint8Array; bus:0|1|2; rxMs:number}`. Emits `{t:"frame"}`-shaped JSON for the bridge.

- [ ] **Step 1: Write vcan-backed tests** for frame parse (11-bit ID, DLC 0–8, bus mapping can0→0/can1→1/can2→2) and reconnect-on-error. Assert literal frame bytes.
- [ ] **Step 2: Run `npm test -w @teslacanmodder/pi`** — expect FAIL.
- [ ] **Step 3: Implement reader using Node `net`/`socketcan` with a fake transport injected in tests.**
- [ ] **Step 4: Run focused test + full `npm run test:all`** — expect PASS.
- [ ] **Step 5: Commit** — `feat(pi): SocketCAN reader with vcan tests`.

### Task A3 — Command bridge

**Files:**

- Create: `pi/src/http/bridge.ts`, `pi/test/bridge.test.ts`
- Modify: `pi/src/index.ts` (mount bridge)

**Interfaces:**

- Consumes: `createCanReader()` (A2), command registry from `@teslacanmodder/protocol`.
- Produces: `POST /api/command` with bearer token, 1 MiB body cap, 415 on non-JSON, `GET /health`, `GET /status`.

- [ ] **Step 1: Write failing tests** for 401 without token, 415 without Content-Type, 413 oversized body, and a `status` command returning JSON from a fake reader.
- [ ] **Step 2: Run focused tests — expect RED.**
- [ ] **Step 3: Implement bridge** following the hardened `tools/serial-http-bridge.js` pattern (constant-time token compare, default loopback bind, timeouts).
- [ ] **Step 4: Run `npm test -w @teslacanmodder/pi` + `npm run lint:all` — PASS.**
- [ ] **Step 5: Commit** — `feat(pi): authenticated command bridge`.

### Task A4 — Recorder and provenance

**Files:**

- Create: `pi/src/record/recorder.ts`, `pi/test/record.test.ts`

**Interfaces:**

- Consumes: `CanFrame` (A2).
- Produces: `createRecorder(dir)` → JSONL files with `{vehicle,bus,firmware,date}` provenance header + frames; `stop()` flushes and closes streams (no fire-and-forget).

- [ ] **Step 1: Test that a flushed file has a provenance header and N frames, and that a stream error rejects.**
- [ ] **Step 2: RED.**
- [ ] **Step 3: Implement with awaited drain + error listener.**
- [ ] **Step 4: GREEN + lint.**
- [ ] **Step 5: Commit** — `feat(pi): capture recorder with provenance`.

## Phase B — Deployment

### Task B1 — nginx portal + systemd units

**Files:**

- Create: `pi/deploy/nginx.conf`, `pi/deploy/tcm-can.service`, `pi/deploy/README.md`
- **Do not** modify Sentry USB files.

**Interfaces:**

- Consumes: A3 server on `127.0.0.1:8090`; Sentry USB on its own port.
- Produces: `/` → Sentry UI, `/tcm` → our client/static bundle; separate service units.

- [ ] **Step 1: Write a config test** (regex/structural check) asserting two distinct upstreams, no shared root, and that `/tcm` proxies WebSocket/HTTP correctly.
- [ ] **Step 2: RED.**
- [ ] **Step 3: Write nginx + systemd files + runbook.**
- [ ] **Step 4: GREEN; document `systemctl status` checks.**
- [ ] **Step 5: Commit** — `feat(pi): nginx portal and systemd units`.

### Task B2 — Wiring guide + listen-only bring-up

**Files:**

- Create: `docs/guides/pi-can-bringup.md` (canonical doc; generated bundle regenerated in its own commit)

**Steps:**

- [ ] Pin table: VCC 3.3 V to MCP2515, TJA1050 Vdd to 5 V via cut/jumper OR 3.3 V transceiver module; GND common; SCK GPIO11, MOSI GPIO10, MISO GPIO9, CS GPIO8 (CE0) / GPIO7 (CE1) / free GPIO for bus 3, INT GPIO25 (+ distinct INT for extra buses). Level shifters where any line is 5 V.
- [ ] Device-tree: `dtparam=spi=on`, `dtoverlay=mcp2515-can0,oscillator=<module crystal>,interrupt=25`, `dtoverlay=spi-bcm2835` (+ `spi-dma` on old kernels); second/third buses use distinct CS/INT. Verify `/sys/bus/spi/devices/spi0.0/net/can0` exists, then `ip link set can0 up type can bitrate 500000`.
- [ ] Acceptance: `ip -details link show can0` reports 500 kbps, `candump can0` sees expected X179 frames with 120 Ω termination; record hardware provenance (Pi model, module crystal, transceiver variant, wiring photo).
- [ ] Explicit "DO NOT ENABLE TX" section until Task B3 passes.
- [ ] Cite both guides in the doc (read-only, no code copied); note the forum thread was 403 and unused.
- [ ] Regenerate in-app docs bundle in a separate commit; commit guide first.

### Task B3 — RT measurement gate

**Files:**

- Create: `pi/src/rt/measure.ts`, `pi/test/rt.test.ts`, `pi/deploy/rt-gate.md`

**Interfaces:**

- Produces: a command that runs `cyclictest` and an RX→TX probe, writes a signed gate artifact `pi/gate/rt-result.json` with measured numbers; TX backend reads that artifact.

- [ ] **Step 1: Unit-test the gate decision function** against boundary values (99/100/101 µs, 1.9/2.0/2.1 ms, 4.9/5.0/5.1 ms) — pure, no hardware.
- [ ] **Step 2: RED / implement / GREEN.**
- [ ] **Step 3: Document the bench procedure** (isolated core, PREEMPT_RT, load generator) and the exact go/no-go thresholds.
- [ ] **Step 4: Commit** — `feat(pi): RT measurement gate (TX still disabled)`.

## Phase C — Gated TX (only after B3 artifact)

### Task C1 — Linux TX backend + can-review

**Files:**

- Create: `pi/src/can/tx.ts`, `pi/test/tx.test.ts`, `pi/gate/README.md`
- Modify: `pi/src/index.ts` (feature flag `PI_CAN_TX=0` default)

**Steps:**

- [ ] **Step 1: Test that TX is unreachable when gate artifact missing/invalid or flag off.**
- [ ] **Step 2: Implement backend** using the same frame builders/CRC rules from our firmware (independent test oracle), never copying Sentry code.
- [ ] **Step 3: Walk `docs/checklists/can-review-checklist.md` in the report** with one-line answers; any CAN mutation needs a listen-only capture reference.
- [ ] **Step 4: GREEN native/vcan tests + lint; commit.**

### Task C2 — Sentry integration smoke

**Files:**

- Create: `pi/src/sentry/connector.ts`, `pi/test/sentry.test.ts`
- Modify: `pi/deploy/README.md`

**Steps:**

- [ ] Read `drive-data.json` (Sentry USB) for clip↔drive correlation; no Sentry code copied, no writes to its storage.
- [ ] Test with a fixture; assert missing file degrades gracefully.
- [ ] Document launching Sentry Studio / Sentry Drive as separate apps; record that 2026.20 dashcam encryption must be off for Sentry USB.
- [ ] Commit.

## Phase D — Closure

### Task D1 — Full verification and ADR update

- [ ] Run `npm run test:all`, `npm run lint:all`, typechecks; save output under `.opencode/reports/`.
- [ ] Update `.opencode/reports/rpi-spike/ADR-2026-11-22.md` with measured results (or state that hardware verification is still UNVERIFIED).
- [ ] Mark capture-gated items openly; run final whole-branch review.

## Definition of done for a phase

1. Every task has a passing test or a measured report.
2. `npm run test:all` + `npm run lint:all` green on the merged tree.
3. CAN mutation tasks have the checklist walked and a capture reference; no TX without the gate artifact.
4. New findings go to `.opencode/analysis/findings-intake.md`, not silently into the current phase.
5. One PR per logical change; `main` only via PR.
