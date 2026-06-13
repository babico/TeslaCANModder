#pragma once

/**
 * @file firmware/lib/core/can/recorder.h
 * @brief Circular buffer recorder for capturing CAN frames with timestamps
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"

static constexpr size_t CAN_RECORDER_SIZE = 256; // Maximum number of frames stored in the ring buffer

/**
 * @brief Single recorded CAN frame with metadata
 */
struct CanRecorderEntry
{
	unsigned long timestamp; // Capture time in milliseconds
	uint32_t id;            // CAN frame identifier
	uint8_t bus;            // Bus index the frame was received on
	uint8_t dlc;            // Data length code (0-8)
	uint8_t data[8];        // Payload bytes
};

/**
 * @brief Circular buffer that stores captured CAN frames for later retrieval
 */
struct CanRecorder
{
	CanRecorderEntry entries[CAN_RECORDER_SIZE]; // Ring buffer storage
	uint16_t head;                               // Next write position
	uint16_t count;                              // Number of valid entries (up to CAN_RECORDER_SIZE)
	uint32_t captured;                           // Total frames captured since last reset
	uint32_t dropped;                            // Frames lost due to buffer overflow
	unsigned long lastCaptureMs;                 // Timestamp of most recent capture
	bool enabled;                                // Whether recording is active
};

static CanRecorder canRecorder = {{{0, 0, 0, 0, {0}}}, 0, 0, 0, 0, 0, false};

/**
 * @brief Reset recorder counters and position without changing enabled state
 */
inline void canRecorderReset()
{
	canRecorder.head = 0;
	canRecorder.count = 0;
	canRecorder.captured = 0;
	canRecorder.dropped = 0;
	canRecorder.lastCaptureMs = 0;
}

/**
 * @brief Initialize the recorder in a disabled state with all counters zeroed
 */
inline void canRecorderInit()
{
	canRecorderReset();
	canRecorder.enabled = false;
}

/**
 * @brief Start recording CAN frames
 * @param clearBuffer If true, reset all counters and stored entries before starting
 */
inline void canRecorderStart(bool clearBuffer = true)
{
	if (clearBuffer)
	{
		canRecorderReset();
	}
	canRecorder.enabled = true;
}

/**
 * @brief Stop recording CAN frames
 */
inline void canRecorderStop()
{
	canRecorder.enabled = false;
}

/**
 * @brief Check if the recorder is currently active
 * @return True if recording is enabled
 */
inline bool canRecorderEnabled()
{
	return canRecorder.enabled;
}

/**
 * @brief Get the number of valid entries currently in the buffer
 * @return Entry count (0 to CAN_RECORDER_SIZE)
 */
inline uint16_t canRecorderCount()
{
	return canRecorder.count;
}

/**
 * @brief Get the maximum number of entries the buffer can hold
 * @return Buffer capacity (CAN_RECORDER_SIZE)
 */
inline uint16_t canRecorderCapacity()
{
	return CAN_RECORDER_SIZE;
}

/**
 * @brief Get the total number of frames captured since last reset
 * @return Cumulative capture count
 */
inline uint32_t canRecorderCapturedTotal()
{
	return canRecorder.captured;
}

/**
 * @brief Get the total number of frames dropped due to buffer overflow
 * @return Cumulative drop count
 */
inline uint32_t canRecorderDroppedTotal()
{
	return canRecorder.dropped;
}

/**
 * @brief Get the timestamp of the most recently captured frame
 * @return Millisecond timestamp of last capture
 */
inline unsigned long canRecorderLastCaptureMs()
{
	return canRecorder.lastCaptureMs;
}

/**
 * @brief Capture a CAN frame into the recorder ring buffer
 * @param f The CAN frame to record
 * @param bus Bus index the frame was received on
 * @param now Current timestamp in milliseconds
 */
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

	canRecorder.head = (canRecorder.head + 1) % CAN_RECORDER_SIZE; // Wrap around
	if (canRecorder.count < CAN_RECORDER_SIZE)
	{
		canRecorder.count++;
	}
	else
	{
		canRecorder.dropped++; // Buffer full, oldest entry overwritten
	}

	canRecorder.captured++;
	canRecorder.lastCaptureMs = now;
}

/**
 * @brief Retrieve a recorded entry by logical index (oldest first)
 * @param index Zero-based index into the recorded entries (0 = oldest)
 * @return Pointer to the entry, or nullptr if index is out of range
 */
inline const CanRecorderEntry *canRecorderGet(uint16_t index)
{
	if (index >= canRecorder.count)
	{
		return nullptr;
	}

	uint16_t pos;
	if (canRecorder.count < CAN_RECORDER_SIZE)
	{
		pos = index; // Buffer not yet full, entries start at 0
	}
	else
	{
		pos = (canRecorder.head + index) % CAN_RECORDER_SIZE; // Wrap from head for oldest-first order
	}

	return &canRecorder.entries[pos];
}
