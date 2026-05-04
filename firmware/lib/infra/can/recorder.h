#pragma once

#include "core/types.h"

#define CAN_RECORDER_SIZE 256

struct CanRecorderEntry
{
	unsigned long timestamp;
	uint32_t id;
	uint8_t bus;
	uint8_t dlc;
	uint8_t data[8];
};

struct CanRecorder
{
	CanRecorderEntry entries[CAN_RECORDER_SIZE];
	uint16_t head;
	uint16_t count;
	uint32_t captured;
	uint32_t dropped;
	unsigned long lastCaptureMs;
	bool enabled;
};

static CanRecorder canRecorder = {{{0, 0, 0, 0, {0}}}, 0, 0, 0, 0, 0, false};

inline void canRecorderReset()
{
	canRecorder.head = 0;
	canRecorder.count = 0;
	canRecorder.captured = 0;
	canRecorder.dropped = 0;
	canRecorder.lastCaptureMs = 0;
}

inline void canRecorderInit()
{
	canRecorderReset();
	canRecorder.enabled = false;
}

inline void canRecorderStart(bool clearBuffer = true)
{
	if (clearBuffer)
	{
		canRecorderReset();
	}
	canRecorder.enabled = true;
}

inline void canRecorderStop()
{
	canRecorder.enabled = false;
}

inline bool canRecorderEnabled()
{
	return canRecorder.enabled;
}

inline uint16_t canRecorderCount()
{
	return canRecorder.count;
}

inline uint16_t canRecorderCapacity()
{
	return CAN_RECORDER_SIZE;
}

inline uint32_t canRecorderCapturedTotal()
{
	return canRecorder.captured;
}

inline uint32_t canRecorderDroppedTotal()
{
	return canRecorder.dropped;
}

inline unsigned long canRecorderLastCaptureMs()
{
	return canRecorder.lastCaptureMs;
}

inline void canRecorderCapture(const Frame &f, uint8_t bus, unsigned long now)
{
	if (!canRecorder.enabled)
	{
		return;
	}

	CanRecorderEntry &entry = canRecorder.entries[canRecorder.head];
	entry.timestamp = now;
	entry.id = f.id;
	entry.bus = bus;
	entry.dlc = f.dlc;

	for (uint8_t i = 0; i < 8; i++)
	{
		entry.data[i] = (i < f.dlc) ? f.data[i] : 0;
	}

	canRecorder.head = (canRecorder.head + 1) % CAN_RECORDER_SIZE;
	if (canRecorder.count < CAN_RECORDER_SIZE)
	{
		canRecorder.count++;
	}
	else
	{
		canRecorder.dropped++;
	}

	canRecorder.captured++;
	canRecorder.lastCaptureMs = now;
}

inline const CanRecorderEntry *canRecorderGet(uint16_t index)
{
	if (index >= canRecorder.count)
	{
		return nullptr;
	}

	uint16_t pos;
	if (canRecorder.count < CAN_RECORDER_SIZE)
	{
		pos = index;
	}
	else
	{
		pos = (canRecorder.head + index) % CAN_RECORDER_SIZE;
	}

	return &canRecorder.entries[pos];
}
