# Wi-Fi Board Comparative Audit

This document compares the legacy Wi-Fi-capable firmware in `legacy/tesla-fsd-open-mod-main` against the active TeslaCANModder architecture in `hardware/` and `web/`.

It is an engineering reference, not a request to port the legacy code as-is.

## Executive Summary

The legacy Wi-Fi folder contains four board families:

- `CanFeather_RP2040.ino`
- `ESP32C3_WiFiBridge.ino`
- `CanFeather_ESP32_WiFi.ino`
- `CanFeather_ESP32S3_TWAI.ino`
- `CanFeather_ESP8366_WiFi.ino`

In practical terms, those files form three real product models:

1. `RP2040 + ESP32-C3 bridge`
2. `ESP32 + MCP2515 + integrated Wi-Fi dashboard`
3. `ESP32-S3 + TWAI + integrated Wi-Fi dashboard`

Blunt recommendation:

- keep TeslaCANModder's current architecture
- do not adopt the legacy board-hosted dashboards as the main product direction
- if Wi-Fi support is added later, target a Wi-Fi board family and adapt it to TeslaCANModder's existing board protocol instead of adapting TeslaCANModder to the legacy HTTP/dashboard model

Primary long-term recommendation:

- `ESP32-S3 TWAI`

Primary short-term stepping-stone recommendation:

- `ESP32 + MCP2515`

Historical reference only:

- `ESP8266`

Main architectural difference versus the current repo:

- the legacy Wi-Fi firmware owns the dashboard, transport server, configuration storage, and CAN logic inside one monolithic `.ino`
- TeslaCANModder splits firmware behavior from the browser UI and uses a board protocol instead of board-hosted HTML

## Current TeslaCANModder Baseline

### Firmware

TeslaCANModder currently assumes:

- Arduino Uno primary build target
- MCP2515 external CAN controller
- modular firmware split into:
  - `board`
  - `can`
  - `drivers`
  - `handlers`
  - `packages`
- one runtime-switchable image for `legacy`, `hw3`, and `hw4`
- USB as the primary transport
- HC-05 as an optional secondary serial transport
- line-based JSON protocol for `boot`, `status`, `frame`, `log`, `ack`, `error`, and `pong`

### Web

TeslaCANModder's web app assumes:

- React dashboard in `web/`
- browser-side transport session
- board-pushed status and frame messages
- monitor state built around board-emitted frame events
- no embedded board-hosted dashboard
- no HTTP polling contract with the board

### Resulting design rules

The active repo is built around one separation:

- firmware emits a machine-readable protocol
- the external web app owns the actual dashboard UX

That separation is the single biggest difference between the current codebase and the legacy Wi-Fi firmware.

## Legacy Wi-Fi Variant Inventory

## A. RP2040 + ESP32-C3 Wi-Fi bridge

### Files

- `legacy/tesla-fsd-open-mod-main/CanFeather_RP2040.ino`
- `legacy/tesla-fsd-open-mod-main/ESP32C3_WiFiBridge.ino`

### What it is

- RP2040 board owns CAN/FSD logic
- ESP32-C3 hosts Wi-Fi AP, server, and dashboard
- UART links the two boards

This is effectively a split architecture:

- CAN compute board
- network/UI bridge board

### Implications

- two firmware images
- two flashing flows
- extra UART wiring
- more power and setup complexity
- clear separation between CAN logic and network hosting

### Why it matters

This is the closest legacy design to a transport bridge concept, but it is also the most operationally complex for end users. It does not fit TeslaCANModder's current "single board plus external web app" story.

### TeslaCANModder fit

- behavior reference: useful
- architecture reference: partial
- future primary product path: weak

## B. ESP32 + MCP2515 + integrated Wi-Fi dashboard

### File

- `legacy/tesla-fsd-open-mod-main/CanFeather_ESP32_WiFi.ino`

### What it is

- single ESP32 board
- external MCP2515 CAN controller
- integrated Wi-Fi AP/server/dashboard in the same firmware

### Implications

- one firmware image
- still relies on MCP2515 concepts close to the current Uno path
- easier CAN migration than RP2040 bridge
- still monolithic and not protocol-compatible with TeslaCANModder

### Why it matters

This is the easiest conceptual stepping-stone from the current Uno + MCP2515 architecture because the CAN controller assumptions remain similar. It is a reasonable migration bridge if the goal is to reuse more driver-level and message-shaping ideas before moving to a native CAN MCU.

### TeslaCANModder fit

- behavior reference: strong
- hardware migration reference: strong
- future primary product path: medium

## C. ESP32-S3 + TWAI + integrated Wi-Fi dashboard

### File

- `legacy/tesla-fsd-open-mod-main/CanFeather_ESP32S3_TWAI.ino`

### What it is

- single ESP32-S3 board
- native TWAI CAN peripheral
- integrated Wi-Fi server and dashboard
- no MCP2515 dependency

### Implications

- fewer wiring points
- no external CAN controller board
- best hardware simplification
- largest driver/platform delta versus the current Uno build

### Why it matters

This is the cleanest long-term hardware path if TeslaCANModder ever grows beyond the Uno. It reduces the number of moving parts and removes MCP2515-specific hardware assumptions, but it also requires the most deliberate firmware-driver work.

### TeslaCANModder fit

- behavior reference: strong
- hardware migration reference: strongest long-term
- future primary product path: strongest

## D. ESP8266 Wi-Fi variant

### File

- `legacy/tesla-fsd-open-mod-main/CanFeather_ESP8366_WiFi.ino`

### What it is

- historical Wi-Fi variant in the same family

### Why it is not recommended

- weaker platform fit than ESP32-class boards
- weaker future story than ESP32-S3 TWAI
- no advantage strong enough to justify choosing it over newer ESP32-family paths

## Behavior and Feature Comparison

| Capability | TeslaCANModder Today | RP2040 + Bridge | ESP32 + MCP2515 Wi-Fi | ESP32-S3 TWAI Wi-Fi |
|---|---|---|---|---|
| CAN transport hardware | MCP2515 | MCP2515 on RP2040 side | MCP2515 | Native TWAI |
| Wi-Fi built in | No | Yes, on bridge board | Yes | Yes |
| Board-hosted dashboard | No | Yes | Yes | Yes |
| External React app compatibility | Native | No, not directly | No, not directly | No, not directly |
| USB serial support | Yes | Yes | Usually yes | Usually yes |
| Bluetooth support | Optional HC-05 | No built-in equivalent | No direct equivalent in legacy design | No direct equivalent in legacy design |
| FSD toggle | Yes | Yes | Yes | Yes |
| Profile injection | Yes | Yes | Yes | Yes |
| HW3 speed offset | Yes | Yes | Yes | Yes |
| HW4 ISA suppression | Yes | Yes | Yes | Yes |
| Nag suppression | Yes | Yes | Yes | Yes |
| Stream/sniffer/history | Partial, protocol-driven | Yes | Yes | Yes |
| Raw CAN send | Limited today | Yes | Yes | Yes |
| OTA/update path | No | Some legacy support | Some legacy support | Some legacy support |
| Wi-Fi configuration | No | Yes | Yes | Yes |
| Setup complexity | Low-medium | High | Medium | Medium |
| Number of boards required | 1 main board + peripherals | 2 boards | 1 board + CAN board | 1 board |
| Fit with TeslaCANModder architecture | Native | Low | Medium | High long-term |

### Interpretation

- TeslaCANModder already covers the most important FSD/runtime behavior
- legacy Wi-Fi code adds board-hosted transport/server/dashboard behavior, not a fundamentally different FSD core
- the biggest missing capability in TeslaCANModder is Wi-Fi transport and board-side service features, not core CAN modification logic

## Architecture Delta Analysis

## A. UI ownership mismatch

### Legacy

- dashboard is embedded in firmware as HTML, CSS, and JavaScript
- board serves the UI itself

### Current

- dashboard lives in `web/`
- firmware only emits protocol messages

### Consequence

- legacy dashboard code is not reusable as React component code
- porting should happen at the behavior/API layer, not by copying HTML

## B. Transport mismatch

### Legacy

- HTTP endpoints
- AJAX polling
- Wi-Fi AP and server semantics
- UART bridge in the split-board design

### Current

- serial-style board session
- unified message model over USB or HC-05

### Consequence

Adding Wi-Fi later requires one of these choices:

1. keep TeslaCANModder's current message schema and add a Wi-Fi transport for it
2. add a translation layer in `web` that speaks legacy HTTP endpoints
3. support two protocol families

Only the first option matches the current architecture cleanly.

## C. Firmware structure mismatch

### Legacy

- one large `.ino`
- web server, state persistence, logs, CAN logic, and UI strings mixed together

### Current

- behavior split by layer and concern

### Consequence

- legacy code is valid as a feature reference
- legacy structure is not a good transplant target

## D. Monitoring mismatch

### Legacy

- stats endpoints
- sniffer buffers
- polling for log and history views

### Current

- board-pushed frame stream
- browser-side monitor aggregation

### Consequence

- sniffer/history ideas are reusable
- endpoint-polling monitor design is not

## Reuse Classification

## Compatible behavior to reuse

- FSD bit logic references
- profile mapping behavior
- HW3 speed offset semantics
- HW4 ISA suppression semantics
- sniffer and stats feature concepts
- raw CAN send concept
- Wi-Fi transport concept
- board-side stream/accounting concepts

## Requires redesign before reuse

- HTTP endpoints
- embedded dashboard HTML
- EEPROM-backed Wi-Fi settings flow
- UART bridge protocol
- OTA/update flow
- board-owned AJAX polling model

## Do not port directly

- giant inline HTML pages
- monolithic `.ino` structure
- duplicate board-specific UI logic
- any design that creates two permanent UI product families unless explicitly intended

## Recommendation

## Primary long-term recommendation: ESP32-S3 TWAI

Choose `ESP32-S3 TWAI` as the future Wi-Fi target if TeslaCANModder ever adds first-class Wi-Fi board support.

### Why

- single board
- native CAN
- built-in Wi-Fi
- avoids MCP2515 dependency
- fewer wiring points
- cleanest long-term board story

### Important limitation

- not the easiest short-term port from the current Uno build
- requires new driver/platform work

## Secondary pragmatic recommendation: ESP32 + MCP2515

Choose `ESP32 + MCP2515` first only if the goal is a lower-risk stepping-stone that preserves more current MCP2515 assumptions.

### Why

- closer to current Uno + MCP2515 architecture
- easier driver migration path than TWAI
- still viable for Wi-Fi transport work

### Limitation

- keeps the external CAN controller dependency
- weaker long-term simplification than ESP32-S3 TWAI

## Not recommended as the primary product path

- `RP2040 + ESP32-C3 bridge`
- `ESP8266`
- board-hosted legacy dashboards as the main product UI

The RP2040 bridge is technically interesting, but it increases flashing complexity, wiring complexity, and product complexity. It is better treated as reference material than as TeslaCANModder's default future direction.

## Migration Options

## Option 1. Preserve current protocol and add Wi-Fi transport

### Shape

- board still speaks TeslaCANModder message schema
- Wi-Fi transport exposes that same logical board protocol
- `web` remains the only dashboard

### Complexity

- medium-high

### Architectural cleanliness

- highest

### UX consistency

- highest

### Maintenance risk

- lowest of the three options

### Recommendation

- recommended

## Option 2. Add a translation layer in `web`

### Shape

- `web` learns legacy endpoint shapes
- firmware protocol unification is deferred

### Complexity

- medium initially

### Architectural cleanliness

- medium-low

### UX consistency

- medium

### Maintenance risk

- high over time

### Recommendation

- acceptable only as a short proof-of-concept

## Option 3. Support dual UI/product families

### Shape

- keep current React app
- also keep board-hosted legacy dashboards

### Complexity

- high

### Architectural cleanliness

- lowest

### UX consistency

- lowest

### Maintenance risk

- highest

### Recommendation

- not recommended

## What Would Need To Change In TeslaCANModder Later

## Hardware and firmware

- add a new board target instead of mutating the Uno path into a mixed product
- add a Wi-Fi-capable transport implementation
- preserve the current logical board protocol where possible
- keep CAN logic separate from Wi-Fi transport/service code
- add board-side configuration only if it maps cleanly onto current state and command concepts

### If choosing ESP32 + MCP2515

- new MCP2515-capable ESP32 environment
- ESP32 serial/network transport implementation
- optional staged migration of current driver abstractions

### If choosing ESP32-S3 TWAI

- new TWAI driver path
- new board target and build setup
- clearer long-term board architecture, but larger initial delta

## Web

- add Wi-Fi transport selection without changing dashboard ownership
- keep the same dashboard semantics and monitor model
- avoid importing embedded HTML from legacy firmware
- optionally support manual host entry or discovery later

## Documentation

- keep current Uno flow as primary until Wi-Fi is truly supported
- document Wi-Fi boards as future/experimental until implemented
- separate:
  - supported now
  - legacy reference
  - future migration path

## Guide Structure

This is the recommended user-facing guide shape for future documentation updates.

## A. Current supported TeslaCANModder setup

Mark this section clearly as:

- supported now
- current source of truth

Cover only:

- Uno + MCP2515
- USB-first flash
- X179 power path
- CAN connection
- optional HC-05
- current React web app

## B. Legacy Wi-Fi board reference

Mark this section clearly as:

- reference only
- not current TeslaCANModder setup

For each legacy board family, explain:

- hardware used
- where the dashboard lives
- how transport works
- how it differs from TeslaCANModder today

## C. Future Wi-Fi migration path

Mark this section clearly as:

- future migration concept
- not implemented today

Explain:

- recommended future board path
- how transport would differ
- what would stay the same in the web app
- what setup steps would change

## Step Format For Each Guide Subsection

Use this same structure every time:

1. Purpose
2. Hardware involved
3. Connection model
4. What the user sees in UI
5. Exact difference versus current TeslaCANModder
6. Why this matters
7. If ported later, what would need to change

## Practical Comparison Tables To Include

- board architecture table
- connection method table
- flashing and firmware-count table
- CAN controller table
- dashboard location table
- TeslaCANModder compatibility table
- recommended-path table

Each table should answer one practical question only.

## Final Conclusion

The legacy Wi-Fi code is useful because it proves:

- the Tesla FSD CAN logic can live on Wi-Fi-capable boards
- stream, stats, sniffer, and board-side service features are valuable

But the legacy Wi-Fi code is not a clean architectural base for TeslaCANModder because:

- it owns the dashboard in firmware
- it uses different transport/API assumptions
- it mixes multiple concerns into monolithic sketches

The correct direction for TeslaCANModder is:

- keep the current modular firmware and external React app
- treat the legacy Wi-Fi sketches as behavioral and hardware reference material
- if Wi-Fi support is added, preserve TeslaCANModder's protocol and dashboard model instead of inheriting the old embedded-dashboard architecture
