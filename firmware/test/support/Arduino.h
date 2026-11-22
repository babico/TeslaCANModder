#pragma once

/**
 * @file firmware/test/support/Arduino.h
 * @brief Empty native-test stand-in for the Arduino framework header.
 *
 * Needed only by suites that pull in `client/command/dispatch.h` transitively
 * (its `messages.h` → `output.h` → `state.h` → `features.h` chain includes
 * <Arduino.h>). Deliberately declares no `ARDUINO` macro so `core/forward.h`
 * still supplies the native F() shim.
 */
