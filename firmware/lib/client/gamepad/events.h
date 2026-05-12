#pragma once

/**
 * @file firmware/lib/client/gamepad/events.h
 * @brief HID report decoding and lock-free button event queue for BLE gamepad input
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "client/gamepad/state.h"

#if BOARD_ENABLE_BLE

/**
 * @brief Push a button event into the lock-free circular event queue.
 * @param btn Button index (0-15) or hold-flagged event byte.
 */
static inline void gpEvtPush(uint8_t btn)
{
	uint8_t h = gpEvtH.load(std::memory_order_relaxed);
	uint8_t n = (h + 1) % GP_EVT_SZ;
	if (n != gpEvtT.load(std::memory_order_acquire)) // drop event if queue is full
	{
		gpEvtBuf[h] = btn;
		gpEvtH.store(n, std::memory_order_release);
	}
}

/**
 * @brief Pop the next button event from the circular event queue.
 * @param btn Output reference receiving the event byte.
 * @return True if an event was available, false if queue was empty.
 */
static inline bool gpEvtPop(uint8_t &btn)
{
	uint8_t t = gpEvtT.load(std::memory_order_relaxed);
	if (t == gpEvtH.load(std::memory_order_acquire))
		return false;
	btn = gpEvtBuf[t];
	gpEvtT.store((t + 1) % GP_EVT_SZ, std::memory_order_release);
	return true;
}

/**
 * @brief Decode a raw HID gamepad report into button states and axis values.
 * @param d Pointer to the raw HID report bytes.
 * @param len Length of the report in bytes.
 */
static void gpDecodeReport(const uint8_t *d, size_t len)
{
	if (len == 0)
		return;
	// Skip leading report-ID byte if present (value 0-3 with sufficient length)
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
	gpButtons = (uint16_t)d[0] | ((uint16_t)d[1] << 8); // 16-bit button bitmask (little-endian)

	unsigned long now = millis();
	uint16_t rising = gpButtons & ~gpButtonsPrev; // newly pressed buttons
	uint16_t falling = gpButtonsPrev & ~gpButtons; // newly released buttons
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

	// Axes are packed sequentially after the 2-byte button field
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

/**
 * @brief Drain the event queue and execute bound commands for each button event.
 * @param s Reference to the global firmware state.
 * @param now Current timestamp in milliseconds (from millis()).
 */
static void gamepadFlushEvents(State &s, unsigned long now)
{
	if (!gpEnabled || !gpConnected)
		return;
	uint8_t evt;
	while (gpEvtPop(evt))
	{
		bool hold = (evt & GP_EVT_HOLD_FLAG) != 0; // high bit distinguishes hold from press
		uint8_t btn = evt & 0x7F; // lower 7 bits are the button index
		if (btn >= GAMEPAD_BTN_COUNT)
			continue;
		const char *cmd = hold ? gpBindingHold[btn] : gpBinding[btn];
		if (cmd && cmd[0] != '\0')
			executeCommand(cmd, s, now);
	}
}

#endif // BOARD_ENABLE_BLE
