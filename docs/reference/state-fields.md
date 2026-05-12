---
title: State Fields Reference
description: All fields in the firmware State struct with types, defaults, and persistence
category: reference
folder: reference
tags: [state, firmware, types, fields]
order: 11
icon: 📋
---

# State Fields Reference

All fields in `firmware/lib/core/types.h` → `struct State`. Fields marked **NVS** are persisted across reboots. Fields marked **RO** are read-only (decoded from CAN).

## Core Features

| Field              | Type    | Default | Persist | Description                              |
| ------------------ | ------- | ------- | ------- | ---------------------------------------- |
| `variant`          | Variant | HW4     | NVS     | Active hardware variant (HW4/HW3/Legacy) |
| `fsdEnabled`       | bool    | false   | NVS     | FSD CAN modification active              |
| `fsdForceEnabled`  | bool    | false   | NVS     | Force FSD edits even without UI FSD bit  |
| `nagMode`          | NagMode | OFF     | NVS     | Nag suppression strategy                 |
| `speedProfile`     | int     | 1       | NVS     | Speed profile (0–3)                      |
| `profileOverride`  | bool    | false   | NVS     | User-pinned profile (vs. stalk tracking) |
| `speedOffset`      | int     | 0       | NVS     | Speed offset (HW4: 0–63, HW3: 0–100)     |
| `offsetOverride`   | bool    | false   | NVS     | User-pinned offset                       |
| `isaChimeSuppress` | bool    | false   | NVS     | ISA speed chime suppression (HW4)        |
| `summonInject`     | bool    | false   | —       | Summon injection allowed                 |
| `streamEnabled`    | bool    | false   | —       | Frame streaming active                   |
| `streamCount`      | ulong   | 0       | —       | Frames streamed counter                  |
| `rawCanListen`     | bool    | false   | —       | Unfiltered CAN listening                 |

## CAN Bus Health

| Field           | Type  | Default | Description                       |
| --------------- | ----- | ------- | --------------------------------- |
| `lastFrameMs`   | ulong | 0       | Last received CAN frame timestamp |
| `chassisOnline` | bool  | false   | CAN frames are flowing            |
| `standby`       | bool  | false   | No CAN traffic for 10s+           |
| `lastReinitMs`  | ulong | 0       | Last MCP2515 reinit attempt       |

## Summon

| Field             | Type    | Default | Description            |
| ----------------- | ------- | ------- | ---------------------- |
| `summonRemaining` | uint8   | 0       | Burst frames remaining |
| `summonLastMs`    | ulong   | 0       | Last burst frame sent  |
| `lastCtrl[8]`     | uint8[] | 0       | Cached 0x273 frame     |
| `hasCtrl`         | bool    | false   | 0x273 frame seen       |
| `summonDirection` | enum    | FORWARD | Forward/Reverse        |
| `summonMode`      | enum    | STOP    | Active summon mode     |

## BMS Telemetry (RO)

| Field                    | Type  | CAN ID | Description                     |
| ------------------------ | ----- | ------ | ------------------------------- |
| `bmsVoltage`             | float | 0x132  | Pack voltage (V)                |
| `bmsCurrent`             | float | 0x132  | Pack current (A, neg=discharge) |
| `bmsPower`               | float | —      | Pack power (kW, computed)       |
| `bmsSoc`                 | float | 0x132  | State of charge (%)             |
| `bmsTempMin`             | int8  | 0x312  | Min cell temp (°C)              |
| `bmsTempMax`             | int8  | 0x312  | Max cell temp (°C)              |
| `bmsWhPerKm`             | float | —      | Energy consumption (Wh/km)      |
| `bmsNominalFullPack`     | float | 0x352  | Full capacity (kWh)             |
| `bmsNominalRemaining`    | float | 0x352  | Energy remaining (kWh)          |
| `bmsIdealRemaining`      | float | 0x352  | Ideal energy remaining (kWh)    |
| `bmsCellVoltageMax`      | float | 0x332  | Max cell voltage (V)            |
| `bmsCellVoltageMin`      | float | 0x332  | Min cell voltage (V)            |
| `bmsMaxRegenPower`       | float | 0x252  | Max regen power (kW)            |
| `bmsMaxDischargePower`   | float | 0x252  | Max discharge power (kW)        |
| `bmsSocUI`               | float | 0x292  | SoC shown to user               |
| `bmsSocMax`              | float | 0x292  | Maximum SoC                     |
| `bmsSocAvg`              | float | 0x292  | Average SoC                     |
| `bmsInitialFullPack`     | float | 0x292  | Factory capacity (kWh)          |
| `bmsExpectedRange`       | float | 0x33A  | Expected range (km)             |
| `bmsIdealRange`          | float | 0x33A  | Ideal range (km)                |
| `bmsRatedConsumption`    | float | 0x33A  | Rated Wh/km                     |
| `bmsPowerDissipation`    | float | 0x312  | Thermal power (kW)              |
| `bmsFlowRequest`         | float | 0x312  | Coolant flow (LPM)              |
| `bmsCoolTarget`          | float | 0x312  | Cooling target (°C)             |
| `bmsHeatTarget`          | float | 0x312  | Heating target (°C)             |
| `bmsPackTMin`            | float | 0x312  | Pack min temp (°C)              |
| `bmsPackTMax`            | float | 0x312  | Pack max temp (°C)              |
| `bmsStationaryHeatPower` | float | 0x252  | Stationary heat budget (kW)     |
| `bmsHvacPowerBudget`     | float | 0x252  | HVAC power budget (kW)          |
| `bmsPrecondAllowed`      | bool  | 0x212  | BMS allows preconditioning      |
| `bmsContactorState`      | uint8 | 0x212  | Contactor state (0–7)           |
| `bmsMinBusVoltage`       | float | 0x2D2  | Min HV bus voltage (V)          |
| `bmsMaxChargeCurrent`    | float | 0x2D2  | Max charge current (A)          |
| `bmsExpectedRemaining`   | float | 0x352  | Expected remaining (kWh)        |
| `bmsEnergyToCharge`      | float | 0x352  | Energy to full (kWh)            |
| `bmsFullyCharged`        | bool  | 0x352  | Fully charged flag              |
| `bmsKwhDischargeTotal`   | float | 0x3D2  | Lifetime discharged (kWh)       |
| `bmsKwhChargeTotal`      | float | 0x3D2  | Lifetime charged (kWh)          |
| `bmsChargeTimeToFull`    | float | 0x132  | Hours to full charge            |

## Nag Alert Suppression

All nag-related state in one place. See `docs/reference/commands.md` for the
unified command interface (`nag:mode:<name>`).

### Persisted fields

| Field                    | Type    | Default | Persist      | Description                                                              |
| ------------------------ | ------- | ------- | ------------ | ------------------------------------------------------------------------ |
| `nagMode`                | NagMode | OFF     | NVS nagMode  | Active suppression strategy (off/bit19/legacy/safe/natural/organic/full) |
| `nagOrganicDriverBypass` | bool    | false   | NVS nagOrgDB | Organic: stop injection when real hands-on detected                      |

### Organic mode state (runtime + one persisted flag)

| Field                   | Type   | Default | Persist | Description                                         |
| ----------------------- | ------ | ------- | ------- | --------------------------------------------------- |
| `nagOrganicRealHandsOn` | uint8  | 0       | RO      | Last observed incoming EPAS handsOnLevel (0–3)      |
| `nagOrganicPrevState`   | uint8  | 0xFF    | —       | Previous `dasHandsOnState` for transition detection |
| `nagOrg1EnterMs`        | ulong  | 0       | —       | State 1 grace-hold entry timestamp                  |
| `nagOrg2EnterMs`        | ulong  | 0       | —       | State 2 pause entry timestamp                       |
| `nagOrg2WalkRaw`        | int16  | 2048    | —       | Persistent state-2 random-walk torque (raw)         |
| `nagOrg2HoldUntilMs`    | ulong  | 0       | —       | State 2 level-2 hold expiry                         |
| `nagOrgStrongEnterMs`   | ulong  | 0       | —       | State 3-5 group entry timestamp                     |
| `nagOrgFramesUntilExc`  | uint16 | 175     | —       | Frames until next grip-excursion pulse              |
| `nagOrgExcFrames`       | uint8  | 0       | —       | Frames remaining in current grip pulse              |
| `nagOrgLastRaw`         | int16  | 2048    | —       | Last generated organic-mode torque (raw)            |
| `nagOrgLastLevel`       | uint8  | 0       | —       | Last generated organic-mode HandsOnLevel            |

### DAS signals used by all echo modes (read-only)

| Field                  | Type   | CAN ID       | Description                                    |
| ---------------------- | ------ | ------------ | ---------------------------------------------- |
| `steeringMode`         | uint8  | 0x370 byte 0 | EPAS mode (0=FAIL, 1=COMFORT, 2=STD, 3=SPORT)  |
| `dasHandsOnState`      | uint8  | 0x39B byte 5 | DAS hands-on demand (0–15)                     |
| `dasApState`           | uint8  | 0x39B byte 1 | DAS autopilot state (0=UNAVAIL, 3–6=active)    |
| `naturalNagLastMs`     | ulong  | —            | Last natural nag injection time                |
| `naturalNagIntervalMs` | uint16 | —            | Current natural injection interval (150–350ms) |

## Auto Lane Change

| `dasLaneChangeState` | uint8 | 0 | RO | DAS lane change state (0x39B) |
| `alcLastConfirmMs` | ulong | 0 | — | Last ALC injection time |

## Safety Cues (RO)

| Field                | Type  | CAN ID | Description              |
| -------------------- | ----- | ------ | ------------------------ |
| `turnSignalLeft`     | bool  | 0x3F5  | Left turn signal active  |
| `turnSignalRight`    | bool  | 0x3F5  | Right turn signal active |
| `bsmLeftLevel`       | uint8 | 0x399  | Left BSM warning (0–2)   |
| `bsmRightLevel`      | uint8 | 0x399  | Right BSM warning (0–2)  |
| `doorFrontLeftOpen`  | bool  | —      | Front-left door open     |
| `doorFrontRightOpen` | bool  | —      | Front-right door open    |
| `doorRearLeftOpen`   | bool  | —      | Rear-left door open      |
| `doorRearRightOpen`  | bool  | —      | Rear-right door open     |
| `frunkOpen`          | bool  | —      | Frunk open               |
| `trunkOpen`          | bool  | —      | Trunk open               |
| `cruiseSetSpeedKph`  | float | —      | Cruise set speed (km/h)  |
| `mapSpeedLimitKph`   | float | —      | Map speed limit (km/h)   |

## OTA & Hardware Detection

| Field               | Type  | Default | Description                      |
| ------------------- | ----- | ------- | -------------------------------- |
| `otaInProgress`     | bool  | false   | Tesla OTA detected (0x318)       |
| `txPaused`          | bool  | false   | All TX paused during OTA         |
| `detectedHW`        | uint8 | 0       | Auto-detected HW (0/2/3)         |
| `variantAutoDetect` | bool  | true    | Auto-switch variant on detection |
| `canClockReqMHz`    | uint8 | 8       | Requested MCP2515 crystal (MHz)  |
| `canClockMHz`       | uint8 | 8       | Active clock after fallback      |

## Autopilot Tier & Ban Detection

| Field               | Type   | Default | Persist | Description                              |
| ------------------- | ------ | ------- | ------- | ---------------------------------------- |
| `gtwAutopilotTier`  | int8   | -1      | RO      | AP tier (-1=unk, 0=NONE, 3=FSD, 4=BASIC) |
| `banShieldEnabled`  | bool   | false   | NVS     | Ban threat monitoring active             |
| `banThreatLevel`    | uint8  | 0       | —       | Current threat level (0–5)               |
| `banDetectionCount` | uint16 | 0       | —       | Cumulative ban events                    |

## GTW Shield

| Field                 | Type      | Description                      |
| --------------------- | --------- | -------------------------------- |
| `gtwSnapshot[8][8]`   | uint8[][] | Cached GTW_carConfig mux frames  |
| `gtwSnapshotValid[8]` | bool[]    | Per-mux capture flag             |
| `gtwShieldArmed`      | bool      | Actively blocking config changes |
| `gtwShieldBlocks`     | uint32    | Frames blocked since arm         |

## Feature Flags (NVS)

| Field                   | Type  | Default | Description                       |
| ----------------------- | ----- | ------- | --------------------------------- |
| `enhancedAutopilot`     | bool  | false   | EAP/Summon unlock (bit46)         |
| `evdEnabled`            | bool  | false   | Emergency vehicle detection (HW4) |
| `tlsscRestore`          | bool  | false   | TLSSC restore on 0x331            |
| `preconditionEnabled`   | bool  | false   | Battery preconditioning           |
| `trackModeEnabled`      | bool  | false   | Track Mode                        |
| `eceR79Bypass`          | bool  | false   | ECE R79 steering limit bypass     |
| `seatbeltEmulation`     | bool  | false   | Rear seatbelt emulation           |
| `wiperPersistEnabled`   | bool  | false   | Wiper speed persistence           |
| `mirrorAutoFoldEnabled` | bool  | false   | Auto fold on lock                 |
| `singleShotTx`          | bool  | false   | MCP2515 one-shot TX               |
| `rateLimitEnabled`      | bool  | false   | Per-CAN-ID TX rate limiting       |
| `driveModeOverride`     | uint8 | 0       | Ghost mode (0=off, 1–3)           |
| `canSimEnabled`         | bool  | false   | CAN simulation mode               |

## Region

| Field                  | Type  | Default | Persist | Description               |
| ---------------------- | ----- | ------- | ------- | ------------------------- |
| `regionCode`           | uint8 | 0       | RO      | Detected region (0–5)     |
| `regionSpoofCode`      | uint8 | 0       | NVS     | Spoof region (0=off, 1–5) |
| `hasRegion`            | bool  | false   | —       | Region detected from CAN  |
| `chineseGatewayLocked` | bool  | false   | RO      | CN market GTW lock        |

## TPMS (RO)

| Field             | Type    | CAN ID | Description                |
| ----------------- | ------- | ------ | -------------------------- |
| `tpmsPressure[4]` | float[] | 0x219  | FL/FR/RL/RR pressure (bar) |
| `tpmsTemp[4]`     | int8[]  | 0x219  | FL/FR/RL/RR temp (°C)      |

## Powertrain Telemetry (RO)

| Field           | Type  | CAN ID | Description               |
| --------------- | ----- | ------ | ------------------------- |
| `vehicleSpeed`  | float | 0x257  | Speed (km/h)              |
| `gearState`     | uint8 | 0x118  | Gear (1=P, 2=R, 3=N, 4=D) |
| `accelPedal`    | uint8 | 0x118  | Pedal position (0–100%)   |
| `steeringAngle` | float | 0x129  | Steering angle (degrees)  |
| `rearMotorRpm`  | int16 | 0x106  | Rear motor RPM            |
| `frontMotorRpm` | int16 | 0x115  | Front motor RPM           |

## Firmware Version (RO)

| Field       | Type   | CAN ID | Description               |
| ----------- | ------ | ------ | ------------------------- |
| `fwYear`    | uint16 | 0x392  | Vehicle FW year           |
| `fwRelease` | uint8  | 0x392  | Release number            |
| `fwMinor`   | uint8  | 0x392  | Minor version             |
| `fwBuild`   | uint32 | 0x392  | Build number              |
| `fwCompat`  | uint8  | —      | Compatibility level (0–3) |

## MQTT Bridge (NVS)

| Field           | Type   | Default | Description            |
| --------------- | ------ | ------- | ---------------------- |
| `mqttEnabled`   | bool   | false   | MQTT publishing active |
| `mqttHost[64]`  | char[] | ""      | Broker hostname        |
| `mqttPort`      | uint16 | 1883    | Broker port            |
| `mqttInterval`  | uint16 | 2000    | Publish interval (ms)  |
| `mqttConnected` | bool   | false   | Currently connected    |

## Vehicle Identity (RO)

| Field              | Type   | CAN ID | Description                |
| ------------------ | ------ | ------ | -------------------------- |
| `platformModel`    | uint8  | 0x398  | Tesla model (0–5)          |
| `platformHwGen`    | uint8  | 0x398  | HW generation (0–3)        |
| `platformSwYear`   | uint16 | 0x392  | Software year              |
| `platformSwWeek`   | uint8  | 0x392  | Software week              |
| `platformFsdProto` | uint8  | —      | FSD protocol (V12/V13/V14) |
| `platformResolved` | bool   | false  | Identity fully resolved    |

## WiFi API (NVS)

| Field            | Type   | Default | Description                      |
| ---------------- | ------ | ------- | -------------------------------- |
| `apiKey[33]`     | char[] | ""      | 32-char hex key (auto-generated) |
| `apiKeyRequired` | bool   | false   | Require X-API-Key header         |
