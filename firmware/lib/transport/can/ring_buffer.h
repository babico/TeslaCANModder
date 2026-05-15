#pragma once

/**
 * @file firmware/lib/transport/can/ring_buffer.h
 * @brief Lock-free ring buffer for distributing CAN frames to multiple consumers
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"

#define RING_BUF_SIZE 256 // Power of 2 for fast modulo via bitmask

/**
 * @brief Single entry in the frame ring buffer
 */
struct RingEntry
{
	Frame frame;           // CAN frame data
	uint8_t bus;           // Bus index the frame was received on
	uint32_t seq;          // Monotonic sequence number for gap detection
	unsigned long timestamp;
};

/**
 * @brief Ring buffer that distributes CAN frames to multiple independent consumers
 *
 * Each consumer maintains its own read index so slow consumers do not block
 * fast ones. Overflow drops oldest frames; consumers detect gaps via sequence numbers.
 */
struct FrameRingBuffer
{
	RingEntry entries[RING_BUF_SIZE];
	volatile uint32_t writeIdx; // Next write position (monotonic, masked on access)
	uint32_t seqCounter;        // Global sequence counter incremented per push

	FrameRingBuffer() : writeIdx(0), seqCounter(0) {}
};

/**
 * @brief Global ring buffer instance shared by all producers and consumers
 */
static FrameRingBuffer canRingBuffer;

/**
 * @brief Push a CAN frame into the ring buffer
 * @param f The CAN frame to store
 * @param bus Bus index the frame was received on
 * @param now Current timestamp in milliseconds
 */
inline void ringPush(const Frame &f, uint8_t bus, unsigned long now)
{
	uint32_t idx = canRingBuffer.writeIdx & (RING_BUF_SIZE - 1); // Bitmask modulo for power-of-2 size
	RingEntry &e = canRingBuffer.entries[idx];
	e.frame = f;
	e.bus = bus;
	e.seq = canRingBuffer.seqCounter++;
	e.timestamp = now;
	canRingBuffer.writeIdx++;
}

/**
 * @brief Per-consumer read state for the shared ring buffer
 *
 * Each consumer (WiFi, BLE, Serial, Recording) holds its own RingConsumer
 * to independently track read position and detect overflow.
 */
struct RingConsumer
{
	uint32_t readIdx;  // Next read position (tracks writeIdx)
	uint32_t lastSeq;  // Last sequence number seen (for gap detection)
	uint32_t dropped;  // Count of frames lost due to consumer falling behind

	RingConsumer() : readIdx(0), lastSeq(0), dropped(0) {}
};

/**
 * @brief Check if a consumer has unread frames available
 * @param c The consumer to query
 * @return True if there are frames the consumer has not yet read
 */
inline bool ringHasData(const RingConsumer &c)
{
	return c.readIdx < canRingBuffer.writeIdx;
}

/**
 * @brief Get the number of unread frames for a consumer, capped at buffer size
 * @param c The consumer to query
 * @return Number of available frames (0 to RING_BUF_SIZE)
 */
inline uint32_t ringAvailable(const RingConsumer &c)
{
	uint32_t avail = canRingBuffer.writeIdx - c.readIdx;
	if (avail > RING_BUF_SIZE)
		avail = RING_BUF_SIZE; // Cap to buffer capacity
	return avail;
}

/**
 * @brief Peek at the next unread frame for a consumer
 * @param c The consumer (may be advanced if overflow detected)
 * @return Pointer to the next ring entry, or nullptr if no data available
 */
inline const RingEntry *ringPeek(RingConsumer &c)
{
	if (!ringHasData(c))
		return nullptr;
	// Detect if consumer was lapped (writer overwrote unread entries)
	if (canRingBuffer.writeIdx - c.readIdx > RING_BUF_SIZE)
	{
		uint32_t lost = (canRingBuffer.writeIdx - c.readIdx) - RING_BUF_SIZE;
		c.dropped += lost;
		c.readIdx = canRingBuffer.writeIdx - RING_BUF_SIZE; // Skip to oldest available
	}
	uint32_t idx = c.readIdx & (RING_BUF_SIZE - 1); // Bitmask modulo
	return &canRingBuffer.entries[idx];
}

/**
 * @brief Advance the consumer past the last peeked entry
 * @param c The consumer to advance
 */
inline void ringAdvance(RingConsumer &c)
{
	c.readIdx++;
}

/**
 * @brief Reset a consumer to the current write position, skipping all pending frames
 * @param c The consumer to reset
 */
inline void ringReset(RingConsumer &c)
{
	c.readIdx = canRingBuffer.writeIdx;
	c.dropped = 0;
}

/**
 * @brief Get the total number of frames pushed into the ring buffer since boot
 * @return Cumulative frame count
 */
inline uint32_t ringTotalFrames()
{
	return canRingBuffer.seqCounter;
}
