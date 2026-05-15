#pragma once

/**
 * @file firmware/lib/interface/gamepad/gamepad.h
 * @brief Public include point for the BLE HID Gamepad Central subsystem
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#if BOARD_ENABLE_BLE

#include "interface/gamepad/state.h"
#include "interface/gamepad/events.h"
#include "interface/gamepad/storage.h"
#include "interface/gamepad/ble.h"
#include "interface/gamepad/api.h"
#include "interface/gamepad/drive.h"

#endif // BOARD_ENABLE_BLE
