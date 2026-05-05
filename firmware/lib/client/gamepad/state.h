#pragma once
// ── BLE HID Gamepad Shared State ─────────────────────────────────────────────

#if BOARD_ENABLE_BLE

#include <NimBLEDevice.h>
#include <Preferences.h>
#include <atomic>
#include <string.h>
#include "core/forward.h"
#include "core/types.h"
#include "vehicle/can/feature/das_drive.h"

void executeCommand(const char *cmd, State &s, unsigned long now);

#define GAMEPAD_BTN_COUNT 16
#define GAMEPAD_CMD_MAXLEN 48
#define GP_HOLD_MS 500
#define GP_MAX_SCAN 8
#define GP_EVT_SZ 32
#define GP_EVT_HOLD_FLAG 0x80

static const char *const kGpBtnName[GAMEPAD_BTN_COUNT] = {
	"A", "B", "X", "Y", "LB", "RB", "Back", "Start", "L3", "R3", "DUp", "DDown", "DLeft", "DRight", "Btn14", "Btn15"};

static char gpBinding[GAMEPAD_BTN_COUNT][GAMEPAD_CMD_MAXLEN] = {
	"drive:on", "drive:off", "horn",	  "light:highbeam:auto", "turn:left3",	   "turn:right3",	  "turn:hazard",
	"turn:off", "wiper:1",	 "wiper:off", "light:dome:on",		 "light:dome:off", "light:fog:front", "light:fog:rear",
	"lock",		"unlock"};

static bool gpEnabled = false;
static bool gpConnected = false;
static bool gpScanning = false;
static char gpPairedAddr[18] = {};
static unsigned long gpLastReconnMs = 0;
static constexpr uint32_t GP_RECONNECT_MS = 5000;

static uint16_t gpButtons = 0;
static uint16_t gpButtonsPrev = 0;
static uint8_t gpAxes[6] = {};
static uint8_t gpRaw[16] = {};
static uint8_t gpRawLen = 0;

static uint8_t gpAxisDz[6] = {6, 6, 6, 6, 8, 8};
static uint8_t gpAxisExpo[6] = {0, 0, 0, 0, 0, 0};
static uint8_t gpAxisInvMask = 0x00;


static int8_t gpRssi = 0;
static uint8_t gpBatteryPct = 0xFF;
static uint8_t gpReconnFails = 0;
static bool gpAutoRescanArmed = false;
static char gpLastSeenName[33] = {};

static unsigned long gpBtnDownMs[GAMEPAD_BTN_COUNT] = {};
static uint16_t gpHoldFiredMask = 0;
static char gpBindingHold[GAMEPAD_BTN_COUNT][GAMEPAD_CMD_MAXLEN] = {};

struct GpDevice
{
	char addr[18];
	char name[33];
};
static GpDevice gpDevices[GP_MAX_SCAN];
static uint8_t gpDeviceCount = 0;

static uint8_t gpEvtBuf[GP_EVT_SZ];
static std::atomic<uint8_t> gpEvtH{0};
static std::atomic<uint8_t> gpEvtT{0};

static NimBLEClient *gpClient = nullptr;
static NimBLEScan *gpScan = nullptr;

#define GP_NVS_NS "tcm_gpad"
#define GP_BIND_NS "tcm_gbnd"
static Preferences gpPrefs;

#endif // BOARD_ENABLE_BLE
