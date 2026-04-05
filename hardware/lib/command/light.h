#pragma once
#include <string.h>
#include "core/types.h"
#include "protocol/light.h"

// ── Light Command Execution ──────────────────────────────────────────────────

static bool execLightCmd(const char* cmd, State& s) {
  if (!s.hasCtrl) return false;
  
  if (strcmp(cmd, "light:fog:front") == 0) {
    controlFrontFog(s);
    return true;
  }
  if (strcmp(cmd, "light:fog:rear") == 0) {
    controlRearFog(s);
    return true;
  }
  if (strcmp(cmd, "light:highbeam:auto") == 0) {
    controlAutoHighBeam(s);
    return true;
  }
  if (strcmp(cmd, "light:ambient") == 0) {
    controlAmbientLight(s);
    return true;
  }
  if (strcmp(cmd, "light:home") == 0) {
    controlHomeLight(s);
    return true;
  }
  if (strcmp(cmd, "light:dome:off") == 0) {
    controlDomeLight(DOME_OFF, s);
    return true;
  }
  if (strcmp(cmd, "light:dome:on") == 0) {
    controlDomeLight(DOME_ON, s);
    return true;
  }
  if (strcmp(cmd, "light:dome:auto") == 0) {
    controlDomeLight(DOME_AUTO, s);
    return true;
  }
  return false;
}
