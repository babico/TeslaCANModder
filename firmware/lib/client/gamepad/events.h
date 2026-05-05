#pragma once
// ── Gamepad HID Report Decode + Button Event Queue ──────────────────────────

#include "client/gamepad/state.h"

#if BOARD_ENABLE_BLE

static inline void gpEvtPush(uint8_t btn)
{
	uint8_t h = gpEvtH.load(std::memory_order_relaxed);
	uint8_t n = (h + 1) % GP_EVT_SZ;
	if (n != gpEvtT.load(std::memory_order_acquire))
	{
		gpEvtBuf[h] = btn;
		gpEvtH.store(n, std::memory_order_release);
	}
}

static inline bool gpEvtPop(uint8_t &btn)
{
	uint8_t t = gpEvtT.load(std::memory_order_relaxed);
	if (t == gpEvtH.load(std::memory_order_acquire))
		return false;
	btn = gpEvtBuf[t];
	gpEvtT.store((t + 1) % GP_EVT_SZ, std::memory_order_release);
	return true;
}

static void gpDecodeReport(const uint8_t *d, size_t len)
{
	if (len == 0)
		return;
	if (len > 4 && d[0] < 4)
	{
		d++;
		len--;
	}

	size_t cp = len > sizeof(gpRaw) ? sizeof(gpRaw) : len;
	memcpy(gpRaw, d, cp);
	gpRawLen = (uint8_t)cp;

	if (len < 2)
		return;
	gpButtonsPrev = gpButtons;
	gpButtons = (uint16_t)d[0] | ((uint16_t)d[1] << 8);

	unsigned long now = millis();
	uint16_t rising = gpButtons & ~gpButtonsPrev;
	uint16_t falling = gpButtonsPrev & ~gpButtons;
	for (int i = 0; i < GAMEPAD_BTN_COUNT; i++)
	{
		uint16_t mask = (uint16_t)(1u << i);
		if (rising & mask)
		{
			gpEvtPush((uint8_t)i);
			gpBtnDownMs[i] = now;
		}
		if (falling & mask)
		{
			gpBtnDownMs[i] = 0;
			gpHoldFiredMask &= (uint16_t)~mask;
		}
	}

	if (len >= 4)
	{
		gpAxes[0] = d[2];
		gpAxes[1] = d[3];
	}
	if (len >= 6)
	{
		gpAxes[2] = d[4];
		gpAxes[3] = d[5];
	}
	if (len >= 7)
		gpAxes[4] = d[6];
	if (len >= 8)
		gpAxes[5] = d[7];
}

static void gamepadFlushEvents(State &s, unsigned long now)
{
	if (!gpEnabled || !gpConnected)
		return;
	uint8_t evt;
	while (gpEvtPop(evt))
	{
		bool hold = (evt & GP_EVT_HOLD_FLAG) != 0;
		uint8_t btn = evt & 0x7F;
		if (btn >= GAMEPAD_BTN_COUNT)
			continue;
		const char *cmd = hold ? gpBindingHold[btn] : gpBinding[btn];
		if (cmd && cmd[0] != '\0')
			executeCommand(cmd, s, now);
	}
}

#endif // BOARD_ENABLE_BLE
