---
title: tesla-fsd.netlify.app
description: A local mirror and extraction of the tesla-fsd.netlify.app website, which serves as a guide and firmware distribution si
category: legacy
folder: legacy
tags: [legacy, community, external]
author: tesla
repo: fsd.netlify.app
---

# tesla-fsd.netlify.app

## Overview

A local mirror and extraction of the tesla-fsd.netlify.app website, which serves as a guide and firmware distribution site for Tesla FSD CAN bus enabler boards. Contains decoded firmware source files for multiple board variants (ESP8266, ESP32, ESP32-S3, ESP32-C3, RP2040, Arduino UNO), board-to-code mappings, and rendered documentation. Includes a PowerShell script to rebuild the mirror from the live site.

## Technical Details

- **Platform**: Multiple (ESP8266, ESP32, ESP32-S3, ESP32-C3, RP2040, Arduino UNO)
- **Language**: C++ (Arduino .ino files), HTML/CSS (site), PowerShell (mirror script)
- **CAN Interface**: Various (MCP2515, ESP32 TWAI, ESP8266 SPI)
- **License**: None

## Architecture

- `index.html` — Full site mirror with Turkish-language documentation, GitHub-styled UI
- `codes/` — Decoded firmware files extracted from base64 blobs embedded in the site:
  - `CanFeather_ESP8266_WiFi.ino` (1656 lines)
  - `CanFeather_ESP32_WiFi.ino` (1771 lines)
  - `CanFeather_RP2040.ino` (1093 lines)
  - `ESP32C3_WiFiBridge.ino` (769 lines)
  - `CanFeather_ESP32S3_TWAI.ino` (1676 lines)
  - `CanFeather_ArduinoUno.ino` (649 lines)
  - `CanFeather_ESP32_LITE.ino` (459 lines)
- `boards/` — Per-board HTML/text exports
- `markdown/` — Structured markdown summaries of each board
- `board-code-map.csv` — Maps board IDs to firmware code IDs
- `recreate_site_mirror.ps1` — PowerShell script to re-scrape and rebuild all artifacts
- `SKILL.md` — Instructions for the mirror rebuild process

## CAN Bus Integration

The decoded firmware files contain the same FSD CAN enabler logic adapted for different hardware platforms. Each variant handles CAN IDs 1006/1016/1021 for FSD activation. The WiFi-enabled variants (ESP8266, ESP32, ESP32-S3) add OTA update and web dashboard capabilities. The ESP32-C3 variant serves as a WiFi bridge for the RP2040 Feather.

## Relevance to Our Project

Provides extracted source code for every board variant of the FSD enabler, making it easy to compare implementations across platforms. The board-code mapping and markdown summaries are useful reference material.

- **Reusability**: Medium
- **Key Takeaways**:
  - Comprehensive collection of FSD enabler firmware for 6+ board variants
  - WiFi-enabled variants show how to add OTA and web dashboards to CAN devices
  - ESP32-C3 WiFi bridge pattern (RP2040 handles CAN, ESP32-C3 handles WiFi)
  - Board-code-map.csv provides a useful cross-reference
  - Turkish-language documentation targets a different community
