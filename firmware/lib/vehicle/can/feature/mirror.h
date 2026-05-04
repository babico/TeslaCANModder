#pragma once
#include <string.h>
#include "core/forward.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/burst.h"

// ── Mirror Bit Helpers (0x273 UI_vehicleControl) ─────────────────────────────
// These helpers set individual mirror-related bit fields in the 0x273 frame.
// Tesla mirrors are controlled via the UI_vehicleControl CAN message shared
// with wipers, windows, and other body controls.

enum MirrorFoldRequest
{
	MIRROR_IDLE = 0,
	MIRROR_FOLD = 1,
	MIRROR_UNFOLD = 2
};

// Set fold/unfold request in byte 3 bits [1:0]
inline void setMirrorFold(Frame &f, MirrorFoldRequest req)
{
	if (f.dlc < 4)
		return;
	f.data[3] = (f.data[3] & ~0x03) | (req & 0x03); // bits 24-25
}

// Enable/disable heated mirrors via byte 3 bit 2
inline void setMirrorHeat(Frame &f, bool enable)
{
	if (f.dlc < 4)
		return;
	if (enable)
		f.data[3] |= 0x04; // bit 26
	else
		f.data[3] &= ~0x04;
}

// Enable/disable automatic fold-on-lock via byte 6 bit 4
inline void setAutoFoldMirrors(Frame &f, bool enable)
{
	if (f.dlc < 7)
		return;
	if (enable)
		f.data[6] |= 0x10; // bit 52
	else
		f.data[6] &= ~0x10;
}

// Enable/disable mirror dip when shifting to reverse via byte 6 bit 5
inline void setMirrorDipOnReverse(Frame &f, bool enable)
{
	if (f.dlc < 7)
		return;
	if (enable)
		f.data[6] |= 0x20; // bit 53
	else
		f.data[6] &= ~0x20;
}

// ── Mirror Control (0x273) ───────────────────────────────────────────────────
// Each control function copies the last-seen 0x273 frame from State, modifies
// the relevant bits, then burst-sends the modified frame on BUS_VEHICLE.
// Burst ensures the ECU sees the command despite periodic factory frames.

// Fold or unfold mirrors (50 burst frames, 20 ms interval)
static void controlMirrorFold(MirrorFoldRequest req, State &s)
{
	Frame f = {CAN_ID_UI_VEHICLE_CTRL, 8};
	memcpy(f.data, s.lastCtrl, 8);
	setMirrorFold(f, req);

	startBurst(s, f, BUS_VEHICLE, 50, 20);
}

// Activate heated mirrors (30 burst frames, 20 ms interval)
static void controlMirrorHeat(State &s)
{
	Frame f = {CAN_ID_UI_VEHICLE_CTRL, 8};
	memcpy(f.data, s.lastCtrl, 8);
	setMirrorHeat(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

// Toggle factory auto-fold-on-lock setting (30 burst frames)
static void controlAutoFoldMirrors(State &s)
{
	Frame f = {CAN_ID_UI_VEHICLE_CTRL, 8};
	memcpy(f.data, s.lastCtrl, 8);
	setAutoFoldMirrors(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

// Dip passenger mirror when reversing (30 burst frames)
static void controlMirrorDip(State &s)
{
	Frame f = {CAN_ID_UI_VEHICLE_CTRL, 8};
	memcpy(f.data, s.lastCtrl, 8);
	setMirrorDipOnReverse(f, true);

	startBurst(s, f, BUS_VEHICLE, 30, 20);
}

// ── Mirror Auto-Fold on Lock ────────────────────────────────────────────────
// Monitors vehicle lock state transitions and automatically folds mirrors on
// lock / unfolds on unlock. Requires mirrorAutoFoldEnabled and hasCtrl.

inline void mirrorAutoFoldCheck(State &s, bool vehicleLocked)
{
	if (!s.mirrorAutoFoldEnabled)
		return;
	if (!s.hasCtrl)
		return;

	if (vehicleLocked && !s.vehicleLockedState)
	{
		// Just locked → fold mirrors
		controlMirrorFold(MIRROR_FOLD, s);
	}
	else if (!vehicleLocked && s.vehicleLockedState)
	{
		// Just unlocked → unfold mirrors
		controlMirrorFold(MIRROR_UNFOLD, s);
	}
	s.vehicleLockedState = vehicleLocked;
}

// ── Mirror Command Execution ─────────────────────────────────────────────────
// Handles all mirror:* commands including fold, unfold, heat, autofold, dip,
// and the mirror:autofold:on/off toggle for lock-based auto-folding.

static bool execMirrorCmd(const char *cmd, State &s)
{
	if (!s.hasCtrl)
		return false;

	if (strcmp(cmd, "mirror:fold") == 0)
	{
		controlMirrorFold(MIRROR_FOLD, s);
		return true;
	}
	if (strcmp(cmd, "mirror:unfold") == 0)
	{
		controlMirrorFold(MIRROR_UNFOLD, s);
		return true;
	}
	if (strcmp(cmd, "mirror:heat") == 0)
	{
		controlMirrorHeat(s);
		return true;
	}
	if (strcmp(cmd, "mirror:autofold") == 0)
	{
		controlAutoFoldMirrors(s);
		return true;
	}
	if (strcmp(cmd, "mirror:dip") == 0)
	{
		controlMirrorDip(s);
		return true;
	}
	return false;
}

// Auto-fold enable/disable toggle (persisted via NVS)
inline bool execMirrorAutoFoldCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "mirror:autofold:on") == 0)
	{
		s.mirrorAutoFoldEnabled = true;
		return true;
	}
	if (strcmp(cmd, "mirror:autofold:off") == 0)
	{
		s.mirrorAutoFoldEnabled = false;
		return true;
	}
	return false;
}
