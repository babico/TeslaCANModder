---
title: Legacy Index
description: Archived upstream and community Tesla CAN projects used for research and comparison
category: legacy
order: 1
---

# Legacy Repository Index

`legacy/` is a research archive. These repositories are retained for reverse-engineering notes, comparison work, and feature archaeology. They are not part of the active release pipeline.

## How To Use This Section

- Start with the high-relevance repositories below for active protocol and firmware comparison work.
- Use the per-repo markdown files in this folder as normalized summaries.
- Treat archived code as reference material, not as a drop-in source of truth for the active firmware.

## High-Relevance Repos (Phase 9 Feature Mining)

| Submodule | Author | License | Hardware | Key Innovation |
| --------- | ------ | ------- | -------- | ------------- |
| [hypery11-flipper-tesla-fsd](hypery11-flipper-tesla-fsd.md) | hypery11 | GPL-3.0 | Flipper Zero, ESP32 | 0x7FF Ban Shield, TLSSC Restore (0x331), ban detection, MCP2515 8MHz |
| [slxslx-tesla-open-can-mod-slx-repo](slxslx-tesla-open-can-mod-slx-repo.md) | slxslx | GPL-3.0 | ESP32, RP2040, M5Stack, Feather M4 | 8+ board variants, web dashboard, OTA, MCP2515 8MHz crystal |
| [Shayennn-FUCKYOU-TESLA-FSD](Shayennn-FUCKYOU-TESLA-FSD.md) | Shayennn | GPL-3.0/MIT | Feather M4, ESP32, ESP32-IDF | Shared `vehicle_logic.h`, CI + sanitizer tests, ESP-IDF v6.0 |
| [EzeLLM-fsd-spoofing](EzeLLM-fsd-spoofing.md) | EzeLLM | GPL-3.0 | Nano, ESP32, RP2040 | Wiring guides, Canada workaround, $15 Nano build |

## Category Overview

- FSD / CAN mod forks
- CAN analysis and logging tools
- BMS and battery projects
- Dashboards and client apps
- Tesla API, BLE, and related utilities

Use [Legacy Comparison](COMPARISON.md) for cross-project synthesis.

## Related Docs

- [Legacy Comparison](COMPARISON.md)
- [Root Documentation Index](../README.md)
