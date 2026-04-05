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
   - **USB Only** — lightest, no BT or dual CAN
   - **USB + Bluetooth** — adds HC-05 support
   - **USB + Dual CAN** — adds second MCP2515
   - **Full** — everything enabled
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

# USB only
pio run -e uno_usb -t upload

# USB + Bluetooth
pio run -e uno_usb_bt -t upload

# USB + Dual CAN
pio run -e uno_usb_mcp2 -t upload

# Full (USB + BT + Dual CAN)
pio run -e uno_full -t upload
```

### Build Without Upload
```bash
pio run -e uno_usb
```

### Monitor Serial Output
```bash
pio device monitor -b 115200
```

## Build Flags

Each firmware variant is controlled by build flags in `platformio.ini`:

| Flag | Default | Description |
|------|---------|-------------|
| BOARD_ENABLE_BT | 0 | Enable HC-05 Bluetooth |
| BOARD_ENABLE_MCP2515_2 | 0 | Enable second MCP2515 |

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
