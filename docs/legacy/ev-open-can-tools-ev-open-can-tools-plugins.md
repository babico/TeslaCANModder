---
title: ev-open-can-tools / ev-open-can-tools-plugins
description: JSON plugin files for the ev-open-can-tools plugin engine. Not relevant to our firmware — we compile features directly into the binary rather than using a runtime plugin system.
category: legacy
folder: legacy
tags: [legacy, community, external]
author: ev-open-can-tools
repo: ev-open-can-tools-plugins
---

# ev-open-can-tools / ev-open-can-tools-plugins

## Overview

JSON plugin files for the ev-open-can-tools runtime plugin engine. Plugins are installed via the dashboard without firmware recompilation. Not relevant to our project — we compile features directly into the firmware binary via build flags and NVS-backed runtime toggles.

The plugins are kept as a submodule for reference only. The CAN signal targets they encode (bit positions, frame IDs) are useful as a cross-check against our own handler implementations.

## Available Plugins (signal reference only)

| Plugin | HW | CAN target |
|--------|----|-----------|
| `ad-activation-hw3/hw4.json` | Both | 0x3FD mux-0 bit 46 (FSD), bit 60 (FSDv14) |
| `bypass-tlssc-hw3/hw4.json` | Both | 0x331 byte 0 bits 5-0 = 0x1B |
| `summon-eu-unlock-hw3/hw4.json` | Both | 0x3FD mux-0 summon bits |
| `isa-chime-suppress-hw4.json` | HW4 | 0x399 byte 1 bit 5 |
| `emergency-vehicle-detection-hw4.json` | HW4 | 0x3FD mux-0 bit 59 |
| `hw4-speed-offset-plus-5/7/10/15.json` | HW4 | 0x3FD mux-2 byte 1 bits 5-0 |

## Relevance to Our Project

- **Reusability**: Low — plugin JSON format is specific to ev-open-can-tools engine
- **Key Takeaways**:
  - EVD bit 59 on 0x3FD mux-0 confirmed (not yet in our codebase)
  - Speed offset field on 0x3FD mux-2 byte 1 bits 5-0 confirmed (our `offsets.h` is a stub)
  - All other targets already implemented in our firmware
