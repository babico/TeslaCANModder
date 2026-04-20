#pragma once

// ── Log Ring Buffer ──────────────────────────────────────────────────────────
// 256-entry ring buffer for debug event logging, queryable via web/serial.
// Each entry stores a short message and timestamp.

#define LOG_RING_SIZE    256
#define LOG_MSG_MAX_LEN  64

struct LogEntry {
  char msg[LOG_MSG_MAX_LEN];
  unsigned long timestamp;
  bool used;
};

struct LogRing {
  LogEntry entries[LOG_RING_SIZE];
  uint16_t head;
  uint16_t count;
};

static LogRing logRing = {{{"\0", 0, false}}, 0, 0};

inline void logRingInit() {
  logRing.head = 0;
  logRing.count = 0;
  for (uint16_t i = 0; i < LOG_RING_SIZE; i++) {
    logRing.entries[i].used = false;
  }
}

inline void logRingPush(const char* msg, unsigned long ms) {
  LogEntry& e = logRing.entries[logRing.head];
  uint8_t len = 0;
  while (msg[len] && len < LOG_MSG_MAX_LEN - 1) {
    e.msg[len] = msg[len];
    len++;
  }
  e.msg[len] = '\0';
  e.timestamp = ms;
  e.used = true;
  logRing.head = (logRing.head + 1) % LOG_RING_SIZE;
  if (logRing.count < LOG_RING_SIZE) logRing.count++;
}

// Get entry at index (0 = oldest available)
inline const LogEntry* logRingGet(uint16_t index) {
  if (index >= logRing.count) return nullptr;
  uint16_t pos;
  if (logRing.count < LOG_RING_SIZE) {
    pos = index;
  } else {
    pos = (logRing.head + index) % LOG_RING_SIZE;
  }
  return &logRing.entries[pos];
}

inline uint16_t logRingCount() {
  return logRing.count;
}

// Return entries newer than 'since' (a previously returned head index).
// 'since' should be the value of logRingHead() from the last poll.
// Fills up to maxOut entries into 'out', returns count filled.
// Use logRingHead() after calling this to get the new cursor for next poll.
inline uint16_t logRingHead() {
  return logRing.head;
}

inline uint16_t logRingReadSince(uint16_t since, LogEntry* out, uint16_t maxOut) {
  // Compute how many new entries were pushed since 'since'
  uint16_t total = logRing.count;
  // since is a raw head position; delta = (current head - since) mod ring size
  uint16_t current = logRing.head;
  uint16_t delta = (current >= since)
    ? (current - since)
    : (LOG_RING_SIZE - since + current);
  if (delta > total) delta = total;
  if (delta > maxOut) delta = maxOut;
  if (delta == 0) return 0;

  // Oldest of the delta entries starts at (current - delta) mod LOG_RING_SIZE
  uint16_t pos = (current + LOG_RING_SIZE - delta) % LOG_RING_SIZE;
  for (uint16_t i = 0; i < delta; i++) {
    out[i] = logRing.entries[pos];
    pos = (pos + 1) % LOG_RING_SIZE;
  }
  return delta;
}
