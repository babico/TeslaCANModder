#pragma once

// ── FSD Commands — umbrella include ──────────────────────────────────────────
// Split into focused headers; this file remains for backward compatibility.
// parseBoolCmd is defined in command/system.h (must be included before this).

#include "command/fsd_toggle.h"
#include "command/nag.h"
#include "command/profile.h"
#include "command/offset.h"
#include "command/isa_chime.h"
#include "command/summon_inject.h"
#include "command/summon_cmd.h"
#include "command/variant.h"
