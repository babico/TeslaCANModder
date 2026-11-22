# Unified Pi + Sentry-Six design — 2026-11-22

## Intent

One Raspberry Pi box that manages Tesla dashcam footage (Sentry USB UI), exposes drive/FSD analytics (Sentry Studio / Sentry Drive), and runs our CAN command tooling — with one web entry point. The maintainer explicitly wants the existing MCP2515 CAN card driven from the Pi, and the two products as separate services behind a single portal.

## Decisions (from the maintainer)

1. **Pi drives the CAN card directly.** Reuse the existing 1–3 MCP2515 modules; ESP32 MCU is removed from the path. SPI + INT go to Pi GPIO; SocketCAN `can0`/`can1`/`can2` map to chassis/vehicle/body (X179 13-14 / 9-10 / 2-3).
2. **Two services, one entry point.** Sentry USB keeps its own service/UI; our tooling runs as a separate service. nginx is the single portal with two routes. No shared code, no shared session.
3. **Sentry USB is the base platform.** It already boots on Pi 4/5, provides dashcam USB mass storage, BLE telemetry, archive, and the UI the maintainer likes. We do not fork it.
4. **No Sentry code copied into our repo.** Sentry USB is PolyForm Noncommercial 1.0 (source-available, not open source); integration is at HTTP/API/iframe level only. Sentry Studio and Sentry Drive are MIT and may be used as separate apps.
5. **No injection until measured.** The just-finished RPi ADR stands as a safety gate: stock Linux user-space CAN TX is NO-GO for 100 Hz `0x370` echo and 50 Hz DAS injection. Requirements: PREEMPT_RT + isolated core, `cyclictest <100 µs`, RX→TX p99.99 <2 ms, worst <5 ms under load, plus `can-review-checklist` and a listen-only bench capture. Until then the CAN service is **listen-only** and the written safety gate stays ESP32-owned.

## Hardware facts to respect

- **Power is the trap.** The common MCP2515 module uses a TJA1050 transceiver that requires **4.75–5.25 V**, while Pi GPIO is 3.3 V. Two working approaches (both cited below): (a) keep MCP2515 VCC at 3.3 V and cut/jumper the module's TJA1050 Vdd trace to a separate 5 V feed (tolgakarakurt), or (b) use a module with a 3.3 V transceiver (e.g. SN65HVD230). Do NOT blindly follow guides that wire VCC to 3.3 V on a stock TJA1050 module — the transceiver may not transmit reliably. Level-shift SCK/MOSI/CS/INT/MISO if any side is 5 V.
- Overlay: `dtoverlay=mcp2515-can0,oscillator=<8000000|16000000>,interrupt=25` + `dtparam=spi=on` (+ `dtoverlay=spi-bcm2835`, `spi-dma` on older kernels). Crystal value MUST match the module. Bitrate 500000. 120 Ω termination and common ground between Pi and X179 bus are mandatory.
- SPI0: SCK GPIO11, MOSI GPIO10, MISO GPIO9, CE0 GPIO8 / CE1 GPIO7; INT on a free GPIO (e.g. GPIO25). One overlay per bus with distinct CS/INT.
- References (read-only, no code copied; neither repo has a clear license): https://38-3d.co.uk/blogs/blog/using-the-mcp2515-with-the-raspberry-pi and https://github.com/tolgakarakurt/CANBus-MCP2515-Raspi. The Raspberry Pi forum thread (viewtopic.php?t=141052) returned 403 and was NOT used.
- Pi 3B/3B+ cannot be the dashcam USB drive (USB hub chip) — Pi 4B or 5.
- Sentry USB warns: 2026.20 AMD cars with "Encrypt Dashcam Recordings" enabled cannot be read — encryption must be off.

## Architecture

```
Tesla X179 ── MCP2515 x1-3 ──(SPI0 + level shift)── Pi 4/5
                                      │
                    SocketCAN can0/can1/can2 (kernel mcp2515)
                                      │
        ┌─────────────────────────────┴────────────────────────────┐
        │  tcm-can service (new)      │  Sentry USB service        │
        │  - CAN RX/decode/stream     │  - dashcam USB + archive   │
        │  - JSON-RPC /api/command    │  - its React web UI        │
        │  - LISTEN-ONLY until gate   │  - BLE telemetry           │
        └──────────────┬──────────────┴──────────────┬─────────────┘
                       │                             │
                  nginx single portal: / -> Sentry UI, /tcm -> our client
                       │
        Sentry Studio / Sentry Drive launched as separate apps on demand
```

## Non-goals

- S3 / multi-ESP32 cluster decisions — out of scope of this request.
- Porting Sentry USB/Studio/Drive code into our repo.
- Enabling CAN injection before the measurement gate passes.
- Rewriting our Expo client from scratch now — it is embedded at `/tcm` behind nginx.

## What must be built

1. `pi/` Node service: SocketCAN reader, command bridge reusing `@teslacanmodder/protocol`, JSON-RPC over HTTP, status/health, recorder-to-JSONL writer.
2. Linux CAN backend for our header-only C++ driver (or Node SocketCAN equivalent) with native tests and a clear seam; no Arduino/ESP32 dependencies in the shipping Pi path.
3. nginx portal config + systemd units for both services; Sentry USB image used as the base.
4. Device-tree overlay wiring guide + safe listen-only bring-up procedure.
5. RT measurement harness and the written go/no-go gate before any TX path is enabled.

## Open items (need hardware or user decision)

- Exact Pi model / HAT choice and available MCP2515 crystals.
- Whether nginx can serve Sentry UI on `/` without breaking its hardcoded asset paths (needs lab check).
- Whether to read Sentry USB's `drive-data.json` for clip↔drive correlation in our `/tcm` panels (API-level, no code copy).
- Commercial licensing question if this product is ever sold (PolyForm Noncommercial).

## Effort (from the RPi spike, UNVERIFIED)

- Supervisor + two services + portal: 12–24 h.
- CAN Linux backend + native tests: 16–28 h (hybrid) — full port with BLE/RT: 62–140 h.
- RT/CAN lab spike + measurement gate: 42–84 h.
- Total for this design: roughly 70–136 h before any injection is allowed.
