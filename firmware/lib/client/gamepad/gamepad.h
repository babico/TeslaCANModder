#pragma once
// ── BLE HID Gamepad Central ──────────────────────────────────────────────────
// Public include point. Implementation is split by concern:
//   state.h   — shared constants/globals
//   events.h  — HID report decode + button event queue
//   storage.h — NVS load/save
//   ble.h     — NimBLE callbacks, scan/connect helpers
//   api.h     — command/API-facing gamepad functions
//   drive.h   — analog axes → DAS drive control

#if BOARD_ENABLE_BLE

#include "client/gamepad/state.h"
#include "client/gamepad/events.h"
#include "client/gamepad/storage.h"
#include "client/gamepad/ble.h"
#include "client/gamepad/api.h"
#include "client/gamepad/drive.h"

#endif // BOARD_ENABLE_BLE
