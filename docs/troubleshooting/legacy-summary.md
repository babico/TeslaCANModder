---
title: Legacy Summary
title_tr: Eski Repo Özeti
description: Synthesis of 83 community Tesla CAN bus repositories
category: troubleshooting
folder: troubleshooting
tags: [legacy, summary, history]
order: 16
icon: 📚
---

# Legacy Repository Synthesis

> Analysis of 83 Tesla CAN bus community repositories collected in `legacy/`

## Executive Summary

This synthesis covers **83 open-source repositories** spanning the Tesla CAN bus modification ecosystem. The collection represents the full breadth of community effort around Tesla vehicle hacking — from simple Arduino sketches that send a single FSD-enable frame, to full-stack dashboard applications with real-time CAN decoding, DBC databases, and multi-platform firmware.

**Key findings:**

- **FSD CAN Mod is the dominant use case** — 14 repos (17%) focus specifically on FSD region-gate bypass, with another 9 general CAN mod repos that include FSD as a feature. Most are forks or ports of the same Starmixcraft/CanFeather origin.
- **ESP32 is the ecosystem's platform of choice** — mentioned in 28+ repos, followed by MCP2515 (22+), Arduino-generic (21+), Raspberry Pi (12), and RP2040 (10).
- **License compliance is a major risk** — 38 repos (46%) have no license specified, 22 have unknown/unreviewed licenses, and only 17 are clearly MIT. Only 1 is GPL-3.0.
- **High code duplication** — Many FSD mod repos contain near-identical CAN frame logic copy-pasted across projects rather than maintained as a shared library.
- **30 repos have High CAN relevance** — these contain core CAN bus functionality with significant code that directly applies to our project.
- **Our project (Tesla-CAN-Mod) is the only one** that combines a multi-handler architecture (HW3/HW4/Legacy), 29+ discrete vehicle features, BLE/WiFi/Serial I/O, native unit tests, and multi-platform builds (UNO + ESP32) in a single unified firmware.

---

## Category Breakdown

| Category | Count | Representative Repos |
| -------- | ----- | -------------------- |
| **FSD CAN Mod** | 14 | hypery11-flipper-tesla-fsd, JelloEa-tesla-fsd-controller, tesla-fsd-can-mod-main, tesla-fsd.netlify.app, herrfrei-tesla-fsd-canbus-esp32 |
| **CAN Mod (General)** | 9 | JelloEa-Tesla-Open-CAN-Mod, tesla-open-can-mod-main, alzza-tesla-open-can-mod, ev-open-can-tools, tuncasoftbildik-tesla-can-mod |
| **CAN Monitoring/Analysis** | 8 | ekr-candash, bruvv-tesla-can-explorer, mikegapinski-tesla-can-explorer, nicholasyangyang-ESP32-dash-direct, BluedDot-IT-TeslaCANalyzer |
| **CAN Database/Decoding** | 5 | joshwardell-model3dbc, talas9-tesla_can_signals, LeeGaHyeon-tesla_CAN_traffic_decode, mcirish-TeslaModelS_RefreshCAN.dbc, evoffer-can-decoder-firmware |
| **CAN Logging** | 4 | tumik-S3XY-candump, JonnoFTW-rpi-can-logger, jsamuel1-tesla_canlogjs, amzoo-TMS_2016_LOGS |
| **Other/Utility** | 27 | rossklonowski-CANserver, hanswolff-TeslaCanBusInspector, tesberry-tesberry, SergeyStaroletov-Tesla-CAN-packets-generator, Adminius-ESP32-ScanMyTesla |
| **Tesla API/App** | 3 | jiezaichan-teslaAuthFlutter, rjyo-homebridge-tesla-remote, rrrovalle-tesla-car-app |
| **Steering/EPAS** | 3 | gregjhogan-tesla-pre-ap-epas-patch, sydneyg007-Tesla-Model-3-EPAS-emulator, riderx-autosteerplus |
| **Battery/Charging** | 3 | DemiVis-charge-port-opener, jomytec-My_TeslaBMS, oliwiah-Tesla_Battery_Range_Calc_React |
| **Camera/Sentry** | 2 | bobmorane83-TeslaCam, denysvitali-tesla-sentry-viewer |
| **Performance/Speed** | 2 | ColinM-sys-tesla-can-boost, kangbumhee-TLA-SpeedAlert |
| **BLE/Bluetooth** | 1 | wimaha-TeslaBleHttpProxy |
| **Nag Removal** | 1 | nicolozak-nag-killer |
| **Firmware** | 1 | monster-xxx-tesla-can-controller |
| **Total** | **83** | |

### Category Insights

- The **FSD CAN Mod + General CAN Mod** categories (23 repos, 28%) represent the core of the ecosystem and the most directly relevant work to our project.
- The **"Other" category is large (27 repos)** because many repos are specialized utilities (CAN-to-Bluetooth adapters, telemetry dashboards, CAN packet generators, CRC analysis) that don't fit neatly into vehicle-feature categories.
- **CAN Monitoring/Analysis tools** (8 repos) are a rich source of signal documentation and DBC knowledge even when their code isn't directly reusable.
- **Steering/EPAS repos** contain safety-critical CAN knowledge (chassis bus messages, EPAS firmware patching) that informs our architecture but should never be directly copied.

---

## Technology Stack Analysis

### Hardware Platforms

| Platform | Repo Count | Notes |
| -------- | --------- | ----- |
| **ESP32** (all variants) | 28 | Dominant MCU — S2, S3, C3 variants all represented |
| **MCP2515 CAN Controller** | 22 | Most common external CAN interface (SPI-based) |
| **Arduino** (generic/UNO) | 21 | Many repos use Arduino framework regardless of MCU |
| **Raspberry Pi** | 12 | Used for logging, dashboards, and BLE proxy |
| **RP2040** | 10 | Growing presence, especially in Open CAN Mod forks |
| **Flipper Zero** | 3 | hypery11, J0811, ylovex75 — FSD mod via Electronic Cats CAN add-on |
| **CAN Transceiver** (SN65HVD230 etc.) | 6 | Explicitly mentioned transceiver ICs |
| **STM32** | 1 | nicholasyangyang-my-tt |
| **Not specified** | 28 | Software-only repos or undocumented hardware |

### Software Frameworks

| Framework/Language | Notable Repos |
| ------------------ | ------------- |
| **Arduino** | 30+ repos use Arduino framework |
| **PlatformIO** | JelloEa-Tesla-Open-CAN-Mod, tesla-open-can-mod-main, bobmorane83-TeslaCam |
| **C/C++ (bare)** | hypery11, J0811, uhi22, SergeyStaroletov |
| **Python** | tumik-S3XY-candump, ColinM-sys-tesla-can-boost, gregjhogan, automotive-stuff |
| **C#/.NET** | hanswolff-TeslaCanBusInspector |
| **Java/Android** | ekr-candash, kangbumhee-TLA-SpeedAlert |
| **Go** | wimaha-TeslaBleHttpProxy |
| **JavaScript/Node.js** | jsamuel1-tesla_canlogjs, tesberry-tesberry |
| **React/Vue** | stylylsty-TelemetryX, riderx-autosteerplus, oliwiah |
| **Flutter/Dart** | jiezaichan-teslaAuthFlutter |
| **DBC Files** | joshwardell-model3dbc, mcirish, bobmorane83, ColinM-sys |

### CAN Communication Approaches

| Approach | Usage |
| -------- | ----- |
| **MCP2515 via SPI** | Most common — standard Arduino CAN shield approach |
| **ESP32 TWAI** (built-in CAN) | tesla-open-can-mod-main, JelloEa, ESP32-native repos |
| **SAME51 CAN** | JelloEa-Tesla-Open-CAN-Mod (M4 CAN Feather) |
| **SocketCAN** (Linux) | Raspberry Pi repos (JonnoFTW, automotive-stuff, tesberry) |
| **Panda Protocol** (WiFi) | tumik-S3XY-candump (Enhauto Commander/S3XY buttons) |
| **Flipper Zero GPIO** | hypery11, J0811 — MCP2515 via Flipper SPI |

---

## License Landscape

| License | Count | Repos |
| ------- | ----- | ----- |
| **Not specified** | 38 | tesla-fsd-can-mod-main, herrfrei, nicolozak, rossklonowski, bobmorane83, and 33 others |
| **Unknown (file exists)** | 22 | JelloEa-Tesla-Open-CAN-Mod, hypery11, tesla-open-can-mod-main, alzza, and 18 others |
| **MIT** | 17 | joshwardell-model3dbc, hanswolff, riderx-autosteerplus, tuncasoftbildik, monster-xxx, and 12 others |
| **Apache-2.0** | 3 | Akisoft41-TeslapLX, mgerczuk-TeslaCANPi, wimaha-TeslaBleHttpProxy |
| **Creative Commons** | 2 | canhackers-jupiter, ColinM-sys-tesla-can-boost |
| **GPL-3.0** | 1 | JelloEa-tesla-fsd-controller |

### License Risk Assessment

- **72% of repos (60/83) lack a clear, standard open-source license.** This is a significant risk for any code adoption.
- **MIT repos are safe to reference** — permissive, no copyleft concerns.
- **GPL-3.0** (JelloEa-tesla-fsd-controller) requires careful handling — any derivative work must also be GPL-3.0.
- **CC-licensed repos** (canhackers-jupiter, ColinM-sys) may have non-commercial restrictions — review specific CC variant before use.
- **"Unknown (file exists)"** repos have a LICENSE file but the license type wasn't auto-identified — manual review needed before any code integration.

---

## High-Value Repositories

The following repositories are the most relevant for our Tesla-CAN-Mod project based on CAN relevance, code quality, hardware alignment, and feature coverage.

### Tier 1: Direct Architecture References

| # | Repository | Why It's Valuable | Integration Potential |
| - | --------- | ----------------- | ------------------- |
| 1 | **tesla-open-can-mod-main** | The upstream Open CAN Mod — multi-driver architecture (TWAI, MCP2515, SAME51, mock), HW3/HW4/Legacy handlers, web dashboard, PlatformIO with tests. 88 files, 30 CAN files. Our project's closest ancestor. | Reference architecture. Our handler/dispatch pattern was likely influenced by this. Cross-check signal definitions. |
| 2 | **JelloEa-Tesla-Open-CAN-Mod** | JelloEa's fork with identical driver abstraction, 32 files, test suite for HW3/HW4/Legacy handlers, TWAI filter tests. Clean PlatformIO structure. | Validate our test cases against theirs. Compare handler logic for HW3→HW4 message differences. |
| 3 | **hypery11-flipper-tesla-fsd** | Most comprehensive FSD mod — Flipper Zero + ESP32 port, nag killer, ISA chime suppression, OTA guard, battery preconditioning, live BMS dashboard. 72 files. CAN dictionary in `enhauto-re/`. | Primary reference for CAN message IDs (0x398 GTW_carConfig for HW detect), FSD v14 logic, nag implementation, ISA chime. ESP32 port code is directly comparable to ours. |
| 4 | **joshwardell-model3dbc** | The canonical Model 3/Y DBC file — community gold standard for CAN signal definitions. MIT licensed. | **Safe to reference directly (MIT).** Cross-validate our signal byte/bit offsets against Model3CAN.dbc. |

### Tier 2: Feature-Specific References

| # | Repository | Why It's Valuable | Integration Potential |
| - | --------- | ----------------- | ------------------- |
| 5 | **rossklonowski-CANserver** | ESP32 CAN server with ESP-NOW WiFi bridge, web dashboard, 168 files, 82 CAN files. Real-time battery/current/power telemetry. | Reference for WiFi data streaming patterns, ESP-NOW multi-device architecture, web UI approach. |
| 6 | **ekr-candash** | Android instrument cluster (CANdash app), 153 files, 34 CAN files. Blind spot monitoring, live dashboards. | CAN signal decoding for 300+ signals, UI/UX patterns for mobile companion. |
| 7 | **hanswolff-TeslaCanBusInspector** | .NET CAN inspector for Model S/3/X, 127 files, 119 CAN files. MIT licensed. Strongly typed CAN message models. | **Safe to reference (MIT).** Message type definitions, signal value types (Ampere, Celsius, KiloWatt), session/timeline analysis. |
| 8 | **bobmorane83-TeslaCam** | ESP32-S3 wireless dashboard with CAN bridge, camera feed, DBC files. 3 DBC variants for Model 3. | Multiple DBC file variants for signal cross-referencing. CAN-to-wireless bridge architecture. |
| 9 | **ColinM-sys-tesla-can-boost** | Python CAN toolkit — read/write/analyze, ghost mode, live dashboards, drive recording. Contains Model3CAN.dbc. | CAN write command reference (horn, lights, drive mode), signal analysis tools for development/debugging. |
| 10 | **tesla-fsd.netlify.app** | Decoded firmware files for 7 board variants (ESP32, ESP32-S3, ESP8266, RP2040, Arduino UNO, Feather). Board-code mapping CSV. | Compare our per-board implementations against decoded firmware. Identify CAN messages we may have missed across variants. |

### Tier 3: Specialized Knowledge

| # | Repository | Why It's Valuable | Integration Potential |
| - | --------- | ----------------- | ------------------- |
| 11 | **uhi22-tesla-crc** | Tesla CAN CRC-8 analysis — reverse-engineered CRC calculation for frames 0x229, 0x249. | Critical for any CAN frame that requires counter/CRC validation. Reference for our CRC implementation. |
| 12 | **gregjhogan-tesla-pre-ap-epas-patch** | EPAS firmware patch enabling steering-over-CAN on pre-AP vehicles. | Understanding EPAS CAN protocol — relevant to our chassis bus handling. |
| 13 | **sydneyg007-Tesla-Model-3-EPAS-emulator** | EPAS emulation on Chassis and Party CAN buses (2019 Model 3 Performance). | Reference for chassis bus message IDs and EPAS signal formats. |
| 14 | **nicolozak-nag-killer** | CAN 0x880 counter-echo research — deterministic CAN frame echo with counter-based arbitration at 500 kbps. | Direct reference for our nag.h implementation. LILYGO T-CAN485 hw reference. |
| 15 | **wimaha-TeslaBleHttpProxy** | BLE-to-HTTP proxy in Go, Apache-2.0. Queue-based BLE command processing for Tesla vehicles. | **Safe to reference (Apache-2.0).** BLE command protocol patterns for our BLE I/O layer. |

---

## Cross-Repository Patterns

### CAN Message IDs Commonly Used

| CAN ID | Purpose | Found In |
| ------ | ------- | -------- |
| `0x398` | `GTW_carConfig` — HW3/HW4 auto-detection | hypery11, J0811, JelloEa, tesla-open-can-mod-main |
| `0x118` | Drive state / gear selector | ekr-candash, rossklonowski, ColinM-sys |
| `0x229` | Motor torque / drive inverter | uhi22-tesla-crc, multiple decoding repos |
| `0x249` | Motor data (HW variant-specific) | uhi22-tesla-crc |
| `0x257` | Speed data | kangbumhee-TLA-SpeedAlert, ColinM-sys |
| `0x292` | BMS voltage/current | rossklonowski, jamiejones85 |
| `0x321` | VCFRONT (lights, turn signals) | bobmorane83-TeslaCam |
| `0x3B6` | UI speed (odometer) | multiple repos |
| `0x521` | Shunt current data | jamiejones85-ESP32TeslaShuntCan |
| `0x880` | EPAS status (nag-related) | nicolozak-nag-killer |

### Hardware Configurations

- **Most common setup:** ESP32 + MCP2515 module via SPI (8 MHz crystal) + SN65HVD230 transceiver
- **Budget option:** Arduino UNO + MCP2515 CAN shield (~$5-10)
- **Advanced option:** Adafruit Feather M4 CAN (SAME51 with built-in CAN FD) — used by JelloEa
- **Flipper option:** Flipper Zero + Electronic Cats CAN Bus Add-On + OBD-II cable
- **Recommended bus speed:** 500 kbps standard across all repos
- **OBD-II connection:** Most repos connect via the diagnostic OBD-II port for CAN access

### Communication Architecture Patterns

1. **Direct SPI CAN (MCP2515):** Most common. Arduino library `mcp2515.h` or `mcp_can.h`. Simple but limited to one CAN bus.
2. **ESP32 TWAI (built-in):** No external controller needed. Used by newer firmware. Supports hardware filtering.
3. **Multi-driver abstraction:** Only in tesla-open-can-mod and our project — `can_driver.h` interface with `twai_driver.h`, `mcp2515_driver.h`, `same51_driver.h`, `mock_driver.h` implementations.
4. **WiFi bridge:** ESP-NOW (rossklonowski) or TCP/WebSocket (stylylsty, tesberry) for wireless CAN data streaming.
5. **BLE bridge:** Adminius-ESP32-ScanMyTesla (CAN→BLE for ScanMyTesla app).

### Feature Implementation Patterns

| Feature | Common Approach | Repos |
| ------- | -------------- | ----- |
| **FSD Enable** | Send specific CAN frame based on HW version detection from 0x398 | 14+ FSD repos |
| **HW Detection** | Read `GTW_carConfig` (0x398), check byte for HW3 vs HW4 | hypery11, JelloEa, tesla-open-can-mod |
| **Nag Removal** | Echo/modify EPAS status frame (0x880) with counter rotation | nicolozak, hypery11 |
| **ISA Speed Chime** | Suppress or modify ISA compliance frame | hypery11 |
| **Speed Offset** | Modify speedometer display value in CAN frame | Multiple CAN mod repos |
| **OTA Guard** | Block or filter OTA update CAN messages | hypery11 |

---

## Conflict Analysis

### License Conflicts

| Conflict | Details | Impact |
| -------- | ------- | ------ |
| **GPL-3.0 contamination** | JelloEa-tesla-fsd-controller is GPL-3.0. Any code derived from it must be GPL-3.0. | Must verify our handler logic was not derived from this repo. |
| **CC Non-Commercial** | canhackers-jupiter and ColinM-sys-tesla-can-boost use CC licenses — may prohibit commercial use. | Do not adopt code from these repos without verifying the specific CC variant. |
| **Unlicensed code** | 38 repos have no license = default copyright (all rights reserved). Cannot legally copy code. | Reference for patterns/knowledge only — do not copy code verbatim. |

### Duplicate Functionality

| Function | Duplicated Across | Notes |
| -------- | ----------------- | ----- |
| **FSD CAN enable** | 14+ repos, most forked from same CanFeather origin | Massive duplication — our project consolidates this properly. |
| **MCP2515 driver** | Nearly every Arduino-based repo includes its own copy | Our project uses the autowp/arduino-mcp2515 library — better approach. |
| **DBC definitions** | joshwardell, bobmorane83, ColinM-sys all include Model3CAN.dbc copies | joshwardell is the canonical source (MIT). |
| **HW3/HW4 detection** | hypery11, JelloEa, tesla-open-can-mod, multiple forks | Same 0x398 byte-check logic everywhere. Our variant.h consolidates this. |

### Incompatible Approaches

| Aspect | Approach A | Approach B | Our Resolution |
| ------ | --------- | --------- | ------------- |
| **CAN bus speed** | 500 kbps (standard) | 125 kbps (some BMS repos) | 500 kbps — matches vehicle spec |
| **CAN controller** | MCP2515 (SPI, universal) | ESP32 TWAI (built-in, ESP32-only) | Support both via driver abstraction |
| **Frame handling** | Polling loop | Interrupt-driven | Platform-dependent — both supported |
| **Config storage** | Hardcoded | EEPROM/NVS | NVS on ESP32, EEPROM on UNO |
| **Firmware update** | Manual flash | OTA via WiFi | OTA on ESP32, manual on UNO |

---

## Recommended Integration Roadmap

### Priority 1: Validate Against (Immediate)

1. **joshwardell-model3dbc** (MIT) — Cross-validate all CAN signal byte/bit offsets in our feature headers against the canonical DBC file. Any discrepancies should be flagged and verified on-vehicle.
2. **uhi22-tesla-crc** — Ensure our CRC-8 implementation matches the reverse-engineered algorithm for frames that use counter/CRC (0x229, 0x249).
3. **hypery11-flipper-tesla-fsd: `enhauto-re/CAN_DICTIONARY.md`** — Compare our CAN message documentation against this comprehensive CAN dictionary.

### Priority 2: Reference for Feature Parity (Short-term)

1. **hypery11 ESP32 port** — Compare FSD v14 handler logic, OTA guard implementation, battery preconditioning trigger against our corresponding features (fsd.h, precondition.h).
2. **nicolozak-nag-killer** — Validate our nag.h 0x880 echo logic against this focused implementation.
3. **tesla-open-can-mod-main** — Compare TWAI filter configuration and web dashboard implementation against our I/O layer.

### Priority 3: Adopt Patterns (Medium-term)

1. **rossklonowski-CANserver** — Evaluate ESP-NOW for multi-device wireless CAN bridge architecture (our WiFi layer could support this).
2. **wimaha-TeslaBleHttpProxy** (Apache-2.0) — Reference BLE command queue architecture for our BLE I/O robustness.
3. **hanswolff-TeslaCanBusInspector** (MIT) — Reference strongly-typed CAN message model pattern for potential code-generation from DBC files.

### Priority 4: Documentation Reference (Ongoing)

1. **bobmorane83-TeslaCam DBC files** — 3 variants of Tesla Model 3 DBC for signal coverage analysis.
2. **ColinM-sys-tesla-can-boost** — Ghost mode and CAN write command reference for future features.
3. **ekr-candash** — Signal decoding for 300+ CAN signals — useful for expanding our feature coverage.

---

## Gap Analysis

### What Our Project Does That No Legacy Repo Does

| Capability | Our Project | Legacy Ecosystem |
| --------- | ----------- | --------------- |
| **Unified multi-handler architecture** | HW3, HW4, Legacy handlers in single firmware | Each repo targets one HW variant or copies code per variant |
| **29+ discrete vehicle features** | fsd, nag, isa_chime, bms, climate, lock, mirror, seat, window, wiper, trunk, sentry, summon, track_mode, regen, pedal, power, charge, precondition, display, offsets, light, stop, stream, can_raw, can_clock, ban_shield, profile, variant | Most repos implement 1-3 features max |
| **Multi-platform single codebase** | UNO + ESP32 from same lib/ tree | Separate codebases per platform |
| **Driver abstraction layer** | core/driver/ with per-platform headers | Only tesla-open-can-mod has this; most repos have hardcoded drivers |
| **Native unit test suite** | PlatformIO native test environment | Only JelloEa/tesla-open-can-mod have tests |
| **BLE + WiFi + Serial I/O** | All three I/O channels in single firmware | Repos typically support only one channel |
| **Feature toggle persistence** | NVS/EEPROM-backed configuration | Most repos are hardcoded on/off |
| **Build infrastructure** | CI scripts, firmware rename, Docker support | Minimal build automation in legacy repos |
| **Comprehensive documentation** | docs/ with protocol specs, checklists, troubleshooting | Most repos have only a README |
| **Unified client companion** | client/ browser + native targets | Only a few repos (ekr-candash, riderx) have comparable companion apps |

### What Legacy Repos Do That We May Not

| Capability | Legacy Source | Our Gap |
| --------- | ----------- | ------- |
| **Flipper Zero support** | hypery11, J0811 | Not targeted — different hardware platform |
| **Linux SocketCAN integration** | JonnoFTW, tesberry, automotive-stuff | Our firmware is MCU-only; no Raspberry Pi Linux target |
| **Android native app** | ekr-candash, kangbumhee-TLA-SpeedAlert | Our client native target may not have Android CAN bridge features yet |
| **CAN data recording/playback** | tumik-S3XY-candump, ColinM-sys drive_recorder | stream.h exists but dedicated recording may need expansion |
| **Ghost mode / drive mode mod** | ColinM-sys-tesla-can-boost | Not in our feature list — may be intentionally excluded |
| **ESP-NOW multi-device mesh** | rossklonowski-CANserver | Our WiFi layer is AP/client, not ESP-NOW |
| **ScanMyTesla Bluetooth bridge** | Adminius-ESP32-ScanMyTesla | Our BLE speaks our own protocol, not ScanMyTesla format |
| **Full DBC file generation** | joshwardell, bobmorane83 | We define signals in headers, not DBC format |
| **Tailgate foot activation** | evoffer-can-decoder-firmware | Specialized feature not in our scope |
| **EPAS firmware patching** | gregjhogan-tesla-pre-ap-epas-patch | Firmware patching not in scope (CAN-level only) |

---

## Appendix: Full Repository Index

| # | Repository | Category | License | Platform | CAN Relevance |
| - | --------- | -------- | ------- | -------- | ------------- |
| 1 | 1-v-1-tesla-fsd-can-mod | FSD CAN Mod | Not specified | Not specified | None |
| 2 | 1-v-1-tesla-open-can-mod | CAN Mod | Unknown (file exists) | ESP32, RP2040, Arduino, RPi, MCP2515 | High (25 files) |
| 3 | Adminius-ESP32-ScanMyTesla | Other | Unknown (file exists) | ESP32, Arduino, CAN Transceiver | Medium (5 files) |
| 4 | Akisoft41-TeslapLX | Other | Apache-2.0 | ESP32 | Medium (5 files) |
| 5 | alzza-tesla-open-can-mod | CAN Mod | Unknown (file exists) | ESP32, RP2040, Arduino, MCP2515 | High (39 files) |
| 6 | amzoo-TMS_2016_LOGS | CAN Logging | Not specified | Not specified | Low (1 file) |
| 7 | Arkay92-TeslaCANInterpreter | Other | MIT | Not specified | Low (2 files) |
| 8 | automotive-stuff-Tesla_canbus | Other | Not specified | Raspberry Pi | Medium (8 files) |
| 9 | bbrightwell-forklift-dash | CAN Monitoring | Not specified | Raspberry Pi | High (12 files) |
| 10 | binfen1-tesla-fsd-can-mod | FSD CAN Mod | Not specified | MCP2515 | Low (2 files) |
| 11 | BluedDot-IT-TeslaCANalyzer | CAN Monitoring | Not specified | Not specified | Low (1 file) |
| 12 | bobmorane83-TeslaCam | Camera/Sentry | Not specified | ESP32, Arduino, MCP2515 | High (17 files) |
| 13 | bruvv-tesla-can-explorer | CAN Monitoring | Unknown (file exists) | Not specified | High (21 files) |
| 14 | canhackers-jupiter | Other | CC | Not specified | Low (3 files) |
| 15 | cbusillo-TeslaPiCAN | Other | Not specified | Not specified | None |
| 16 | codethaumaturge-911-tesla-gauges | Other | MIT | Raspberry Pi | Low (1 file) |
| 17 | ColinM-sys-tesla-can-boost | Performance/Speed | CC | Not specified | High (13 files) |
| 18 | DemiVis-charge-port-opener | Battery/Charging | Not specified | Not specified | Low (1 file) |
| 19 | denysvitali-tesla-sentry-viewer | Camera/Sentry | MIT | Not specified | None |
| 20 | devon-smith-six-pack-capacitor-tesla-coil | Other | Not specified | Not specified | Low (1 file) |
| 21 | dongho74s-tesla-open-can-mod | CAN Mod | Unknown (file exists) | ESP32, RP2040, Arduino, RPi, MCP2515 | High (22 files) |
| 22 | ekr-candash | CAN Monitoring | Unknown (file exists) | Not specified | High (34 files) |
| 23 | enstw-tesla-can-mod-guide | CAN Mod | Not specified | RP2040, MCP2515 | Low (1 file) |
| 24 | erikhedb-WE-EV-CAN-Dashboard_POC | CAN Monitoring | Not specified | Raspberry Pi, MCP2515 | Medium (9 files) |
| 25 | ev-open-can-tools | CAN Mod | Unknown (file exists) | ESP32, RP2040, Arduino, RPi, MCP2515 | High (27 files) |
| 26 | evoffer-can-decoder-firmware | CAN Database | Not specified | Not specified | Low (1 file) |
| 27 | gregjhogan-tesla-pre-ap-epas-patch | Steering/EPAS | Unknown (file exists) | Not specified | Low (1 file) |
| 28 | hanswolff-TeslaCanBusInspector | Other | MIT | Not specified | High (119 files) |
| 29 | herrfrei-tesla-fsd-canbus-esp32 | FSD CAN Mod | Not specified | ESP32, MCP2515 | Low (3 files) |
| 30 | honeer-Tesla-ESP-CAN | Other | Not specified | ESP32, Arduino, MCP2515 | Medium (8 files) |
| 31 | hypery11-flipper-tesla-fsd | FSD CAN Mod | Unknown (file exists) | ESP32, Arduino, Flipper Zero, MCP2515 | High (22 files) |
| 32 | ibmthinkpad-model3mon | CAN Monitoring | MIT | Arduino | Low (2 files) |
| 33 | iubns-tesla-fsd-can-mod | FSD CAN Mod | Unknown (file exists) | RP2040, Arduino, MCP2515 | Medium (4 files) |
| 34 | J0811-flipper-tesla-fsd | FSD CAN Mod | Unknown (file exists) | Arduino, Flipper Zero, MCP2515 | Medium (4 files) |
| 35 | jamiejones85-ESP32TeslaShuntCan | Other | MIT | ESP32 | Low (2 files) |
| 36 | jberstler-tesla-warmer | Other | MIT | Not specified | None |
| 37 | JelloEa-tesla-fsd-controller | FSD CAN Mod | GPL-3.0 | ESP32, Arduino | Medium (7 files) |
| 38 | JelloEa-Tesla-Open-CAN-Mod | CAN Mod | Unknown (file exists) | ESP32, RP2040, Arduino, RPi, MCP2515 | High (21 files) |
| 39 | jiezaichan-teslaAuthFlutter | Tesla API/App | Unknown (file exists) | Not specified | Medium (6 files) |
| 40 | jomytec-My_TeslaBMS | Battery/Charging | Not specified | Not specified | None |
| 41 | JonnoFTW-rpi-can-logger | CAN Logging | Not specified | Raspberry Pi | High (26 files) |
| 42 | joshwardell-model3dbc | CAN Database | MIT | Not specified | Low (2 files) |
| 43 | jsamuel1-tesla_canlogjs | CAN Logging | MIT | Raspberry Pi | High (11 files) |
| 44 | juamiso-tesla-fsd-can-enabler | FSD CAN Mod | Not specified | ESP32, RP2040, Arduino, MCP2515, CAN Xcvr | Medium (6 files) |
| 45 | jvanakker-tesla-fsd-can-mod | FSD CAN Mod | Not specified | MCP2515 | Low (2 files) |
| 46 | kangbumhee-TLA-SpeedAlert | Performance/Speed | Not specified | Not specified | High (22 files) |
| 47 | krconv-tesla_can_decoding | Other | MIT | Not specified | Low (3 files) |
| 48 | LeeGaHyeon-tesla_CAN_traffic_decode | CAN Database | Not specified | Not specified | Low (2 files) |
| 49 | linesoft2-tesla-fsd-can-mod-fork | FSD CAN Mod | Not specified | MCP2515 | Low (1 file) |
| 50 | MatthewDriver-TeslaCAN | Other | MIT | Not specified | High (21 files) |
| 51 | mcirish-TeslaModelS_RefreshCAN.dbc | CAN Database | Not specified | Not specified | Low (1 file) |
| 52 | mgerczuk-TeslaCANPi | Other | Apache-2.0 | Arduino, Raspberry Pi | High (31 files) |
| 53 | mhpetiwala-TeslaCAN | Other | MIT | Not specified | High (21 files) |
| 54 | mikegapinski-tesla-can-explorer | CAN Monitoring | Unknown (file exists) | Not specified | High (21 files) |
| 55 | monster-xxx-tesla-can-controller | Firmware | MIT | ESP32, CAN Transceiver | Low (1 file) |
| 56 | MrStarTraveller-tesla-fsd-can-mod | FSD CAN Mod | Unknown (file exists) | ESP32, RP2040, Arduino, RPi, MCP2515 | High (22 files) |
| 57 | nicholasyangyang-ESP32-dash-direct | CAN Monitoring | Not specified | ESP32, CAN Transceiver | Medium (6 files) |
| 58 | nicholasyangyang-my-tt | Other | Unknown (file exists) | RP2040, Arduino, RPi, STM32, MCP2515 | High (18 files) |
| 59 | nicolozak-nag-killer | Nag Removal | Not specified | ESP32, CAN Transceiver | Low (2 files) |
| 60 | oliwiah-Tesla_Battery_Range_Calc_React | Battery/Charging | Not specified | Not specified | None |
| 61 | rafal83-Car-Light-Sync | Other | Unknown (file exists) | ESP32, CAN Transceiver | High (37 files) |
| 62 | riderx-autosteerplus | Steering/EPAS | MIT | Not specified | Low (2 files) |
| 63 | rjyo-homebridge-tesla-remote | Tesla API/App | MIT (package.json) | Not specified | Low (1 file) |
| 64 | rossklonowski-CANserver | Other | Not specified | ESP32, Arduino | High (82 files) |
| 65 | rrrovalle-tesla-car-app | Tesla API/App | Not specified | Not specified | Medium (5 files) |
| 66 | RuairidhScott-Brown-TeslaCAN | Other | Not specified | Not specified | Medium (4 files) |
| 67 | sahilcc7-tesla_can | Other | Unknown (file exists) | ESP32, RP2040, Arduino, RPi, MCP2515 | High (22 files) |
| 68 | SergeyStaroletov-Tesla-CAN-packets-generator | Other | Not specified | Arduino | Medium (5 files) |
| 69 | SlipknotTN-Tesla_CanBus_Reader | Other | MIT | Not specified | Medium (4 files) |
| 70 | stylylsty-TelemetryX | Other | Not specified | Not specified | Low (1 file) |
| 71 | sydneyg007-Tesla-Model-3-EPAS-emulator | Steering/EPAS | Not specified | ESP32 | Low (3 files) |
| 72 | sydneyg007-Tesla-Model-3-Front-DI-emulator | Other | Not specified | ESP32 | Medium (5 files) |
| 73 | talas9-tesla_can_signals | CAN Database | Not specified | Not specified | High (18 files) |
| 74 | tesberry-tesberry | Other | Not specified | Raspberry Pi | High (14 files) |
| 75 | tesla-fsd-can-mod-main | FSD CAN Mod | Not specified | MCP2515 | Low (2 files) |
| 76 | tesla-fsd-can-mod-2-main | FSD CAN Mod | Unknown (file exists) | RP2040, Arduino, MCP2515 | Medium (4 files) |
| 77 | tesla-fsd.netlify.app | FSD CAN Mod | Not specified | ESP32, RP2040, Arduino, MCP2515 | High (20 files) |
| 78 | tesla-open-can-mod-main | CAN Mod | Unknown (file exists) | ESP32, RP2040, Arduino, RPi, MCP2515, CAN Xcvr | High (30 files) |
| 79 | tumik-S3XY-candump | CAN Logging | Unknown (file exists) | Raspberry Pi | Medium (6 files) |
| 80 | tuncasoftbildik-tesla-can-mod | CAN Mod | MIT | ESP32, Arduino | Medium (9 files) |
| 81 | uhi22-tesla-crc | Other | Not specified | Not specified | None |
| 82 | wimaha-TeslaBleHttpProxy | BLE/Bluetooth | Apache-2.0 | Raspberry Pi | Low (3 files) |
| 83 | ylovex75-tesla-open-can-mod-release | CAN Mod | Unknown (file exists) | ESP32, Arduino, Flipper Zero, CAN Xcvr | High (43 files) |

---

*Generated from automated analysis of 83 legacy repository documentation files. Manual verification recommended for all integration decisions.*
