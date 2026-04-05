#pragma once
#include <string.h>
#include "core/types.h"
#include "protocol/lock.h"

// ── Lock Command Execution ───────────────────────────────────────────────────

static bool execLockCmd(const char* cmd, State& s) {
  if (!s.hasCtrl) return false;
  
  if (strcmp(cmd, "lock") == 0) {
    controlLock(LOCK, s);
    return true;
  }
  if (strcmp(cmd, "unlock") == 0) {
    controlLock(UNLOCK, s);
    return true;
  }
  if (strcmp(cmd, "lock:child") == 0) {
    controlChildLock(s);
    return true;
  }
  if (strcmp(cmd, "horn") == 0) {
    controlHorn(s);
    return true;
  }
  return false;
}
