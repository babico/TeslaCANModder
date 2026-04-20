# Legacy Feature Comparison Matrix

Compares our project (Tesla-CAN-Mod) against the four highest-relevance legacy repositories.

## Feature Matrix

| Feature | Tesla-CAN-Mod | hypery11 | slxslx | Shayennn | EzeLLM |
| ------- | ------------ | -------- | ------ | -------- | ------ |
| **FSD Enable (bit46)** | ✅ | ✅ | ✅ | ✅ | ✅ |
| **FSD V14 (bit60, HW4)** | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Nag Killer (0x370 echo)** | ✅ | ✅ | ✅ | ✅ | — |
| **Speed Profiles (5 levels HW4)** | ✅ | ✅ | ✅ | ✅ | ✅ |
| **ISA Chime Suppress (0x399)** | ✅ | ✅ | ✅ | — | — |
| **Summon Inject** | ✅ | — | ✅ (ASS) | — | — |
| **Seatbelt Emulation** | ✅ | — | — | — | — |
| **Ban Shield (0x7FF)** | ✅ | ✅ | — | — | — |
| **TLSSC Restore (0x331)** | ✅ (basic) | ✅ (full) | — | — | — |
| **Ban Detection (0x7FF mux2)** | — | ✅ | — | — | — |
| **Natural Nag (Gaussian jitter)** | — | — | — | — | — |
| **ALC Auto-Confirm** | — | — | — | — | — |
| **Emergency Vehicle Detect** | — | ✅ | ✅ | — | — |
| **Battery Precondition (0x082)** | ✅ | ✅ | — | — | — |
| **Drive Mode Control** | ✅ | — | — | — | — |
| **Mirror Autofold** | ✅ | — | — | — | — |
| **Trunk/Frunk Control** | ✅ | — | — | — | — |
| **Window Control** | ✅ | — | — | — | — |
| **Turn Signals** | ✅ | — | — | — | — |
| **Wiper Persist** | ✅ | — | — | — | — |
| **Track Mode** | ✅ | ✅ | — | — | — |
| **Climate/HVAC** | ✅ | — | — | — | — |
| **Charge Control** | ✅ | — | — | — | — |
| **MQTT Bridge** | ✅ | — | — | — | — |
| **BMS Telemetry** | ✅ | ✅ (Party CAN) | — | — | — |
| **TPMS Reading** | ✅ | — | — | — | — |
| **Powertrain Telemetry** | ✅ | — | — | — | — |
| **Region Spoofing** | ✅ (ECE R79) | — | — | — | ✅ |
| **MCP2515 8MHz Crystal** | — | ✅ | ✅ | — | — |
| **Steering Tune (0x101)** | — | ✅ | — | — | — |

## CAN ID Coverage

| CAN ID | Hex | Signal | Tesla-CAN-Mod | hypery11 | slxslx | Shayennn | EzeLLM |
| ------ | --- | ------ | ------------ | -------- | ------ | -------- | ------ |
| 69 | 0x045 | Stalk (Legacy) | ✅ | ✅ | — | — | — |
| 130 | 0x082 | Trip Planning / Precondition | ✅ | ✅ | — | — | — |
| 257 | 0x101 | Steering Tune (Chassis) | — | ✅ | — | — | — |
| 297 | 0x129 | Steering Angle | — | ✅ | — | — | — |
| 306 | 0x132 | Pack Voltage / Current | — | ✅ | — | — | — |
| 658 | 0x292 | SOC % | — | ✅ | — | — | — |
| 787 | 0x313 | Track Mode | ✅ | ✅ | — | — | — |
| 786 | 0x312 | Battery Temp | — | ✅ | — | — | — |
| 817 | 0x331 | DAS Config (TLSSC) | ✅ | ✅ | — | — | — |
| 880 | 0x370 | EPAS Status (Nag) | ✅ | ✅ | ✅ | ✅ | — |
| 920 | 0x398 | GTW carConfig (HW detect) | ✅ | ✅ | ✅ | — | ✅ |
| 921 | 0x399 | ISA Speed Limit | ✅ | ✅ | ✅ | — | — |
| 923 | 0x39B | DAS Status | — | ✅ | — | — | — |
| 962 | 0x3C2 | VCLEFT (Yoke Blinker) | — | — | — | — | — |
| 1006 | 0x3EE | AP Control (Legacy) | ✅ | ✅ | ✅ | ✅ | ✅ |
| 1011 | 0x3F3 | Seatbelt | ✅ | — | — | — | — |
| 1013 | 0x3F5 | Lighting | ✅ | ✅ | — | — | — |
| 1016 | 0x3F8 | Follow Distance | ✅ | ✅ | ✅ | ✅ | ✅ |
| 1021 | 0x3FD | AP Control (HW3/HW4) | ✅ | ✅ | ✅ | ✅ | ✅ |
| 2047 | 0x7FF | GTW carConfig Eth (Ban) | ✅ | ✅ | — | — | — |

## Board Support

| Board | Tesla-CAN-Mod | hypery11 | slxslx | Shayennn | EzeLLM |
| ----- | ------------ | -------- | ------ | -------- | ------ |
| ESP32-S3 | ✅ | ✅ | ✅ | — | — |
| ESP32 (generic) | — | — | ✅ | ✅ | ✅ |
| Arduino Uno | ✅ | — | — | — | — |
| Arduino Nano | — | — | — | — | ✅ |
| Flipper Zero | — | ✅ | — | — | — |
| Adafruit Feather RP2040 | — | — | ✅ | — | ✅ |
| Adafruit Feather M4 CAN | — | — | ✅ | ✅ | — |
| M5Stack Atomic | — | — | ✅ | — | — |
| LilyGo TCAN485 | — | — | ✅ | — | — |

## Features We Lack (Phase 9 Mining Targets)

| Feature | Source Repo | Priority | Complexity |
| ------- | ---------- | -------- | --------- |
| Ban Detection (AP tier readback from 0x7FF mux2) | hypery11 | HIGH | Low |
| Natural Nag Killer (Gaussian jitter on 0x370) | hypery11 issue #18 | HIGH | Medium |
| ALC Auto-Confirm (0x249 stalk / 0x3C2 Yoke inject) | hypery11 issue #18 | MEDIUM | Medium-High |
| MCP2515 8MHz Crystal support | slxslx, hypery11 | MEDIUM | Low |
| Emergency Vehicle Detection (HW4 bit59) | hypery11, slxslx | LOW | Low |
| Steering Tune Control (0x101 Chassis CAN) | hypery11 | LOW | Low |
| Steering Angle Parsing (0x129) | hypery11 | LOW | Low |
| DAS Status Parsing (0x39B hands-on + lane change) | hypery11 | MEDIUM | Low |

## Architecture Comparison

| Aspect | Tesla-CAN-Mod | hypery11 | slxslx | Shayennn |
| ------ | ------------ | -------- | ------ | -------- |
| Build System | PlatformIO | Flipper SDK + PlatformIO | PlatformIO | Makefile + PlatformIO |
| Test Framework | Unity (PlatformIO) | Custom | PlatformIO native | Custom + sanitizers |
| CAN Driver | MCP2515 SPI | Flipper CAN HAL | MCP2515 / TWAI | MCP2515 / TWAI / IDF |
| WiFi Dashboard | ✅ ESPAsyncWebServer | ✅ ESP32 port | ✅ ESP32 | ✅ ESP-IDF |
| BLE Control | ✅ NimBLE | — | — | — |
| Mobile App | ✅ Expo/RN | — | — | — |
| Protocol Layer | ✅ TypeScript | — | — | — |
| Config Persistence | NVS | — | — | Flash |
| Feature Count | 43 handlers | 32+ handlers | ~15 handlers | ~8 handlers |
