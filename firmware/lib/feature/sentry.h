#pragma once
#include <string.h>
#include "core/forward.h"
#include "infra/can.h"
#include "infra/burst.h"

// ── Sentry Mode Control (0x284) ──────────────────────────────────────────────

static void controlSentry(bool enable, State &s)
{
	Frame f;
	f.id = CAN_ID_SENTRY;
	f.dlc = 5;
	f.data[0] = enable ? 0x20 : 0x00;
	f.data[1] = 0x00;
	f.data[2] = 0x00;
	f.data[3] = 0x00;
	f.data[4] = 0x00;

	startBurst(s, f, BUS_BODY, 30, 20);
}

// ── Sentry Command Execution ────────────────────────────────────────────────

static bool execSentryCmd(const char *cmd, State &s)
{
	if (s.variant == LEGACY)
		return false;

	if (strcmp(cmd, "sentry:on") == 0)
	{
		controlSentry(true, s);
		return true;
	}
	if (strcmp(cmd, "sentry:off") == 0)
	{
		controlSentry(false, s);
		return true;
	}
	return false;
}
