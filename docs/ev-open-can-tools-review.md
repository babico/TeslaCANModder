# ev-open-can-tools + Plugin System — Implementable Features Review

**Source repos reviewed:**

- `legacy/ev-open-can-tools-ev-open-can-tools` — full firmware codebase
- `legacy/ev-open-can-tools-plugins` — community plugin JSON library (added as git submodule)

**Reviewer scope:** What is not yet in our firmware, what we have better, what to adopt.

---

## Executive Summary

| Category                                | ev-open-can-tools has                      | We have                                                | Action                          |
| --------------------------------------- | ------------------------------------------ | ------------------------------------------------------ | ------------------------------- |
| Plugin Engine (runtime JSON rules)      | ✅ Full system (SPIFFS + WiFi OTA install) | ❌ None                                                | **Implement**                   |
| Web Dashboard                           | ✅ Full HTTP dashboard (WiFi AP + STA)     | ✅ WiFi dashboard (different stack)                    | **Merge patterns**              |
| Bus `mux_mask` field in rules           | ✅ Yes (`"mux_mask": 7`)                   | ❌ Hard-coded `& 0x07` everywhere                      | **Add to protocol**             |
| `bus` field per rule (`VEH`, `CHASSIS`) | ✅ Yes (multi-bus plugins)                 | ❌ None — our dispatch is bus-aware but plugins aren't | **Add to protocol**             |
| `signal` annotation in ops              | ✅ Yes (human-readable CAN signal names)   | ❌ None                                                | **Add as optional field**       |
| `description` per op                    | ✅ Yes                                     | ❌ None                                                | **Add as optional field**       |
| Plugin priority + enable/disable        | ✅ Yes (per-plugin toggle + sort)          | ❌ None                                                | **Implement**                   |
| Plugin test mode (rule preview)         | ✅ Yes (`PluginTestState`)                 | ❌ None                                                | **Implement**                   |
| CAN Sniffer ring buffer                 | ✅ Yes (30-frame ring, HTTP endpoint)      | ✅ Similar (our `sendStream`)                          | **Better than theirs**          |
| CAN Recorder (2000 frames)              | ✅ Yes (SPIFFS download)                   | ❌ None                                                | **Implement**                   |
| `or_byte` + `and_byte` operations       | ✅ Yes                                     | ❌ Only `set_bit` / `set_byte` conceptually            | **Implement**                   |
| `checksum` op (auto Tesla CRC)          | ✅ Yes                                     | ✅ `computeVehicleChecksum()` exists                   | **Wire into plugin engine**     |
| RGB status LED                          | ✅ Yes (injecting/idle/standby colors)     | ❌ None                                                | **Nice to have**                |
| OTA firmware via HTTP                   | ✅ Yes (`/update` OTA, WiFi STA mode)      | ✅ Yes                                                 | **Already have**                |
| WiFi STA (internet client)              | ✅ Yes (firmware download, beta channel)   | ✅ Yes                                                 | **Already have**                |
| Hidden AP (SSID broadcast off)          | ✅ Yes (`apHidden`)                        | ❌ None                                                | **Implement**                   |
| Write Probe (TX→RX echo check)          | ✅ Yes (`DashWriteProbe`)                  | ❌ None                                                | **Implement**                   |
| NAG killer (EPAS torque spoof)          | ❌ Simple legacy only                      | ✅ Full (legacy/safe/natural modes)                    | **Ours is better**              |
| BMS telemetry                           | ❌ None                                    | ✅ Full multi-mux BMS decode                           | **Ours is vastly better**       |
| TLSSC restore (0x331 spoof)             | ✅ `bypass-tlssc` plugin                   | ✅ `tlssc.h` native                                    | **Both have, theirs is plugin** |
| Summon EU unlock                        | ✅ Plugin (bits 19, 47)                    | ✅ Native `summon.h`                                   | **Ours is native**              |
| Auto Lane Change (ALC)                  | ❌ None                                    | ✅ `auto_lane_change.h`                                | **Ours is better**              |
| Firmware version compat info            | ❌ None                                    | ✅ `fw_compat.h`                                       | **Ours is better**              |
| Ban shield / GTW shield                 | ❌ None                                    | ✅ `ban_shield.h`                                      | **Ours is better**              |
| LHD (Left Hand Drive) mode              | ✅ Beta plugin (bit 41)                    | ❌ None                                                | **Implement**                   |
| FSD stops control (TSLLC)               | ✅ Plugin (bits 38, 39)                    | ❌ Not as fine-grained                                 | **Evaluate**                    |
| Continue-on-green with CIPV             | ✅ Plugin (bit 39)                         | ❌ None                                                | **Implement**                   |

---

## 1. Plugin Engine — HIGH PRIORITY ★★★

**What they have:** A full JSON-driven runtime plugin system (SPIFFS + WiFi). Users install plugins without reflashing. Up to 8 plugins × 16 rules × 8 ops each.

**Plugin schema (from their repo):**

```json
{
	"name": "Example Plugin",
	"version": "1.0",
	"author": "ev-open-can-tools",
	"description": "Human readable",
	"rules": [
		{
			"id": 1021,
			"bus": "VEH",
			"mux": 0,
			"mux_mask": 7,
			"ops": [
				{
					"type": "set_bit",
					"bit": 46,
					"val": 1,
					"signal": "UI_autosteerEnabled",
					"description": "Enable FSD"
				},
				{ "type": "set_byte", "byte": 1, "val": 7, "mask": 63 },
				{ "type": "or_byte", "byte": 1, "val": 32 },
				{ "type": "and_byte", "byte": 1, "val": 0xff },
				{ "type": "checksum" }
			],
			"send": true
		}
	]
}
```

**Op types:**

| Op         | What it does                                                              |
| ---------- | ------------------------------------------------------------------------- |
| `set_bit`  | Set/clear a single bit by bit-index (0-63)                                |
| `set_byte` | Set a byte with mask: `data[byte] = (data[byte] & ~mask) \| (val & mask)` |
| `or_byte`  | `data[byte] \|= val`                                                      |
| `and_byte` | `data[byte] &= val`                                                       |
| `checksum` | Auto-compute Tesla vehicle checksum into byte[7]                          |

**What we need to build:**

- `PluginEngine` struct stored in NVS/LittleFS (replace SPIFFS with LittleFS — SPIFFS is deprecated)
- JSON schema parser using `ArduinoJson` (wire already available in WiFi build)
- Hook in `handleMessage()` after our existing dispatch returns (just like their `appPluginProcess` hook)
- Serial commands: `plugin:list`, `plugin:install`, `plugin:remove:<idx>`, `plugin:toggle:<idx>`
- WiFi endpoints: `POST /plugin/install`, `GET /plugin/list`, `DELETE /plugin/<idx>`
- Schema additions to `serial-output.schema.json` for plugin status output

**Key difference from theirs:** Our dispatch is multi-bus aware. Plugin rules need to carry a `bus` field (0=CHASSIS, 1=VEHICLE, 2=BODY) otherwise plugins only apply on BUS_CHASSIS.

**Effort:** ~400 lines. High value for user extensibility.

---

## 2. New Op Types: `or_byte` + `and_byte` — MEDIUM PRIORITY ★★

Our current dispatch uses raw bit-level ops in C. The plugin engine op types cover:

- `or_byte` — useful for toggling flags without clearing neighbors (e.g. ISA chime suppress: `byte[1] |= 0x20`)
- `and_byte` — useful for clearing flags with a mask

These don't exist in our current handler code as distinct primitives. Should be implemented in the plugin engine from day one.

---

## 3. Plugin Schema Fields: `bus`, `mux_mask`, `signal`, `description` — MEDIUM PRIORITY ★★

**`bus` field:** Their newest plugin (TSLLC/continue-on-green) adds `"bus": "VEH"`. Without this, plugins can't target vehicle bus frames like 0x273 (UI_vehicleControl). We need this.

**`mux_mask` field:** They use `"mux_mask": 7` to specify which bits of byte[0] to use for mux matching. Allows flexible mux definitions. We hard-code `& 0x07` everywhere.

**`signal` + `description` fields:** Optional documentation annotations per op. Zero runtime cost; they improve the dashboard UX and make plugins self-explaining. Add them to the schema parser but ignore values at runtime.

---

## 4. LHD (Left Hand Drive) Mode — LOW-MEDIUM PRIORITY ★

**Their Beta plugin:**

```json
{ "id": 1016, "ops": [{ "type": "set_bit", "bit": 41, "val": 0 }] }
```

Clears `UI_drivingSide` bit 41 on frame 0x3F8 (ID 1016), switching the AP to LHD mode.

**Status in our codebase:** Not implemented. Frame 0x3F8 is not in our vehicle bus filter list.

**To implement natively:**

1. Add `CAN_ID_UI_DRIVE_SIDE 0x3F8` to `infra/can.h`
2. Add `bool lhdEnabled` to State and persist it
3. Add to BUS_VEHICLE filter list
4. In dispatch: if `lhdEnabled`, clear bit 41 and retransmit
5. Add `lhd:on/off` serial command

Alternatively: this is a perfect candidate for the plugin engine — no native code needed once plugins are implemented.

---

## 5. FSD Stops Control + Continue-on-Green (TSLLC bits 38/39) — MEDIUM PRIORITY ★★

**Their HW3 plugin:**

```json
{
	"id": 1021,
	"mux": 0,
	"ops": [
		{ "type": "set_bit", "bit": 38, "val": 1, "signal": "UI_fsdStopsControlEnabled" },
		{ "type": "set_bit", "bit": 39, "val": 1, "signal": "UI_fsdContinueOnGreenWithCIPV" }
	]
}
```

**In our codebase:** Our `tlssc.h` spoofs `0x331` (DAS_autopilotConfig) to set the SELF_DRIVING tier. That's a gateway-level approach. The plugin approach directly manipulates the UI frame (`0x3FB` / 1021) to enable individual FSD sub-features:

- Bit 38: `UI_fsdStopsControlEnabled` — stop-sign and traffic-light handling
- Bit 39: `UI_fsdContinueOnGreenWithCIPV` — continue on green when lead car present

These are orthogonal to our TLSSC feature and worth adding as dedicated flags. Both are on the CHASSIS bus (frame 1021 mux 0), which we already intercept.

**To implement:**

1. Add `bool fsdStopsControl` + `bool fsdContinueOnGreen` to State
2. In `hw3.h` / `hw4.h` mux-0 handler, set bits 38/39 when enabled
3. Add `fsd-stops:on/off` + `fsd-green:on/off` serial commands

---

## 6. CAN Recorder (2000-frame SPIFFS dump) — MEDIUM PRIORITY ★★

**What they have:**

- 2000-frame circular ring buffer (`RecFrame recBuf[2000]`)
- Records all observed CAN frames with timestamps
- HTTP endpoint: `GET /can/recorder/download` returns CSV/JSON
- Start/stop via web dashboard toggle

**What we have:** `sendStream` pushes live frames over serial/BLE/WiFi but no persistent recording.

**To implement:**

- Add a `RecFrame` ring buffer to State (or a separate static struct in the WiFi board.h)
- WiFi route: `GET /can/record/start`, `GET /can/record/stop`, `GET /can/record/download`
- Serial command: `canrec:start` / `canrec:stop`

Note: 2000 × (4+1+8) bytes = ~26KB. LittleFS can hold this easily; RAM cannot for a static array. Use LittleFS streaming.

---

## 7. Write Probe (TX→RX Echo Verification) — LOW PRIORITY ★

**What they have:** `DashWriteProbe` — after injecting a modified CAN frame, the dashboard listens for the echo of that exact frame ID back on the bus. If the received bytes match what was sent, it confirms injection success. States: `PENDING → MATCH | DIFFERENT | FAILED`.

**What we have:** `sendLog()` reports injection but doesn't verify it was accepted on the bus.

**To implement:** Simple feedback loop — after `driverSend()`, arm a one-shot listener for the same CAN ID on the same bus, compare received data against sent data within a timeout window.

---

## 8. Hidden WiFi AP — LOW PRIORITY ★

**What they have:** `apHidden` flag — when true, the WiFi AP SSID is not broadcast. Users must know the SSID to connect. Adds a layer of obscurity.

**To implement:**

- Add `bool apHidden` to State, persist it
- In `io/wifi/esp32/init.h`: pass `WiFi.softAP(ssid, pass, channel, hidden)` with hidden=1
- Add `ap-hidden:on/off` serial command
- Expose in WiFi config JSON output

---

## 9. Plugin Community Library Alignment

Now that the plugins submodule is at `legacy/ev-open-can-tools-plugins/`, these plugins are directly testable with our hardware once the plugin engine is implemented. All plugin JSON files are compatible with our hardware variants:

| Plugin                                         | Target  | CAN Frame                        | Status in our firmware                      |
| ---------------------------------------------- | ------- | -------------------------------- | ------------------------------------------- |
| `All HW/summon-eu-unlock.json`                 | HW3+HW4 | 1021 mux=1, bits 19+47           | ✅ Native (summon.h)                        |
| `HW3/ad-activation-hw3.json`                   | HW3     | 1021 mux=0, bit 46               | ✅ Native (hw3.h)                           |
| `HW3/bypass-tlssc-hw3.json`                    | HW3     | 1021 mux=0, bits 38+46           | ✅ TLSSC via tlssc.h + hw3; bit 38 missing  |
| `HW3/enable-TSLLC-and-continue-on-green.json`  | HW3     | 1021 mux=0, bits 38+39           | ❌ bits 38+39 not set natively              |
| `HW4/fsd-activation-only-hw4.json`             | HW4     | 1021 mux=0, bits 46+60           | ✅ Native hw4.h (bit 60 = HW4 extended bit) |
| `HW4/bypass-tlssc+fsd-hw4.json`                | HW4     | 1021 mux=0, bits 38+46+60        | ✅ Partial (bit 38 missing)                 |
| `HW4/emergency-vehicle-detection+fsd-hw4.json` | HW4     | 1021 mux=0, bits 46+59+60        | ✅ Partial (bit 59 = EVD, not set natively) |
| `HW4/isa-chime-suppress-hw4.json`              | HW4     | 921, or_byte 1 val=32 + checksum | ✅ Native (isaChimeSuppress)                |
| `HW4/hw4-speed-offset-plus-{5,7,10,15}.json`   | HW4     | 1021 mux=2, set_byte             | ✅ Native (speedOffset)                     |
| `Beta/LHD.json`                                | HW3+HW4 | 1016, bit 41                     | ❌ Not implemented                          |

**Gap identified:** Bit 38 (`UI_fsdStopsControlEnabled`) and bit 59 (EVD enable on HW4) are not set natively in our `hw4.h` / `hw3.h` even when FSD is enabled. These are additive and safe to add.

---

## 10. Architecture Differences — What We Do Better

These exist in ev-open-can-tools but our implementation is significantly superior:

| Feature                   | ev-open-can-tools      | Us                                                 |
| ------------------------- | ---------------------- | -------------------------------------------------- |
| BMS telemetry             | ❌ None                | ✅ 15+ decoded fields, multi-mux                   |
| Nag killer                | Basic (EPAS bit clear) | ✅ legacy/safe/natural modes with adaptive timing  |
| Auto Lane Change          | ❌ None                | ✅ Full ALC confirm with stalk/Palladium injection |
| Multi-bus (3 buses)       | ❌ Single bus          | ✅ CHASSIS + VEHICLE + BODY                        |
| Firmware compat detection | ❌ None                | ✅ `fw_compat.h` with version decode               |
| GTW Shield (0x7FF)        | ❌ None                | ✅ `ban_shield.h` — learning + armed modes         |
| Ban detect                | ❌ None                | ✅ `ban_detect.h` — telemetry anomaly scoring      |
| Protocol versioning       | ❌ None                | ✅ `packages/protocol/` with schema validation     |
| Single-shot TX mode       | ❌ None                | ✅ `single_shot.h` — avoids cascading bus errors   |
| OTA safety (pause TX)     | ❌ None                | ✅ `otaInProgress` flag halts all injection        |

---

## Recommended Implementation Order

1. **Plugin Engine** (highest leverage — makes all plugin JSON installable without reflash)
    - Implement `PluginData`, `PluginRule`, `PluginOp` structs in `feature/plugin_engine.h`
    - Store in LittleFS (not SPIFFS — deprecated on ESP32 Arduino 3.x)
    - Hook after `handleMessage()` in dispatch
    - Expose via serial commands + WiFi REST endpoints

2. **Op types `or_byte` + `and_byte`** — needed on day one of plugin engine

3. **Bus-aware plugin rules** — `bus` field required for vehicle-bus plugins

4. **FSD stops control + continue-on-green bits** (38, 39) — 5-line change in hw3.h/hw4.h, high community demand

5. **EVD bit 59 (HW4)** — set when `emergencyVehicleDetection` is enabled (matches plugin expectation)

6. **LHD mode** — either native or deferred to plugin engine

7. **CAN Recorder** — useful for diagnostics, implement with LittleFS streaming

8. **Hidden AP** — low effort, privacy improvement

9. **Write Probe** — nice dashboard UX, low priority for firmware

---

## Files to Create/Modify

| File                                                       | Change                                                                                                          |
| ---------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------- |
| `firmware/lib/feature/plugin_engine.h`                     | New — full plugin engine                                                                                        |
| `firmware/lib/core/types.h`                                | Add `PluginSlot pluginSlots[]`, `fsdStopsControl`, `fsdContinueOnGreen`, `evdEnabled`, `lhdEnabled`, `apHidden` |
| `firmware/lib/handler/hw4.h`                               | Set bits 38, 59 when respective flags enabled                                                                   |
| `firmware/lib/handler/hw3.h`                               | Set bits 38, 39 when respective flags enabled                                                                   |
| `firmware/lib/io/serial/esp32/commands.h`                  | Add `fsd-stops:on/off`, `fsd-green:on/off`, `evd:on/off`, `lhd:on/off`, `plugin:*` commands                     |
| `firmware/lib/io/wifi/esp32/routes.h`                      | Add `/plugin/*` REST endpoints                                                                                  |
| `firmware/lib/core/persist/esp32/board.h`                  | Persist new flags + plugin slots                                                                                |
| `firmware/lib/io/serial/schemas/serial-output.schema.json` | Add plugin status fields                                                                                        |
