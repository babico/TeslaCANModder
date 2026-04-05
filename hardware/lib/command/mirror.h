#pragma once
#include <string.h>
#include "core/types.h"
#include "protocol/mirror.h"

// ── Mirror Command Execution ─────────────────────────────────────────────────

static bool execMirrorCmd(const char* cmd, State& s) {
  if (!s.hasCtrl) return false;
  
  if (strcmp(cmd, "mirror:fold") == 0) {
    controlMirrorFold(MIRROR_FOLD, s);
    return true;
  }
  if (strcmp(cmd, "mirror:unfold") == 0) {
    controlMirrorFold(MIRROR_UNFOLD, s);
    return true;
  }
  if (strcmp(cmd, "mirror:heat") == 0) {
    controlMirrorHeat(s);
    return true;
  }
  if (strcmp(cmd, "mirror:autofold") == 0) {
    controlAutoFoldMirrors(s);
    return true;
  }
  if (strcmp(cmd, "mirror:dip") == 0) {
    controlMirrorDip(s);
    return true;
  }
  return false;
}
