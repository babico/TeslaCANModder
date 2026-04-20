---
title: LeeGaHyeon-tesla_CAN_traffic_decode
description: A Python-based CAN data decoder that parses raw CAN bus CSV captures from a Tesla Model 3 using the Model3CAN.dbc file. 
category: legacy
folder: legacy
tags: [legacy, community, external]
author: LeeGaHyeon
repo: tesla_CAN_traffic_decode
---

# LeeGaHyeon-tesla_CAN_traffic_decode

## Overview

A Python-based CAN data decoder that parses raw CAN bus CSV captures from a Tesla Model 3 using the Model3CAN.dbc file. It reads DBC signal definitions, matches them against captured CAN traffic, extracts physical values using bit-level decoding (little-endian), and outputs decoded results. Uses multiprocessing and numba JIT for performance.

## Technical Details

- **Platform**: Desktop (Python)
- **Language**: Python 3
- **CAN Interface**: N/A — processes offline CSV captures, does not interface with CAN hardware directly
- **License**: None (no LICENSE file present)

## Architecture

- `[VERIFIED][1101]AUTO_DBC_PARSER.py` — Main script implementing:
  - `load_dbc_file()` — Parses DBC file to extract message IDs, DLCs, signal definitions, and comment metadata
  - `signal_normalization()` — Converts DBC signal text into structured data (name, bit position, length, endianness, scale, offset, min, max, unit)
  - `intel_convert_bit_CAN()` — Converts hex CAN data to bit arrays (little-endian byte reversal)
  - `calculate_pysical()` — Computes physical values from raw bits using DBC scale/offset formulas
  - Uses `multiprocessing.Pool` and `numba.jit` for parallel processing of large CSV datasets
- `Model3CAN.dbc` — Copy of Josh Wardell's Tesla Model 3 DBC file

The script processes CSV files organized in folders by driver name, producing `[Decode]` prefixed output files.

## CAN Bus Integration

No direct CAN hardware integration. Processes offline CAN captures:

- Input: CSV files with CAN frame data (hex format)
- DBC parsing extracts: CAN ID, signal name, bit start position, bit length, endianness (little-endian only), scale factor, offset, min/max, unit
- Bit-level extraction reverses byte bit ordering for little-endian signals, then applies `physical_value = raw_value * scale + offset`
- Only little-endian (Intel byte order) signals are processed; big-endian is not implemented

## Relevance to Our Project

Provides a custom DBC parser implementation in Python — useful as a reference for understanding DBC signal extraction at the bit level, though the `cantools` library (used in other repos) is a more robust alternative.

- **Reusability**: Low
- **Key Takeaways**:
  - Manual DBC parsing and bit-level signal extraction implementation
  - Little-endian CAN signal decoding with byte reversal pattern
  - Physical value calculation: `raw * scale + offset`
  - Performance optimization with multiprocessing and numba JIT
  - Better to use `cantools` library for production DBC parsing
