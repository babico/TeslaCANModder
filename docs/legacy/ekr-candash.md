---
title: ekr-candash
description: CANdash is an Android app that turns a phone or tablet into an instrument cluster for Tesla Model 3/Y. It connects to a 
category: legacy
folder: legacy
tags: [legacy, community, external]
author: ekr
repo: candash
---

# ekr-candash

## Overview

CANdash is an Android app that turns a phone or tablet into an instrument cluster for Tesla Model 3/Y. It connects to a CANserver device over Wi-Fi to receive real-time CAN data and displays a dashboard with speed, power meter, battery SOC, blind spot monitoring (via ultrasonic sensors), and other vehicle metrics. Designed to be mounted as an always-on dash display.

## Technical Details

- **Platform**: Android (phone/tablet)
- **Language**: Java/Kotlin (Android Gradle project)
- **CAN Interface**: Indirect — connects to a CANserver (jwardell.com) over Wi-Fi, which bridges CAN bus to network
- **License**: GPL v3

## Architecture

- `android/` — Standard Android Gradle project
  - `android/app/` — Main application module
  - `android/build.gradle` — Root build configuration
  - `android/gradle/` — Gradle wrapper
- The app connects to a CANserver via Wi-Fi (either CANserver hotspot or phone hotspot)
- Auto-discovers CANserver IP via scanning
- Displays real-time telemetry in a custom dashboard view

## CAN Bus Integration

Indirect CAN integration via CANserver hardware bridge:

- Requires a jwardell CANserver physically connected to the vehicle's CAN bus (dual bus version for blind spot monitoring)
- CANserver bridges CAN data to Wi-Fi network
- App receives decoded CAN signals over the network (not raw CAN frames)
- Displays: speed, power (kW/HP with min/max tracking), battery SOC (% or distance), blind spot alerts (from rear ultrasonic sensors), turn signal + blind spot camera data, night mode following car settings
- No direct CAN frame parsing in the app itself — the CANserver handles decoding

## Relevance to Our Project

Moderately relevant — demonstrates a polished CAN data visualization approach for Tesla vehicles, but the CAN interaction is abstracted behind the CANserver hardware.

- **Reusability**: Medium
- **Key Takeaways**:
  - UI/UX design patterns for a Tesla instrument cluster display
  - CANserver as a CAN-to-Wi-Fi bridge architecture
  - Blind spot monitoring implementation using ultrasonic sensor CAN data
  - Auto-launch via Tasker for always-on vehicle displays
  - Dual-bus CANserver needed for blind spot data (separate CAN buses for different vehicle systems)
