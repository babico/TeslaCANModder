# Troubleshooting

## Board Not Connecting

- Use a **data-capable USB cable** (not charge-only)
- Install **CH340** or **CP2102** drivers for your board
- Close Arduino IDE or any other serial monitor (only one app can use the port)
- Use **Chrome** or **Edge** (WebSerial is not supported in Firefox/Safari)
- ESP32: Hold the **BOOT** button during upload if flashing fails

## No CAN Frames

### Arduino
- Check MCP2515 wiring: CS → D10, INT → D2, SPI pins (D11/D12/D13)
- Verify **8 MHz crystal** on MCP2515 module
- Check CAN-H/CAN-L connections to X179 pins 13/14

### ESP32
- Check MCP2515 #1 wiring: CS → GPIO 15, INT → GPIO 34
- Check SPI lines: SCK → GPIO 18, MISO → GPIO 19, MOSI → GPIO 23
- Verify MCP2515 is powered from **5V** (VIN pin), not 3.3V
- Check CAN-H/CAN-L connections per bus (see [Hardware Setup](hardware-setup))

### General
- Vehicle must be **powered on** (screen active)
- Console shows "CAN bus silent" → wiring issue
- Use `can:raw:on` to see all bus traffic (not just variant-specific IDs)

## MCP2515 Bus Not Detected

- Must flash a matching firmware variant (1-CAN or 3-CAN)
- Boot log shows "MCP2515 #N not detected" if wiring is wrong
- Check SPI lines are shared correctly between modules
- Bus 0: CS → GPIO 15, INT → GPIO 34
- Bus 1: CS → GPIO 27, INT → GPIO 35
- Bus 2: CS → GPIO 26, INT → GPIO 33

## WiFi Not Connecting

- Must flash a **WiFi-enabled** firmware variant
- Default AP SSID: `TeslaCANModder`, Password: `teslacan123`
- Test: `http://192.168.4.1/api/ping` after connecting
- Check Console for "WiFi AP started" message
- If STA connection fails, device falls back to AP mode automatically

## BLE Not Working

- Must flash a **BLE-enabled** firmware variant
- Device advertises as `TeslaCANModder`
- Use a Nordic UART compatible app (nRF Connect, LightBlue)
- iOS: Make sure Bluetooth is enabled in Settings
- If device doesn't appear, try power-cycling the ESP32
- BLE and WiFi can operate simultaneously

## Summon Not Working

- Must be **HW3** or **HW4** variant
- Console error "Waiting for 0x273" = no cached control frame yet
- Enable streaming, verify CAN ID 627 appears in frame table
- If 627 is missing even in raw mode: X179 pins 9/10 may not be Vehicle Control bus
- Summon requires **3 CAN buses** to be connected

## FSD Not Activating

- FSD must be **selected in the vehicle's UI** — the mod only activates when `isFSDSelectedInUI` is true
- Check variant matches your vehicle hardware
- Console should show "HW4: FSD mod active on CAN" (or HW3/Legacy equivalent)
- If no log appears, verify CAN ID 1021 (HW3/HW4) or 1006 (Legacy) is being received

## Board Stops After Car Sleep

- Board enters **standby** when CAN goes silent
- LED blinks slowly in standby mode
- Auto-recovers when car wakes and CAN resumes
- Check Console for "Standby" / "Resuming" logs
- If using a powerbank: ensure it doesn't cut off at low current draw

## Vehicle Commands Not Working

- Vehicle commands require **3 CAN buses** connected
- Error "Waiting for 0x273 frame" means the control frame hasn't been cached yet
- Drive the vehicle briefly to generate CAN traffic on Bus 1
- Verify Bus 1 (GPIO 27/35) and Bus 2 (GPIO 26/33) are wired correctly
