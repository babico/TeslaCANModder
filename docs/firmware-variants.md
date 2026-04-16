# Firmware Variants

TeslaCANModder supports multiple build configurations for different hardware setups. Select the variant that matches your board and connectivity needs. CAN bus lanes are controlled independently via build flags.

## Arduino Uno Variants

| Environment | Bluetooth | Use Case |
| ---------- | --------- | -------- |
| `uno` | No | Serial only |
| `uno_bt` | Yes (HC-05) | Serial + Bluetooth |

## ESP32 Variants

All ESP32 variants use MCP2515 modules over SPI. No TWAI or SN65HVD230 needed.

| Environment | WiFi | BLE | Use Case |
| ---------- | ---- | --- | -------- |
| `esp32` | No | No | Serial only |
| `esp32_wifi` | Yes | No | WiFi REST API |
| `esp32_ble` | No | Yes | BLE control |
| `esp32_wifi_ble` | Yes | Yes | WiFi + BLE |

## CAN Bus Flags

Each CAN bus lane is enabled independently. The FSD bus is always on.

| Flag | Default | X179 Pins | Bus Function |
| ---- | ------- | --------- | ----------- |
| `BUS_FSD_ACTIVE` | 1 (always) | 13-14 | FSD / Autopilot |
| `BUS_VEHICLE_ACTIVE` | 0 | 9-10 | Vehicle Control (mirror, lock, climate, charge, drive) |
| `BUS_BODY_ACTIVE` | 0 | 2-3 | Body Control (window, sentry, trunk) |

### Other Build Flags

| Flag | Values | Description |
| ---- | ------ | ----------- |
| `BOARD_ENABLE_WIFI` | 0 or 1 | Enable WiFi AP/STA + REST API |
| `BOARD_ENABLE_BLE` | 0 or 1 | Enable BLE (NimBLE Nordic UART) |
| `BOARD_CAN_CLOCK_MHZ` | 8 | MCP2515 crystal frequency |

## Building with PlatformIO

```bash
# Build a specific variant
pio run -e esp32_wifi_ble

# Build with Vehicle + Body buses enabled
PLATFORMIO_BUILD_FLAGS="-DBUS_FSD_ACTIVE=1 -DBUS_VEHICLE_ACTIVE=1 -DBUS_BODY_ACTIVE=1" pio run -e esp32_wifi

# Build and upload
pio run -e esp32_wifi -t upload

# Run native tests
pio test -e native
```

## Flashing via Web UI

1. Go to the **Flasher** tab in the web UI
2. Select your board type and connectivity
3. Toggle which CAN buses to enable (FSD + Vehicle + Body)
4. Click **Build & Download** or **Flash via USB**
5. Monitor the console for progress and boot messages

> Vehicle controls (mirror, lock, climate, charge, drive) require `BUS_VEHICLE_ACTIVE=1`. Body controls (window, sentry) require `BUS_BODY_ACTIVE=1`. FSD features are always available.
