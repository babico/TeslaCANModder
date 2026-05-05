#pragma once
#include "core/forward.h"

// ── Speed Profile Encoding ───────────────────────────────────────────────────
// Writes the speed profile into the FSD mux frame (HW3/Legacy v12/v13 or HW4).

inline void setSpeedProfileV12V13(Frame &f, int profile)
{
	if (f.dlc < 7)
		return;
	f.data[6] = (f.data[6] & ~0x06) | ((profile & 0x03) << 1);
}

inline void writeHW4SpeedProfile(Frame &f, int profile)
{
	if (f.dlc < 8)
		return;
	f.data[7] = (f.data[7] & ~0x70) | ((profile & 0x07) << 4);
}

// ── Follow Distance → Speed Profile Mapping ──────────────────────────────────
// Maps the follow distance CAN value to a speed profile index (-1 = invalid).

inline int mapHW3FollowDistToProfile(uint8_t fd)
{
	static const int8_t map[] = {-1, 2, 1, 0};
	return fd < 4 ? map[fd] : -1;
}

inline int mapHW4FollowDistToProfile(uint8_t fd)
{
	static const int8_t map[] = {-1, 3, 2, 1, 0, 4};
	return fd < 6 ? map[fd] : -1;
}

// ── Speed Profile Command (profile:N, sp:N, profile:auto, profile:lock/unlock) ──
static bool executeProfileCmd(const char *cmd, State &s)
{
	const char *arg = nullptr;
	if (strncmp(cmd, "profile:", 8) == 0)
		arg = cmd + 8;
	else if (strncmp(cmd, "sp:", 3) == 0)
		arg = cmd + 3;
	else
		return false;
	if (!s.features().profile)
		return false;

	if (strcmp(arg, "auto") == 0)
	{
		s.profileOverride = false;
		resetHandlerLogFlags();
		saveSettings(s);
		return true;
	}
	if (strcmp(arg, "lock") == 0)
	{
		s.profileOverride = true;
		resetHandlerLogFlags();
		saveSettings(s);
		return true;
	}
	if (strcmp(arg, "unlock") == 0)
	{
		s.profileOverride = false;
		resetHandlerLogFlags();
		saveSettings(s);
		return true;
	}
	int p = atoi(arg);
	if (p < 0 || p > 4)
		return false;
	s.speedProfile = p;
	s.profileOverride = true;
	resetHandlerLogFlags();
	saveSettings(s);
	return true;
}
