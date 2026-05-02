---
title: Unified Feature Implementation Checklist
description: Consolidated execution checklist with detailed tasks, dependencies, and acceptance criteria
category: checklists
folder: checklists
tags: [checklist, firmware, roadmap, implementation]
order: 90
---

# Unified Feature Implementation Checklist

This document tracks implementation work for features and improvements identified from:

- hypery11/flipper-tesla-fsd
- ev-open-can-tools/ev-open-can-tools

## Usage

- Mark task checkbox when implementation is complete and validated.
- Do not mark complete until acceptance criteria and tests both pass.
- Keep this file as the source of truth for implementation progress.

## Progress Snapshot

- [x] Create unified implementation checklist
- [x] Start AP Injection Gate scaffold (state + status + basic commands)
- [x] Complete AP Injection Gate enforcement and persistence (implementation + validation complete)
- [x] AP Injection Gate dashboard controls and WiFi tests complete

## Master Checklist

### Phase 1 - Safety and Injection Control

- [x] P1-01 Add AP gate runtime fields and open predicate
- [x] P1-02 Add AP gate serial commands (on/off/status)
- [x] P1-03 Expose AP gate in status payloads (serial and REST)
- [x] P1-04 Seed parked and summon signals in runtime loop
- [x] P1-05 Enforce AP gate on write/injection paths
- [x] P1-06 Persist AP gate setting in NVS
- [x] P1-07 Add AP gate dashboard controls
- [x] P1-08 Add AP gate tests (commands, transitions, dispatch)

### Phase 2 - Controlled Parity Additions

- [x] P2-01 Add assist nav-enable toggles (0x3F8 bits 13, 48, 49)
- [x] P2-02 Add assist hands-off toggle (0x3F8 bit 14)
- [x] P2-03 Add assist dev-mode toggle (0x3F8 bit 5)
- [x] P2-04 Add lane-graph toggle (0x3FD mux1 bit 45)
- [x] P2-05 Add telemetry-off experimental toggle (0x3F8 bit 43)
- [x] P2-06 Add fallback variant detection when 0x398 is absent
- [x] P2-07 Improve Legacy to HW3 runtime transition resilience
- [x] P2-08 Add/refresh tests for all parity toggles and transitions

### Phase 4 - Dashboard and Diagnostics

- [x] P4-01 Add AP gate control section in dashboard
- [x] P4-02 Add CAN recorder (start/stop/download)
- [x] P4-03 Add hidden AP setting and persistence
- [x] P4-04 Add write probe (TX to RX confirmation)
- [x] P4-05 Add dashboard UX warnings for high-risk operations

### Phase 5 - Documentation and Release Gates

- [x] P5-01 Update command reference for all new commands
- [x] P5-02 Update CAN reference for newly used frame bits and IDs
- [x] P5-03 Update security guide for experimental toggles
- [x] P5-04 Run native firmware tests
- [x] P5-05 Run esp32_wifi and esp32_wifi_ble build checks
- [x] P5-06 Add release checklist entry for AP gate and high-risk features

## Detailed Task Descriptions

## P1-01 AP Gate State Model

Status: Implemented.

Description: Add AP gate runtime fields and open predicate in firmware state.
Dependencies: None.
Deliverables: State fields for enabled, AP-active, parked, summoning, and computed open state.
Acceptance criteria: Status payloads can report AP gate state, and gate open logic is deterministic and side-effect free.

## P1-02 AP Gate Commands

Status: Implemented.

Description: Add serial command surface for AP gate toggling and introspection.
Dependencies: P1-01.
Deliverables: apgate:on, apgate:off, apgate:status.
Acceptance criteria: Commands acknowledge success and update state immediately, and the status command reports enabled/open/reason signals.

## P1-03 AP Gate Status Exposure

Status: Implemented.

Description: Expose AP gate values in boot/status/state/compact outputs and REST status.
Dependencies: P1-01.
Deliverables: Additional JSON fields in serial and WiFi routes.
Acceptance criteria: Fields are present in all expected payloads, and values remain consistent across serial and REST.

## P1-04 AP/Park/Summon Runtime Signal Wiring

Status: Implemented (initial).

Description: Seed parked and summon runtime signals from current telemetry and summon lifecycle.
Dependencies: P1-01.
Deliverables: Parked signal from DI gear decode, summon signal from summon state.
Acceptance criteria: Parked becomes true for Park and standby or silent fallback, and summon state reflects burst lifecycle start and stop.

## P1-05 AP Gate Write-Path Enforcement

Status: Implemented and validated.

Description: Enforce AP gate constraints before transmitting mutation frames.
Dependencies: P1-01, P1-04.
Deliverables: Central gate check used by feature write paths.
Acceptance criteria: No feature injection occurs while the gate is closed, and AP, Park, and Summon gate-open reasons are respected.

## P1-06 AP Gate Persistence

Status: Implemented and validated.

Description: Persist AP gate enabled flag in NVS and restore on boot.
Dependencies: P1-02.
Deliverables: Persist/load wiring and defaults.
Acceptance criteria: The AP gate setting survives reboot, and corrupt or absent NVS entries fall back safely.

## P1-07 AP Gate Dashboard Controls

Status: Implemented and validated.

Description: Add dashboard controls and indicators for AP gate.
Dependencies: P1-03, P1-06.
Deliverables: UI toggle + live status display.
Acceptance criteria: The toggle updates the backend immediately, and dashboard state mirrors serial status.

## P1-08 AP Gate Test Pack

Status: Implemented and validated.

Description: Add unit and dispatch tests for AP gate behavior.
Dependencies: P1-05.
Deliverables: tests for command path, transitions, and enforcement.
Acceptance criteria: Tests cover AP-only, Park-only, and Summon-only openings, plus waiting-state and standby transitions.

## P2-01 to P2-08 Controlled Parity Work

Status: Implemented and validated.

Description: Add high-value parity toggles and detection reliability improvements.
Dependencies: Phase 1 complete.
Deliverables: Assist toggles for nav, hands-off, dev-mode, and lane-graph; an experimental telemetry-off toggle; and fallback-detection and transition-resilience fixes.
Acceptance criteria: Bit-level modifications are validated by tests on the correct mux and variant, and experimental features are off by default and documented.

## P4-01 to P4-05 Dashboard and Diagnostics Expansion

- [x] Completed

Description: Add operator-facing tooling around the AP gate plus recorder/probe utility.
Dependencies: Phase 2 complete.
Deliverables: Dashboard AP gate controls, recorder and write-probe tools, and the hidden AP toggle with warnings.
Acceptance criteria: Tools are non-blocking and recover from reconnect, and recorder output is downloadable and bounded.

## P5-01 to P5-06 Release Gates

Status: Completed.

Description: Finalize docs, tests, and release readiness checks.
Dependencies: Completed implementation phases.
Deliverables: Updated docs and references, recorded test and build results, and release checklist updates.
Acceptance criteria: There are no unresolved TODOs for shipped features, and validation commands pass for required targets.

## Notes

This checklist is intentionally detailed so each item can become a standalone issue if needed.
Keep implementation checkmarks synchronized with merged code and passing tests.
