#pragma once
#include <stdint.h>

// ── TX Rate Limiter ──────────────────────────────────────────────────────────
// Per-CAN-ID rate limiting to prevent bus flooding.
// Tracks last TX timestamp per ID; skips send if interval < threshold.
// Default: max 10 Hz (100ms min interval) per CAN ID.

#define RATE_LIMIT_SLOTS   32    // Max unique CAN IDs tracked
#define RATE_LIMIT_DEFAULT 100   // Default minimum interval (ms) = 10 Hz

struct RateLimitEntry {
  uint32_t id;
  unsigned long lastTxMs;
};

static RateLimitEntry rateLimitTable[RATE_LIMIT_SLOTS];
static uint8_t rateLimitCount = 0;
static uint16_t rateLimitIntervalMs = RATE_LIMIT_DEFAULT;

inline void rateLimitInit() {
  rateLimitCount = 0;
  rateLimitIntervalMs = RATE_LIMIT_DEFAULT;
}

inline void rateLimitSetInterval(uint16_t ms) {
  rateLimitIntervalMs = ms > 0 ? ms : RATE_LIMIT_DEFAULT;
}

// Returns true if TX is allowed for this CAN ID at the given time
inline bool rateLimitCheck(uint32_t id, unsigned long nowMs) {
  // Search for existing entry
  for (uint8_t i = 0; i < rateLimitCount; i++) {
    if (rateLimitTable[i].id == id) {
      if (nowMs - rateLimitTable[i].lastTxMs < rateLimitIntervalMs) {
        return false;  // Too soon — rate limited
      }
      rateLimitTable[i].lastTxMs = nowMs;
      return true;
    }
  }
  // New ID: add entry if space available
  if (rateLimitCount < RATE_LIMIT_SLOTS) {
    rateLimitTable[rateLimitCount].id = id;
    rateLimitTable[rateLimitCount].lastTxMs = nowMs;
    rateLimitCount++;
  }
  return true;
}

// ── Self-Echo Detection ──────────────────────────────────────────────────────
// Flags recently sent CAN IDs to discard looped-back frames.
// Uses a small ring buffer of recently transmitted frame IDs.

#define ECHO_RING_SIZE 16

struct EchoRing {
  uint32_t ids[ECHO_RING_SIZE];
  uint8_t head;
};

static EchoRing echoRing = {{0}, 0};

inline void echoRingInit() {
  echoRing.head = 0;
  for (uint8_t i = 0; i < ECHO_RING_SIZE; i++) echoRing.ids[i] = 0xFFFFFFFF;
}

// Record a transmitted frame ID
inline void echoRingRecord(uint32_t id) {
  echoRing.ids[echoRing.head] = id;
  echoRing.head = (echoRing.head + 1) % ECHO_RING_SIZE;
}

// Check if a received frame was recently sent (echo)
inline bool echoRingCheck(uint32_t id) {
  for (uint8_t i = 0; i < ECHO_RING_SIZE; i++) {
    if (echoRing.ids[i] == id) {
      echoRing.ids[i] = 0xFFFFFFFF;  // Consume the echo flag
      return true;
    }
  }
  return false;
}
