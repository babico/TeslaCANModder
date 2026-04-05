# End-to-End User Scenarios & Test Plan

## Scenario 1: First-Time Setup & Connection

**User Story:** New user unboxes Arduino Uno + MCP2515, wires to Tesla X179, connects via USB.

| Step | Action | Expected Result | Status |
|------|--------|----------------|--------|
| 1.1 | Wire MCP2515 to Arduino (SPI + CS=D10, INT=D2) | Physical wiring complete | — |
| 1.2 | Connect Arduino to laptop via USB | Board powers on, LED blinks | — |
| 1.3 | Open web dashboard in Chrome | Dashboard loads, shows "Not Connected" | — |
| 1.4 | Click "Connect USB" | Browser serial prompt appears | — |
| 1.5 | Select Arduino COM port | Boot message received, HW status populates | — |
| 1.6 | Verify boot JSON | `{"t":"boot","hw":"ArduinoUnoR3CH340",...}` with defaults | — |
| 1.7 | Status messages arrive every 500ms | Uptime counter ticks, rate shows msg/s | — |

**Pass Criteria:** Dashboard shows connected state with correct variant, EEPROM defaults applied.

---

## Scenario 2: FSD + Nag Configuration (Daily Driver)

**User Story:** User enables FSD and nag suppression, sets preferred speed profile.

| Step | Action | Expected Result |
|------|--------|----------------|
| 2.1 | Click "FSD Enable" | ACK received, FSD badge shows ON |
| 2.2 | Click "Nag Suppression Enable" | ACK received, nag badge shows ON |
| 2.3 | Click "Profile 2: Hurry" | Profile changes to 2 (PINNED) |
| 2.4 | Click "Profile Auto" | Profile follows stalk position (AUTO) |
| 2.5 | (HW4) Click "ISA Chime Suppress" | ISA chime shows SUPPRESSED |
| 2.6 | (HW3) Set speed offset to 60% | Offset shows 60% (PINNED) |
| 2.7 | Verify EEPROM in status | All settings reflected in "Saved Settings" panel |

**Pass Criteria:** All settings persist in EEPROM and survive board reset.

---

## Scenario 3: Powerbank Deployment (Car Parked → Walk Away → Return)

**User Story:** User configures board via laptop, unplugs USB, powers board with powerbank in car, walks away, returns next day.

| Step | Action | Expected Result |
|------|--------|----------------|
| 3.1 | Configure FSD=ON, Nag=ON, Profile=3 via dashboard | Settings saved to EEPROM |
| 3.2 | Disconnect USB from laptop | Board still powered by USB (laptop) |
| 3.3 | Plug powerbank into Arduino USB | Board restarts, loads EEPROM settings |
| 3.4 | Place board + powerbank in car, close door | Board running standalone |
| 3.5 | Drive car — CAN frames flow | Board applies FSD/Nag/Profile mods in real-time |
| 3.6 | Park car, walk away | Vehicle CAN bus goes silent after ~15 min |
| 3.7 | Board detects no CAN traffic (10s timeout) | **NEW:** Board enters standby, LED slow-blink |
| 3.8 | Board in standby | MCP2515 in listen-only, reduced polling |
| 3.9 | Return to car, open door | Vehicle CAN bus wakes, frames resume |
| 3.10 | Board detects CAN traffic | **NEW:** Auto-recovery, re-applies filters, resumes normal operation |
| 3.11 | All mods active again (FSD/Nag/Profile) | No user interaction needed — fully autonomous |

**Critical Bug (Current):** Step 3.10 fails — board doesn't recover properly because:
- Frame caches (`hasCtrl`, `hasClimate`, `hasCharge`, `hasDrive`) are stale/zeroed
- MCP2515 may enter error state after prolonged silence
- No CAN timeout detection exists
- No auto-reinit of CAN controller

**Fix Required:** See "Powerbank Resilience" implementation below.

---

## Scenario 4: Vehicle Controls (Quick Actions)

**User Story:** User sends vehicle control commands via dashboard.

| Step | Action | Expected Result |
|------|--------|----------------|
| 4.1 | Click "Unlock" | 30x 0x273 burst sent, car unlocks |
| 4.2 | Click "Frunk Open" | 50x 0x273 burst sent, frunk opens |
| 4.3 | Click "Mirror Fold" | 50x 0x273 burst sent via ctrlBus |
| 4.4 | Click "Seat FL: 3" | Seat heating command sent |
| 4.5 | Click "Vent Open" | 0x119 frame sent on bus 0 |
| 4.6 | Click "Sentry On" | 0x284 frame sent on bus 0 |
| 4.7 | Click "Climate Keep" | 0x2F3 modified frame sent on bus 1 |
| 4.8 | Click "Pedal: Sport" | 0x334 modified frame sent on bus 1 |

**Pre-condition:** Board must have cached the relevant CAN frames (0x273, 0x2F3, 0x333, 0x334). If not cached, error message: "Waiting for 0x273 frame" or similar.

---

## Scenario 5: Variant Switching (HW4 → HW3 → Legacy)

**User Story:** User switches between Tesla hardware variants.

| Step | Action | Expected Result |
|------|--------|----------------|
| 5.1 | Click "HW3" in connection bar | Variant switches, filters re-applied |
| 5.2 | Dashboard shows HW3 features | Speed offset slider visible, ISA chime hidden |
| 5.3 | Click "Legacy" | Variant switches, vehicle controls hidden |
| 5.4 | Dashboard shows Legacy features | Only FSD/Nag/Profile (0-2) available |
| 5.5 | Click "HW4" | Full features restored |

---

## Scenario 6: CAN Stream & Debugging

**User Story:** User enables raw CAN monitoring for diagnostics.

| Step | Action | Expected Result |
|------|--------|----------------|
| 6.1 | Click "Start Stream" | Stream enabled, frames populate FrameTable |
| 6.2 | Frames show ID, DLC, hex data, bus number | Rolling 100-frame buffer, newest at top |
| 6.3 | Type `can:raw:on` in console | All CAN IDs pass through (no filter) |
| 6.4 | Observe high-traffic IDs | Verify both bus 0 and bus 1 frames visible |
| 6.5 | Type `can:raw:off` in console | Filters re-applied, only relevant IDs pass |
| 6.6 | Click "Stop Stream" | FrameTable stops updating |

---

## Scenario 7: Summon (Remote Move)

**User Story:** User activates summon to move car forward/reverse.

| Step | Action | Expected Result |
|------|--------|----------------|
| 7.1 | Ensure 0x273 frame cached | hasCtrl = true |
| 7.2 | Click "Summon Forward" | 30x 0x273 burst with summon bits set @ 20ms |
| 7.3 | Car begins moving forward | Visible vehicle movement |
| 7.4 | Click "Stop" | summonRemaining reset to 0 |
| 7.5 | Click "Summon Reverse" | 30x reverse burst |
| 7.6 | Click "Stop" | Car stops |

---

## Scenario 8: Dual-CAN Bus Operation

**User Story:** User has both MCP2515 modules connected for full control.

| Step | Action | Expected Result |
|------|--------|----------------|
| 8.1 | Boot with `BOARD_ENABLE_MCP2515_2=1` | Boot message shows `bus2:1` |
| 8.2 | Bus 0 receives VehicleBus frames | 0x399, 0x3FD, 0x3F8, 0x273 |
| 8.3 | Bus 1 receives PowertrainBus frames | 0x2F3, 0x333, 0x334, 0x273 |
| 8.4 | Climate/Charge/Drive commands work | Sent on bus 1 (ctrlBus) |
| 8.5 | Mirror/Lock/Trunk commands work | Sent on bus 0 or ctrlBus |
| 8.6 | Bus 1 init fails gracefully | Single-bus mode, bus 1 commands show error |

---

## Scenario 9: Bluetooth Connection (HC-05)

**User Story:** User connects via Bluetooth instead of USB.

| Step | Action | Expected Result |
|------|--------|----------------|
| 9.1 | Build with `BOARD_ENABLE_BT=1` | Boot message shows `cap:"usb+bluetooth"` |
| 9.2 | Pair phone/laptop with HC-05 | BT pairing successful @ 9600 baud |
| 9.3 | Send commands via BT serial | Same JSON protocol, same ACK responses |
| 9.4 | USB and BT operate simultaneously | Both receive status messages |

---

## Scenario 10: Power Cycle Recovery

**User Story:** Board loses power briefly (e.g., powerbank hiccup, cable wiggle).

| Step | Action | Expected Result |
|------|--------|----------------|
| 10.1 | Board running normally with FSD/Nag/Profile set | All active |
| 10.2 | Powerbank disconnects for 2 seconds | Board loses power |
| 10.3 | Powerbank reconnects | Board reboots from EEPROM |
| 10.4 | Settings restored | FSD/Nag/Profile match pre-disconnect |
| 10.5 | CAN frames resume flowing | Board applies mods immediately |
| 10.6 | **NEW:** Frame caches rebuild | hasCtrl/hasClimate/hasCharge/hasDrive repopulate from live CAN |

---

## Scenario 11: Dashboard Mobile Access

**User Story:** User accesses dashboard from phone while in car.

| Step | Action | Expected Result |
|------|--------|----------------|
| 11.1 | Open dashboard URL on phone browser | Responsive layout loads |
| 11.2 | Controls visible and usable | Touch-friendly buttons, proper spacing |
| 11.3 | Connect via USB OTG | Web Serial works on Android Chrome |
| 11.4 | All feature sections accessible | Scrollable, no hidden controls |

---

## Scenario 12: Error Handling & Edge Cases

| Case | Trigger | Expected Result |
|------|---------|----------------|
| 12.1 | Send command without CAN connection | Error: "CAN init failed" |
| 12.2 | Send vehicle cmd without 0x273 cached | Error: "Waiting for 0x273 frame" |
| 12.3 | Send climate cmd without 0x2F3 cached | Error: "Need 0x2F3 frame cached" |
| 12.4 | Send unknown command | Error: "Unknown command" |
| 12.5 | USB cable disconnects mid-session | Dashboard shows "Not Connected" |
| 12.6 | Rapid button clicking (spam) | Board processes sequentially, no crash |
| 12.7 | EEPROM corrupted/empty | Board uses safe defaults |
| 12.8 | MCP2515 crystal mismatch | Init tries 8MHz then 16MHz |
