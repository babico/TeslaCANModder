---
title: uhi22-tesla-crc
description: A pure-C analysis toolkit that reverse-engineers the CRC algorithm used in Tesla CAN bus messages (specifically IDs 0x22
category: legacy
folder: legacy
tags: [legacy, community, external]
author: uhi22
repo: tesla-crc
---

# uhi22-tesla-crc

## Overview

A pure-C analysis toolkit that reverse-engineers the CRC algorithm used in Tesla CAN bus messages (specifically IDs 0x229 and 0x249). The project proves that Tesla uses CRC-8/OPENSAFETY (polynomial 0x2F) with per-ID "magic byte" tables indexed by the alive counter. It also includes a sub-folder for Volkswagen MQB CAN CRC analysis.

## Technical Details

- **Platform**: Desktop (GCC / MinGW)
- **Language**: C
- **CAN Interface**: N/A (offline analysis of CAN log files)
- **License**: None (no LICENSE file present)

## Architecture

- `tesla-crc.c` — Standalone CRC analysis program. Hardcodes CAN 0x229 magic-byte table, creates messages for various gear-stalk positions (idle, full-down, park, full-up, half-down), and demonstrates CRC calculation via XOR of magic bytes and payload CRC.
- `tesla-crc-filereader.c` — Reads CAN log files (`249_sortedFile.txt`, `229_sortedFile.txt`), parses hex-formatted CAN frames, computes CRC-8/OPENSAFETY over the payload (with a virtual trailing 0x00 byte), XORs with the per-alive-counter magic byte, and validates against the logged CRC. Achieved 870/870 (0x249) and 178/178 (0x229) matches.
- `opensafety-crc8-demo.c` — Minimal demo of the CRC-8/OPENSAFETY algorithm (`generateCrc8Opensafety()`).
- `229_sortedFile.txt` / `249_sortedFile.txt` — Raw CAN log samples used for verification.
- `vag/` — Volkswagen MQB ESP_21 (0x0FD) CRC analysis with its own file reader and CSV logs.

## CAN Bus Integration

Directly analyzes Tesla CAN message IDs:

- **0x229** — 3-byte gear-stalk message (CRC + alive counter + payload). Button bits encode full-down (0x40), full-up (0x20), half-down (0x30), park (byte 2 bit 0).
- **0x249** — 4-byte message with same CRC scheme.

Algorithm: CRC-8/OPENSAFETY computed over payload bytes (high nibble of byte 1 through a virtual trailing 0x00), then XOR'd with a per-message-ID magic byte table indexed by the 4-bit alive counter.

## Relevance to Our Project

This is directly relevant — our firmware must compute correct CRCs for any CAN frames it modifies or injects. The proven CRC-8/OPENSAFETY algorithm and magic-byte-table approach documented here is the same scheme used in our handlers.

- **Reusability**: High
- **Key Takeaways**:
  - Tesla uses CRC-8/OPENSAFETY (poly 0x2F, init 0x00, no final XOR)
  - Alive counter is NOT included in CRC input; instead it indexes a 16-entry magic-byte table unique to each CAN ID
  - A virtual 0x00 byte is appended after the last CAN byte before CRC calculation
  - The magic-byte table can be extracted from idle-state log captures
