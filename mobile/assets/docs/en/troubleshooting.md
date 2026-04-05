# Troubleshooting

Common issues and solutions for TeslaCANModder.

## Connection Issues

### "Web Serial not supported"
- Use Chrome or Edge (version 89+). Firefox and Safari do not support Web Serial API.
- On mobile, use the Bluetooth connection instead of USB.

### Can't see the Arduino in port list
- Check USB cable — use a data cable, not charge-only
- Install CH340 drivers if your Arduino uses CH340 chip
- Try a different USB port
- On Windows, check Device Manager for COM port

### Bluetooth won't connect
- Ensure HC-05 module is powered (LED blinking)
- Pair HC-05 first in phone Bluetooth settings (default PIN: 1234)
- HC-05 must be in data mode, not AT command mode
- Check voltage divider on RX line (5V → 3.3V)

### "Disconnected" immediately after connecting
- Baud rate mismatch — firmware uses 115200
- Another app may be using the serial port
- Try power cycling the Arduino

## CAN Bus Issues

### "CAN Waiting" — never goes online
- Check MCP2515 wiring (especially CS and INT pins)
- Verify X179 connector is fully seated
- Car must be "awake" (touch screen, open door, or charging)
- Check CAN-H / CAN-L connections — do not swap them

### "CAN Standby" — bus went silent
- Normal when car goes to sleep
- Board auto-recovers when car wakes up
- If persistent, check 12V power supply

### No frames appearing
- Start streaming first (click "Stream" button)
- Verify variant selection matches your vehicle
- Check MCP2515 crystal — must be 8 MHz

## Feature Issues

### FSD not activating
- Verify correct variant is selected (HW4 vs HW3)
- Check that FSD shows "ON" in EEPROM panel
- Some features require the car to be in Park or Drive
- After enabling, drive briefly — FSD activates in motion

### Profile changes not taking effect
- Profile changes apply on next acceleration event
- If pinned profile isn't working, try "Auto" then re-select
- Check the EEPROM panel to confirm the value was saved

### Speed offset not available
- Speed offset is HW3-only feature
- Switch to HW3 variant if your vehicle supports it

### ISA chime not suppressing
- ISA chime suppression is HW4-only
- The chime may take one drive cycle to fully suppress

## Firmware Issues

### Flash failed
- Ensure correct firmware variant is selected
- Close any other serial monitor connected to the port
- Try the PlatformIO CLI method: `cd hardware && pio run -e uno_usb -t upload`
- If using CH340, install latest drivers

### Board not responding after flash
- Wait 5 seconds after flash for reboot
- Press the reset button on Arduino
- If stuck, reflash using PlatformIO CLI

## Hardware Issues

### MCP2515 #2 not detected
- Check CS pin (D9) and INT pin (D3) wiring
- Ensure using a dual-CAN firmware variant
- Check boot message — it should report `bus2: true`

### Buck converter running hot
- Normal for 12V→5V at 150mA — use a module rated for 3A
- Ensure proper ventilation in the enclosure
- Check for short circuits in wiring

### Board resets randomly
- Power supply unstable — check buck converter
- CAN bus noise — add 120Ω termination resistor if needed
- Memory full — reduce streaming rate or clear frame buffer
