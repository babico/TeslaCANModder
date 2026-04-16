#!/usr/bin/env python3
"""
Firmware Resource Regression Check

Builds firmware for target environments and checks binary size + RAM usage
against defined thresholds. Exits non-zero if any threshold is exceeded.

Usage:
    python scripts/check_firmware_size.py [--env ENV ...]

Without args, checks: uno, esp32
"""

import subprocess
import sys
import re
import argparse

# ── Thresholds ────────────────────────────────────────────────────────────────
# Flash and RAM limits per board (bytes)
# Arduino Uno: 32256 bytes flash, 2048 bytes RAM
# ESP32: 1310720 bytes flash (1.25 MB), 327680 bytes RAM (320 KB)

THRESHOLDS = {
    "uno": {
        "flash_max": 30000,       # ~93% of 32256 — new features add ~2KB
        "ram_max": 1950,          # ~95% of 2048 — BMS/state fields added
        "flash_total": 32256,
        "ram_total": 2048,
    },
    "uno_bt": {
        "flash_max": 31000,       # ~96% — BT + new features
        "ram_max": 1980,          # ~97%
        "flash_total": 32256,
        "ram_total": 2048,
    },
    "esp32": {
        "flash_max": 1100000,     # ~84% of 1310720 — new features add overhead
        "ram_max": 300000,        # ~92% of 327680
        "flash_total": 1310720,
        "ram_total": 327680,
    },
    "esp32_wifi": {
        "flash_max": 1200000,     # WiFi + new features
        "ram_max": 310000,
        "flash_total": 1310720,
        "ram_total": 327680,
    },
    "esp32_ble": {
        "flash_max": 1200000,     # BLE + new features
        "ram_max": 310000,
        "flash_total": 1310720,
        "ram_total": 327680,
    },
    "esp32_wifi_ble": {
        "flash_max": 1280000,     # WiFi + BLE + new features
        "ram_max": 320000,
        "flash_total": 1310720,
        "ram_total": 327680,
    },
}


def build_and_check(env: str) -> bool:
    """Build firmware for env and check size against thresholds. Returns True if OK."""
    if env not in THRESHOLDS:
        print(f"⚠  No thresholds defined for '{env}', skipping")
        return True

    limits = THRESHOLDS[env]
    print(f"\n{'='*60}")
    print(f"Building: {env}")
    print(f"{'='*60}")

    result = subprocess.run(
        ["pio", "run", "-e", env, "-v"],
        capture_output=True,
        text=True,
    )

    output = result.stdout + result.stderr

    if result.returncode != 0:
        print(f"✗ Build failed for {env}")
        print(output[-2000:] if len(output) > 2000 else output)
        return False

    # Parse size from PlatformIO output
    # AVR format:  "Program: NNNNN bytes (XX.X% Full)"  /  "Data: NNNNN bytes (XX.X% Full)"
    # ESP32 format: various, but pio run -v prints avr-size or esptool output

    flash_used = None
    ram_used = None

    # Try AVR-style output (Arduino Uno)
    flash_match = re.search(r'Program:\s+(\d+)\s+bytes', output)
    data_match = re.search(r'Data:\s+(\d+)\s+bytes', output)

    if flash_match:
        flash_used = int(flash_match.group(1))
    if data_match:
        ram_used = int(data_match.group(1))

    # Try ESP32-style output (esptool)
    if flash_used is None:
        esp_flash = re.search(r'Flash:\s+\[=*\s*\]\s+[\d.]+%\s+\(used\s+(\d+)\s+bytes', output)
        if esp_flash:
            flash_used = int(esp_flash.group(1))

    if ram_used is None:
        esp_ram = re.search(r'RAM:\s+\[=*\s*\]\s+[\d.]+%\s+\(used\s+(\d+)\s+bytes', output)
        if esp_ram:
            ram_used = int(esp_ram.group(1))

    ok = True

    if flash_used is not None:
        pct = (flash_used / limits["flash_total"]) * 100
        status = "✓" if flash_used <= limits["flash_max"] else "✗ EXCEEDED"
        print(f"  Flash: {flash_used:>8} / {limits['flash_total']:>8} bytes ({pct:.1f}%) "
              f"[limit: {limits['flash_max']}] {status}")
        if flash_used > limits["flash_max"]:
            ok = False
    else:
        print(f"  Flash: (could not parse size output)")

    if ram_used is not None:
        pct = (ram_used / limits["ram_total"]) * 100
        status = "✓" if ram_used <= limits["ram_max"] else "✗ EXCEEDED"
        print(f"  RAM:   {ram_used:>8} / {limits['ram_total']:>8} bytes ({pct:.1f}%) "
              f"[limit: {limits['ram_max']}] {status}")
        if ram_used > limits["ram_max"]:
            ok = False
    else:
        print(f"  RAM:   (could not parse size output)")

    return ok


def main():
    parser = argparse.ArgumentParser(description="Check firmware resource usage")
    parser.add_argument("--env", nargs="+", default=["uno", "esp32"],
                        help="Environments to check (default: uno esp32)")
    args = parser.parse_args()

    failed = []
    for env in args.env:
        if not build_and_check(env):
            failed.append(env)

    print(f"\n{'='*60}")
    if failed:
        print(f"✗ Resource threshold exceeded for: {', '.join(failed)}")
        sys.exit(1)
    else:
        print(f"✓ All {len(args.env)} environments within resource thresholds")
        sys.exit(0)


if __name__ == "__main__":
    main()
