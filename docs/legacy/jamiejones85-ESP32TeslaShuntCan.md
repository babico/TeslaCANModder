---
title: jamiejones85-ESP32TeslaShuntCan
description: An ESP32 project that reads current measurements from a Tesla P100D shunt sensor via SPI and broadcasts the readings ove
category: legacy
folder: legacy
tags: [legacy, community, external]
author: jamiejones85
repo: ESP32TeslaShuntCan
---

# jamiejones85-ESP32TeslaShuntCan

## 1. Provenance

- Submodule path: `legacy/jamiejones85-ESP32TeslaShuntCan`
- Pinned gitlink (canonical form): `160000 5e5a686704a3c78a894b96094f6fa9f814b53888 0`
- `git -C legacy/jamiejones85-ESP32TeslaShuntCan rev-parse HEAD` → `5e5a686704a3c78a894b96094f6fa9f814b53888` — matches the gitlink; worktree clean (`status --porcelain` empty).
- Last commit: `5e5a686` "Update README.md" (2021-10-14). Exactly 1 commit locally; the whole tree (6 files, 393 insertions) landed in that single commit.
- Upstream URL: `https://github.com/jamiejones85/ESP32TeslaShuntCan`
- License: MIT (`LICENSE`, (c) 2021 Jamie Jones, FULL-READ). `Shunt.h:1-25` embeds a second, older MIT notice (c) 2018 Tom deBree, crediting `https://github.com/Tom-evnut/Tesla-Current-Shunt` (`Shunt.h:2`).
- Analysis date: 2026-11-21.

Upstream files read in full (FULL-READ, every line — the entire tracked inventory):

| File                      | Lines read |
| ------------------------- | ---------- |
| `.gitignore`              | 1-32       |
| `ESP32TeslaShuntCan.ino`  | 1-44       |
| `LICENSE`                 | 1-21       |
| `README.md`               | 1-6        |
| `Shunt.cpp`               | 1-251      |
| `Shunt.h`                 | 1-39       |

- Files explicitly NOT read: none — `git ls-files` lists exactly these 6 paths; no hidden dot-directories exist in this repo.
- Replace-mode notes: YAML frontmatter above is preserved byte-for-byte from the replaced doc (its description line carries no trailing whitespace, so nothing was stripped; the line ends mid-word at "…readings ove" — an inherited truncation, preserved as-is and not silently fixed). The replaced doc contained no `## Upstream (...)` sections, so nothing else was preserved. Pre-replace body is retrievable as `git show HEAD:docs/legacy/jamiejones85-ESP32TeslaShuntCan.md` (52 lines) and is cited that way in section 6.

Our-side files read for parity (CODE-READ unless noted):

- `firmware/lib/vehicle/can/feature/telemetry/bms.h` lines 1-80 (voltage/current/SoC/temp decoders).
- `firmware/lib/vehicle/can/ids.h:37` (`CAN_ID_BMS_HV_BUS`).
- `firmware/lib/core/driver/esp32/board.h` lines 60-104 (`driverInit`, `initMcpBus`).
- `firmware/lib/vehicle/can/feature/misc/can_sim.h` lines 30-69 (`canSimTick`).
- `firmware/test/test_esp32_can_recorder/test_can_recorder.cpp:252-253` (SPI 10 MHz pinning, grep-anchored only).
- RANGE-CITED (18 lines each, schema-level claims only): `client/src/assets/can-decoder/legacy_modelsx_amd.json` lines 51131-51148 and `client/src/assets/can-decoder/legacy_modelsx_intel.json` lines 242168-242185. `client/src/assets/can-decoder/legacy_mcu2.json` was searched by grep only ("ShuntAsic" labels), not line-read.
- `docs/legacy/jamiejones85-ESP32TeslaShuntCan.md` (HEAD version, all 52 lines) — read after the code, for contradiction hunting only.

## 2. Executive verdict

A 393-line single-purpose Arduino sketch: it wakes a Tesla P100D pack shunt ASIC over SPI (through an isolation board), polls its current register every 10 ms, and broadcasts the raw 32-bit value as CAN ID 0x521 at 500 kbit/s. Three mechanisms exist: the opaque 8-transaction SPI init sequence, the `0x41` current read with big-endian reassembly, and the periodic CAN TX loop. Nothing here touches FSD, nag, or any Model 3/Y X179 surface — the project lives in the Model S/X salvage-pack world. Nothing should be ported as-is: the init bytes are undocumented magic with no datasheet, no capture, and no tests behind them, and the repo's own README misstates both the units (mA, no scale exists in code) and the endianness (little-endian, code writes big-endian). The only transferable ideas are per-transaction `SPISettings` discipline for sharing the SPI fabric with a non-MCP2515 device, and the concept of an independently measured pack current — both recorded in section 11 as ignore/until-salvage-pack-integration.

## 3. Evidence map

| Source file                           | Lines      | What it proves                                                       | Grade     |
| ------------------------------------- | ---------- | -------------------------------------------------------------------- | --------- |
| `ESP32TeslaShuntCan.ino`              | 9          | `Task ms10(10, -1, &ms10Task)` — 10 ms infinite scheduler task       | CODE-READ |
| `ESP32TeslaShuntCan.ino`              | 13-20      | `setup()` = shunt init + `CAN.begin(500E3)` + scheduler; return unchecked | CODE-READ |
| `ESP32TeslaShuntCan.ino`              | 22-38      | 10 ms task: read current, print, build and send one CAN frame       | CODE-READ |
| `ESP32TeslaShuntCan.ino`              | 28         | TX CAN ID is `0x521`                                                 | CODE-READ |
| `ESP32TeslaShuntCan.ino`              | 29-36      | Frame bytes: `00 00` + current 32-bit MSB-first + `00 00`            | CODE-READ |
| `ESP32TeslaShuntCan.ino`              | 24         | Comment remnant `//60000` above the current print                    | CODE-READ |
| `Shunt.cpp`                           | 3          | `int buff[18]` — shared global SPI buffer                            | CODE-READ |
| `Shunt.cpp`                           | 4          | Chip select = `SS`                                                   | CODE-READ |
| `Shunt.cpp`                           | 6-11       | `resetBuffer()` writes indices 1..18 → `buff[18]` is out of bounds   | CODE-READ |
| `Shunt.cpp`                           | 13-221     | 8-transaction SPI init sequence, exact bytes cataloged in section 5  | CODE-READ |
| `Shunt.cpp`                           | 17 et al.  | `SPISettings(500000, MSBFIRST, SPI_MODE0)` on all 9 transactions      | CODE-READ |
| `Shunt.cpp`                           | 223-251    | `readCurrent()`: `0x41` + 6 dummies + `0x9A`; BE assembly of buff[3..6] | CODE-READ |
| `Shunt.cpp`                           | 248        | No unit scale anywhere — raw count returned                          | CODE-READ |
| `Shunt.h`                             | 1-25       | Tom deBree (2018) MIT credit; true lineage is Tom-evnut/Tesla-Current-Shunt | CODE-READ |
| `README.md`                           | 4          | Requires isolated SPI link (easyeda isolator board)                  | CODE-READ |
| `README.md`                           | 6          | Claims mA units and "Litten Endian" bytes 2,3,4,5                    | CODE-READ |
| `LICENSE`                             | 1-21       | MIT, (c) 2021 Jamie Jones                                            | CODE-READ |

No TEST-READ evidence exists: this repo ships no tests. No CAPTURE evidence exists in this repo: there is no logged frame, no decode tool, and no sample output.

## 4. Architecture and build surface

- Entry points: `setup()` and `loop()` (`ESP32TeslaShuntCan.ino:12,42`). All work happens in `ms10Task()` (`ESP32TeslaShuntCan.ino:22`), driven by `TaskScheduler` (`Task ms10`, `.ino:9`).
- Shared core vs platform port: none — this is a single-target ESP32 Arduino sketch with a two-file `Shunt` class; there is no portability layer, no `platformio.ini`, no build flags, no CI.
- Dependencies: Arduino core, `CAN.h` (library identity unpinned — see section 17), `TaskScheduler.h`, `SPI.h`.
- Dependency boundaries: `Shunt.cpp` owns SPI; `.ino` owns CAN and scheduling; the only interface is `initialiseShunt()` / `readCurrent()` declared in `Shunt.h:31-38`.
- Hardware boundary: the shunt ASIC sits on the HV side of an SPI isolator board (`README.md:4`); the ESP32 side sees a plain 500 kHz MODE0 SPI device with chip select on `SS` (`Shunt.cpp:4,17`).
- RX path: none. The sketch never registers a CAN receive callback and never reads the bus; the pipeline below is TX-only.

```mermaid
flowchart LR
    ShuntAsic["Tesla P100D shunt ASIC<br/>(HV side, isolated)"] -->|"SPI 500 kHz MODE0<br/>CS = SS"| Read["readCurrent()<br/>Shunt.cpp:223"]
    Read -->|"raw int32, BE assembly<br/>buff[3..6]"| Task["ms10Task @ 10 ms<br/>.ino:22"]
    Task -->|"Serial print + frame build"| Frame["CAN frame 0x521<br/>DLC 8, bytes 2-5 current"]
    Frame -->|"TX only, 500 kbit/s<br/>no RX, no filter"| Bus["CAN bus"]
    classDef path fill:#1e3a5f,stroke:#4a7fb5,color:#fff
    class Read,Task,Frame path
```

## 5. Mechanism catalogue

### SPI shunt initialization sequence

- **Claim (README / summary):** README is silent about init. `Shunt.h:1-2` credits Tom deBree's Tesla-Current-Shunt.
- **Code path:** `ESP32TeslaShuntCan.ino:13` → `Shunt::initialiseShunt()` (`Shunt.cpp:13`) → 8 sequential CS-low transactions, each inside its own `SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0))`.
- **Frame spec:** exact bytes sent per transaction (SPI, not CAN):

| # | Bytes sent                              | Cited lines   |
| - | --------------------------------------- | ------------- |
| 1 | `7F 7F 21 DE 00 00 00 80 00`            | Shunt.cpp:21-31 |
| 2 | `4F 00 00 00 00 00 00 BC`               | Shunt.cpp:50-57 |
| 3 | `4E 00 00 00 00 00 00 63`               | Shunt.cpp:75-82 |
| 4 | `4E 00 00 00 00 00 00 63` (duplicate of 3) | Shunt.cpp:101-108 |
| 5 | `16 00 00 02 00 00 00 33`               | Shunt.cpp:127-134 |
| 6 | `14 00 00 DC 05 C2 01 83`               | Shunt.cpp:152-159 |
| 7 | `13 00 00 04 01 02 08 94`               | Shunt.cpp:177-184 |
| 8 | `17 00 00 00 00 00 00 C0`               | Shunt.cpp:203-210 |

- **Gates and timing:** none — no response validation, no retry, no timeout; MISO replies are captured into `buff[]` and printed to Serial but never checked.
- **State machine:** None found (linear one-shot sequence at boot; no states, no re-arm).
- **Tests:** None found.
- **Evidence grade:** CODE-READ (bytes exact); semantics of every command byte UNVERIFIED — no datasheet, no capture, no upstream reference repo in our legacy tree.
- **Verdict:** CONFIRMED as bytes / UNVERIFIED as protocol meaning.
- **Adapt to TeslaCANModder?** never port as-is (opaque magic; nothing to independently validate).

### SPI current read

- **Claim (README / summary):** README.md:6 says "Milliamps" — no scale exists in code.
- **Code path:** `ms10Task()` (`ESP32TeslaShuntCan.ino:23`) → `Shunt::readCurrent()` (`Shunt.cpp:223`) → sends `0x41`, clocks 6 zero dummies into `buff[1..6]`, one more transfer `0x9A` into `buff[8]`, CS high, returns the assembled value.
- **Frame spec:** response byte 0 lands in `buff[1]`; the returned value is `buff[6]<<24 \| buff[5]<<16 \| buff[4]<<8 \| buff[3]` (`Shunt.cpp:248`) — a 32-bit big-endian assembly of response bytes 2..5. `buff[7]` is never written (index gap, `.cpp:229-233`). Negative values survive only via signed-overflow wraparound of the `int` arithmetic. No counter, no checksum validation of the reply; the `0x9A` trailing byte's purpose (PEC? keep-alive?) is UNVERIFIED.
- **Gates and timing:** called every 10 ms; no freshness or validity gate on the returned data.
- **State machine:** None found (stateless single-shot read).
- **Tests:** None found.
- **Evidence grade:** CODE-READ (mechanics); units (mA vs raw count) UNVERIFIED.
- **Verdict:** CONFIRMED as mechanics / units UNVERIFIED.
- **Adapt to TeslaCANModder?** no impact (independent shunt sensing is irrelevant to our X179 surface unless salvage-pack integration ever happens).

### Periodic CAN broadcast (0x521)

- **Claim (README / summary):** README.md:6 and the replaced doc both say little-endian; see section 6.
- **Code path:** `ms10Task()` → `CAN.beginPacket(0x521)` (`ESP32TeslaShuntCan.ino:28`) → 8 × `CAN.write()` → `CAN.endPacket()` (`.ino:37`) → physical TX at 500 kbit/s.
- **Frame spec:** ID `0x521` (1313), DLC 8, period 10 ms (100 Hz). Bytes: `data[0]=0x00`, `data[1]=0x00`, `data[2..5]` = current as 32-bit **big-endian** (`data[2]` = MSB, written first at `.ino:31`), `data[6]=0x00`, `data[7]=0x00`. No counter byte, no checksum byte — unlike real Tesla frames, nothing protects or sequences this frame.
- **Gates and timing:** unconditional broadcast every 10 ms; `CAN.begin(500E3)` and `CAN.endPacket()` return values are ignored (`.ino:14,37`); no bus-off handling, no filters, no RX.
- **State machine:** None found (single periodic state; no enable/disable, no re-arm).
- **Tests:** None found.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED.
- **Adapt to TeslaCANModder?** already present at the pattern level — our firmware already does periodic work via millis-delta ticks (e.g. `canSimTick`, `firmware/lib/vehicle/can/feature/misc/can_sim.h:33-40`); the 0x521 frame itself has no impact on our platform.

## 6. Summary vs code contradiction table

Pre-replace doc cited as `git show HEAD:docs/legacy/jamiejones85-ESP32TeslaShuntCan.md#<line>`; upstream README cited as live file.

| Claim                                                                 | Code reality                                                                                                       | Verdict      |
| --------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------ | ------------ |
| `#35` "Reads raw ADC values and converts to milliamps" + `README.md:6` "Milliamps is put out" | No scale factor anywhere: `Shunt.cpp:248` returns the raw 32-bit count; nothing multiplies or divides it          | CONTRADICTED |
| `#41` "little-endian 32-bit signed integer in bytes 2–5" + `README.md:6` "Litten Endian" | `ESP32TeslaShuntCan.ino:31-34` writes MSB first (`0xff000000 >> 24` into byte 2) — big-endian on the wire        | CONTRADICTED |
| `#37` "CAN output added by joromy"                                     | "joromy" appears nowhere in the pinned tree (`grep -nE 'joromy'` → no output); `Shunt.h:2` credits only Tom deBree | UNVERIFIED (network-only attribution, cannot be checked locally) |

Not contradictions, for the record: `#22` "License: MIT" (matches `LICENSE`), `#21` CAN.h (matches `.ino:2`), `#34-36` 10 ms task / 500 kbit/s / SPI 500 kHz MODE0 (all confirmed), `#46` "does not interact with the vehicle's autopilot or FSD CAN messages" (confirmed — 0x521 is the only CAN activity). The `#42` "custom CAN message (not a standard Tesla CAN ID)" claim is neither confirmed nor contradicted by code and is tracked in section 17.

## 7. CAN frame and signal inventory

| ID (hex/dec)  | Bus              | Period | DLC | Byte/bit layout                                    | Scale/units              | Counter | Checksum | R/W | Grade     |
| ------------- | ---------------- | ------ | --- | -------------------------------------------------- | ------------------------ | ------- | -------- | --- | --------- |
| 0x521 (1313)  | single TWAI bus   | 10 ms  | 8   | b0=0x00, b1=0x00, b2..b5 = current int32 big-endian, b6=0x00, b7=0x00 | raw shunt count (mA claim unproven) | none    | none     | TX only (broadcast) | CODE-READ |

That single TX frame is the repo's entire CAN surface: no frame is ever received, decoded, or mutated.

## 8. Constants and tuning table

| Constant                | Value                          | File:line                              | Meaning                                                    | Grade                              |
| ----------------------- | ------------------------------ | -------------------------------------- | ---------------------------------------------------------- | ---------------------------------- |
| CAN TX ID               | 0x521                          | ESP32TeslaShuntCan.ino:28              | broadcast current frame                                    | CODE-READ                          |
| CAN bitrate             | 500E3                          | ESP32TeslaShuntCan.ino:14              | 500 kbit/s                                                 | CODE-READ                          |
| Task period             | 10 (ms), iterations -1         | ESP32TeslaShuntCan.ino:9              | 100 Hz poll+broadcast forever                              | CODE-READ                          |
| SPI clock               | 500000                         | Shunt.cpp:17,46,71,97,123,148,173,199,224 | 500 kHz on all 9 transactions                          | CODE-READ                          |
| SPI mode/order          | MSBFIRST, SPI_MODE0            | Shunt.cpp:17 (and 8 more)              | SPI phase/polarity                                          | CODE-READ                          |
| Chip select             | SS                             | Shunt.cpp:4                            | ESP32 default VSPI SS pin                                  | CODE-READ                          |
| Read command            | 0x41                           | Shunt.cpp:228                          | read-current SPI command (semantic inferred)              | CODE-READ (meaning UNVERIFIED)     |
| Read trailer            | 0x9A                           | Shunt.cpp:233                          | dummy byte clocking byte 7 of reply (PEC? keep-alive?)     | CODE-READ (meaning UNVERIFIED)    |
| Init transaction 1      | 7F 7F 21 DE 00 00 00 80 00     | Shunt.cpp:21-31                         | unlock/config write (semantic UNVERIFIED)                  | CODE-READ (meaning UNVERIFIED)    |
| Init transactions 2-4   | 4F..BC; 4E..63; 4E..63 (dup)    | Shunt.cpp:50-57,75-82,101-108          | config writes; 0x4E block sent twice                       | CODE-READ (meaning UNVERIFIED)    |
| Init transaction 5      | 16 00 00 02 00 00 00 33        | Shunt.cpp:127-134                       | config write (semantic UNVERIFIED)                         | CODE-READ (meaning UNVERIFIED)    |
| Init transaction 6      | 14 00 00 DC 05 C2 01 83        | Shunt.cpp:152-159                       | config write (semantic UNVERIFIED)                         | CODE-READ (meaning UNVERIFIED)    |
| Init transaction 7      | 13 00 00 04 01 02 08 94        | Shunt.cpp:177-184                       | config write (semantic UNVERIFIED)                         | CODE-READ (meaning UNVERIFIED)    |
| Init transaction 8      | 17 00 00 00 00 00 00 C0        | Shunt.cpp:203-210                       | config write (semantic UNVERIFIED)                         | CODE-READ (meaning UNVERIFIED)    |
| Trailer bytes           | 0xBC, 0x63, 0x33, 0x83, 0x94, 0xC0 | Shunt.cpp:57,82,134,159,184,210     | fixed last byte per transaction (PEC/CRC? command echo?)   | CODE-READ (meaning UNVERIFIED)    |
| Buffer size             | 18 (`int buff[18]`)            | Shunt.cpp:3                            | global SPI staging buffer                                  | CODE-READ                          |
| Reset loop bound        | 1..19 → writes buff[18]        | Shunt.cpp:7-9                          | one-element out-of-bounds write each reset                 | CODE-READ (defect)                 |
| Assembly radix          | 256 shifts                     | Shunt.cpp:248                          | big-endian reassembly of buff[3..6]                       | CODE-READ                          |
| Comment remnant         | `//60000`                       | ESP32TeslaShuntCan.ino:24              | unexplained; possible mA full-scale hint from an earlier revision | CODE-READ (meaning UNVERIFIED) |

Trivial constants skipped per skill: formatting widths, `I` loop-variable style.

## 9. Failure modes and edge cases

| Failure                                              | Handling code path                                                    | Tested? | Grade     |
| ---------------------------------------------------- | --------------------------------------------------------------------- | ------- | --------- |
| `CAN.begin(500E3)` fails (no transceiver, bus short)  | none — return value ignored at `ESP32TeslaShuntCan.ino:14`           | no      | CODE-READ |
| `CAN.endPacket()` TX failure (bus-off, ACK loss)      | none — return value ignored at `ESP32TeslaShuntCan.ino:37`            | no      | CODE-READ |
| Out-of-bounds write `buff[18]`                       | none — `resetBuffer()` bound bug at `Shunt.cpp:7-9` silently clobbers adjacent global memory on every call | no | CODE-READ |
| Shunt absent / isolator dead (MISO floats)           | none — `readCurrent()` returns stale `buff[3..6]` from the previous call; `resetBuffer()` is not invoked between reads (`Shunt.cpp:223-251`) | no | CODE-READ |
| Watchdog, brownout, reboot state, OTA                 | None found — no watchdog feed, no persistence, no OTA guard exists     | no      | CODE-READ (absence) |
| Queue/scheduler overflow                              | None found — single 10 ms task, `TaskScheduler` defaults              | no      | CODE-READ (absence) |
| Serial used before/without `Serial.begin()`           | none — `Serial.print*` at `ESP32TeslaShuntCan.ino:25-26,39` with no `Serial.begin()` anywhere in the repo; impact depends on ESP32 core behavior | no | CODE-READ (absence), impact UNVERIFIED |
| Reentrancy on shared `buff[]`                        | none — global buffer shared by init and read, no guard (`Shunt.cpp:3`) | no      | CODE-READ |
| `buff[7]` index gap in read                           | cosmetic — value never consumed (`Shunt.cpp:229-233`)                 | no      | CODE-READ |

No error branch in this repo is tested; the repo has no tests at all.

## 10. Test coverage map

None found — `git ls-files` returns 6 files and none is a test; no CI config exists.

Critical paths with no test, and the risk in one line each:

- SPI init byte sequence — risk: any transcription error bricks or misconfigures an HV-side ASIC with no feedback.
- Current reassembly endianness (`Shunt.cpp:248`) — risk: a byte swap produces plausible-but-wrong magnitudes that nothing would catch.
- `0x521` frame layout — risk: receivers see a different byte order/units than intended; the README/code split shows this already happened.
- `resetBuffer()` bounds — risk: silent RAM corruption adjacent to `buff[]`, undefined victim.

## 11. Parity matrix with TeslaCANModder

| Signal/frame                | Upstream decode/inject (cite)                                                        | Our decode/inject (cite)                                                                                       | Delta                                                                 | Action                                          |
| --------------------------- | ------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------- | ----------------------------------------------- |
| HV pack current             | measured at shunt: `readCurrent()` `Shunt.cpp:223-251`, raw int32, no scale, never on our bus | decoded from CAN: `decodeBmsCurrent()` `firmware/lib/vehicle/can/feature/telemetry/bms.h:29-33` (0x132 bytes 2-3, signed BE, ×0.1 A) | independent sensor vs BMS-reported value; S/X salvage-pack vs our 3/Y X179 platform | ignore                                           |
| CAN ID 0x521                | TX broadcast `ESP32TeslaShuntCan.ino:28-37`                                            | absent: `grep -rnE '0x521' firmware packages tools` → no output; vendored MSX catalog carries unrelated `BCFALCD_*` frames at 0x521 on its ETH bus (`client/src/assets/can-decoder/legacy_modelsx_amd.json:51138`, RANGE-CITED) | no overlap with any frame we decode, mutate, or inject                | ignore                                           |
| Synthetic current source    | real measurement broadcast on the physical bus at 100 Hz (`ESP32TeslaShuntCan.ino:9,28`) | `canSimTick()` `firmware/lib/vehicle/can/feature/misc/can_sim.h:33-53` — synthetic 0x132 (~375 V / ~5 A) into the decode pipeline only, 200 ms interval (`can_sim.h:38`), never on the physical bus | bench-decode testing vs physical broadcast; opposite purposes         | no impact                                       |
| SPI peripheral discipline   | per-transaction `SPISettings(500000, MSBFIRST, SPI_MODE0)` on every access (`Shunt.cpp:17,224`) | shared SPI fabric for up to 3 MCP2515s: `SPI.begin(...)` `firmware/lib/core/driver/esp32/board.h:84`, per-device settings owned by the MCP2515 driver (tests pin 10 MHz, `firmware/test/test_esp32_can_recorder/test_can_recorder.cpp:252-253`) | if a non-MCP2515 SPI sensor ever joined our fabric it must bring its own beginTransaction settings — upstream is a working example of that discipline | ignore (revisit only if salvage-pack HW integration happens) |

No `already present` row above rests on our-side memory: each row cites a read line in our tree.

## 12. Surface inventory and security notes

| Surface                    | Semantics                                                                                              | Grade     |
| -------------------------- | ------------------------------------------------------------------------------------------------------ | --------- |
| USB Serial (print only)    | every 10 ms prints `" "` + current value (`.ino:25-26`); init dumps raw SPI replies as hex (`.cpp:39,64,89,115,141,166,191,217`, one per init transaction) | CODE-READ |
| CAN TX 0x521               | unidirectional broadcast, no command input of any kind                                                 | CODE-READ |

- Command/transport surfaces (serial commands, web, BLE, MQTT): None found — the device accepts no input.
- Hardcoded credentials / keys / certs / tokens: None found.
- Untrusted archives / dangerous build flags: None found (no build system beyond the Arduino sketch).
- HV safety note: the design deliberately puts an isolation board between ESP32 logic ground and the HV-side shunt (`README.md:4`). Any hardware reuse must keep that isolation; this is a wiring requirement, not a code one.
- Pin conflict note: chip select is `SS` (`Shunt.cpp:4`); combining this sketch's hardware with our MCP2515 SPI fabric would need a distinct CS pin and per-transaction settings (UNVERIFIED for our board pin map).

## 13. Adaptation assessment

| Mechanism                    | Value                                        | Cost                    | Risk                                                       | Evidence                        | Decision                                     |
| ---------------------------- | -------------------------------------------- | ----------------------- | ---------------------------------------------------------- | ------------------------------- | -------------------------------------------- |
| SPI init sequence            | none for our platform (S/X salvage-pack HW) | isolator board + wiring | unvalidated writes to an HV-side ASIC; no datasheet/capture | CODE-READ, meaning UNVERIFIED   | never port as-is                             |
| Current read + BE assembly   | low (independent current sense)              | moderate                 | units and endianness both unproven; wrong bytes look valid | CODE-READ, units UNVERIFIED     | no impact (revisit only with a bench capture) |
| 100 Hz broadcast             | pattern-level only                           | trivial                 | none to us — different frame, different bus                | CODE-READ                       | already present (millis-delta tick pattern, `can_sim.h:33-40`) |
| Per-transaction SPISettings  | low-moderate (future SPI sensor additions)   | trivial                  | none                                                        | CODE-READ                       | no impact now; candidate only if non-MCP2515 SPI hardware is ever attached |

Every decision above rests on CODE-READ or our-side CODE-READ; no SUMMARY-ONLY or UNVERIFIED claim drives a port.

## 14. Do-not-copy list

- `Shunt.cpp:21-31,50-57,75-82,101-108,127-134,152-159,177-184,203-210` — opaque register writes with zero semantic documentation; copying propagates unvalidated magic into shipping firmware.
- `Shunt.cpp:248` — endianness reassembly whose own README contradicts it; nothing derived from this line without an independent capture.
- `Shunt.cpp:6-11` — one-past-the-end loop bound (`buff[18]` write); the bug ships in `resetBuffer()`.
- `ESP32TeslaShuntCan.ino:14,37` — unchecked `CAN.begin`/`endPacket` with no error path; not a pattern to repeat.
- `Shunt.cpp:3` — global shared SPI staging buffer with no reentrancy guard.

Any future behavior port (byte layout, timing) must be an independent implementation with an independent test oracle and a listen-only bench capture, per `docs/checklists/can-review-checklist.md`; a passing snapshot of our own output proves nothing.

## 15. History and evolution

- The local history is a single commit: `5e5a686` "Update README.md" (2021-10-14) adding all 6 files at once (`git log --stat`: 6 files, 393 insertions). Everything earlier was squashed or imported from outside git — no evolution, no rollbacks, no abandoned approaches are visible locally. Whether real pre-import history exists on GitHub is UNVERIFIED (network-only claim).
- The commit message says "Update README.md" while actually creating the whole tree — further evidence the pinned history is not the development history.
- True lineage: `Shunt.h:1-25` embeds Tom deBree's 2018 MIT notice and `Shunt.h:2` names `https://github.com/Tom-evnut/Tesla-Current-Shunt`. That upstream-of-the-upstream repo is **not** present in our `legacy/` tree, so the origin of the init bytes and the mA claim cannot be traced locally (UNVERIFIED).
- No changelog, no tags, no branches beyond the default exist in the submodule.

## 16. Re-verification recipe

Pinned snapshot:

```bash
OLD=5e5a686704a3c78a894b96094f6fa9f814b53888
git ls-files -s legacy/jamiejones85-ESP32TeslaShuntCan   # -> 160000 5e5a686704a3c78a894b96094f6fa9f814b53888 0
git -C legacy/jamiejones85-ESP32TeslaShuntCan rev-parse HEAD   # must match
git -C legacy/jamiejones85-ESP32TeslaShuntCan fetch --tags --prune
git -C legacy/jamiejones85-ESP32TeslaShuntCan log --oneline $OLD..origin/main
git -C legacy/jamiejones85-ESP32TeslaShuntCan diff --stat $OLD..origin/main
```

Upstream anchors (run inside the submodule or on its files; every one was executed at write time and hit as shown):

```text
git -C legacy/jamiejones85-ESP32TeslaShuntCan grep -nE 'CAN\.beginPacket\(0x521\)'
  -> ESP32TeslaShuntCan.ino:28
git -C legacy/jamiejones85-ESP32TeslaShuntCan grep -nE '0xff000000.*>> *24'
  -> ESP32TeslaShuntCan.ino:31   (MSB-first = big-endian wire order)
git -C legacy/jamiejones85-ESP32TeslaShuntCan grep -nE 'SPISettings\(500000' -- Shunt.cpp
  -> Shunt.cpp:17,46,71,97,123,148,173,199,224   (9 transactions)
git -C legacy/jamiejones85-ESP32TeslaShuntCan grep -nE 'SPI\.transfer\(0x41\)'
  -> Shunt.cpp:228
git -C legacy/jamiejones85-ESP32TeslaShuntCan grep -nE 'buff\[6\]\*256\+buff\[5\]'
  -> Shunt.cpp:248
git -C legacy/jamiejones85-ESP32TeslaShuntCan grep -nE 'Task ms10\(10'
  -> ESP32TeslaShuntCan.ino:9
git -C legacy/jamiejones85-ESP32TeslaShuntCan grep -nE 'chipSelectPin = SS'
  -> Shunt.cpp:4
git -C legacy/jamiejones85-ESP32TeslaShuntCan grep -nE 'int buff\[18\]'
  -> Shunt.cpp:3
git -C legacy/jamiejones85-ESP32TeslaShuntCan grep -nE 'for \(int I = 1; I < 19'
  -> Shunt.cpp:7   (buff[18] out-of-bounds bound)
git -C legacy/jamiejones85-ESP32TeslaShuntCan grep -nE 'transfer\(0x9A\)'
  -> Shunt.cpp:233
```

Negative anchors (absence claims):

```text
git -C legacy/jamiejones85-ESP32TeslaShuntCan grep -nE 'Serial\.begin|milli'
  -> no output   (no Serial init; no mA scale anywhere)
git -C legacy/jamiejones85-ESP32TeslaShuntCan grep -nE 'joromy'
  -> no output   (attribution exists only in the replaced doc)
```

Our-side drift anchors (the `already present` / `ignore` decisions rest on these):

```text
git grep -nE 'decodeBmsCurrent' -- firmware/lib/vehicle/can/feature/telemetry/bms.h
  -> bms.h:29 (decoder), bms.h:42 (power product)
git grep -nE 'CAN_ID_BMS_HV_BUS' -- firmware/lib/vehicle/can/ids.h
  -> ids.h:37
git grep -nE 'SPI\.begin\(' -- firmware/lib/core/driver/esp32/board.h
  -> board.h:84
git grep -nE 'canSimTick' -- firmware/lib/vehicle/can/feature/misc/can_sim.h
  -> can_sim.h:33
git grep -rnE '0x521' -- firmware packages tools
  -> no output   (0x521 absent from all shipping code)
git grep -nE '0x521' -- client/src
  -> legacy_modelsx_amd.json:51138, legacy_modelsx_intel.json:242175 (vendored MSX catalog, BCFALCD_* frames)
  -> docs/_generated-docs.ts:7424 (auto-generated from the replaced doc; refreshes when the docs bundle regenerates)
```

If line numbers drift more than ~10 after an upstream bump, re-read the whole function; do not trust this document's numbers. Pre-pin upstream history was NOT reviewed — there is nothing before the pin locally (single squashed commit).

## 17. Open questions and explicitly not investigated

- Is 0x521 the genuine Tesla shunt message on the pack-internal CAN, or custom? The replaced doc's `#42` claim ("not a standard Tesla CAN ID") is unconfirmed: our vendored MSX catalog shows unrelated `BCFALCD_*` frames at 0x521 on the ETH bus (`legacy_modelsx_amd.json:51138`, RANGE-CITED), but no pack-CAN capture exists. A listen-only capture on a Model S/X pack harness, or the Tom-evnut repo, would settle it.
- Units: README says mA; code has no scale. Would settle: bench capture against a known load.
- Endianness: README says little-endian, code writes big-endian — which one does the receiving device actually accept? Would settle: capture of a genuine shunt frame.
- Semantics of every init byte and of the trailers `0xBC/0x63/0x33/0x83/0x94/0xC0` (PEC? CRC? echo?) — needs the shunt ASIC datasheet; the ASIC's identity is itself UNVERIFIED ("ShuntAsic" labels in our vendored `legacy_mcu2.json:53619` suggest Tesla calls it a shunt ASIC; grep-only, not line-read).
- Which `CAN.h` library this compiles against (collin80/ESP32SJA1000 vs other) — no `platformio.ini`, no `library.properties`; UNVERIFIED.
- The duplicated `0x4E` transaction (`Shunt.cpp:75-82` and `101-108`) — intentional double-write or bug; UNVERIFIED.
- The `//60000` comment at `ESP32TeslaShuntCan.ino:24` — remnant of an earlier scale or threshold; UNVERIFIED.
- `Serial.print` without `Serial.begin()` — behavior depends on ESP32 core version; UNVERIFIED.
- The "joromy" attribution (`git show HEAD:docs/legacy/jamiejones85-ESP32TeslaShuntCan.md#37`) — network-only, not checkable from the pinned tree.
- Pre-pin upstream history was not reviewed (nothing exists locally before `5e5a686`).
- `client/src/docs/_generated-docs.ts:7424` still mirrors the replaced doc's little-endian/milliamps text until the generated docs bundle is regenerated; it is an auto-generated file and was deliberately not hand-edited (outside this task's one-doc scope).
