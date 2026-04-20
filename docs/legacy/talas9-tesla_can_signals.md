---
title: talas9-tesla_can_signals
description: A collection of Tesla CAN bus signal definitions in JSON format covering Model 3, Model Y, Model S, and Model X. Include
category: legacy
folder: legacy
tags: [legacy, community, external]
author: talas9
repo: tesla_can_signals
---

# talas9-tesla_can_signals

## Overview

A collection of Tesla CAN bus signal definitions in JSON format covering Model 3, Model Y, Model S, and Model X. Includes compact DBC-like signal databases and legacy self-test parser definitions for each vehicle model. This is a data-only repository with no executable code.

## Technical Details

- **Platform**: N/A (data files only)
- **Language**: JSON
- **CAN Interface**: N/A
- **License**: None

## Architecture

The repo is organized by vehicle model:

- `Model3/` — Contains `Model3_ETH.compact.json` and `legacy_self_test_parser.json`
- `ModelY/` — Contains `ModelY_ETH.compact.json` and `legacy_self_test_parser.json`
- `ModelS/` — Contains multiple bus-specific JSON files (BDY, BFT, CH, ETH, OBDII, PT, PT_BMSDBG, TH) and self-test parser
- `ModelX/` — Same structure as ModelS with bus-specific compact JSON files and self-test parser

The compact JSON files contain CAN message and signal definitions. The `legacy_self_test_parser.json` files contain Tesla factory self-test definitions with metric names, descriptions, and associated tasks (e.g., seat heat, occupancy sensors, armrest latches).

## CAN Bus Integration

The JSON files define CAN signal databases for Tesla vehicles — message IDs, signal names, bit positions, factors, offsets, and units. They serve as reference data for decoding CAN bus traffic, similar to DBC files but in JSON format. Covers multiple CAN buses on Model S (BDY, BFT, CH, ETH, OBDII, PT).

## Relevance to Our Project

Provides comprehensive CAN signal definitions that can be used as a reference database for decoding Tesla CAN messages across multiple vehicle models and bus types.

- **Reusability**: High
- **Key Takeaways**:
  - Multi-model CAN signal database (3/Y/S/X)
  - Model S has the most detailed coverage with separate bus-specific databases
  - Legacy self-test parser data could be useful for diagnostics features
  - JSON format is easy to ingest programmatically
