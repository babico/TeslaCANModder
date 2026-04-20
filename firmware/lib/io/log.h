#pragma once

// Forward-declared logging functions (defined in serial.h)
// Handlers can include this to emit log messages to the monitor
void sendLog(const char* msg);
void sendLog(const __FlashStringHelper* msg);
