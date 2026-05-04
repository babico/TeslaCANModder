---
title: alzza/tesla-can-moniter
description: LILYGO T-Display-S3 ESP-NOW receiver that displays Tesla CAN telemetry on a TFT screen. Companion to a CAN-bus transmitter (OBD2 or piggyback device).
category: legacy
folder: legacy
tags: [legacy, community, external, monitor, esp-now, display]
author: alzza
repo: tesla-can-moniter
---

# alzza/tesla-can-moniter

## Overview

A LILYGO T-Display-S3 (ESP32-S3) wireless display companion for Tesla CAN bus monitoring. Receives a packed 60-byte telemetry struct over ESP-NOW from a separate transmitter and renders a 4-page touchscreen UI showing EAP state, nag killer activity, per-bus frame rates, EPAS torque, and bus health counters.

## Technical Details

- **Platform**: LILYGO T-Display-S3 (ESP32-S3, 16 MB Flash, 8 MB PSRAM), TFT_eSPI ST7789 170×320 px
- **Language**: C++ (Arduino/PlatformIO)
- **CAN Interface**: None directly — receives decoded CAN telemetry over ESP-NOW WiFi (channel 1)
- **Transmitter side**: Separate device (CAN-to-ESP-NOW bridge) that reads Tesla CAN buses and packs data into `struct_message` (60 bytes)
- **Libraries**: `bodmer/TFT_eSPI`, `espressif/esp_now` (via Arduino-ESP32)
- **License**: Not specified

## Architecture

The project is a single-binary receiver:

- **`src/main.cpp`** — Setup + loop. Calls `initEspNowReceiver()`, `ensureReceiverChannel()`, `decodeTelemetryPacket()`, and `uiRender()`. Manages button state machine for page navigation (top/bottom buttons on T-Display-S3). Uses `Preferences` NVS to persist brightness and page across reboots.

- **`src/ui.h`** — `UiState` struct: the decoded telemetry fields used by the renderer:
  - `uptime`, `hzA`, `hzB` — uptime seconds, frame rate per CAN bus
  - `nag`, `eap`, `nagMode` — nag-killer active, EAP active, mode string
  - `twaiState`, `echoCount`, `txFailCount` — bus health, TX diagnostics
  - `aFramesTotal`, `aFrames1021`, `aEapModified` — A-bus (Chassis) frame counters
  - `bFramesTotal`, `bFrames880`, `bFrames921`, `bBusoffCount` — B-bus (Vehicle) frame counters
  - `torqueNm`, `stealthTorqueNm` — EPAS torque values (injected vs. natural)
  - `linked` — ESP-NOW link state

- **`src/ui.cpp`** — 4-page renderer using TFT_eSPI:
  - **Page 0** (Main): B-bus frame rate Hz, EPAS torque, stealth torque, nag/EAP status
  - **Page 1** (A-channel): A-bus Hz, total frames, `aFrames1021` (EAP frame count), `aEapModified`, uptime
  - **Page 2** (B-channel): B-bus Hz, total frames, `bFrames880` (EPAS/nag frames), `echoCount`, TWAI state, bus-off count
  - **Page 3** (System): CPU MHz, WiFi/BT state, brightness, heap, uptime

## Telemetry Packet Format (`struct_message`, 60 bytes)

```cpp
struct struct_message {
    uint32_t uptime;          // seconds since boot
    float    hzA;             // Chassis bus frame rate
    float    hzB;             // Vehicle bus frame rate
    uint8_t  nag;             // nag killer active
    uint8_t  eap;             // EAP mode active
    char     nagMode[8];      // "LEGACY", "NATURAL", etc.
    uint8_t  twaiState;       // 0=init, 1=ok, 2=bus_off, 3=recover
    uint32_t echoCount;       // nag killer echo frames sent
    uint32_t txFailCount;     // TX failures
    uint32_t aFramesTotal;    // Chassis total frames
    uint32_t aFrames1021;     // Chassis 0x3FD (EAP) frames
    uint32_t aEapModified;    // Chassis EAP frames we modified
    uint32_t bFramesTotal;    // Vehicle total frames
    uint32_t bFrames880;      // Vehicle 0x370 (EPAS nag) frames
    uint32_t bFrames921;      // Vehicle 0x399 (BSM) frames
    uint32_t bBusoffCount;    // Vehicle bus-off events
    float    torqueNm;        // EPAS torque set (Nm)
    float    stealthTorqueNm; // Natural torque computed (Nm)
    uint8_t  linked;          // ESP-NOW link alive
};  // 60 bytes total; compat check at >=52, legacy at >=22
```

## CAN Bus Integration

No direct CAN access — all CAN work is done by a separate transmitter device. The monitor receives pre-decoded metrics over ESP-NOW. Key CAN frame IDs tracked by the transmitter and surfaced in the UI:

| CAN ID | Bus         | Signal               | Field                         |
| ------ | ----------- | -------------------- | ----------------------------- |
| 0x3FD  | A (Chassis) | EAP FSD mux frame    | `aFrames1021`, `aEapModified` |
| 0x370  | B (Vehicle) | EPAS nag killer echo | `bFrames880`, `echoCount`     |
| 0x399  | B (Vehicle) | Blind spot monitor   | `bFrames921`                  |

## Relevance to Our Project

This repo provided the inspiration for CAN diagnostic counters added to our firmware in the same session as the submodule add. Specifically, the `UiState` field set defined the complete set of per-bus metrics worth tracking:

- **`canNagEchoCount`** — wired into `board.h` dispatch when nag killer echo fires on 0x370
- **`canEapModCount`** — wired into `hw4.h`, `hw3.h`, `legacy.h` at mux=1 nag-suppress path
- **`canTxFailCount`** — accumulated from `driverSend()` failures (MCP2515 `sendMessage` non-OK return)
- **`canBusOffCount`** — polled via `driverPollBusErrors()` reading MCP2515 `EFLG_TXBO`
- **`canFrames[3]`** — total frames per bus, incremented in `handleMessage` for each received frame
- **`canFrameRateHz[3]`** — rolling 1-second window frame rate per bus

All counters are exposed in `can` object of `sendBoot` and `sendStatus` serial output messages.

- **Reusability**: Medium — diagnostic field set directly ported to our State struct; ESP-NOW TX pattern intentionally excluded (we have no T-Display-S3 in our hardware stack)
- **Key Takeaways**:
  - The 60-byte packed telemetry struct is a clean reference for which CAN-layer metrics are worth exposing for real-time diagnostics
  - TWAI bus-off detection and auto-recovery pattern is directly applicable to MCP2515 (EFLG_TXBO bit)
  - Per-ID frame counting (not just total) gives more targeted visibility into feature-critical frames (0x370, 0x3FD, 0x399)
  - Separate Hz calculation with a 1-second sliding window avoids noisy single-sample fluctuation
  - The `stealthTorqueNm` vs `torqueNm` split shows the value of tracking both injected and computed torque separately
