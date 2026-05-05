#pragma once
#include "vehicle/can/fwd.h"

// ── Wiper Bit Helpers (0x273 UI_vehicleControl) ─────────────────────────────
// Wiper speed is encoded in byte 7 bits [2:0] of the UI_vehicleControl frame.
// Tesla supports 4 wiper states: off (0), intermittent (1), normal (2), fast (3).

enum WiperRequest
{
	WIPER_OFF = 0,
	WIPER_1 = 1,
	WIPER_2 = 2,
	WIPER_3 = 3
};

// Write wiper speed into byte 7 lower 3 bits
inline void setWiperRequest(Frame &f, WiperRequest speed)
{
	if (f.dlc < 8)
		return;
	f.data[7] = (f.data[7] & ~0x07) | (speed & 0x07); // bits 56-58
}

// ── Wiper Control (0x273) ────────────────────────────────────────────────────
// Copies the last-seen 0x273 frame, sets the requested wiper speed,
// and burst-sends it (20 frames, 20 ms apart) to override factory wipers.

static void controlWiper(WiperRequest speed, State &s)
{
	Frame f = makeCtrlFrame(s);
	setWiperRequest(f, speed);

	startBurst(s, f, BUS_VEHICLE, 20, 20);
}

// ── Wiper Speed Persistence ─────────────────────────────────────────────────
// Tesla resets wiper speed to auto on each drive. These functions persist the
// last-set wiper speed via NVS and re-inject it on boot/wake via controlWiper.

// Restore the persisted wiper speed on boot/wake (requires hasCtrl)
inline void wiperPersistRestore(State &s)
{
	if (!s.wiperPersistEnabled)
		return;
	if (s.savedWiperSpeed == 0)
		return;
	if (!s.hasCtrl)
		return;

	WiperRequest req = (WiperRequest)s.savedWiperSpeed;
	controlWiper(req, s);
}

// Save current wiper speed to State (later persisted to NVS by saveSettings)
inline void wiperPersistSave(State &s, uint8_t speed)
{
	if (!s.wiperPersistEnabled)
		return;
	s.savedWiperSpeed = speed;
}

// ── Wiper Command Execution ──────────────────────────────────────────────────
// Handles wiper:off/1/2/3 direct speed commands and wiperpersist:on/off toggle.

static bool executeWiperCmd(const char *cmd, State &s)
{
	if (!s.hasCtrl)
		return false;

	if (strcmp(cmd, "wiper:off") == 0)
	{
		controlWiper(WIPER_OFF, s);
		return true;
	}
	if (strcmp(cmd, "wiper:1") == 0)
	{
		controlWiper(WIPER_1, s);
		return true;
	}
	if (strcmp(cmd, "wiper:2") == 0)
	{
		controlWiper(WIPER_2, s);
		return true;
	}
	if (strcmp(cmd, "wiper:3") == 0)
	{
		controlWiper(WIPER_3, s);
		return true;
	}
	return false;
}

// Wiper persistence enable/disable toggle (persisted via NVS)
static bool executeWiperPersistCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "wiperpersist:on") == 0)
	{
		s.wiperPersistEnabled = true;
		return true;
	}
	if (strcmp(cmd, "wiperpersist:off") == 0)
	{
		s.wiperPersistEnabled = false;
		return true;
	}
	return false;
}
