---
title: Flasher Quickstart
description: Complete guide to flashing the Tesla CAN Mod firmware and connecting via USB or Bluetooth
category: guides
folder: guides
tags: [setup, flasher, firmware]
order: 3
icon: 🔧
---

## Overview

This guide covers:

- **Flashing firmware** to your ESP32 hardware
- **USB serial connection** setup (Windows, macOS, Linux)
- **Bluetooth COM port** configuration for wireless connection
- **Hardware requirements** and troubleshooting

## Hardware Requirements

| Component                      | Description                     | Notes                                          |
| ------------------------------ | ------------------------------- | ---------------------------------------------- |
| **ESP32 Board**                | Any ESP32 (WROOM, WROVER, etc.) | Must support USB-UART (CH340 or CP2102 common) |
| **USB Cable**                  | USB-A to Micro-USB or USB-C     | Check your ESP32 variant                       |
| **Vehicle CAN Interface**      | MCP2515 or equivalent           | Jumpered to ESP32 via SPI                      |
| **Optional: Bluetooth Module** | For wireless connection         | ESP32 built-in; optional external module       |

## Part 1: Flashing Firmware

### Step 1: Prepare Your Environment

**On Windows:**

```powershell
# Install Python 3.11+ if not already installed
python --version

# Install esptool
pip install esptool

# Verify installation
esptool.py version
```

**On macOS/Linux:**

```bash
brew install python3
pip3 install esptool
esptool.py version
```

### Step 2: Download Firmware

Navigate to the [releases page](../../releases/) and download the latest `firmware.bin` for your board:

- `firmware-hw3.bin` for HW3 (Model 3 / Standard)
- `firmware-hw4.bin` for HW4 (Model Y / High Perf)

### Step 3: Connect ESP32 to Computer

1. **Plug in USB cable** to your ESP32
2. **Verify COM port is detected:**
    - **Windows**: Device Manager → COM Ports (note the port, e.g., `COM7`)
    - **Mac**: `ls /dev/cu.usbserial*` or `ls /dev/cu.SLAB_USBtoUART*`
    - **Linux**: `ls /dev/ttyUSB*` or `dmesg | grep tty`

### Step 4: Flash the Firmware

```powershell
# Windows
esptool.py --chip esp32 --port COM7 --baud 460800 write_flash 0x1000 firmware-hw3.bin

# macOS/Linux
esptool.py --chip esp32 --port /dev/cu.SLAB_USBtoUART --baud 460800 write_flash 0x1000 firmware-hw3.bin
```

**Expected output:**

```
Connecting....
Chip is ESP32-WROOM-32 (revision 1)
Features: WiFi, BT, Dual Core, 240MHz, VRef calibration in efuse, Coding Scheme None
MAC: 30:ae:a4:22:33:ff
Uploading stub...
Running stub...
Stub running...
Changing baud rate to 460800
Changed.
Writing at 0x00001000... (10%)
Writing at 0x00005000... (20%)
...
Wrote 1048576 bytes at address 0x1000 in 23.5 seconds
Hash of data verified.

Hard resetting via DTR pin...
```

### Step 5: Verify Flash Success

Disconnect and reconnect the USB cable. You should see the blue LED blink or stay on, indicating the device is running.

## Part 2: USB Serial Connection (Windows)

### Setup COM Port Access

**Windows 10/11 Automatic:**
When you connect the ESP32, Windows will auto-install the USB-UART drivers. Verify in Device Manager.

**Windows 7 or Custom Drivers:**

1. Download the appropriate driver:
    - **CH340 drivers**: [WCH CH340 Windows](http://www.wch-ic.com/en/products/WCH341.html)
    - **CP2102 drivers**: [Silicon Labs CP210x](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
2. Right-click the unknown device in Device Manager
3. **Install from computer** → Select the driver folder
4. Restart Windows

### Connect from Application

In the **Tesla CAN Modder** client:

1. Go to the **Monitor** tab
2. Select **"COM Serial"** transport
3. Select your **COM port** (e.g., `COM7`)
4. Click **"Apply Transport"**
5. Click **"Fetch Status"** to verify connection

## Part 3: Bluetooth COM Port Connection (Windows & macOS)

### macOS Setup

**Enable Bluetooth on macOS:**

1. Open **System Preferences** → **Bluetooth**
2. Click **"Turn Bluetooth On"**
3. Put your ESP32 in pairing mode (depends on external module or firmware config)
4. Click **"Pair"** when the device appears
5. In Terminal, verify the serial port:

    ```bash
    ls /dev/cu.ESP32*  # or similar
    ```

**Connect from Application:**

1. Go to **Monitor** tab
2. Select **"Bluetooth COM"** transport
3. Select your **Bluetooth device** from the list
4. Click **"Apply Transport"**
5. Click **"Fetch Status"**

### Windows Setup (Native Bluetooth SPP)

**Enable Bluetooth Serial Port:**

1. **Pair the device first:**
    - Settings → Bluetooth & devices → Add device
    - Put ESP32 in pairing mode
    - Select your device and pair

2. **Create Bluetooth COM Port:**
    - Devices & Printers (Control Panel)
    - Right-click your paired device → Properties
    - Go to **Services** tab
    - Check **"Serial Port"** service
    - Note the **COM port** (e.g., `COM9`)

3. **Connect from Application:**
    - Go to **Monitor** tab
    - Select **"Bluetooth COM"** transport
    - Select the COM port from the dropdown
    - Click **"Apply Transport"**
    - Click **"Fetch Status"** to verify

**Troubleshooting Windows Bluetooth:**

- If COM port doesn't appear, restart Bluetooth service: `net stop bthserv && net start bthserv`
- Check Device Manager under **Ports (COM & LPT)** for the Bluetooth device

## Part 4: REST API over WiFi Connection

The easiest connection method for development.

### Hardware Setup

Your ESP32 firmware must support WiFi. Verify by checking your device's AP list:

1. On your phone, open WiFi settings
2. Look for **"TeslaCAN_xxxxx"** or **"ESP32_xxxxx"**
3. Connect to it (default password: `tesla1234`)

### Application Setup

In the **Tesla CAN Modder** client:

1. Go to **Monitor** tab
2. Select **"REST API"** transport
3. Set **Base URL**: `http://192.168.4.1` (adjust IP as needed)
4. Set **Command Path**: `/api/command`
5. Set **Status Path**: `/api/status`
6. Click **"Apply Transport"**
7. Click **"Fetch Status"** to test

#### Alternative: Home Network Setup

If your ESP32 is connected to your home WiFi:

1. Find ESP32 IP address:
    - Check router DHCP client list, OR
    - Run: `ping esp32.local` (if mDNS is supported)

2. Update Base URL: `http://<esp32-ip>:8080` (adjust port as needed)
3. Apply transport and test

## Part 5: Bluetooth Module Setup (Optional External)

If using an external Bluetooth module (HC-05, HC-06):

### Wiring (HC-05 Example)

```
HC-05           ESP32
VCC      →      5V (or 3.3V with voltage divider)
GND      →      GND
TX       →      GPIO16 (RX)
RX       →      GPIO17 (TX)
```

### Configure Module

**Using AT Commands:**

1. Connect via USB UART during setup (before flashing)
2. Use serial terminal (PuTTY, Arduino IDE, or esptool) at 38400 baud
3. Send AT commands:

    ```
    AT                              # Should respond OK
    AT+NAME=ESP32_CAN              # Set device name
    AT+PIN=1234                     # Set PIN code
    AT+ROLE=0                       # Set as slave
    AT+UART=9600,0,0               # Match ESP32 firmware baud
    AT+RESET                        # Restart module
    ```

## Troubleshooting

| Issue                              | Solution                                                       |
| ---------------------------------- | -------------------------------------------------------------- |
| **Device not detected on COM**     | Reinstall USB drivers; try different USB cable                 |
| **"Port in use" error**            | Close other serial monitor apps (Arduino IDE, PuTTY)           |
| **Bluetooth pairing fails**        | Ensure device is in pairing mode; check Bluetooth drivers      |
| **"Fetch Status" returns timeout** | Verify IP/COM port correct; check device power; restart device |
| **Garbled data in Monitor**        | Check baud rate matches (usually 9600 or 115200)               |
| **WiFi AP not visible**            | Device may be in STA mode; check firmware config               |

## Next Steps

✅ **Once you have connections working:**

1. Navigate to the **Drive** tab to see live vehicle state
2. Try **Controls** → **Send Command**
3. Open **Monitor** to inspect CAN frames and decoder data
4. Check [getting-started.md](./getting-started.md) for feature walkthrough

## Additional Resources

- [ESP32 Hardware Reference](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [MCP2515 CAN Bus Controller](https://www.nxp.com/products/wireless-connectivity/long-range-wireless/proprietary-long-range/mcp2515-standalone-can-controller-with-spi-interface)
- [Tesla CAN Protocol](../reference/can-protocol.md)
- [Hardware Setup Guide](./hardware-setup.md)
