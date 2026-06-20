---
title: mikegapinski-tesla-can-explorer
description: An open-source web-based research portal for browsing decoded Tesla CAN frames and signal value maps. Contains comprehen
category: legacy
folder: legacy
tags: [legacy, community, external]
author: mikegapinski
repo: tesla-can-explorer
---

# mikegapinski-tesla-can-explorer

## Overview

An open-source web-based research portal for browsing decoded Tesla CAN frames and signal value maps. Contains comprehensive JSON datasets extracted from Tesla firmware libraries (`libQtCarCANData.so`, `libQtCarVAPI.so`) for both Model 3 (MCU2 Intel / MCU3 AMD) and Model S/X variants, covering firmware version 2026.2. Sponsored by the Tesla Android project.

## Technical Details

- **Platform**: Static web application (any HTTP server)
- **Language**: JavaScript, HTML, CSS
- **CAN Interface**: N/A (reference data only, no direct CAN interaction)
- **License**: 0BSD (BSD Zero Clause License)

## Architecture

- `app.js` — Main application logic: loads JSON datasets, implements search/filter/sort across frames, signals, enums, and VAPI aliases; supports MCU2, MCU3, and Model S/X data sources
- `index.html` — Single-page app with split-panel layout for frame list and signal details
- `styles.css` — Application styling
- `data/can_frames_decoded_all_values_mcu2.json` — Complete decoded CAN frames for Model 3 MCU2 (Intel)
- `data/can_frames_decoded_all_values_mcu3.json` — Complete decoded CAN frames for Model 3 MCU3 (AMD)
- `data/can_frames_decoded_all_values_modelsx_amd.json` — Model S/X MCU3 (AMD)
- `data/can_frames_decoded_all_values_modelsx_intel.json` — Model S/X MCU2 (Intel)
- `data/can_frames_decoded_all_values.json` — Combined/unified frame dataset (all variants)
- `data/vapi_can_digest_*.json` — VAPI CAN digest for each variant
- `data/vapi_can_digest.json` — Combined VAPI CAN digest
- `data/vapi_eth_signal_aliases_*.csv` — Ethernet signal alias mappings
- `data/can_frames_decoded_enum_values_*.csv` — Enum value CSV exports
- `enable_portal_source.sh` — Portal launch helper script

## CAN Bus Integration

No direct CAN bus interaction. This is a reference database containing decoded CAN frame definitions extracted from Tesla firmware binaries. The datasets include:

- Frame names, addresses, bus assignments, and module associations
- Signal definitions with bit positions, scaling factors, and enum value maps
- VAPI (Vehicle API) alias mappings between CAN signals and higher-level interfaces
- Coverage across Model 3 MCU2/MCU3 and Model S/X MCU2/MCU3 variants

## Relevance to Our Project

Extremely valuable reference resource. The decoded CAN frame datasets from firmware 2026.2 provide authoritative signal definitions that can validate and extend our own CAN protocol documentation. The VAPI digest files reveal how Tesla maps CAN signals to application-level interfaces.

- **Reusability**: High
- **Key Takeaways**:
  - Comprehensive decoded CAN frame database from Tesla firmware 2026.2
  - Covers Model 3 (MCU2/MCU3) and Model S/X (MCU2/MCU3) variants
  - VAPI CAN digest reveals Tesla's internal signal-to-API mapping
  - Extracted from `libQtCarCANData.so` and `libQtCarVAPI.so`
  - 0BSD license allows unrestricted use
  - Ethernet signal alias CSVs show CAN-to-Ethernet bridge mappings

## Upstream (2026-06-20)

Upstream GitHub repo returns **404** (deleted or renamed). Submodule entry commented out in `.gitmodules`; `[submodule]` section removed from `.git/config`; entry removed from `.git/modules/`. Working tree files preserved for local reference. The decoded CAN datasets from firmware 2026.2 are still usable locally even without the live repo. Re-check the URL periodically in case the repo is restored. See `docs/legacy/upstream-review-2026-06-20.md`.
