#pragma once

/**
 * @file firmware/lib/client/gamepad/state.h
 * @brief Shared constants, globals, and type declarations for the BLE HID gamepad subsystem
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#if BOARD_ENABLE_BLE

#include <NimBLEDevice.h>
#include <Preferences.h>
#include <atomic>
#include <string.h>
#include "core/forward.h"
#include "core/types.h"
#include "vehicle/can/feature/das/das_drive.h"

/**
 * @brief Execute a named command string against the firmware state.
 * @param cmd Null-terminated command string (e.g. "drive:on").
 * @param s Reference to the global firmware state.
 * @param now Current timestamp in milliseconds.
 */
void executeCommand(const char *cmd, State &s, unsigned long now);

#define GAMEPAD_BTN_COUNT 16       // Maximum number of supported gamepad buttons
#define GAMEPAD_CMD_MAXLEN 48      // Maximum length of a command binding string
#define GP_HOLD_MS 500             // Hold threshold in milliseconds before firing hold event
#define GP_MAX_SCAN 8              // Maximum number of BLE devices tracked during scan
#define GP_EVT_SZ 32               // Circular event queue capacity (must be power of 2 or mod-safe)
#define GP_EVT_HOLD_FLAG 0x80      // High bit flag indicating a hold event in the queue byte

/**
 * @brief Human-readable names for each gamepad button index (0-15).
 */
static const char *const kGpBtnName[GAMEPAD_BTN_COUNT] = {
	"A", "B", "X", "Y", "LB", "RB", "Back", "Start", "L3", "R3", "DUp", "DDown", "DLeft", "DRight", "Btn14", "Btn15"};

/**
 * @brief Default command bindings for short-press on each button.
 */
static char gpBinding[GAMEPAD_BTN_COUNT][GAMEPAD_CMD_MAXLEN] = {
	"drive:on", "drive:off", "horn",	  "light:highbeam:auto", "turn:left3",	   "turn:right3",	  "turn:hazard",
	"turn:off", "wiper:1",	 "wiper:off", "light:dome:on",		 "light:dome:off", "light:fog:front", "light:fog:rear",
	"lock",		"unlock"};

static bool gpEnabled = false;             // Whether gamepad subsystem is active
static bool gpConnected = false;           // Whether a gamepad is currently connected
static bool gpScanning = false;            // Whether BLE scan is in progress
static char gpPairedAddr[18] = {};         // MAC address of the paired gamepad (string form)
static unsigned long gpLastReconnMs = 0;   // Timestamp of last reconnection attempt
static constexpr uint32_t GP_RECONNECT_MS = 5000; // Minimum interval between reconnect attempts

static uint16_t gpButtons = 0;            // Current button bitmask from latest HID report
static uint16_t gpButtonsPrev = 0;        // Previous button bitmask for edge detection
static uint8_t gpAxes[6] = {};            // Axis values: [LX, LY, RX, RY, LT, RT]
static uint8_t gpRaw[16] = {};            // Raw HID report buffer for diagnostics
static uint8_t gpRawLen = 0;              // Length of last raw report

static uint8_t gpAxisDz[6] = {6, 6, 6, 6, 8, 8};  // Per-axis deadzone thresholds
static uint8_t gpAxisExpo[6] = {0, 0, 0, 0, 0, 0}; // Per-axis expo curve values
static uint8_t gpAxisInvMask = 0x00;      // Bitmask of inverted axes (bit N = axis N)


static int8_t gpRssi = 0;                 // Last measured RSSI of connected gamepad
static uint8_t gpBatteryPct = 0xFF;        // Battery percentage (0xFF = unknown)
static uint8_t gpReconnFails = 0;          // Consecutive reconnection failure count
static bool gpAutoRescanArmed = false;     // Whether auto-rescan triggers on disconnect
static char gpLastSeenName[33] = {};       // Display name of the last seen/paired device

static unsigned long gpBtnDownMs[GAMEPAD_BTN_COUNT] = {}; // Timestamp when each button was pressed
static uint16_t gpHoldFiredMask = 0;       // Bitmask tracking which buttons have fired hold events
static char gpBindingHold[GAMEPAD_BTN_COUNT][GAMEPAD_CMD_MAXLEN] = {}; // Hold command bindings

/**
 * @brief Discovered BLE gamepad device entry from scan results.
 */
struct GpDevice
{
	char addr[18];  // BLE MAC address string
	char name[33];  // Device advertised name
};
static GpDevice gpDevices[GP_MAX_SCAN];    // Scan result device list
static uint8_t gpDeviceCount = 0;          // Number of devices found in last scan

static uint8_t gpEvtBuf[GP_EVT_SZ];       // Circular event queue buffer
static std::atomic<uint8_t> gpEvtH{0};    // Event queue head (producer index)
static std::atomic<uint8_t> gpEvtT{0};    // Event queue tail (consumer index)

static NimBLEClient *gpClient = nullptr;   // Active NimBLE client connection
static NimBLEScan *gpScan = nullptr;       // NimBLE scan instance

#define GP_NVS_NS "tcm_gpad"               // NVS namespace for gamepad settings
#define GP_BIND_NS "tcm_gbnd"              // NVS namespace for button bindings
static Preferences gpPrefs;                // ESP32 Preferences handle for NVS access

#endif // BOARD_ENABLE_BLE
