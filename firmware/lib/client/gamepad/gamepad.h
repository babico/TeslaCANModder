#pragma once

/**
 * @file firmware/lib/client/gamepad/gamepad.h
 * @brief Public include point for the BLE HID Gamepad Central subsystem
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#if BOARD_ENABLE_BLE

#include "client/gamepad/state.h"
#include "client/gamepad/events.h"
#include "client/gamepad/storage.h"
#include "client/gamepad/ble.h"
#include "client/gamepad/api.h"
#include "client/gamepad/drive.h"

#endif // BOARD_ENABLE_BLE
