---
title: amzoo-TMS_2016_LOGS
description: A collection of raw CAN bus log files captured from a 2016 Tesla Model S (AP1, single motor). The logs cover normal driv
category: legacy
folder: legacy
tags: [legacy, community, external]
author: amzoo
repo: TMS_2016_LOGS
---

# amzoo-TMS_2016_LOGS

## Overview

A collection of raw CAN bus log files captured from a 2016 Tesla Model S (AP1, single motor). The logs cover normal driving scenarios and Summon mode operations, recorded with SavvyCAN via an EVTV ESP32_CAN adapter on CAN6 (Chassis) and CAN3 (Powertrain) buses.

## Technical Details

- **Platform**: SavvyCAN v199 on macOS, EVTV ESP32_CAN logger
- **Language**: N/A (data files only — CSV format)
- **CAN Interface**: EVTV ESP32_CAN adapter connected to CAN6 (Chassis) and CAN3 (Powertrain)
- **License**: None

## Architecture

```mermaid
flowchart LR
    Car["2016 Model S (AP1)"] --> EVTV["EVTV ESP32_CAN<br/>(CAN6 + CAN3)"]
    EVTV --> Logs["Raw CAN logs<br/>(CSV, SavvyCAN)"]
    Logs --> Files["Driving + Summon scenarios"]
    classDef path fill:#1e3a5f,stroke:#4a7fb5,color:#fff
    class Logs,Files path
```
Pure data repository with no code:

```
drive/
  drive1_dirty.csv
  drive2_warnings.csv
  drive3_highway.csv
  drive4_highway.csv
  drive5_stopandgo.csv
  drive6_stopanggo_clean.csv

summon/
  summon1_fwd_short_car.csv   ... summon10_return_short_fob.csv
  (19 files covering forward/backward/return summon at various distances,
   triggered from car or key fob)
```

## CAN Bus Integration

No active CAN integration — this is passive log data. However, the logs contain:

**Vehicle details:**

- 2016 Model S, AP1, single motor, VIN prefix 5YJSA1E18GF1
- Features: Summon, Autosteer (intermittent), Adaptive Cruise

**Bus configuration:**

- Bus 0: CAN6 Chassis
- Bus 1: CAN3 Powertrain

**Drive logs:** 6 CSV files covering dirty/warning/highway/stop-and-go driving scenarios
**Summon logs:** 19 CSV files covering short/long/max distance forward/backward/return summon operations, triggered from both car and key fob, including abort and warning scenarios

## Relevance to Our Project

Useful as reference CAN data for understanding Tesla Model S (AP1) bus traffic patterns, especially Summon mode behavior.

- **Reusability**: Low
- **Key Takeaways**:
  - Documents CAN6 (Chassis) and CAN3 (Powertrain) bus layout for Model S
  - Summon mode CAN traffic across multiple scenarios could help decode Summon-related message IDs
  - Older AP1 vehicle — CAN structure may differ from Model 3/Y HW3/HW4
  - CSV format compatible with SavvyCAN for replay and analysis
