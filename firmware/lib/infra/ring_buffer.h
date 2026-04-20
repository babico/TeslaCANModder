#pragma once
#include "core/types.h"

// ── 5.2 Ring Buffer Frame Distribution ──────────────────────────────────────
// FreeRTOS-compatible lock-free ring buffer for distributing CAN frames to
// multiple consumers (WiFi, BLE, Serial, Recording) without blocking the
// CAN ISR or main loop.
//
// Each consumer maintains its own read index, so slow consumers don't block
// fast ones. Overflow drops oldest frames (consumer detects gap via sequence).

#define RING_BUF_SIZE 256  // power of 2 for fast modulo

struct RingEntry {
  Frame frame;
  uint8_t bus;
  uint32_t seq;          // monotonic sequence number
  unsigned long timestamp;
};

struct FrameRingBuffer {
  RingEntry entries[RING_BUF_SIZE];
  volatile uint32_t writeIdx;  // next write position (monotonic, mod on access)
  uint32_t seqCounter;         // global sequence counter

  FrameRingBuffer() : writeIdx(0), seqCounter(0) {}
};

// Single global ring buffer
static FrameRingBuffer canRingBuffer;

// ── Producer API (called from main loop after driverRead) ────────────────────

inline void ringPush(const Frame& f, uint8_t bus, unsigned long now) {
  uint32_t idx = canRingBuffer.writeIdx & (RING_BUF_SIZE - 1);
  RingEntry& e = canRingBuffer.entries[idx];
  e.frame = f;
  e.bus = bus;
  e.seq = canRingBuffer.seqCounter++;
  e.timestamp = now;
  canRingBuffer.writeIdx++;
}

// ── Consumer API ─────────────────────────────────────────────────────────────

struct RingConsumer {
  uint32_t readIdx;   // next read position (tracks writeIdx)
  uint32_t lastSeq;   // last sequence seen (gap detection)
  uint32_t dropped;   // count of frames dropped due to overflow

  RingConsumer() : readIdx(0), lastSeq(0), dropped(0) {}
};

// Returns true if consumer has unread frames
inline bool ringHasData(const RingConsumer& c) {
  return c.readIdx < canRingBuffer.writeIdx;
}

// How many unread frames (capped at buffer size)
inline uint32_t ringAvailable(const RingConsumer& c) {
  uint32_t avail = canRingBuffer.writeIdx - c.readIdx;
  if (avail > RING_BUF_SIZE) avail = RING_BUF_SIZE;
  return avail;
}

// Read next frame. Returns nullptr if no data.
inline const RingEntry* ringPeek(RingConsumer& c) {
  if (!ringHasData(c)) return nullptr;
  // Check if we've been lapped (overflow)
  if (canRingBuffer.writeIdx - c.readIdx > RING_BUF_SIZE) {
    uint32_t lost = (canRingBuffer.writeIdx - c.readIdx) - RING_BUF_SIZE;
    c.dropped += lost;
    c.readIdx = canRingBuffer.writeIdx - RING_BUF_SIZE;
  }
  uint32_t idx = c.readIdx & (RING_BUF_SIZE - 1);
  return &canRingBuffer.entries[idx];
}

// Advance consumer past the last peeked entry
inline void ringAdvance(RingConsumer& c) {
  c.readIdx++;
}

// Reset consumer to current write position (skip all pending)
inline void ringReset(RingConsumer& c) {
  c.readIdx = canRingBuffer.writeIdx;
  c.dropped = 0;
}

// Get total frames pushed since boot
inline uint32_t ringTotalFrames() {
  return canRingBuffer.seqCounter;
}
