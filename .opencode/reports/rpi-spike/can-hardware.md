# Raspberry Pi portability spike — CAN hardware / realtime

Date: 2026-11-22. Read-only spike: no firmware/client/protocol files were changed. Only this report was written.

Claim tags: **CODE-READ** = verified in this repo; **WEB-CITED** = external source with URL; **UNVERIFIED** = estimate / snapshot / no source located. All prices and availability are a **Nov-2026 snapshot, UNVERIFIED**. The maintainer's earlier Turkey/Nov-2026 SoC price snapshot was **not present anywhere in the repo** (searched `docs/**`, `.opencode/**`, root markdown); no local price artifact could be reused, so every number below is flagged UNVERIFIED and should be re-quoted from that snapshot or a live vendor page before use.

## 1. Hardware options for Tesla X179 CAN work

### 1.1 Native CAN — answer: NO Pi in scope has it

- No Raspberry Pi in this scope (Pi 4, Pi 5, Zero 2 W, CM4/CM5) has an onboard CAN controller. The Broadcom SoC and Pi 5's RP1 do not expose a CAN peripheral; CAN must be added as SPI HAT, USB adapter, or integrated telematics board. **WEB-CITED**: <https://www.autopi.io/blog/raspberry-pi-can-bus-explained/> ("The Raspberry Pi does not have built-in CAN support in the Broadcom SoC", "What it does not have by default is a native CAN controller. That means CAN support must be added through extra hardware.").
- RP2040 / Pico also has **no** native CAN; RP2040 CAN boards (e.g. Adafruit Feather RP2040 CAN Bus, generic Pico + MCP2515/MCP2518FD boards) bolt an external SPI CAN controller onto the MCU. **WEB-CITED** (generic RP2040 + MCP2515 @500 kbps): <https://github.com/calberoni/OTA_by_CAN_RP2040/blob/main/README.md>. Specific board silicon is **UNVERIFIED** without a datasheet in hand.
- Consequence: any Pi path still needs an external CAN controller. "Native CAN Pi" does not exist as a shortcut. **WEB-CITED** as above.

### 1.2 Controller / board comparison

| Option                        | CAN silicon                            | 500 kbps capable                                      | Channels practical                                    | Notes                                                                                                                                                                           | Tag                                                                              |
| ----------------------------- | -------------------------------------- | ----------------------------------------------------- | ----------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------- |
| Pi 4 (BCM2711)                | external MCP2515 / MCP2518FD HAT       | Yes                                                   | 3 with 3×CS + 3×IRQ on SPI0/SPI1                      | Mature PREEMPT_RT community docs; ~3–6 W; SD wear                                                                                                                               | WEB-CITED (HAT types) + UNVERIFIED (power)                                       |
| Pi 5 (BCM2712 + RP1)          | external SPI CAN HAT                   | Yes                                                   | 3 with 3×CS + 3×IRQ                                   | Most CPU headroom for core isolation; RP1 RT maturity must be measured; ~5–12 W, active cooler                                                                                  | WEB-CITED + UNVERIFIED                                                           |
| Pi Zero 2 W                   | external SPI CAN HAT                   | Yes                                                   | 2–3 tight (unpopulated 40-pin header needs soldering) | 512 MB / quad A53; marginal for Linux + 3 SocketCAN ifaces + REST; boot/wifi shared                                                                                             | UNVERIFIED (marginality is an estimate)                                          |
| CM4 (+ carrier)               | external SPI CAN HAT                   | Yes                                                   | 3                                                     | eMMC option removes SD-wear concern; carrier + cooling cost; no native CAN                                                                                                      | WEB-CITED (AutoPi CM4 telematics) + UNVERIFIED                                   |
| MCP2515 HAT/module            | MCP2515, 8/16/20 MHz crystal           | Yes — our own firmware runs 500 kbps with 8/16/20 MHz | 1 per CS                                              | Classic CAN 2.0B, small RX FIFOs (2 buffers) → drops if user-space stalls; many cheap modules are 5 V logic (needs level shifting to Pi 3.3 V); mainline Linux `mcp2515` driver | CODE-READ (`core/driver/common.h:14-65`) + UNVERIFIED (FIFO depth / logic level) |
| MCP2518FD HAT                 | MCP2518FD                              | Yes                                                   | 1–2 per HAT                                           | CAN FD + classic, deeper FIFOs, 3.3 V, mainline `mcp251xfd` driver; pricier; better for high-load capture                                                                       | WEB-CITED (AutoPi lists MCP2518FD/MCP2517FD for FD) + UNVERIFIED (FIFO depth)    |
| RP2040 board + MCP2515/2518FD | external                               | Yes                                                   | 1–2 per board, 3 with CS mux                          | Runs deterministic MCU firmware; reuse autowp driver pattern; talk to Pi via USB serial                                                                                         | WEB-CITED (RP2040+Ota-by-CAN) + UNVERIFIED                                       |
| USB CAN adapter               | vendor-specific (`slcan`, `gs_usb`, …) | Usually Yes                                           | 1 per dongle                                          | Easiest bring-up; USB + driver jitter makes it a poor TX-timing choice                                                                                                          | UNVERIFIED                                                                       |

- 500 kbps is not the hard part: our own driver sets `CAN_500KBPS` across 8/16/20 MHz crystals with fallback probing. **CODE-READ**: `firmware/lib/core/driver/common.h:20-65`. The hard part is worst-case latency and RX overflow, not bit rate. **CODE-READ + analysis**.

### 1.3 Pin / SPI / IRQ practical constraints

- Our ESP32 layout needs 3×CS + 3×INT + shared SPI (SCK/MISO/MOSI). **CODE-READ**: `core/driver/esp32/board.h:22-40` and `core/can/bus.h:12-29`. A Pi must provide the same: SPI0 (GPIO 10/9/11 + CE0/CE1 GPIO 8/7) plus extra GPIO for CS2 and three free IRQ GPIOs. Exact Pi pin mapping is **UNVERIFIED** without a board in hand.
- MCP2515 INT is active-low and needs a real GPIO IRQ line, not a polled pin; our ESP32 path uses `attachInterrupt(..., FALLING)` with one ISR per bus. **CODE-READ**: `core/driver/esp32/board.h:42-75`. On Linux the equivalent is a device-tree GPIO-irq property on the HAT overlay; a wrong overlay means silent frame loss. **UNVERIFIED** until tested.
- Shared SPI fabric: 3 controllers on one SPI bus serialize their register accesses; with a 100 Hz 0x370 stream plus DAS frames this is fine on bandwidth but creates IRQ bursts. Our ring buffer is 256 entries with per-consumer drop detection. **CODE-READ**: `core/can/ring_buffer.h:12-43,105-117`.
- Electrical: X179 is a Tesla-specific connector; a harness/breakout and correct 120 Ω termination are required. Cheap MCP2515 modules run their transceiver at 5 V and must not be wired straight to Pi 3.3 V logic. **UNVERIFIED** (harness/level-shifting specifics need bench check).
- Frame shape is unchanged: `Frame {uint32_t id; uint8_t dlc; uint8_t data[8]}` maps 1:1 to SocketCAN `struct can_frame`. **CODE-READ**: `core/types.h:30-37`; same layout in the test fake `fake_mcp2515.h:20-27`.

## 2. Linux CAN stack vs our header-only driver

### 2.1 What our code actually requires of a CAN backend

- Platform seam is the `driver*` API in `core/forward.h`: `driverSend`, `driverGetAndResetTxFails`, `driverGetAndResetBusOffEvents`, `driverPollBusErrors`, `applyFilters`, `saveSettings`. **CODE-READ**: `core/forward.h:41-80`.
- ESP32 backend implements: per-bus init/reinit, 8/16/20 MHz bitrate fallback, 6 hardware accept filters, one-shot TX, EFLG bus-off auto-recovery, per-bus ISR flags, and TX-fail/bus-off counters guarded by `portDISABLE_INTERRUPTS()`. **CODE-READ**: `core/driver/esp32/board.h` and `core/driver/common.h:73-260`.
- `core/can/**` (bus indices, health, software ID bitmask filter, 256-frame recorder, multi-consumer ring) is header-only and platform-agnostic apart from `millis()`. **CODE-READ**: `core/can/bus.h`, `health.h`, `id_filter.h`, `recorder.h`, `ring_buffer.h`.
- Native tests already stub the backend: `fake_mcp2515.h` fakes the controller + `SPIClass`; `stubs.h` records `driverSend` calls; `test_native_driver` injects pin/clock macros. **CODE-READ**: `test/support/fake_mcp2515.h`, `test/support/stubs.h:15-37`, `test/test_native_driver/test_driver.cpp:13-26`.

### 2.2 Mapping to SocketCAN

| Existing function            | Linux implementation                                                                                                                | Tag                                                 |
| ---------------------------- | ----------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------- |
| `driverInit()`               | bring up `can0..2` via netlink / `ip link set canX type can bitrate 500000`; `socket(PF_CAN, SOCK_RAW, CAN_RAW)` + `bind()` per bus | UNVERIFIED (design sketch)                          |
| `driverRead()`               | `poll()`/`recvfrom()` on RX sockets; convert `can_frame` → `Frame`                                                                  | UNVERIFIED                                          |
| `driverSend()`               | `sendto()` on per-bus socket; errno increments TX-fail counter                                                                      | UNVERIFIED                                          |
| `driverSetBusFilters()`      | `setsockopt(CAN_RAW_FILTER)` per socket, or keep `swFilterAccept()` in software (cheap 256-byte bitmask)                            | CODE-READ (sw filter exists) + UNVERIFIED (sockopt) |
| `driverPollBusErrors()`      | `CAN_RAW_ERR_FILTER` / netlink error frames; bus-off recovery via `ip link set down/up` or kernel `restart-ms`                      | UNVERIFIED                                          |
| `driverSetSingleShot()`      | `ip link set canX type can one-shot on` if driver supports it; verify for mcp2515/mcp251xfd                                         | UNVERIFIED                                          |
| `mcpClockMHz` / `canclock:*` | no userspace meaning on Linux; keep reported as fixed/detected or drop from status                                                  | CODE-READ (fields exist)                            |
| `millis()`                   | `clock_gettime(CLOCK_MONOTONIC, ...)` shim                                                                                          | UNVERIFIED                                          |

- Feature/handler code (nag, DAS, variant handlers, dispatch, status encoding) can stay unchanged: it only calls `driverSend` and consumes `Frame`. This is the main reason a Linux port is tractable. **CODE-READ** (`vehicle/can/handler/bus/vehicle.h:322-348`, `handler/ticks.h`).
- Arduino-isms (`attachInterrupt`, `SPI`, `portDISABLE_INTERRUPTS`) are confined to `core/driver/esp32/**`; `common.h` only uses the critical-section macro for counter atomicity, replaceable with `std::atomic`. **CODE-READ**.
- Native tests need no rewrite for feature logic. Add a `vcan`-based integration suite (`modprobe vcan`, `ip link add vcan0 type vcan`) for the SocketCAN backend plus unit tests for `can_frame`↔`Frame`, filter mapping, error-frame handling and drop accounting. **UNVERIFIED** (new work).
- Out of CAN scope but required for a real port: transports (serial JSON-RPC contract, WiFi REST, NimBLE) and NVS persistence need Linux equivalents (TCP/WS + file-based config). This likely dominates total effort. **CODE-READ** (contract in `docs/reference/commands.md`, NVS usage in `State`), **UNVERIFIED** (effort).

## 3. Realtime — jitter vs our deadlines

### 3.1 Our actual deadlines

- 0x370 nag echo: frame-triggered, one echo per incoming 0x370 frame; task brief says 100 Hz (10 ms cadence). **CODE-READ** for per-frame echo: `vehicle/can/handler/bus/vehicle.h:322-348`; natural mode uses 150–350 ms intervals, organic is frame-count gated (`core/types.h:505-535`). 100 Hz rate itself is **UNVERIFIED** in code (task-given).
- DAS drive: `DAS_CTRL_INTERVAL_MS 40` (25 Hz), `DAS_STEER_INTERVAL_MS 20` (50 Hz), `DAS_EAC_INTERVAL_MS 100` (10 Hz); steering angle rate limit 5°/20 ms, EPS faults at 12°. **CODE-READ**: `vehicle/can/feature/das/das_drive.h:18-62`.
- Burst/feature cadence: 20 ms for summon, wiper, air recirc, precondition 500 ms, ALC 2 s cooldown, turn signals 100 ms. **CODE-READ**: `handler/ticks.h:19-97`, `feature/comfort/air_recirc.h:41`, `feature/body/turn_signal.h:36`.
- Gate freshness: AP-gate signals must be refreshed within 1000 ms. **CODE-READ**: `core/types.h:162-168,848-872`.

So the binding deadline is **10–20 ms RX→TX** for 0x370 echo and DAS steering; ms-level for everything else.

### 3.2 Known Linux jitter evidence

- Standard Linux is not a real-time system; latency spikes in CAN traffic are possible and may be caused by _any_ subsystem that disables preemption, not just CAN. PREEMPT_RT improves worst case; average throughput is similar. **WEB-CITED**: <https://github.com/erstrom/rt-can-test> (README).
- Vendor figure: worst-case latency under load ~21.5 ms on vanilla kernel vs ~40 µs with PREEMPT_RT; typical RT wake-up 20–50 µs on modern hardware. Treat as order-of-magnitude, not Pi-specific. **WEB-CITED**: <https://www.linutronix.de/en/preempt-rt/> (marketing claim).
- 2026 tutorial: non-RT kernel under load shows hundreds of µs to several ms maximum latency; judge with `cyclictest --mlockall --smp --priority=80 --interval=200 --distance=0` under realistic load. **WEB-CITED**: <https://www.techveda.live/2026/06/21/preempt-rt-latency-cyclictest-part-1/>.
- Pi-specific measurement (~75 µs, 100–150 µs noise) exists in a Simulink real-time thesis but is old and context-limited. **WEB-CITED**: <https://support.dce.felk.cvut.cz/mediawiki/images/7/79/Dp_2017_prudek_martin.pdf>.
- SocketCAN is the normal Linux CAN API and makes CAN interfaces look like `can0` etc. **WEB-CITED**: <https://www.autopi.io/blog/raspberry-pi-can-bus-explained/>, <https://www.kernel.org/doc/html/latest/networking/can.html>.

### 3.3 Verdict and go/no-go rule

- Stock (non-RT) Linux user-space CAN TX: average is fine, **worst case is ms-scale under load** and therefore cannot be trusted for 10–20 ms injection deadlines. **WEB-CITED + analysis**.
- PREEMPT_RT + isolated core + `SCHED_FIFO` + `mlockall` brings worst case to tens of µs, comfortably inside our deadlines — **but only if measured on the exact board**. **WEB-CITED + analysis**.
- Kernel-module TX or keeping ESP32/RP2040 as the CAN endpoint gives the guarantee without depending on userspace scheduling. **Analysis**.

**Concrete go/no-go rule (UNVERIFIED until measured):**

- **NO-GO** for moving 0x370 echo or DAS injection to stock Linux user space, and **NO-GO** for any vehicle injection on a Pi until on-board numbers exist.
- **GO** for Pi as passive capture / REST-BLE host / status UI immediately (no safety TX).
- **GO** for moving injection to Pi only if, on the target board with PREEMPT_RT + one isolated core, all three hold for 1 hour under simultaneous 100% CPU load and saturated CAN bus: `cyclictest` max < 100 µs; end-to-end RX→TX p99.99 < 2 ms; end-to-end RX→TX worst case < 5 ms. Use `rt-can-test` with the CAN adapter looped back to a second channel as the measurement harness. These thresholds are a proposed rule, **UNVERIFIED** as a standard.
- Any change to frame mutation/routing/checksums still requires `docs/checklists/can-review-checklist.md` and bench capture before ship. **CODE-READ** (repo policy in `AGENTS.md` / `CONTRIBUTING.md`).

## 4. Power, boot, SD wear, cost vs keeping ESP32

| Dimension     | ESP32 node (current)     | Pi Zero 2 W                   | Pi 4                      | Pi 5                                          | CM4                   |
| ------------- | ------------------------ | ----------------------------- | ------------------------- | --------------------------------------------- | --------------------- |
| Native CAN    | No — external MCP2515 ×3 | No — HAT                      | No — HAT                  | No — HAT                                      | No — HAT              |
| Power         | ~0.2–0.5 W               | ~1–2 W                        | ~3–6 W                    | ~5–12 W + cooler                              | ~3–8 W                |
| Boot          | <1 s                     | 10–30 s                       | ~10 s                     | ~10–20 s                                      | ~10 s                 |
| Storage wear  | NVS in flash, no SD      | SD (log2ram/RO-rootfs needed) | SD (same)                 | SD (same)                                     | eMMC option avoids SD |
| Realtime      | deterministic ISR        | user-space jitter unless RT   | RT available, mature docs | RT + isolation headroom, RP1 maturity unknown | RT like Pi 4          |
| Relative cost | lowest                   | low                           | medium                    | high                                          | medium-high           |

All power/boot numbers are **UNVERIFIED** estimates. ESP32 wins on jitter, power, boot and cost; Pi wins on Linux tooling (SocketCAN, `candump`, `cansend`, Wireshark, web UI, RAM for logging). **WEB-CITED** (SocketCAN tooling): AutoPi / kernel URLs above. SD-wear mitigations (read-only rootfs + tmpfs/log2ram, high-endurance card, USB SSD, eMMC) are **UNVERIFIED** implementation advice.

## 5. Recommended first lab configuration

**Recommendation: Pi 5 (8 GB) + dual-channel MCP2518FD-based 3.3 V SPI CAN HAT, with the existing ESP32 kept on the bench as the TX/reference node. First exercise is listen-only capture + latency measurement; no injection until the go/no-go gate passes.** Rationale: Pi 5 has the core headroom for isolation, MCP2518FD has deeper FIFOs than MCP2515 for bursty Tesla traffic, and keeping the ESP32 gives an independent oracle. Pi 4 is the fallback if RP1 RT behaviour is poor. **Analysis + WEB-CITED** (HAT controller choices).

Exact parts (prices **UNVERIFIED** Nov-2026 snapshot; re-check against maintainer's TR snapshot or live vendor):

1. Raspberry Pi 5 8 GB — ~$80 (4 GB ~$50).
2. Official 27 W USB-C PSU (~$12) + active cooler (~$5–10).
3. Dual-channel isolated CAN FD HAT based on MCP2518FD (or MCP2515 if budget-bound), 3.3 V SPI, with GPIO irq lines — ~$40–90. Need ≥2 channels (chassis + vehicle); add a third MCP2515 module for body.
4. X179 connector → DB9/terminal breakout harness + 120 Ω termination jumpers — ~$20–50 (custom; **UNVERIFIED**).
5. High-endurance microSD 32–64 GB (~$10–15) or USB SSD + adapter (~$30).
6. Bench 12 V→5 V buck converter with ignition sense (~$10–20) for car-like power.
7. Optional independent capture: USB CAN adapter supported by SocketCAN (`gs_usb`/`slcan`) — ~$20–40.

**Estimated total: ~$200–350 (UNVERIFIED).** Turkey pricing from the referenced maintainer snapshot could not be located; if it gives local Pi/HAT figures, substitute those and scale the rest — do not treat these USD numbers as verified.

Bring-up order: (a) stock Raspberry Pi OS 64-bit + HAT overlay + `ip link set can0 type can bitrate 500000` + `candump` listen-only; (b) compare captured 0x3FD/0x370/0x2B9 frames against the ESP32 recorder; (c) install PREEMPT_RT, isolate a core, run `cyclictest` and `rt-can-test` under load; (d) only then consider extending the `driver*` backend (Section 2) and re-running the can-review checklist.

## 6. Effort estimate and top risks

Effort (**UNVERIFIED** engineering estimates, one engineer):

| Work item                                                                                     | Hours       |
| --------------------------------------------------------------------------------------------- | ----------- |
| HAT bring-up, device-tree overlay, 2–3 SocketCAN interfaces                                   | 6–12        |
| Linux `driver*` backend (RX/TX, filters, errors, bus-off, one-shot, `millis()` shim)          | 12–24       |
| PREEMPT_RT install + core isolation + `cyclictest`/`rt-can-test` measurement                  | 8–16        |
| Tests (vcan integration, `can_frame`↔`Frame`, error/drop accounting, native suite regression) | 8–16        |
| Bench comparison vs ESP32 + can-review checklist walk                                         | 8–16        |
| **CAN + RT spike subtotal**                                                                   | **~42–84**  |
| Adjacent host port (serial contract → TCP/WS, REST/BLE, file persistence)                     | 20–40       |
| **Full Pi port**                                                                              | **~62–124** |

Top risks:

1. **HIGH** — Stock-Linux / RP1 interrupt jitter breaks the 10–20 ms 0x370/DAS deadlines. Mitigation: measure before any TX; split architecture if gate fails. **WEB-CITED + analysis**.
2. **HIGH** — HAT electrical mismatches (5 V MCP2515 modules vs Pi 3.3 V, wrong X179 wiring/termination) cause bus errors or damage. **UNVERIFIED**.
3. **HIGH** — Safety: any injection change must pass `can-review-checklist.md` plus bench capture; a passing unit/snapshot test is not acceptance. **CODE-READ** (repo policy).
4. **MED-HIGH** — SocketCAN RX FIFO overflow / dropped frames when the daemon is descheduled; small-MCP2515 FIFOs. **UNVERIFIED**.
5. **MED** — Feature regressions from losing exact MCP2515 behaviours: 6 hardware filters, one-shot TX, EFLG bus-off auto-recovery, crystal autodetect. **CODE-READ** (they exist) + **UNVERIFIED** (mapping).
6. **MED** — Always-on car power: Pi boot time and standby draw vs ESP32; SD wear without RO-rootfs/tmpfs/eMMC. **UNVERIFIED**.
7. **MED** — Cost/availability of Pi + HAT in Turkey 2026; snapshot unverified. **UNVERIFIED**.
8. **MED** — Scope creep: transport/persistence rewrite, not CAN, dominates full-port effort. **UNVERIFIED**.

## Sources

- <https://www.autopi.io/blog/raspberry-pi-can-bus-explained/> — no native CAN on Pi; SPI HAT controllers MCP2515/MCP2518FD; SocketCAN basics.
- <https://www.kernel.org/doc/html/latest/networking/can.html> — SocketCAN architecture / API.
- <https://github.com/erstrom/rt-can-test> — standard Linux not real-time; latency spikes; PREEMPT_RT improves worst case; measurement method.
- <https://www.linutronix.de/en/preempt-rt/> — vanilla vs PREEMPT_RT worst-case figures (vendor claim).
- <https://www.techveda.live/2026/06/21/preempt-rt-latency-cyclictest-part-1/> — cyclictest methodology and latency ranges (2026).
- <https://support.dce.felk.cvut.cz/mediawiki/images/7/79/Dp_2017_prudek_martin.pdf> — Pi-specific latency observation (old, context-limited).
- <https://github.com/calberoni/OTA_by_CAN_RP2040/blob/main/README.md> — RP2040 + MCP2515 @500 kbps example.
- Repo files cited inline: `firmware/lib/core/can/**`, `firmware/lib/core/driver/**`, `firmware/lib/core/forward.h`, `firmware/lib/core/types.h`, `firmware/lib/vehicle/can/**`, `firmware/test/support/**`, `firmware/platformio.ini`, `docs/reference/commands.md`, `AGENTS.md`, `CONTRIBUTING.md`.
