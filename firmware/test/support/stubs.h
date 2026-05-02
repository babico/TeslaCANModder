#pragma once
#include "core/types.h"

// ── Shared recording stub for driverSend ────────────────────────────────────
// Include AFTER all library headers. Provides a recording driverSend and
// no-op sendLog stubs used by handler and dispatch tests.

struct SendCall
{
	Frame f;
	uint8_t bus;
};
static SendCall stub_sends[16];
static uint8_t stub_send_count = 0;

void driverSend(const Frame &f, uint8_t bus)
{
	if (stub_send_count < 16)
	{
		stub_sends[stub_send_count].f = f;
		stub_sends[stub_send_count].bus = bus;
		stub_send_count++;
	}
}

void sendLog(const char *) {}
void sendLog(const __FlashStringHelper *) {}

void saveSettings(const State &) {}
void resetHandlerLogFlags() {}
void applyFilters(State &) {}
