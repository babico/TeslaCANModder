#pragma once
#include "core/forward.h"
#include "infra/parse.h"

// ── TLSSC Restore ─────────────────────────────────────────────────────────────
// Spoof DAS_autopilotConfig (0x331 / 817) to report SELF_DRIVING tier so
// the AP ECU believes FSD hardware is provisioned at the gateway level.
//
// Target frame: DAS_autopilotConfig
//   byte[0] lower 6 bits → DAS_autopilotBase + DAS_autopilot
//   0x1B = SELF_DRIVING in both DAS_autopilotBase and DAS_autopilot
//   Upper 2 bits preserved (counter / mux).
//
// Source: hypery11/flipper-tesla-fsd fsd_handle_tlssc_restore()
//         community research: gauner1986, kp43h8, MiniCS (issue #18)

// ── TLSSC Command ─────────────────────────────────────────────────────────────
// tlssc:on|off
bool executeTlsscCmd(const char* cmd, State& s) {
  if (strncmp(cmd, "tlssc:", 6) == 0) {
    if (!parseBoolCmd(cmd + 6, s.tlsscRestore, s.tlsscRestore)) return false;
    saveSettings(s);
    return true;
  }
  return false;
}

// ── TLSSC Frame Handler ───────────────────────────────────────────────────────
// Called for every 0x331 frame on the vehicle bus.
// Returns true when frame was modified (caller should retransmit).
bool handleTlssc(Frame& f, State& s) {
  if (!s.tlsscRestore) return false;
  if (f.dlc < 1) return false;

  uint8_t original = f.data[0];
  uint8_t modified = (original & 0xC0) | 0x1B;
  if (modified == original) return false;

  f.data[0] = modified;
  return true;
}
