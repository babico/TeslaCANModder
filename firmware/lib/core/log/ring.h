#pragma once

/**
 * @file firmware/lib/core/log/ring.h
 * @brief Fixed-size ring buffer for debug event logging, queryable via web or serial
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#define LOG_RING_SIZE 256    // Maximum number of log entries retained
#define LOG_MSG_MAX_LEN 64   // Maximum characters per log message (including null terminator)

/**
 * @brief Single entry in the log ring buffer
 */
struct LogEntry
{
	char msg[LOG_MSG_MAX_LEN];  // Null-terminated log message text
	unsigned long timestamp;    // Capture time in milliseconds since boot
	bool used;                  // Whether this slot contains a valid entry
};

/**
 * @brief Circular buffer holding the most recent LOG_RING_SIZE log entries
 */
struct LogRing
{
	LogEntry entries[LOG_RING_SIZE];  // Fixed storage for all log slots
	uint16_t head;                   // Next write position (points past the newest entry)
	uint16_t count;                  // Total valid entries currently stored
};

static LogRing logRing = {{{"\0", 0, false}}, 0, 0};

/**
 * @brief Reset the log ring to an empty state
 */
inline void logRingInit()
{
	logRing.head = 0;
	logRing.count = 0;
	for (uint16_t i = 0; i < LOG_RING_SIZE; i++)
	{
		logRing.entries[i].used = false;
	}
}

/**
 * @brief Push a new message into the ring buffer, overwriting the oldest entry if full
 * @param msg Null-terminated message string to store
 * @param ms Timestamp in milliseconds (typically millis())
 */
inline void logRingPush(const char *msg, unsigned long ms)
{
	LogEntry &e = logRing.entries[logRing.head];
	uint8_t len = 0;
	// Manual copy to avoid pulling in full string library
	while (msg[len] && len < LOG_MSG_MAX_LEN - 1)
	{
		e.msg[len] = msg[len];
		len++;
	}
	e.msg[len] = '\0';
	e.timestamp = ms;
	e.used = true;
	logRing.head = (logRing.head + 1) % LOG_RING_SIZE;
	if (logRing.count < LOG_RING_SIZE)
		logRing.count++;
}

/**
 * @brief Retrieve a log entry by logical index (0 = oldest available)
 * @param index Zero-based index from the oldest retained entry
 * @return Pointer to the log entry, or nullptr if index is out of range
 */
inline const LogEntry *logRingGet(uint16_t index)
{
	if (index >= logRing.count)
		return nullptr;
	uint16_t pos;
	if (logRing.count < LOG_RING_SIZE)
	{
		pos = index;
	}
	else
	{
		// Buffer has wrapped; oldest entry starts at current head position
		pos = (logRing.head + index) % LOG_RING_SIZE;
	}
	return &logRing.entries[pos];
}

/**
 * @brief Return the number of valid entries currently in the ring
 * @return Entry count (0 to LOG_RING_SIZE)
 */
inline uint16_t logRingCount()
{
	return logRing.count;
}

/**
 * @brief Return the current head (write cursor) position for polling
 * @return Raw head index into the entries array
 */
inline uint16_t logRingHead()
{
	return logRing.head;
}

/**
 * @brief Read entries pushed since a previous head snapshot
 * @param since Head value from a prior call to logRingHead()
 * @param out Output array to fill with new entries (oldest first)
 * @param maxOut Maximum number of entries to copy into out
 * @return Number of entries actually written to out
 */
inline uint16_t logRingReadSince(uint16_t since, LogEntry *out, uint16_t maxOut)
{
	uint16_t total = logRing.count;
	// Delta = entries pushed between the saved cursor and current head
	uint16_t current = logRing.head;
	uint16_t delta = (current >= since) ? (current - since) : (LOG_RING_SIZE - since + current);
	if (delta > total)
		delta = total;
	if (delta > maxOut)
		delta = maxOut;
	if (delta == 0)
		return 0;

	// Walk forward from the oldest of the new entries
	uint16_t pos = (current + LOG_RING_SIZE - delta) % LOG_RING_SIZE;
	for (uint16_t i = 0; i < delta; i++)
	{
		out[i] = logRing.entries[pos];
		pos = (pos + 1) % LOG_RING_SIZE;
	}
	return delta;
}
