# Firmware current-state extraction — 2026-11-22

Subagent task P0-00, FIRMWARE subsystem. Read-only on the codebase; this file is the only write.

## 0. Method, grades, and limits

Evidence grades used below, applied to each claim:

- **CODE-READ** — read the actual file on disk in this working tree (which contains 191 pre-existing unstaged changes; I did not revert or stage anything).
- **TEST-READ** — read an actual test source on disk.
- **CAPTURE** — a real-frame/vehicle capture exists. I performed no capture this session; all CAPTURE statements are inherited from `docs/legacy/` deep dives and are labelled **SUMMARY-ONLY (inherited CAPTURE)** here.
- **SUMMARY-ONLY** — inherited from `.opencode/analysis/repo-state-2026-11-22.md` or `docs/legacy/*.md` without re-deriving in this pass.
- **UNVERIFIED** — not checked; explicitly listed in §0.2.

### 0.1 What I read

`git ls-files firmware` (239 files), `firmware/platformio.ini` (571 lines), `docs/superpowers/todo.md`, `.opencode/analysis/repo-state-2026-11-22.md`, and the following source files in full or in targeted regions:

- CAN core/handlers: `core/can/ring_buffer.h`, `core/can/recorder.h`, `core/can/health.h`, `core/driver/common.h`, `core/driver/esp32/board.h`, `core/log/ring.h`, `core/types.h`, `core/forward.h`, `vehicle/can/ids.h`, `vehicle/can/checksum.h`, `vehicle/can/crc8.h`, `vehicle/can/handler/dispatch.h`, `vehicle/can/handler/ticks.h`, `vehicle/can/handler/bus/vehicle.h`, `vehicle/can/handler/bus/chassis.h`, `vehicle/can/handler/variant/hw4.h`, `vehicle/can/handler/variant/hw3.h`, `vehicle/can/handler/variant/legacy.h`, `vehicle/can/handler/frame_readers.h`, `vehicle/can/handler/filters.h`.
- CAN features: `fsd/nag.h`, `fsd/nag/math.h`, `fsd/auto_lane_change.h`, `fsd/nag*`, `das/das_drive.h`, `drive/pedal.h`, `drive/drive_mode.h`, `body/turn_signal.h`, `body/track_mode.h`, `body/summon.h`, `comfort/air_recirc.h`, `safety/single_shot.h`, `safety/ban_shield.h`, `misc/mqtt_bridge.h`, `misc/can_raw.h`, `telemetry/powertrain.h`, plus grep sweeps over all `vehicle/can/feature/**`.
- BLE: `vehicle/ble/feature/key.h`, `msg.h`, `proto.h`, `carserver.h`, `session.h`, `vcsec.h`, `vehicle/ble/handler/dispatch.h`, `io/ble/esp32/ble_client.h`, `tesla_board.h`, `config.h`, `callbacks.h`, `init.h`, `state.h`, `client/gamepad/drive.h`.
- Client/persistence: `client/command/dispatch.h` (targeted regions), `client/api/auth.h`, `client/api/routes.h`, `core/persist/nvs.h`, `native.h`, `keys.h`, `esp32/board.h`, `src/esp32/main.cpp`, `src/main.cpp`, `io/serial/usb/esp32/loop.h`.
- Proto ground truth (read-only reference under `legacy/`): `legacy/teslamotors-vehicle-command/pkg/protocol/protobuf/universal_message.proto`, `vcsec.proto`.
- Tests: `test_native_ap_gate`, `test_native_crc8`, `test_native_checksum`, `test_native_multi_feature`, `test_native_persist`, `test_native_das_drive`, `test_native_ble_distance`, `test_native_command_roundtrip`, `test_native_can_raw`, `test_native_wiper_persist`, `test_esp32_nvs_persist`, plus `git ls-files firmware/test` for the full inventory.

### 0.2 What I did NOT read / did not do

- Did not read all 84 test `.cpp` files — only the 11 suites named above plus greps across `firmware/test`.
- Did not read the full 900+ line `client/command/dispatch.h` or full `vehicle/can/handler/bus/vehicle.h`; I read targeted regions and grepped them.
- Did not read every one of the ~135 feature/transport headers; findings limited to the named files.
- Did not read client/, tools/, packages/protocol/ source (other subsystems' scope).
- Did not build firmware, run `pio test`, or execute any test (read-only task). Build/CI status is **SUMMARY-ONLY** from `.opencode/analysis/repo-state-2026-11-22.md:16`.
- Did not fetch any upstream GitHub/SDK source over the network. All proto truth is from the local `legacy/` submodule checkout.
- Did not verify the "0–100 timer" advertised-but-unimplemented item beyond the sibling analysis — **UNVERIFIED** in this pass.
- Did not verify runtime behaviour on hardware; all wire-format severity ratings are code/proto comparisons, not bench failures.

## 1. Tree map — what actually exists now

### 1.1 Inventory

**CODE-READ** `git ls-files firmware`: 239 tracked files. Layout matches `AGENTS.md` (post-2026-06-20 reorg); the old `lib/interface/`, `lib/transport/`, `lib/feature/`, `lib/handler/`, `lib/infra/` trees do not exist.

| Path                        | Content                                                                                                                                                                                                                                                             | Evidence                                              |
| --------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------- |
| `firmware/lib/core/`        | `can/{bus,health,id_filter,recorder,ring_buffer}.h`, `config/esp32{.h,/board.h}`, `driver/{common.h,esp32/board.h}`, `log/ring.h`, `persist/{keys.h,nvs.h,native.h,esp32/board.h,native/board.h}`, `forward.h`, `platform.h`, `types.h`, `util/parse.h`             | CODE-READ `git ls-files firmware/lib/core`            |
| `firmware/lib/io/`          | `log.h`, `schemas/io.schema.json`, `ble/esp32/{ble_client,callbacks,config,init,print,state,tesla_board,uuids,board}.h`, `serial/{usb,bluetooth}/esp32/*`, `wifi/esp32/{cmd,config,state,board}.h`                                                                  | CODE-READ `git ls-files firmware/lib/io`              |
| `firmware/lib/client/`      | `api/{auth,init,routes}.h`, `command/{dispatch,features,messages}.h`, `common/api_fwd.h`, `dashboard/dashboard.{h,html}`, `gamepad/{api,axis_math,ble,drive,events,gamepad,state,storage}.h`                                                                        | CODE-READ `git ls-files firmware/lib/client`          |
| `firmware/lib/vehicle/can/` | `ids.h`, `crc8.h`, `checksum.h`, `burst.h`, `fwd.h`, `handler/{dispatch,ticks,filters,frame_readers,helpers}.h`, `handler/bus/{chassis,vehicle,body}.h`, `handler/variant/{bits,hw3,hw4,legacy}.h`, `feature/{body,comfort,das,drive,fsd,misc,safety,telemetry}/**` | CODE-READ `git ls-files firmware/lib/vehicle/can`     |
| `firmware/lib/vehicle/ble/` | `msg.h`, `proto.h`, `distance.h`, `feature/{carserver,distance,key,session,vcsec}.h`, `handler/dispatch.h`                                                                                                                                                          | CODE-READ `git ls-files firmware/lib/vehicle/ble`     |
| `firmware/src/`             | `main.cpp`, `esp32/main.cpp`                                                                                                                                                                                                                                        | CODE-READ `git ls-files firmware/src`                 |
| `firmware/test/`            | 84 tracked `test_*/*.cpp` suites + `test/support/{Preferences.h,fake_mcp2515.h,fake_preferences.h,helpers.h,stubs.h}`                                                                                                                                               | CODE-READ `git ls-files 'firmware/test/test_*/*.cpp'` |
| `firmware/platformio.ini`   | 571 lines, 30+ env sections                                                                                                                                                                                                                                         | CODE-READ `firmware/platformio.ini:1-571`             |

### 1.2 Build environments and bus flags

**CODE-READ** `firmware/platformio.ini`:

- Naming convention `_wifi / _ble / _chassis / _vehicle / _body / _8mhz|_16mhz` documented at `:11-29`; "ALL buses are opt-in — no bus is hardcoded active" at `:24-26`.
- `default_envs = esp32_chassis_8mhz` (`:49`).
- `[env:native]` (`:69-86`): `-Ilib -Ilib/vehicle/can -Itest/support -std=c++17`, all three buses on (`:76-78`), WiFi/BLE off (`:80-81`), Unity 2.6.0 (`:83`), `test_filter = test_native_*` (`:86`).
- `[esp32_base]` (`:94-112`): `espressif32`/`esp32dev`/arduino, `upload_port = COM4` and `monitor_port = COM4` (`:99-100`), `lib_deps` MCP2515 v1.3.1 + ArduinoJson 7.0.0 + Preferences/WiFi/WebServer (`:104-109`), `build_src_filter = +<esp32/main.cpp>` (`:112`).
- Add-ons: clock 8/16 MHz (`:115-119`), WiFi (`:122-126`), BLE NimBLE-Arduino 1.4.0 + `-DBOARD_ENABLE_BLE=1` (`:128-130`), bus flags `BUS_CHASSIS_ACTIVE` / `BUS_VEHICLE_ACTIVE` / `BUS_BODY_ACTIVE` (`:133-146`).
- Env families: serial-only chassis/vehicle/body combos (`:152-195`), WiFi-only (`:200-246`), BLE-only (`:252-306`), WiFi+BLE (`:312-370`), 16 MHz variants (`:378-428`), no-chassis passive loggers (`:436-495`), six hardware test envs (`:503-571`). No `_wifi`-without-`_chassis` env exists; passive loggers are serial-only or WiFi+BLE.
- `PC-002` (hardcoded COM4) is **STILL_PRESENT** (`:99-100`).

## 2. CAN pipeline — RX to TX

### 2.1 RX path

**CODE-READ** `firmware/src/esp32/main.cpp:199-213`: `while (driverRead(frame, bus))` → `ringPush` → `sendFrame(...,"rx",...)` → `handleMessage(frame, bus, state)`. Standby path does the same without ring push (`:167-177`). Periodic ticks run before RX each loop (`:182-187`), plus MCP2515 error polling every 250 ms accumulating `txFailCount`/`busOffCount` (`:190-197`).

**CODE-READ** `firmware/lib/vehicle/can/handler/dispatch.h:27-46`: `handleMessage` captures into recorder, updates frame-rate meter, then routes by bus to `handleChassisBus` / `handleVehicleBus` / `handleBodyBus`.

**CODE-READ** `firmware/lib/vehicle/can/handler/bus/vehicle.h`: heavily DLC-guarded decode/capture — 0x273 `hasCtrl` + `lastCtrl` (`:49-53`), 0x334 `lastDrive` (`:67-70`), BMS/TPMS/GTW/DAS status decodes (`:75-318`), 0x370 nag echo (`:319-345`), region spoof (`:355-370`), powertrain/gear (`:381-409`), GTW mux blocks at `:438` and `:528`. Chassis bus handlers are DLC-guarded at `chassis.h:35-57`.

### 2.2 Frame mutation, injection, counters, checksums, DLC guards

**CODE-READ** mutation/injection points:

- Nag unified pipeline: `nagEchoShouldEcho` → `nagEchoCompute` → `nagEchoApply` (`vehicle.h:333-343`; documented in `fsd/nag.h:9-33`). `nagEchoApply` writes `data[7] = nagChecksum(f.data)` (`nag.h:486`), increments a rolling counter and has a DLC guard (doc at `nag.h:20-21`).
- `nagChecksum` = sum bytes 0..6 + ID low/high (`checksum.h:67-75`). **SUMMARY-ONLY (inherited CAPTURE)** repo-state `:42`: matched 25,288 real frames; `MAGIC_0x370` in `crc8.h:57-58` is dead in production (only referenced by `test_crc8.cpp:83-84`, TEST-READ).
- `dasChecksum` generic additive (`checksum.h:25-36`), `computeHW4IsaChecksum` DLC>=8 guard (`checksum.h:47-56`).
- `teslaCrc8` = CRC-8/OPENSAFETY + per-ID magic (`crc8.h:35-79`), tables for 0x229 / 0x249 / 0x370 (`:49-58`).
- ALC 0x249 frame: DLC=3, CRC over 2 bytes with `MAGIC_0x249` (`fsd/auto_lane_change.h:48-59`); static per-TU counter at `:46`. **SUMMARY-ONLY (inherited CAPTURE)** repo-state `:29`: compiled/ran both tools over 1048 real frames; our DLC-3 additive fits 0/1048. Fix candidate requires `docs/checklists/can-review-checklist.md`.
- DAS drive frames: `0x27D` DLC=3 (`:437-442`), `0x488` DLC=4 with `dasChecksum(...,4,3)` (`:464-471`), `0x2B9` DLC=8 (`:486-493`); rates 10/50/25 Hz (`:429-497`); dead-man 150 ms (`DAS_DEADMAN_MS`, `:58`, enforced `:419-424`); 5-frame cancel burst (`:48`, `:423`, `:494-496`).
- Burst path `ticks.h:84-96` sends one frame per tick at `burstDelayMs`; summon (`:26-48`) and precondition (`:58-74`) ticks gate on `apGateOpen()`; `startBurst` also gates (`vehicle/can/burst.h:27`).
- `setTurnSignalRequest` writes `f.data[0]` with **no DLC guard** (`body/turn_signal.h:28-31`), but caller builds DLC=8 (`:46-51`).

### 2.3 Freshness, OTA guard, AP gate

- **Freshness weak**: `hasCtrl` is a boolean "ever captured" flag (`vehicle.h:52`), not timestamped. Summon tick checks `hasCtrl` but no age (`ticks.h:28`). `apGateApActive` is assigned from DAS autopilot status (`vehicle.h:135`) with no timeout; stale AP-active state can keep the gate open. CODE-READ.
- **OTA guard absent**: `CAN_ID_GTW_CAR_STATE 0x318` is defined (`ids.h:52`) and listed in `io.schema.json:1205-1206,1267-1268` as `otaInProgress`/`txPaused`, but **no `.h` file in `firmware/lib` references either symbol** (negative grep over `*.h`), and `State` (`core/types.h`) has no such fields. No handler for 0x318 exists in `vehicle.h`. This confirms the repo-state C-list item (`repo-state...:44-46`) — **CODE-READ (negative grep) + SUMMARY-ONLY** for the claim that docs advertise it.
- **AP gate fails open**: `apGateOpen()` returns true whenever `apInjectionGateEnabled == false` (`core/types.h:837-842`). When enabled it accepts `apGateParked`, which is set true for gear 1 **or 0 or 5** (`vehicle.h:392-393`; `powertrain.h:27` says 5=SNA) and is force-set true on CAN-timeout standby (`main.cpp:149`). CODE-READ.
- **DAS drive bypasses AP gate**: `dasTick` (`das_drive.h:414-498`) contains no `apGateOpen()` call; `drive:on` only calls `dasDriveSetEnabled(true)` (`:515-519`). The documented `das:arm` / `das:disarm` / `das:status` safety gate has **no implementation** (negative grep across `firmware/lib` for those literals). `gamepadDriveTick` requires `gpEnabled && dasDriveIsEnabled() && gpConnected` (`client/gamepad/drive.h:32-37`) but does not consult the AP gate.

## 3. BLE stack

### 3.1 Files and session

**CODE-READ** `vehicle/ble/feature/session.h:40-278`: `TeslaSession<Rng>` does ECDH P-256, extracts X as shared secret (`:137-143`), manual HKDF-SHA256 Extract+Expand with salt = vehicle challenge (`:149-186`), AES-128-GCM encrypt with little-endian counter IV (`:217-257`). All mbedTLS contexts are freed unconditionally after the nested loops (`:189-192`, `:202-204`, `:247`). Compare old todo P1-012 — fixed, below.

**CODE-READ** `vehicle/ble/handler/dispatch.h`: `connectToVehicle` uses last-8-VIN suffix (`:29-53`); `runAuthCommand` (`:62-97`); `executeTeslaCommand` routes `key:gen/show/role`, `vin:`, `key:send`, `wake`, `charge:start/stop/amps/limit`, `climate:on/off` (`:108-295`). Scan filter: `name.endsWith(vinSuffix)` (`io/ble/esp32/ble_client.h:116-120`). **SUMMARY-ONLY** repo-state `:28`: real advert Local Name is `S+sha1(VIN)[:8]C`, so our suffix cannot match.

**CODE-READ** persistence: key, role, VIN in NVS namespace `tcm` via `io/ble/esp32/tesla_board.h:34-37,70-155`; role string `"owner"`→numeric via `roleValue()` (`:124-128`); unknown role defaults to `charging_manager` (`:101`), least-privilege fallback.

### 3.2 Roles, proto fields, add-key

**CODE-READ our side:**

| Item                                | Ours                                                                                                                  | Evidence                                  |
| ----------------------------------- | --------------------------------------------------------------------------------------------------------------------- | ----------------------------------------- |
| ROLE_OWNER / ROLE_CHARGING_MANAGER  | 4 / 5                                                                                                                 | `vehicle/ble/feature/key.h:22-23`         |
| KEY_FORM_FACTOR_NFC_CARD / BLE_KEY  | 3 / 2                                                                                                                 | `vehicle/ble/msg.h:25-26`                 |
| Add-key encoding                    | `key=1` raw 65 bytes, `role=2`, `form_factor=5`, wrapped at UnsignedMessage field 2                                   | `msg.h:63-95`                             |
| Destination / protobuf bytes / uuid | `to_destination=2`, `protobuf_message_as_bytes=5`, `session_info_request=6`, `session_info` parse field 10, `uuid=14` | `msg.h:90-92, 131-133, 312-314, 364-368`  |
| Wake action                         | empty field 25                                                                                                        | `msg.h:165-171`                           |
| Seat heater level/position          | `fieldEmpty(level)` / `fieldEmpty(position)` — enums used as field numbers with wire type 2                           | `vehicle/ble/feature/carserver.h:218-233` |

**CODE-READ upstream proto under `legacy/teslamotors-vehicle-command/`:**

| Item            | Upstream                                                                                                                                            | Evidence                                                                                                                                                                                                                             |
| --------------- | --------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Roles           | OWNER=2, DRIVER=3, FM=4, VEHICLE_MONITOR=5, CHARGING_MANAGER=6                                                                                      | `pkg/protocol/protobuf/keys.proto` values as quoted in `docs/legacy/tesla-local-control-vehicle-command.md:56,235` (SUMMARY-ONLY for this specific file; the 4/5 contradiction is independently repeated in four sibling deep dives) |
| Add-key path    | `UnsignedMessage.WhitelistOperation=16` → `addKeyToWhitelistAndAddPermissions=5` → `PermissionChange{key=1 (PublicKey{PublicKeyRaw=1}), keyRole=4}` | `vcsec.proto:104-108, 121-127` CODE-READ; `PublicKey` at `:43-45`                                                                                                                                                                    |
| Form factor     | `NFC_CARD=1`, `CLOUD_KEY=9`                                                                                                                         | `vcsec.proto:29-36` CODE-READ                                                                                                                                                                                                        |
| RoutableMessage | `to_destination=6`, `protobuf_message_as_bytes=10`, `session_info_request=14`, `session_info=15`, `uuid=51`                                         | `universal_message.proto:84-99` CODE-READ                                                                                                                                                                                            |

Verdict: every BLE envelope/add-key field number and the role constants disagree with the upstream SDK proto. No bench capture exists in this session; severity is high because these are transport-visible wire bytes, not cosmetic.

## 4. Persistence, command dispatch, client API

- **CODE-READ** `core/persist/esp32/board.h:118` + `core/persist/nvs.h:114-157`: `saveSettings` writes all fields incl. `singleShotTx`, `mqttEnabled/Host/Port/Interval`, `wiperPersistEnabled`, `apInjectionGateEnabled`, `varAuto`, nag mode. `loadSettings` at `nvs.h:49-104`.
- **`rawCanListen` has no NVS key at all** — not in load (`nvs.h:63-101`) nor save (`:120-156`); dispatch branch `client/command/dispatch.h:138-144` has no `saveSettings`. P1-004 remains.
- **CODE-READ** dispatch persistence fixed in-place for single-shot (`:788-796` saves at `:791` + `driverSetSingleShot`), MQTT (`:814-821` saves at `:817`), wiper persist (`:730-737` saves at `:733`).
- **CODE-READ** `client/command/dispatch.h:14-17` includes `io/ble/esp32/tesla_board.h` under `BOARD_ENABLE_BLE`; wrapper `static void executeTeslaCommand(const char*)` at `tesla_board.h:188-191` forwards to the templated `Tesla::executeTeslaCommand(s_board, sub)`. The 1-arg call at `dispatch.h:1080` therefore compiles. P1-013 fixed.
- **CODE-READ** REST: `client/api/auth.h:34-57` generates 32-hex-char key with `esp_random()` and persists in `tcm_auth`; `requireAuth` at `:68-88` checks `X-API-Key` then `?apiKey=`, returns 401/403, and **allows all requests when `apiKeyRequired == false`** (default false at `:38`). `client/api/routes.h:29-110` builds the full state JSON; `/api/command` proxies to `executeCommand` (not read in full).
- **CODE-READ** `io/schemas/io.schema.json` advertises `otaInProgress`/`txPaused` (`:1205-1206`) that firmware never emits (see §2.3).

## 5. Native tests — what exists, what they assert

**CODE-READ / TEST-READ** `git ls-files 'firmware/test/test_*/*.cpp'` = 84 suites: 6 `test_esp32_*` hardware suites (`can_driver`, `can_loopback`, `can_recorder`, `nvs_persist`, `wifi_api`, `led_gpio`) and 78 `test_native_*` host suites. Only one BLE-named suite exists: `test_native_ble_distance` (mode-name round trip, `test_ble_distance.cpp:34,168`) — **no BLE wire/proto/encoding test at all**.

Representative assertions actually read:

- `test_native_ap_gate/test_ap_gate.cpp:82-134,137-269`: asserts `apGateOpen()` truth table, that drive-mode/seatbelt ticks and `startBurst` are suppressed when the gate is closed, and that standby sets `apGateParked=true`. It does **not** assert gear 0/5 handling, and its helper `applyStandbyTransitionOnCanTimeout` (`:68-79`) duplicates firmware behaviour rather than exercising `vehicle.h`.
- `test_native_crc8/test_crc8.cpp:19-122`: CRC-8 "123456789"→0x3E vector (`:43-47`), determinism of MAGIC_0x229/0x249/0x370 (`:58-86`), counter affects magic (`:112-122`). No real-frame vector, so the wrong 0x249 DLC/CRC cannot be caught.
- `test_native_checksum/test_checksum.cpp:90-116` and `test_native_nag_organic/test_nag_organic.cpp:390,428,453,564`: assert `data[7] == nagChecksum(data)` — self-consistent, not capture-anchored.
- `test_native_multi_feature/test_multi_feature.cpp:48-95`: asserts independent state flags and command-ownership; it does **not** test pedal vs drive-mode 0x334 bit collision (old P1-006).
- `test_native_persist/test_persist.cpp:122-311` and `test_esp32_nvs_persist/test_nvs_persist.cpp:50-293`: save/load round trips for default/HW3/legacy, ECE/LHD/AP-gate/hidden-AP, plus raw NVS primitive round trips. No round trip for `rawCanListen`.
- `test_native_das_drive/test_das_drive.cpp:263-264,337`: NVS round trip for DAS enable/cap/speed limit. No AP-gate or `das:arm` test.
- `test_native_command_roundtrip/test_command_roundtrip.cpp:50-118`: regen/window/sentry/lock emit expected CAN IDs and burst overwrite behaviour.

Untested areas (negative greps over `firmware/test`): BLE protobuf field numbers/roles/add-key, OTA guard, gear-0/5 park semantics, 0x249 frame shape, ALC injection, `can:raw` persistence, AP-gate bypass by DAS drive, advertised-but-unimplemented commands, 0x401 per-cell decode.

## 6. Old firmware todo re-check (P1 / P5 / P9)

Statuses: **FIXED** = current code no longer has the defect; **STILL_PRESENT** = defect confirmed at current `file:line`; **STALE** = premise no longer matches the tree (path removed, code dead, or claim superseded).

### 6.1 Critical P1 items

| ID                                              | Status                  | Current evidence                                                                                                                                                                                                     |
| ----------------------------------------------- | ----------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| P1-001 seat encode `fieldEmpty`                 | STILL_PRESENT           | `vehicle/ble/feature/carserver.h:222-225` (old path `vehicle/ble/carserver.h` is stale; bug moved to `feature/`)                                                                                                     |
| P1-002 single-shot not saved                    | FIXED                   | `client/command/dispatch.h:791` calls `saveSettings(s)` in the `executeSingleShotCmd` branch; key exists `core/persist/nvs.h:92,149`                                                                                 |
| P1-003 MQTT not saved                           | FIXED                   | `client/command/dispatch.h:817`; keys `nvs.h:93-96,150-153`                                                                                                                                                          |
| P1-004 raw CAN not saved                        | STILL_PRESENT           | `client/command/dispatch.h:138-144` no save; `nvs.h:63-101,120-156` has no `rawCanListen` key at all                                                                                                                 |
| P1-005 wiper persist not saved                  | FIXED                   | `client/command/dispatch.h:733`; key `nvs.h:89,146`                                                                                                                                                                  |
| P1-006 pedal/drive-mode 0x334 collision         | STILL_PRESENT           | `drive/pedal.h:38-48` (`CHILL=0x20`, `SPORT=0x40`) vs `drive/drive_mode.h:94-110` (`CHILL=0x00`, `PERFORMANCE=0x02<<5=0x40`); both mask bits 5:6 of byte 0                                                           |
| P1-007 ring buffer volatile not atomic          | STILL_PRESENT           | `core/can/ring_buffer.h:34` `volatile uint32_t writeIdx`                                                                                                                                                             |
| P1-008 gear 0/5 treated as parked               | STILL_PRESENT           | `handler/bus/vehicle.h:392-393`; `powertrain.h:27` says 5=SNA                                                                                                                                                        |
| P1-009 turn signal gated on 0x273 capture       | STILL_PRESENT           | `body/turn_signal.h:42-52` gates on `s.hasCtrl` but sends `CAN_ID_VCFRONT_LIGHTS` (0x3F5)                                                                                                                            |
| P1-010 air recirc copies 0x2F3 bytes into 0x2AA | STILL_PRESENT           | `comfort/air_recirc.h:38-39` `memcpy(f.data, s.lastClimate, 5)`                                                                                                                                                      |
| P1-011 track-mode off has no explicit encoding  | STILL_PRESENT (partial) | `body/track_mode.h:22-33` zeroes all 8 bytes on disable; no counter preservation; bench pattern still unconfirmed                                                                                                    |
| P1-012 ECDH free leak                           | FIXED / STALE           | code moved to `vehicle/ble/feature/session.h:124-204`; all four `mbedtls_*_free` calls at `:189-192` run before the `if (!ecdhOk) break;` at `:194`; `vehicle/ble/handler/dispatch.h` (old path) has no mbedTLS code |
| P1-013 `executeTeslaCommand` won't compile      | FIXED                   | `client/command/dispatch.h:14-17` includes wrapper header; `io/ble/esp32/tesla_board.h:188-191` supplies 1-arg wrapper                                                                                               |

**Tally P1: FIXED 5, STILL_PRESENT 8, STALE 0** (P1-012 counted FIXED; premise path is stale).

### 6.2 Medium P5 items

| ID                                       | Status        | Current evidence                                                                                                                                            |
| ---------------------------------------- | ------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- |
| P5-001 DAS file-scope statics            | STILL_PRESENT | `das/das_drive.h:75-92` — 17 file-scope statics incl. `static Preferences dasPrefs`                                                                         |
| P5-002 nag PRNG global                   | STILL_PRESENT | `fsd/nag/math.h:16` `static uint32_t _nagPrngState`                                                                                                         |
| P5-003 ALC static counter                | STILL_PRESENT | `fsd/auto_lane_change.h:46`                                                                                                                                 |
| P5-004 low-bit PRNG float                | STILL_PRESENT | `fsd/nag/math.h:36` `(_nagXorshift() & 0xFFFFFF) / 16777216.0f`                                                                                             |
| P5-005 duplicate grip magnitude          | STILL_PRESENT | `fsd/nag.h:279` and `:291`                                                                                                                                  |
| P5-006 health only bus 0 receiving       | STILL_PRESENT | `core/can/health.h:62-67`; buses 1/2 leave `receiving=false` at `:51`                                                                                       |
| P5-007 asymmetric TX fail counter        | STILL_PRESENT | `core/driver/common.h:164,236` plain increment vs `:175-181` interrupt-guarded read/reset                                                                   |
| P5-008 non-static IRAM ISRs              | STILL_PRESENT | `core/driver/esp32/board.h:43-45,49-51`                                                                                                                     |
| P5-009 log ring stale text               | STILL_PRESENT | `core/log/ring.h:42-45` clears only `used`                                                                                                                  |
| P5-010 three `static Preferences prefs`  | STALE         | only `core/persist/nvs.h:14` and `esp32/board.h:14`; `nvs.h` has no includers (negative grep) so no live conflict; `native.h:25` uses a local `Preferences` |
| P5-011 duplicate native persist files    | STALE         | `native.h` and `native/board.h` exist but neither is included by `firmware/lib` or `firmware/test` (negative grep); dead code, not a compile defect         |
| P5-012 seat empty-string guard           | STILL_PRESENT | `comfort/seat.h:125` `cmd[strlen(cmd) - 1]`                                                                                                                 |
| P5-013 unconditional Preferences include | STILL_PRESENT | `das/das_drive.h:15`; native tests supply a stub via `test/support/Preferences.h`                                                                           |
| P5-014 turn-signal DLC guard             | STILL_PRESENT | `body/turn_signal.h:28-31` no `if (f.dlc < 1) return;`                                                                                                      |
| P5-015 dead torque clamps                | STILL_PRESENT | `fsd/nag.h:470-472`                                                                                                                                         |
| P5-016 three GTW_CAR_CFG blocks          | STILL_PRESENT | `handler/bus/vehicle.h:355, 438, 528`                                                                                                                       |

**Tally P5: STILL_PRESENT 14, STALE 2, FIXED 0.**

### 6.3 Low P9 items

| ID                                      | Status        | Current evidence                                                                                             |
| --------------------------------------- | ------------- | ------------------------------------------------------------------------------------------------------------ |
| P9-001 gear comment says 5=Neutral      | STILL_PRESENT | `handler/bus/vehicle.h:392` vs `telemetry/powertrain.h:27` (5=SNA)                                           |
| P9-002 readers in ids.h                 | STILL_PRESENT | `vehicle/can/ids.h:107-121` (`readFollowDistance`, `isFSDSelectedInUI`)                                      |
| P9-003 stale DAS angle on re-enable     | FIXED         | `das/das_drive.h:298` `dasAppliedAngle = 0.0f;` in `dasDriveSetEnabled`                                      |
| P9-004 40-line `buildDasControlFrame`   | STILL_PRESENT | `das/das_drive.h:127-200`                                                                                    |
| P9-005 undocumented hot byte 7 of 0x273 | STILL_PRESENT | `comfort/light.h:106` bits 59-60 with no bit-map comment                                                     |
| P9-006 BMS magic numbers                | STILL_PRESENT | `telemetry/bms.h:95,106,117,128,139,183,291,451,473`                                                         |
| P9-007 wrong gear comment               | STILL_PRESENT | same as P9-001, `vehicle.h:392`                                                                              |
| P9-008 recorder copies 8 bytes          | FIXED         | `core/can/recorder.h:159` `entry.data[i] = (i < f.dlc) ? f.data[i] : 0;`                                     |
| P9-009 25-line header architecture doc  | STILL_PRESENT | `fsd/nag.h:9-33`                                                                                             |
| P9-010 `NAG_ORG_*` units in names       | STILL_PRESENT | `fsd/nag.h:85` `NAG_ORG_RAW_EXC_BASE` with only a comment `// ~3.02 Nm`                                      |
| P9-011 MQTT constants dual definition   | STALE         | `core/persist/keys.h:48-51` uses literal strings; no reference to `MQTT_*` macros from `mqtt_bridge.h:13-18` |
| P9-012 ban shield reset asymmetric      | STILL_PRESENT | `safety/ban_shield.h:29-34` resets only when enabled                                                         |

**Tally P9: FIXED 2, STILL_PRESENT 9, STALE 1.**

**Total firmware old items: FIXED 7, STILL_PRESENT 31, STALE 3 (41 items checked).**

## 7. New findings (verified this pass)

### 7.1 Critical

1. **No OTA TX guard.** `CAN_ID_GTW_CAR_STATE 0x318` defined (`vehicle/can/ids.h:52`) but never handled; `otaInProgress`/`txPaused` absent from all `.h` files and from `State` (`core/types.h`), though advertised in `io.schema.json:1205-1206`. Impact: injection continues during vehicle OTA — the exact scenario the contract promises to pause. Fix: add 0x318 OTA decode in `handler/bus/vehicle.h`, set `s.txPaused`, and check it centrally in `driverSend`/burst/variant TX paths.
2. **BLE RoutableMessage envelope field numbers wrong.** `to_destination=2` vs proto 6; `protobuf_message_as_bytes=5` vs 10; `session_info_request=6` vs 14; `session_info` parser looks for 10 vs 15; `uuid=14` vs 51 (`msg.h:90-92,131-133,312-314,364-368` vs `universal_message.proto:84-99`). Impact: no authenticated CarServer command or session handshake will decode on a real vehicle. Fix: regenerate builders from the SDK proto (can-review-checklist + bench capture before merge).
3. **BLE add-key request encoded against a non-existent schema.** Ours writes `UnsignedMessage.field 2` with a flat `{key=1 raw bytes, role=2, form_factor=5}` (`msg.h:63-95`); proto requires `UnsignedMessage.WhitelistOperation=16 → addKeyToWhitelistAndAddPermissions=5 → PermissionChange{key=1 PublicKey{PublicKeyRaw=1}, keyRole=4}` (`vcsec.proto:104-108,121-127,43-45`). `tesla:key:send` therefore cannot add a key. Fix: rebuild `buildAddKeyRequest` from the proto and add a wire-byte test.
4. **Key roles and form factor wrong.** `ROLE_OWNER=4` / `ROLE_CHARGING_MANAGER=5` (`key.h:22-23`) are FM / VEHICLE_MONITOR upstream (OWNER=2, CHARGING_MANAGER=6); `KEY_FORM_FACTOR_NFC_CARD=3` (`msg.h:25`) vs upstream 1; `roleValue()` maps "owner"→4 (`tesla_board.h:124-127`). Impact: requests the wrong permission class (FM cannot command over BLE on 2023.38+, per SUMMARY-ONLY sibling doc). Fix: set 2/6, NFC=1, add role-value test.
5. **AP gate fail-open + DAS drive bypass.** `apGateOpen()` true when gate disabled (`types.h:837-842`); parked includes gear 0/5 (`vehicle.h:392-393`) and is forced true on CAN timeout (`main.cpp:149`); `dasTick` has no gate check (`das_drive.h:414-498`) and `drive:on` only flips an enable flag (`:515-519`); documented `das:arm` is unimplemented. Impact: steering/torque injection possible while the gate is nominally enforcing. Fix: require a single owner for all gates, gate `dasTick` on `apGateOpen()`, and treat only gear 1 as parked.
6. **ALC 0x249 frame shape wrong.** DLC-3 + additive magic CRC (`auto_lane_change.h:48-59`) vs capture-verified DLC-4 CRC (SUMMARY-ONLY inherited CAPTURE, repo-state `:29`). Impact: lane-change confirmation may be ignored or misread; safety-relevant. Fix: re-derive from capture and route through `docs/checklists/can-review-checklist.md`.
7. **CarServer seat encode emits malformed protobuf.** `seat.fieldEmpty(level)` / `fieldEmpty(position)` (`carserver.h:222-225`) write length-delimited empty messages with enum values as field numbers; correct encoding is varint enum fields. Impact: BLE seat-heater actions invalid. Fix: use `fieldVarint` with the enum field numbers and assert wire bytes in a test.

### 7.2 Medium

8. **11 advertised-but-unimplemented surfaces.** Negative greps across `firmware/lib` confirm no handlers for `smt:*`, `elm327:*`, `gvret:*`, `ha:*`, `espnow:*`, `speedalert:*`, `btnmap:*`, `das:arm|disarm|status` (this last one additional to the sibling list), no `mqttShouldPublish` caller outside `test_mqtt.cpp:87-98` and `mqttConnected` never set true (init only, `core/types.h:717,791`), no 0x401 decode (`ids.h:71` + `handler/filters.h:131` only). "0-100 timer" remains **UNVERIFIED**. Fix: implement or remove from `docs/reference/commands.md`, `io.schema.json`, and client UI.
9. **BLE scan filter cannot match a real advert.** `connectToVehicle` derives last-8-VIN suffix (`vehicle/ble/handler/dispatch.h:36-43`); `ble_client.h:116-120` requires `name.endsWith(suffix)`; real Local Name is `S+sha1(VIN)[:8]C` (SUMMARY-ONLY repo-state `:28`). Fix: derive the hashed advert name from VIN and match exactly.
10. **No BLE wire tests.** Only `test_native_ble_distance` exists (§5). Fix: add `test_native_ble_proto` asserting exact bytes for all builders against SDK proto vectors.
11. **`can:raw` not persisted** (P1-004). Fix: add `PKEY_RAW_CAN` to `persist/keys.h`, load/save in `nvs.h`, and `saveSettings` in the dispatch branch.
12. **0x334 pedal/drive-mode collision** (P1-006). Fix: allocate non-overlapping bit fields and rebuild from `lastDrive`.
13. **Freshness is boolean, not time-bounded.** `hasCtrl` (`vehicle.h:52`) and `apGateApActive` (`:135`) have no expiry. Fix: store timestamps and require age < 1 s in every injection gate.

### 7.3 Low

14. Dead `MAGIC_0x370` table in production (`crc8.h:57-58`); `nagChecksum` is the settled path (SUMMARY-ONLY CAPTURE repo-state `:42`). Fix: delete table and its two determinism-only tests.
15. Stale/dead persistence headers `core/persist/nvs.h`, `native.h`, `native/board.h` (§6.2). Fix: delete or wire into the build.
16. Hardcoded `upload_port = COM4` (`platformio.ini:99-100`). Fix: document `PLATFORMIO_UPLOAD_PORT` override.

Severity note: items 1–7 are transport-visible or injection-safety relevant; per `AGENTS.md` and `CONTRIBUTING.md`, any change to 1/5/6/12/13 must walk `docs/checklists/can-review-checklist.md`.

## 8. Effort / risk / proof table

| Work item                                        | Rough hours   | Risk                          | Test that would prove it                                                                                          |
| ------------------------------------------------ | ------------- | ----------------------------- | ----------------------------------------------------------------------------------------------------------------- |
| OTA guard (0x318 → txPaused, central TX check)   | 3–5           | High (changes TX gating)      | Native test feeding a real 0x318 capture with OTA bit then asserting burst/drive-mode TX count == 0               |
| BLE envelope field numbers (`msg.h`)             | 2–4           | High (wire break until bench) | `test_native_ble_proto` asserting exact field tags 6/10/14/15/51 against SDK vectors                              |
| BLE add-key rebuild + role/form-factor constants | 4–6           | High (pairing path)           | Wire-byte test for `UnsignedMessage.16 → PermissionChange{1,4}` + bench capture of an accepted add-key            |
| AP gate gear/fail-open + DAS gate wiring         | 3–5           | High (safety)                 | Extend `test_native_ap_gate` with gear 0/5 cases and a `dasTick` case with gate closed → 0 sends                  |
| ALC 0x249 DLC/CRC                                | 3–5           | High (safety, capture-first)  | Replay 1048-frame capture; assert generated frame matches accepted frames, plus can-review checklist              |
| CarServer seat `fieldVarint`                     | 0.5           | Medium                        | Wire-byte assertion on `buildSeatHeaterAction` output                                                             |
| `can:raw` persistence                            | 1             | Low                           | `test_native_persist` save/load round trip for `rawCanListen`                                                     |
| 0x334 pedal/drive-mode split                     | 1–2           | Medium                        | `test_native_multi_feature` sets both, asserts both bitfields survive                                             |
| Freshness timestamps for `hasCtrl`/AP            | 2–3           | Medium                        | Native test with stale capture timestamp → no injection                                                           |
| Implement or remove 11 advertised surfaces       | 6–20 (varies) | Medium (API contract)         | Grep-based CI check: every command in `docs/reference/commands.md` has a dispatch branch or is marked unsupported |
| Native BLE proto test suite (new)                | 3–4           | Low                           | New `test_native_ble_proto` suite green in `env:native`                                                           |
| Dead code cleanup (P5-010/011, MAGIC_0x370)      | 1–2           | Low                           | Full `pio test -e native` + `npm run lint:all` still green                                                        |
