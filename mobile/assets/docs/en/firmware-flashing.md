# Firmware Flashing

How to flash firmware to your Arduino Uno using the web app or PlatformIO CLI.

## Web Flasher (Recommended)

The easiest way to flash firmware is through the app's Flasher tab.

### Requirements
- Chrome or Edge browser (Web Serial API)
- Arduino Uno connected via USB
- No other serial monitors open

### Steps
1. Open the app and go to the **Flasher** tab
2. Select your firmware variant:
   - **Serial Only** — lightest, FSD bus only
   - **Serial + Bluetooth** — adds HC-05 support
3. Toggle CAN bus lanes (FSD always on, Vehicle + Body optional)
3. Click **Flash via USB**
4. Select the Arduino serial port when prompted
5. Wait for "Flashed successfully" message
6. The board will auto-reboot with the new firmware

### Troubleshooting
- If flash fails, close other serial connections first
- Try pressing the Arduino reset button before flashing
- Use PlatformIO CLI as a fallback

## PlatformIO CLI

For developers or when the web flasher doesn't work.

### Install PlatformIO
```bash
pip install platformio
```

### Flash Commands
```bash
cd hardware

# Serial only
pio run -e uno -t upload

# Serial + Bluetooth
pio run -e uno_bt -t upload

# ESP32 with WiFi + Vehicle + Body buses
PLATFORMIO_BUILD_FLAGS="-DBUS_VEHICLE_ACTIVE=1 -DBUS_BODY_ACTIVE=1" pio run -e esp32_wifi -t upload
```

### Build Without Upload
```bash
pio run -e uno
```

### Monitor Serial Output
```bash
pio device monitor -b 115200
```

## Build Flags

Each firmware variant is controlled by build flags in `platformio.ini`:

| Flag | Default | Description |
| ---- | ------- | ----------- |
| BUS_FSD_ACTIVE | 1 | FSD bus (always on) |
| BUS_VEHICLE_ACTIVE | 0 | Vehicle control bus |
| BUS_BODY_ACTIVE | 0 | Body control bus |
| BOARD_ENABLE_BT | 0 | Enable HC-05 Bluetooth |

## Firmware Update Process

1. Connect Arduino via USB
2. Flash new firmware (web or CLI)
3. Board reboots automatically
4. EEPROM settings are preserved across updates
5. Verify by checking boot message in Console

## EEPROM Reset

To reset all saved settings to factory defaults:
```json
{"cmd":"factory_reset"}
```
This clears variant, FSD, nag, profile, offset, and all other saved settings.
