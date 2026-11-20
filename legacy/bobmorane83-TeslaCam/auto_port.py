# auto_port.py — PlatformIO extra script: auto-detect ESP32 port by MAC address
# Usage in platformio.ini:
#   extra_scripts = pre:../../auto_port.py   (adjust relative path)
#   custom_mac = E0:72:A1:D8:FE:2C
Import("env")
import serial.tools.list_ports

target_mac = env.GetProjectOption("custom_mac", "")
if target_mac:
    target_mac = target_mac.strip().upper()
    for port in serial.tools.list_ports.comports():
        hwid = (port.hwid or "").upper()
        if f"SER={target_mac}" in hwid:
            env.Replace(UPLOAD_PORT=port.device)
            env.Replace(MONITOR_PORT=port.device)
            print(f"[AUTO_PORT] {target_mac} -> {port.device}")
            break
    else:
        import sys
        print(f"[AUTO_PORT] ERROR: MAC {target_mac} not found — aborting to prevent cross-flash!")
        print(f"[AUTO_PORT] Available ports:")
        for p in serial.tools.list_ports.comports():
            print(f"  {p.device}  hwid={p.hwid}")
        sys.exit(1)
