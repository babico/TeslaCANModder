# Wi-Fi Board Guide

This guide is a structured reference for comparing legacy Wi-Fi board paths against the current TeslaCANModder setup.

It intentionally separates:

- current supported TeslaCANModder setup
- legacy Wi-Fi reference hardware
- future migration concepts

## 1. Current Supported TeslaCANModder Setup

Status:

- supported now
- current source of truth

### Purpose

Bring up TeslaCANModder using the active Uno-based architecture.

### Hardware involved

- Arduino Uno
- MCP2515 with `8 MHz` crystal
- X179 connector
- purchased `9V-36V -> 5V` USB converter
- optional HC-05

### Connection model

- USB first
- X179 power second
- CAN after power is stable
- HC-05 only after wired setup works

### What the user sees in UI

- external React dashboard in `web/`
- no board-hosted dashboard
- serial-based connection flow

### Why this matters

This is the only setup path TeslaCANModder currently supports directly.

## 2. Legacy Wi-Fi Board Reference

Status:

- reference only
- not current TeslaCANModder flow

## A. RP2040 + ESP32-C3 bridge

### Purpose

Split CAN/FSD logic from Wi-Fi/dashboard hosting.

### Hardware involved

- RP2040 CAN board
- ESP32-C3 bridge board
- UART link between them

### Connection model

- CAN logic on RP2040
- Wi-Fi AP/server on ESP32-C3
- browser talks to bridge-hosted dashboard

### What the user sees in UI

- dashboard served from the board stack itself

### Exact difference versus TeslaCANModder

- two firmware images instead of one
- two boards instead of one main board
- dashboard lives on the hardware, not in `web/`

### Why this matters

This model is closest to a network bridge design, but it is much harder to operate and document cleanly for end users.

### If ported later, what would need to change

- define a transport bridge for the current board protocol
- avoid copying the embedded dashboard
- keep `web` as the UI if possible

## B. ESP32 + MCP2515 Wi-Fi

### Purpose

Run CAN logic and Wi-Fi dashboard on one board while keeping MCP2515.

### Hardware involved

- ESP32
- MCP2515 CAN controller

### Connection model

- one MCU for CAN + Wi-Fi
- browser talks to board-hosted server

### What the user sees in UI

- legacy board-served dashboard

### Exact difference versus TeslaCANModder

- single board is simpler than RP2040 bridge
- transport and UI still do not match TeslaCANModder

### Why this matters

This is the easiest conceptual stepping-stone from the current Uno + MCP2515 architecture.

### If ported later, what would need to change

- add ESP32 board target
- preserve current TeslaCANModder message protocol
- add Wi-Fi transport support to current `web`

## C. ESP32-S3 TWAI Wi-Fi

### Purpose

Move to a single-board Wi-Fi-capable design with native CAN.

### Hardware involved

- ESP32-S3 with TWAI

### Connection model

- one board owns CAN and Wi-Fi
- browser talks to board network transport

### What the user sees in UI

- legacy board-served dashboard in the reference design

### Exact difference versus TeslaCANModder

- native CAN instead of MCP2515
- larger firmware-driver delta
- cleaner future hardware design

### Why this matters

This is the best long-term future target if TeslaCANModder ever becomes Wi-Fi-native.

### If ported later, what would need to change

- add a TWAI driver path
- add Wi-Fi transport while preserving TeslaCANModder's protocol
- keep `web` as the main dashboard

## 3. Future Wi-Fi Migration Path

Status:

- future migration concept
- not implemented today

## Recommended path

Long-term:

- `ESP32-S3 TWAI`

Short-term stepping-stone:

- `ESP32 + MCP2515`

## Recommendation rules

- do not copy the legacy embedded dashboard
- do not turn TeslaCANModder into a dual-UI product by default
- keep the external React app as the dashboard
- add Wi-Fi as a transport and board-target expansion, not as a separate UI family

## 4. Practical Comparison Tables

## Board architecture

| Path | Boards | CAN controller | Wi-Fi location |
|---|---|---|---|
| Current TeslaCANModder | 1 main board + MCP2515 module | MCP2515 | none |
| RP2040 + bridge | 2 | MCP2515 on RP2040 side | ESP32-C3 bridge |
| ESP32 + MCP2515 | 1 | MCP2515 | integrated |
| ESP32-S3 TWAI | 1 | TWAI | integrated |

## Dashboard location

| Path | Dashboard lives in firmware | Uses external React app natively |
|---|---|---|
| Current TeslaCANModder | No | Yes |
| RP2040 + bridge | Yes | No |
| ESP32 + MCP2515 | Yes | No |
| ESP32-S3 TWAI | Yes | No |

## Compatibility with TeslaCANModder direction

| Path | Fit with current architecture | Recommendation |
|---|---|---|
| Current TeslaCANModder | Native | current source of truth |
| RP2040 + bridge | Low | reference only |
| ESP32 + MCP2515 | Medium | pragmatic stepping-stone |
| ESP32-S3 TWAI | High long-term | recommended future target |

## 5. What Should Not Be Ported Directly

- embedded legacy HTML dashboards
- monolithic `.ino` structure
- board-specific duplicated UI logic
- endpoint-polling dashboard model as the main TeslaCANModder transport contract

## 6. What Is Worth Reusing

- behavior and bit-logic references
- profile and FSD semantics
- HW3 speed offset semantics
- HW4 ISA behavior
- stream, stats, and sniffer concepts
- Wi-Fi transport ideas

## 7. Decision Guide

Stay on the current Uno path when:

- you want the supported setup
- you want the current `web` app
- you want the simplest documented workflow

Study the legacy Wi-Fi paths when:

- you are evaluating future board migration
- you want to compare hardware topologies
- you want feature inspiration for future TeslaCANModder transport work

Use the Wi-Fi board material as migration reference, not as current setup instructions.
