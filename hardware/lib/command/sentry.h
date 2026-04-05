#pragma once
#include "core/types.h"
#include "protocol/sentry.h"

// ── Sentry Command Execution ────────────────────────────────────────────────

static bool execSentryCmd(const char* cmd, State& s) {
  if (s.variant == LEGACY) return false;
  
  if (strcmp(cmd, "sentry:on") == 0) {
    controlSentry(true, s);
    return true;
  }
  if (strcmp(cmd, "sentry:off") == 0) {
    controlSentry(false, s);
    return true;
  }
  return false;
}
