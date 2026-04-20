---
title: USB and Bluetooth Adapters
description: Detailed guide to establishing USB serial and Bluetooth wireless connections with Tesla CAN Mod
category: guides
folder: guides
tags: [bluetooth, usb, connection]
order: 4
icon: 🔌
---

## Quick Reference

| Connection Type                | Setup Time | Reliability | Use Case                                  |
| ------------------------------ | ---------- | ----------- | ----------------------------------------- |
| **USB Serial (COM)**           | <5 min     | High        | Local development, direct MCU access      |
| **Bluetooth COM**              | 5-10 min   | Medium      | Wireless testing, vehicle-mounted setup   |
| **REST API (WiFi)**            | 2-3 min    | Medium      | Long-range diagnostics, multi-device      |
| **BLE (Bluetooth Low Energy)** | 5-10 min   | Medium      | Client native targets, lowest power drain |

---

## USB Serial Connection (Recommended for First Setup)

### What You Need

- ESP32 board with **built-in USB-UART converter** (CH340, CP2102, or similar)
- **Micro-USB or USB-C cable** (check your ESP32 variant)
- **USB port on your computer** (Windows 10+, macOS 10.13+, any Linux)

### Windows Setup

#### 1. Install USB Drivers (One-Time)

**Most boards come with auto-installing drivers. To manually install:**

**For CH340 chips (common on cheaper boards):**

```
1. Download: http://www.wch-ic.com/en/products/WCH341.html
2. Extract and run the Windows installer
3. Restart your computer
```

**For CP2102/CP2103 chips:**

```
1. Download: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
2. Extract and run the installer
3. Restart your computer
```

#### 2. Find Your COM Port

```powershell
# Open PowerShell as Administrator
Get-PnpDevice -Class Ports | Where-Object { $_.Name -like "*COM*" }

# OR check Device Manager:
# Control Panel → Device Manager → Ports (COM & LPT) → Note the COM number
```

**Example output:**

```
Name                           Status
COMX USB Serial Port          OK
```

#### 3. Connect from Tesla CAN Modder

1. Plug USB cable into ESP32
2. In the app → **Monitor** tab
3. Select **"COM Serial"** transport
4. Select your COM port (e.g., `COM7`)
5. Click **"Apply Transport"**
6. Click **"Fetch Status"** — should see board info

### macOS Setup

#### 1. Check for Drivers

```bash
# Most modern Macs have CH340/CP2102 drivers built-in
# If using an older Mac or custom board:

# Option A: Download CH340 driver
# http://www.wch-ic.com/en/products/WCH341.html

# Option B: Download CP2102 driver
# https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
```

#### 2. Verify Connection

```bash
# List serial ports
ls -la /dev/cu.* /dev/tty.* |grep -i usb

# Example output:
# /dev/cu.SLAB_USBtoUART    (CP2102)
# /dev/cu.wchusbserial1234  (CH340)
```

#### 3. Connect from App

1. Plug in USB cable
2. **Monitor** tab → **"COM Serial"** transport
3. Select your port from dropdown
4. **"Apply Transport"** → **"Fetch Status"**

### Linux Setup

Most distributions have built-in drivers. No setup needed usually.

```bash
# Verify port
ls -l /dev/ttyUSB* /dev/ttyACM*

# Example:
# /dev/ttyUSB0  →  your ESP32

# If permission denied, add your user to dialout group:
sudo usermod -a -G dialout $USER
# Then log out and log back in
```

---

## Bluetooth COM Port Connection (Wireless)

For testing without a USB cable, or for vehicle-mounted hardware.

### Windows Bluetooth Setup

#### 1. Pair the Device

```
1. Settings → Bluetooth & Devices
2. Enable Bluetooth toggle
3. Put ESP32 in pairing mode (depends on firmware)
4. Click "Add Device"
5. Select "Bluetooth"
6. Choose your ESP32 from the list (e.g., "ESP32-12AB")
7. Confirm PIN if prompted (default: usually 1234 or 0000)
```

#### 2. Create Bluetooth COM Port

```
1. Control Panel → Devices and Printers
2. Right-click your paired ESP32 device
3. Properties → Services tab
4. Check "Inbound Serial Port" and/or "Outbound Serial Port"
5. Click Apply
6. Device Manager → Ports (COM & LPT)
7. Note the COM port number (e.g., COM9 for "Bluetooth-Inbound")
```

#### 3. Connect from App

1. **Monitor** tab → **"Bluetooth COM"** transport
2. Select the **Bluetooth COM port** from dropdown
3. **"Apply Transport"** → **"Fetch Status"**

#### Troubleshooting Windows Bluetooth

```powershell
# Reset Bluetooth service if COM port missing:
net stop bthserv
net start bthserv

# Reload serial port devices:
# Device Manager → Action → Scan for hardware changes

# Check current Bluetooth devices:
Get-PnpDevice -Class Bluetooth
```

### macOS Bluetooth Setup

#### 1. Enable Bluetooth

```
System Preferences → Bluetooth → Turn On
```

#### 2. Pair Device

```
1. In Bluetooth preferences, click "Pair New Device"
2. Put ESP32 in pairing mode
3. Select device and confirm PIN
```

#### 3. Verify Serial Port

```bash
# Check for Bluetooth serial devices
ls -la /dev/cu.* | grep -i bluet
ls -la /dev/tty.* | grep -i bluet

# Example output:
# /dev/cu.ESP32-Bluetooth-Inbound
# /dev/cu.ESP32-Bluetooth-Outbound
```

#### 4. Connect from App

1. **Monitor** tab → **"Bluetooth COM"** transport
2. Select your device from dropdown
3. **"Apply Transport"** → **"Fetch Status"**

### Linux Bluetooth Setup

```bash
# Check Bluetooth status
systemctl status bluetooth

# If not running:
sudo systemctl start bluetooth
sudo systemctl enable bluetooth

# Pair device (interactive)
bluetoothctl
> scan on
> pair <MAC_ADDRESS>
# e.g., pair 30:AE:A4:22:33:FF

# Create Bluetooth-to-serial bridge (requires setup)
# See: https://wiki.archlinux.org/title/Bluetooth#Serial_port

# Typically results in: /dev/rfcomm0
```

---

## Connecting via REST API (WiFi)

Easiest method for development, best range.

### Prerequisites

Your firmware must include WiFi support. Check with:

```
Phone WiFi settings → Look for "ESP32_xxxxx" or "TeslaCAN_xxxxx" network
```

### Setup Steps

#### 1. Connect to ESP32 WiFi

```
Phone/Computer WiFi settings
SSID: ESP32_12AB (or similar)
Password: (check your firmware config, default often: tesla1234)
```

#### 2. Find Device IP

##### Option A: Fixed IP (if configured)

```
Use the configured IP (e.g., 192.168.4.1)
```

##### Option B: Dynamic IP

```powershell
# Windows: Find in your router's DHCP client list
# OR ping:
ping esp32.local

# macOS/Linux:
ping esp32.local
# or
nslookup esp32.local
```

#### 3. Configure in App

1. **Monitor** tab → **"REST API"** transport
2. **Base URL**: `http://192.168.4.1` (or your device IP)
3. **Command Path**: `/api/command`
4. **Status Path**: `/api/status`
5. **"Apply Transport"** → **"Fetch Status"**

---

## Connection Troubleshooting Matrix

| Symptom                          | Cause                           | Solution                                                  |
| -------------------------------- | ------------------------------- | --------------------------------------------------------- |
| **Windows: USB not detected**    | Missing driver                  | Download CH340/CP2102 driver; restart                     |
| **macOS: Serial port not found** | Driver missing                  | Download appropriate driver from Silicon Labs or WCH      |
| **Linux: permission denied**     | User not in dialout group       | `sudo usermod -a -G dialout $USER` ; log out              |
| **COM port says "in use"**       | Another app has serial open     | Close Arduino IDE, PuTTY, Debug terminal                  |
| **Bluetooth won't pair**         | Device not in pairing mode      | Restart device; check firmware supports BT                |
| **"Fetch Status" timeout**       | Device offline or wrong IP      | Verify IP, check power, restart device                    |
| **Garbled/corrupt data**         | Baud rate mismatch              | Check firmware baud rate (usually 9600 or 115200)         |
| **WiFi AP not visible**          | Device in STA mode, not AP mode | Check firmware config; may need reflash or config command |

---

## Advanced: Change Connection at Runtime

Once you have one transport working:

1. In **Monitor** → **Connection** section
2. Click a different transport option
3. For transports requiring config (HTTP, COM):
    - Adjust URL/COM port in the UI
    - Click "Apply Transport"
4. **"Fetch Status"** to verify new connection

---

## Next Steps

✅ **You now have a working connection!**

1. Go to **Drive** tab → See live vehicle state
2. Go to **Controls** tab → Try sending a test command
3. Open **Monitor** → Inspect CAN frames, decoder datasets
4. Read [feature-workflows.md](./feature-workflows.md) for usage patterns
