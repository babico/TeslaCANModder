# ESP32 Hardware Validation

This document is the step-by-step validation plan for the `esp32-idf/` firmware when a real
ESP32 board is connected over USB.

It is written so another engineer or AI agent can run it end-to-end without guessing.

## Scope

Validate all major runtime behavior on hardware:

- clean boot and flash
- open AP behavior
- no advertised DHCP gateway or DNS
- password setup and login
- authenticated handler changes
- CAN log API and decoded signal/status panel
- OTA upload and slot switch
- password recovery flow
- fail-safe behavior on repeated TX failures
- coredump retrieval path

## Preconditions

- macOS host
- ESP-IDF v6.0 installed
- board connected over USB
- CAN transceiver wired to the target bus or bench rig
- repository checked out locally

Activate IDF first:

```bash
source ~/.espressif/tools/activate_idf_v6.0.sh
cd /Users/phichawat.luk/tesla-fsd-can-mod/esp32-idf
```

## Expected Runtime Defaults

| Item | Expected value |
|---|---|
| SSID | `TeslaFSD` |
| AP auth | Open |
| AP IP | `192.168.4.1` |
| CAN bitrate | `500000` |
| CAN RX pin | `GPIO27` |
| CAN TX pin | `GPIO26` |
| Status LED | `GPIO2` |
| Default handler | `HW3` |

## 1. Discover The USB Serial Port

List likely ports before plugging in the board:

```bash
ls /dev/cu.usb* /dev/tty.usb* 2>/dev/null
```

Plug in the board, then run it again and identify the new device.

Set it once for the session:

```bash
PORT=/dev/cu.usbserial-XXXX
echo "$PORT"
```

If nothing new appears, stop and resolve the USB/driver issue before continuing.

## 2. Clean Build

```bash
idf.py fullclean
idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.release.defaults" build
```

Expected result:

- build succeeds
- final app image fits inside the `0x1E0000` OTA slot

## 3. Erase, Flash, And Monitor

For a first-pass validation, erase flash so password setup starts from a known state.

```bash
idf.py -p "$PORT" erase-flash flash monitor
```

Expected early boot logs include lines similar to:

- `Tesla FSD CAN Mod starting...`
- `Reset reason: ...`
- `CAN fast path started on core 1`
- `AP started: SSID=TeslaFSD CH=6 (no gateway, no DNS)`
- `Running partition: ota_0 offset=0x10000` or `ota_1`
- `Web server started on port 80`
- `System ready`

If boot loops, panics, or the AP line never appears, stop and capture the monitor log.

## 4. Confirm The AP Is Open

On macOS, list nearby Wi-Fi networks and verify `TeslaFSD` is visible.

Join the network manually from the UI or with command-line tools if available.

Expected result:

- network is open, with no WPA password prompt
- client receives an address in `192.168.4.x`
- ESP32 stays reachable at `192.168.4.1`

Basic reachability check:

```bash
ping -c 3 192.168.4.1
curl -s http://192.168.4.1/api/status
```

Expected status result on erased flash:

- `has_password` is `false`
- `handler_type` is `1`
- `handler_name` is `HW3`

## 5. Validate DHCP Does Not Advertise Router Or DNS

Find the Wi-Fi interface name:

```bash
WIFI_IF=$(networksetup -listallhardwareports | awk '/Wi-Fi|AirPort/{getline; print $2; exit}')
echo "$WIFI_IF"
```

Inspect the DHCP lease packet after joining the AP:

```bash
ipconfig getpacket "$WIFI_IF"
```

Expected result:

- client IP is in `192.168.4.x`
- subnet mask is `255.255.255.0`
- no router option is present, or router is `0.0.0.0`
- no DNS option is present, or DNS is `0.0.0.0`

Optional packet-level validation:

```bash
sudo tcpdump -ni "$WIFI_IF" -vvv -s0 'udp port 67 or udp port 68'
```

Reconnect to the AP while `tcpdump` is running and inspect the DHCP ACK.

## 6. First-Boot Password Setup In Browser

Open:

```text
http://192.168.4.1/
```

Expected UI behavior on clean flash:

- first page shown is `First Boot Setup`
- no login prompt appears before setup

Set a password with at least 4 characters.

Expected results:

- serial log contains `Password set`
- browser transitions into authenticated main UI
- a follow-up `GET /api/status` now returns `has_password: true`

## 7. Validate Challenge-Response Login With curl

This verifies the auth API without relying on the browser UI.

Fetch a challenge:

```bash
curl -s http://192.168.4.1/api/challenge
```

Use this helper to generate the login response and token from a known password.

```bash
PASSWORD='your-password-here'
CHALLENGE_JSON=$(curl -s http://192.168.4.1/api/challenge)
TOKEN=$(python - <<'PY'
import base64, hashlib, json, os
password = os.environ['PASSWORD'].encode()
challenge = base64.b64decode(json.loads(os.environ['CHALLENGE_JSON'])['challenge'])
pw_hash = hashlib.sha256(password).digest()
token = hashlib.sha256(pw_hash + challenge).hexdigest()
print(token)
PY
)
curl -s http://192.168.4.1/api/login \
  -H 'Content-Type: application/json' \
  -d "{\"response\":\"$TOKEN\"}"
```

Expected result:

- HTTP 200
- returned JSON contains the same token hex string

Then call an authenticated endpoint:

```bash
curl -s http://192.168.4.1/api/can_log \
  -H "Authorization: Bearer $TOKEN"
```

Expected result:

- HTTP 200
- JSON body contains `count`, `lost`, and `entries`

Also verify the same endpoint without auth returns `401 Unauthorized` once a password exists.

## 8. Validate Handler Switching

Switch handlers through the API:

```bash
curl -s http://192.168.4.1/api/handler \
  -H "Authorization: Bearer $TOKEN" \
  -H 'Content-Type: application/json' \
  -d '{"type":0}'

curl -s http://192.168.4.1/api/status \
  -H "Authorization: Bearer $TOKEN"
```

Repeat for `type` values `1` and `2`.

Expected result:

- `handler_type` updates immediately
- `handler_name` reports `Legacy`, `HW3`, or `HW4`
- no reboot required

## 9. Validate CAN Log Plumbing

With the board connected to a live bus or replay rig, call:

```bash
curl -s http://192.168.4.1/api/can_log \
  -H "Authorization: Bearer $TOKEN"
```

Expected result:

- `count` grows as frames are seen
- `entries` contain both RX (`dir: 0`) and TX (`dir: 1`) frames when matching IDs are present
- `lost` remains stable or low under normal traffic

If the bus is active but `count` stays at zero, stop and investigate CAN wiring, bitrate, and transceiver state.

## 10. Validate The Decoded Signal Panel

Open the main web UI while CAN traffic is present.

Expected panel fields:

- `Handler`
- `FSD UI`
- `Follow Dist`
- `Driving Side`
- `APM Branch`
- `AP Mode`
- `Speed Profile`
- `Speed Offset`

Important interpretation notes:

- The panel is built from the recent CAN log snapshot, so fields may show `-` or `unknown` until matching frames arrive.
- Legacy speed profile is derived from CAN ID `1006`, mux `0`.
- HW3 speed offset is derived from CAN ID `1021`, mux `2`.
- HW4 speed profile is derived from CAN ID `1021`, mux `2`.

Record whether the displayed values track expected on-car or replayed input changes.

## 11. Validate Password Recovery Arm Flow

From the login screen, click `Forgot Password`, or call:

```bash
curl -s -X POST http://192.168.4.1/api/recovery
```

Expected result:

- status shows `recovery_armed: true`
- browser shows `Recovery armed for 30s...`

Within 30 seconds, perform the expected CAN/UI gesture so the firmware sees FSD transition from off -> on -> off.

Expected result on success:

- serial log contains `Password cleared via recovery flow`
- subsequent `GET /api/status` returns `has_password: false`
- web UI returns to first-boot setup state

If recovery expires, `recovery_armed` returns to `false` without clearing the password.

## 12. Validate OTA Upload And Slot Switch

Rebuild first if needed so you have a fresh `.bin`:

```bash
idf.py build
```

Upload the image through the web UI or directly:

```bash
curl -s http://192.168.4.1/api/ota \
  -H "Authorization: Bearer $TOKEN" \
  --data-binary @build/tesla_fsd_can_mod_esp32.bin
```

Expected results:

- HTTP 200 with `{"ok":true}`
- serial log contains `OTA begin: target=...`
- serial log contains `OTA complete, reboot pending`
- device reboots shortly after the HTTP response completes

After reboot, confirm the running partition changed:

- previous run logged `ota_0`, new run should log `ota_1`
- or vice versa

Also confirm the AP and web UI come back after reboot.

## 13. Validate Fail-Safe Behavior

Two ways to trigger this:

1. Disconnect or misconfigure TX so repeated transmit attempts fail.
2. Force a bus-off condition on a controlled bench setup.

Expected result:

- serial log contains `Fail-safe: ... TX failures ...`
- status LED latches on
- `GET /api/status` reports `fail_safe: true`

Record how quickly the fail-safe activates and whether it remains latched until reboot.

## 14. Validate Latency Counters

With real CAN traffic flowing, call:

```bash
curl -s http://192.168.4.1/api/status \
  -H "Authorization: Bearer $TOKEN"
```

Expected fields:

- `latency_us`
- `max_latency_us`
- `min_latency_us`

Record at least:

- idle bus values
- normal traffic values
- stressed traffic values if available

## 15. Validate Coredump Retrieval Path

Only do this on a bench setup where a forced crash is acceptable.

After a crash/reboot cycle, run from `esp32-idf/`:

```bash
idf.py -p "$PORT" coredump-info
```

If deeper inspection is needed:

```bash
idf.py -p "$PORT" coredump-debug
```

Expected result:

- coredump is readable from flash
- symbols resolve against `build/tesla_fsd_can_mod_esp32.elf`

## 16. Sign-Off Checklist

Mark each item pass or fail:

- build succeeds from clean tree
- board flashes and boots cleanly
- AP is visible and open
- no DHCP router offer
- no DHCP DNS offer
- password setup works
- challenge-response login works
- authenticated APIs reject missing token
- handler switching works
- CAN log captures RX/TX frames
- decoded signal panel updates with live traffic
- recovery clears password only during armed window
- OTA reboots into the alternate slot
- fail-safe latches after repeated TX failures
- coredump tooling can read a stored dump

## Capture For Any Failure

If a step fails, always capture:

- exact command used
- HTTP response or UI error text
- serial monitor output around the failure
- whether CAN traffic was present
- whether the failure reproduces after reboot
