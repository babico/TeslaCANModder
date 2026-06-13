# Tesla CAN ID Reference

Master reference for all CAN IDs used in Tesla-CAN-Mod firmware.
Cross-validated against hypery11, slxslx, Shayennn, EzeLLM, and ev-open-can-tools.

## Bus Topology (X179 Connector)

| Bus | Pins  | Name         | Purpose                            |
| --- | ----- | ------------ | ---------------------------------- |
| 0   | 13-14 | Chassis / AP | Autopilot, DAS, EPAS, FSD control  |
| 1   | 9-10  | Vehicle      | Drive, BMS, body controllers, OTA  |
| 2   | 2-3   | Body         | VCLEFT, VCRIGHT, VCFRONT, lighting |

## CAN ID Table

### Control & FSD

| Hex   | Dec  | Symbol                  | Signal                       | Bus | Direction | Notes                            |
| ----- | ---- | ----------------------- | ---------------------------- | --- | --------- | -------------------------------- |
| 0x045 | 69   | `CAN_ID_LEGACY_STALK`   | Legacy follow-distance stalk | 0   | RX        | Pre-AP vehicles                  |
| 0x3EE | 1006 | `CAN_ID_LEGACY_FSD_MUX` | Legacy AP control (muxed)    | 0   | RX/TX     | HW2.5 and older                  |
| 0x3F8 | 1016 | `CAN_ID_FOLLOW_DIST`    | Follow distance control      | 0   | RX/TX     | HW3                              |
| 0x3FD | 1021 | `CAN_ID_FSD_MUX`        | FSD enable/disable (muxed)   | 0   | RX/TX     | HW4, bit 46/60                   |
| 0x331 | 817  | `CAN_ID_DAS_AP_CONFIG`  | DAS_autopilotConfig          | 0   | RX/TX     | Tier readback, TLSSC target      |
| 0x7FF | 2047 | `CAN_ID_GTW_CONFIG_ETH` | GTW carConfig (Ethernet)     | 1   | RX        | AP tier on mixed/Ethernet bridge |

### DAS (Driver Assistance)

| Hex   | Dec | Symbol               | Signal            | Bus | Direction | Notes                                      |
| ----- | --- | -------------------- | ----------------- | --- | --------- | ------------------------------------------ |
| 0x39B | 923 | `CAN_ID_DAS_STATUS`  | DAS_status        | 1   | RX        | byte5[5:2]=hands-on, byte4[4:0]=laneChange |
| 0x389 | 905 | `CAN_ID_DAS_STATUS2` | DAS_accSpeedLimit | 0   | RX        | ACC speed limit readback                   |
| 0x2B9 | 697 | `CAN_ID_DAS_CONTROL` | DAS_setSpeed      | 0   | RX        | Cruise set-speed readback                  |

### Steering & Stalks

| Hex   | Dec | Symbol                  | Signal                          | Bus | Direction | Notes                                      |
| ----- | --- | ----------------------- | ------------------------------- | --- | --------- | ------------------------------------------ |
| 0x370 | 880 | `CAN_ID_EPAS_TORQUE`    | EPAS3P_sysStatus                | 1   | RX/TX     | Nag killer, steering mode, checksum=sum+ID |
| 0x229 | 553 | `CAN_ID_EPAS_HARNESS`   | EPAS_internalHarness            | 0   | RX        | CRC-8 protected                            |
| 0x249 | 585 | `CAN_ID_DI_STEER`       | DI_steerAssist / SCCM_leftStalk | 0   | RX/TX     | CRC-8 protected, ALC stalk injection       |
| 0x129 | 297 | `CAN_ID_STEERING_ANGLE` | SCCM_steeringAngleSensor        | 0   | RX        | Signed 16-bit / 10 = degrees               |
| 0x3C2 | 962 | `CAN_ID_VCLEFT_SWITCH`  | VCLEFT_switchStatus             | 2   | TX        | Palladium/Yoke turn button injection       |

### Drive & Powertrain

| Hex   | Dec | Symbol                 | Signal               | Bus | Direction | Notes                              |
| ----- | --- | ---------------------- | -------------------- | --- | --------- | ---------------------------------- |
| 0x118 | 280 | `CAN_ID_DI_STATE`      | DI_systemStatus      | 1   | RX        | Track mode, traction control       |
| 0x334 | 820 | `CAN_ID_DRIVE_CONFIG`  | Drive config         | 1   | RX/TX     | Pedal, regen, creep (checksum=sum) |
| 0x257 | 599 | `CAN_ID_VEHICLE_SPEED` | Vehicle speed        | 1   | RX        | km/h readback                      |
| 0x106 | 262 | `CAN_ID_REAR_MOTOR`    | Rear motor RPM       | 1   | RX        |                                    |
| 0x115 | 277 | `CAN_ID_FRONT_MOTOR`   | Front motor RPM      | 1   | RX        |                                    |
| 0x313 | 787 | `CAN_ID_TRACK_MODE`    | UI_trackModeSettings | 1   | RX/TX     |                                    |

### BMS (Battery Management)

| Hex   | Dec  | Symbol                 | Signal                     | Bus | Direction | Notes                             |
| ----- | ---- | ---------------------- | -------------------------- | --- | --------- | --------------------------------- |
| 0x132 | 306  | `CAN_ID_BMS_HV_BUS`    | BMS_hvBusStatus            | 1   | RX        | Voltage, current, power           |
| 0x292 | 658  | `CAN_ID_BMS_SOC`       | BMS_socStatus              | 1   | RX        | State of charge %                 |
| 0x312 | 786  | `CAN_ID_BMS_THERMAL`   | BMS_thermalStatus          | 1   | RX        | Pack temperatures                 |
| 0x33A | 826  | `CAN_ID_BMS_ENERGY`    | UI_energyGraphData         | 1   | RX        | Wh/km consumption                 |
| 0x332 | 818  | `CAN_ID_BMS_MIN_MAX`   | BMS_bmbMinMax              | 1   | RX        | Cell min/max voltages             |
| 0x352 | 850  | `CAN_ID_BMS_ENERGY_ST` | BMS_energyStatus           | 1   | RX        | Degradation metrics               |
| 0x252 | 594  | `CAN_ID_BMS_POWER_AV`  | BMS_powerAvailable         | 1   | RX        | Regen/discharge power limits      |
| 0x212 | 530  | `CAN_ID_BMS_STATUS`    | BMS_status                 | 1   | RX        | HV state, contactor, precondition |
| 0x2D2 | 722  | `CAN_ID_BMS_DRIVE_LIM` | BMS_driveLimits            | 1   | RX        | Bus voltage/current limits        |
| 0x3D2 | 978  | `CAN_ID_BMS_KWH_CNT`   | BMS_kwhCounter             | 1   | RX        | Lifetime energy totals            |
| 0x3F2 | 1010 | `CAN_ID_BMS_KWH_MUX`   | BMS_kwhCountersMultiplexed | 1   | RX        | AC/DC/regen/drive breakdowns      |
| 0x401 | 1025 | `CAN_ID_BMS_BRICK_V`   | BMS_brickVoltages          | 1   | RX        | Individual cell voltages (muxed)  |

### Body & Comfort

| Hex   | Dec  | Symbol                   | Signal                | Bus | Direction | Notes                      |
| ----- | ---- | ------------------------ | --------------------- | --- | --------- | -------------------------- |
| 0x273 | 627  | `CAN_ID_UI_VEHICLE_CTRL` | UI_vehicleControl     | 1   | RX/TX     | Summon, window, etc.       |
| 0x284 | 644  | `CAN_ID_SENTRY`          | Sentry mode           | 1   | RX        |                            |
| 0x2F3 | 755  | `CAN_ID_CLIMATE`         | Climate control       | 1   | RX        |                            |
| 0x333 | 819  | `CAN_ID_CHARGE`          | Charge control        | 1   | RX        |                            |
| 0x3B3 | 947  | `CAN_ID_TRUNK_CTRL`      | Trunk/Glovebox        | 1   | RX/TX     |                            |
| 0x119 | 281  | `CAN_ID_WINDOW_VENT`     | Window vent           | 1   | TX        |                            |
| 0x082 | 130  | `CAN_ID_PRECONDITION`    | UI_tripPlanning       | 1   | RX/TX     | Preconditioning            |
| 0x2AA | 682  | `CAN_ID_AIR_RECIRC`      | Air recirculation     | 1   | RX/TX     |                            |
| 0x3F5 | 1013 | `CAN_ID_VCFRONT_LIGHTS`  | VCFRONT vehicleLights | 1   | RX/TX     | Turn signal status/control |
| 0x3F3 | 1011 | `CAN_ID_SEATBELT_STATUS` | Seatbelt status       | 1   | RX/TX     | Seatbelt emulation target  |
| 0x219 | 537  | `CAN_ID_TPMS`            | TPMS tire pressures   | 1   | RX        |                            |

### Vehicle Status

| Hex   | Dec | Symbol                       | Signal                     | Bus | Direction | Notes                       |
| ----- | --- | ---------------------------- | -------------------------- | --- | --------- | --------------------------- |
| 0x398 | 920 | `CAN_ID_GTW_CAR_CFG`         | GTW_carConfig              | 1   | RX        | HW auto-detect, region code |
| 0x318 | 792 | `CAN_ID_GTW_CAR_STATE`       | GTW_carState               | 1   | RX        | OTA detection               |
| 0x102 | 258 | `CAN_ID_VCLEFT_DOOR_STATUS`  | VCLEFT doors               | 2   | RX        | Door/latch status           |
| 0x103 | 259 | `CAN_ID_VCRIGHT_DOOR_STATUS` | VCRIGHT doors              | 2   | RX        | Trunk/latch status          |
| 0x2E1 | 737 | `CAN_ID_VCFRONT_STATUS`      | VCFRONT status             | 2   | RX        | Frunk, any-door             |
| 0x3A1 | 929 | `CAN_ID_VCFRONT_VEH_STATUS`  | VCFRONT vehicle status     | 2   | RX        | Driver door                 |
| 0x3D9 | 985 | `CAN_ID_UI_GPS_SPEED`        | UI GPS speed / speed limit | 0   | RX        | Map speed limit             |

### ISA (Intelligent Speed Assistance)

| Hex   | Dec | Symbol             | Signal          | Bus | Direction | Notes                         |
| ----- | --- | ------------------ | --------------- | --- | --------- | ----------------------------- |
| 0x399 | 921 | `CAN_ID_ISA_SPEED` | ISA speed limit | 0   | RX        | Speed chime suppression (HW4) |

## CRC-8 Protected Frames

These frames require Tesla-specific CRC-8 computation with per-ID magic tables:

| CAN ID | Symbol               | Magic Table       | Counter Location |
| ------ | -------------------- | ----------------- | ---------------- |
| 0x229  | EPAS_internalHarness | `MAGIC_0x229[16]` | byte[0] & 0x0F   |
| 0x249  | DI_steerAssist       | `MAGIC_0x249[16]` | byte[1] & 0x0F   |
| 0x370  | EPAS_sysStatus       | `MAGIC_0x370[16]` | byte[1] & 0x0F   |

CRC formula: `crc8_opensafety(payload + 0x00) XOR magicTable[counter & 0x0F]`

## Checksum Algorithms

| Type           | Formula                               | Used By                |
| -------------- | ------------------------------------- | ---------------------- |
| Drive checksum | `sum(bytes[0..N-2]) & 0xFF`           | 0x334 pedal/regen/stop |
| EPAS checksum  | `sum(bytes[0..6]) + ID_low + ID_high` | 0x370 nag killer       |
| ISA checksum   | `sum(bytes[0..6]) + ID_low + ID_high` | ISA speed chime        |
| Tesla CRC-8    | OpenSafety CRC + XOR magic table      | 0x229, 0x249, 0x370    |

## Autopilot Tier Values (from 0x7FF mux=2 or 0x331)

| Value | Name         | Description                |
| ----- | ------------ | -------------------------- |
| 0     | NONE         | No autopilot capability    |
| 1     | HIGHWAY      | Basic highway autopilot    |
| 2     | ENHANCED     | Enhanced Autopilot (EAP)   |
| 3     | SELF_DRIVING | Full Self-Driving (FSD)    |
| 4     | BASIC        | Downgraded (ban indicator) |
| -1    | UNKNOWN      | Not yet read               |

## Driver-Assist Bitfields

### 0x3F8 / 1016 `CAN_ID_FOLLOW_DIST`

Used on HW3 and HW4 for follow-distance state plus several parity toggles injected by the firmware.

- Bit 5: `UI_dasDeveloper` — `assist-dev:on` sets this bit.
- Bit 13: `UI_driveOnMapsEnable` — `assist-nav:on` sets this bit.
- Bit 14: `UI_handsOnRequirementDisable` — `assist-hof:on` sets this bit.
- Bit 41: `UI_drivingSide` — `lhd:on` clears this bit for LHD mode.
- Bit 43: `UI_enableTripTelemetry` — `assist-tel:on` clears this bit.
- Bit 48: `UI_hasDriveOnNav` — `assist-nav:on` sets this bit.
- Bit 49: `UI_followNavRouteEnable` — `assist-nav:on` sets this bit.

### 0x3FD / 1021 `CAN_ID_FSD_MUX`

Multiplexed FSD control frame. The firmware modifies different bitfields depending on mux.

#### mux 0

- Bit 38: FSD assist enable path — set when FSD modification is active.
- Bit 39: `UI_fsdContinueOnGreenWithCIPV` — set when FSD modification is active.
- Bit 46: FSD capability enable — set when FSD modification is active.
- Bit 59: Emergency vehicle response — set by `evd:on` on HW4.
- Bit 60: FSD capability enable companion bit — set when FSD modification is active.

#### mux 1

- Bit 19: Nag / hands-on restriction path — cleared when nag suppression is active.
- Bit 20: ECE R79 restriction bit — cleared by `ecer79:on` for EU vehicles.
- Bit 45: Lane graph visualization — set by `lanegraph:on`.
- Bit 46: Enhanced Autopilot / hidden AP bit — set by `eap:on`.
- Bit 47: Summon / nag companion unlock bit — set when nag suppression is active.

### 0x39B / 923 `CAN_ID_DAS_STATUS`

Important AP-gate and auto-lane-change readback fields:

| Location          | Meaning                     | Firmware Use                                |
| ----------------- | --------------------------- | ------------------------------------------- |
| byte[0] bits[3:0] | DAS autopilot status        | Determines whether AP is active for AP gate |
| byte[4] bits[4:0] | `DAS_laneChangeState`       | Drives ALC auto-confirm                     |
| byte[5] bits[5:2] | `DAS_autopilotHandsOnState` | Used by nag killer state-aware behavior     |

### 0x398 / 920 `CAN_ID_GTW_CAR_CFG`

Gateway config fields used by detection and regional feature logic:

| Location          | Meaning                      | Firmware Use                  |
| ----------------- | ---------------------------- | ----------------------------- |
| byte[0] bits[7:6] | Detected hardware generation | Auto-detects HW3 vs HW4       |
| byte[2] bits[7:4] | Region code                  | Region detection and spoofing |

### 0x331 / 817 `CAN_ID_DAS_AP_CONFIG`

| Location          | Meaning                | Firmware Use                                                               |
| ----------------- | ---------------------- | -------------------------------------------------------------------------- |
| byte[0] bits[5:0] | Autopilot tier payload | `tlssc:on` rewrites to `0x1B` (`SELF_DRIVING`) while preserving upper bits |

### 0x7FF / 2047 `CAN_ID_GTW_CONFIG_ETH`

| Location                 | Meaning                | Firmware Use                                   |
| ------------------------ | ---------------------- | ---------------------------------------------- |
| mux 2, byte[5] bits[4:2] | Gateway autopilot tier | Readback for ban detection and tier monitoring |

### 0x3C2 / 962 `CAN_ID_VCLEFT_SWITCH`

Palladium/yoke ALC confirmation injection:

| Location         | Meaning                 | Firmware Use                            |
| ---------------- | ----------------------- | --------------------------------------- |
| byte[0] = `0x29` | Mux A selector          | Required for valid button frame         |
| byte[6]/byte[7]  | Left/right button masks | Auto Lane Change confirmation injection |

## DAS Hands-On States (from 0x39B byte[5] bits[5:2])

| Value | Action                          |
| ----- | ------------------------------- |
| 0     | No request                      |
| 2-7   | Hands-on requested (escalating) |
| 8     | No request                      |
| 9-10  | Hands-on requested              |

## Region Codes (from 0x398 byte[2] bits[7:4])

| Value | Region        |
| ----- | ------------- |
| 0     | Unknown       |
| 1     | North America |
| 2     | Europe (ECE)  |
| 3     | China (PRC)   |
| 4     | Asia-Pacific  |
| 5     | Middle East   |
