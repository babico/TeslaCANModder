# Safe Mode CLI Tool

Python CLI tool for managing safe mode and debugging TeslaCANModder without rebuilding firmware.

## What is Safe Mode?

**Safe Mode blocks ALL CAN frame modifications** while still allowing:
- CAN frame monitoring (read-only)
- Command reception
- Status reporting

This lets you:
- Connect Arduino to car safely
- Debug which feature causes issues
- Test features one at a time
- Monitor CAN traffic without interference

## Installation

```bash
pip install pyserial
```

## Usage

### Show Current Status

```bash
python safe_mode_cli.py status
```

Output:
```
==================================================
TeslaCANModder Status
==================================================
Safe Mode:    🛡️  ENABLED (blocking modifications)
Variant:      HW4
FSD:          OFF
Nag Suppress: OFF
ISA Chime:    OFF
Profile:      1
==================================================
```

### Enable Safe Mode

```bash
python safe_mode_cli.py safe-on
```

**Use this when:**
- First connecting to car
- Car shows emergency brake activation
- Debugging issues

### Disable Safe Mode

```bash
python safe_mode_cli.py safe-off
```

**Only use after verifying car is stable in safe mode.**

### Test Individual Features

Test a feature for 10 seconds then auto-disable:

```bash
# Test FSD
python safe_mode_cli.py test fsd

# Test nag suppression
python safe_mode_cli.py test nag

# Test ISA chime suppression (HW4 only)
python safe_mode_cli.py test isa-chime
```

**Workflow:**
1. Enable safe mode: `python safe_mode_cli.py safe-on`
2. Connect Arduino to car
3. Verify car is stable
4. Test feature: `python safe_mode_cli.py test isa-chime`
5. Watch for emergency brake or issues
6. Feature auto-disables after 10 seconds

### Monitor CAN Frames

```bash
# Monitor for 30 seconds (default)
python safe_mode_cli.py monitor

# Monitor for 60 seconds
python safe_mode_cli.py monitor --duration 60
```

Output:
```
📡 Monitoring CAN frames for 30 seconds...
Press Ctrl+C to stop

[    1234ms] Bus0 ID:0399 DLC:8 Data:0102030405060708
[    1254ms] Bus0 ID:03FD DLC:8 Data:0A0B0C0D0E0F1011
...
```

### Specify Serial Port

```bash
# Windows
python safe_mode_cli.py --port COM3 status

# Linux/Mac
python safe_mode_cli.py --port /dev/ttyUSB0 status
```

## Debugging Emergency Brake Issue

**Step-by-step process:**

1. **Start in safe mode:**
   ```bash
   python safe_mode_cli.py safe-on
   ```

2. **Connect Arduino to car** (powered from car 12V)

3. **Verify car is stable:**
   ```bash
   python safe_mode_cli.py status
   ```
   - Should show `Safe Mode: ENABLED`
   - Car should be normal (no emergency brake)

4. **Test ISA chime (most likely culprit for HW4):**
   ```bash
   python safe_mode_cli.py test isa-chime
   ```
   - Enables ISA suppression for 10 seconds
   - Watch car for emergency brake
   - Auto-disables after 10 seconds

5. **If emergency brake activates:**
   - ISA chime suppression is the problem
   - Checksum or bit modification is wrong
   - Keep safe mode ON

6. **If no emergency brake:**
   - Test next feature:
   ```bash
   python safe_mode_cli.py test nag
   python safe_mode_cli.py test fsd
   ```

7. **Once you identify the problematic feature:**
   - Keep safe mode ON
   - Don't use that feature until fixed
   - Other features can be used safely

## Safe Mode Commands (Serial)

You can also send commands directly via serial monitor (115200 baud):

```
safe:on          # Enable safe mode
safe:off         # Disable safe mode
safe:status      # Get status (same as 'status')
status           # Show full status including safeMode field
```

## Boot Behavior

**Arduino always boots with safe mode ENABLED by default.**

This prevents:
- Emergency brake on connection
- Accidental feature activation
- EEPROM-saved settings from auto-loading

You must explicitly disable safe mode to enable features.

## Example Workflow

```bash
# 1. Enable safe mode and connect
python safe_mode_cli.py safe-on
# Connect Arduino to car

# 2. Verify stable
python safe_mode_cli.py status

# 3. Test ISA chime (HW4)
python safe_mode_cli.py test isa-chime
# Watch for emergency brake for 10 seconds

# 4. If OK, test nag
python safe_mode_cli.py test nag

# 5. If OK, test FSD
python safe_mode_cli.py test fsd

# 6. If all OK, disable safe mode
python safe_mode_cli.py safe-off

# 7. Enable features via web UI
# Now you can use web UI to enable features normally
```

## Troubleshooting

**"Arduino not found"**
- Specify port manually: `--port COM3` (Windows) or `--port /dev/ttyUSB0` (Linux)

**"Failed to get status"**
- Check baud rate (should be 115200)
- Verify Arduino is powered and running
- Try reconnecting USB cable

**"Safe mode ENABLED but features still active"**
- Safe mode only blocks NEW modifications
- Reboot Arduino to clear state
- Verify status shows `safeMode:1`

## License

GPL-3.0
