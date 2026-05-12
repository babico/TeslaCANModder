# Steering Torque Core Snippet

This document extracts only the core steering torque injection algorithm.

It is intentionally stored as Markdown so Arduino will not compile it as a source file.
The code block preserves the current project variable and function names to make porting easier.

```cpp
// ---------------------------------------------------------------------------
// Steering torque injection core snippet.
// ---------------------------------------------------------------------------
// This is a reference extract, not a standalone build unit.
//
// What this snippet contains:
// - DAS_autopilotState / DAS_autopilotHandsOnState state tracking.
// - EPAS torque generation.
// - EPAS HandsOnLevel generation.
// - Steering-angle-based torque direction.
// - Driver hands-on feedback bypass hook.
//
// What this snippet intentionally omits:
// - CAN frame parsing and bit packing.
// - Counter/checksum finalization.
// - BLE/dashboard command handling.
// - Project-specific UI/catalog code.
//
// Raw torque convention used by this project:
//   raw center = 2048
//   raw = 2048 + torqueNm * 100
//   +2.1 Nm -> 2258
//   -2.1 Nm -> 1838
// ---------------------------------------------------------------------------

// True when the next EPAS HandsOnLevel frame should be spoofed.
// This lets torque and HandsOnLevel decisions stay coordinated across
// separate signal callbacks that operate on the same EPAS frame.
static bool g_shouldSpoofHandsOn = false;

// Last generated synthetic HandsOnLevel.
// Used for state-1 grace hold and for smooth state transitions.
static uint8_t g_lastSpoofedHandsOnLevel = 0;

// Historical state-3 target marker retained from the project.
// The current strong-demand logic computes HandsOnLevel from torque magnitude,
// but this variable is still reset with the rest of the state-3 memory.
static uint8_t g_targetHandsOnLevelForState3 = 1;

// Last generated EPAS torsion-bar torque raw value.
// Center is 2048. This is reused for state-1 grace and state-2 hold behavior.
static uint32_t g_lastGeneratedTorqueRaw = 2048;

// Latest real incoming EPAS HandsOnLevel.
// If the project enables driver-feedback bypass and this value is non-zero,
// synthetic injection should stop because the driver may be steering.
static uint32_t g_lastEpasHandsOnRaw = 0;

// Steering angle in degrees, decoded from SCCM_steeringAngle.
// The sign determines injected torque direction.
static float g_sccmSteeringAngleDeg = 0.0f;

// State-1 grace timer and latched values.
// State 1 is normally idle, but if torque was active in state 2 or stronger
// states, we keep the last generated values briefly to avoid an abrupt cutoff.
static unsigned long g_handsState1EnterMs = 0;
static uint32_t g_state1HoldTorqueRaw = 2048;
static uint8_t g_state1HoldHandsLevel = 0;

// State-2 entry timer.
// State 2 deliberately waits 2 seconds before mild torque starts.
static unsigned long g_handsState2EnterMs = 0;

// Strong-demand entry timer.
// States 3, 4, and 5 share this timer and are treated as one strong group.
static unsigned long g_handsState3EnterMs = 0;

// State-2 level-2 hold memory.
// When mild torque first crosses the HandsOnLevel=2 threshold, the current
// torque and level are held for 1 second to avoid twitchy threshold behavior.
static unsigned long g_state2HoldUntilMs = 0;
static uint32_t g_state2HoldTorqueRaw = 2048;
static uint8_t g_state2HoldHandsLevel = 0;
static bool g_state2Level2WasActive = false;

// Latest DAS state values.
uint8_t FeaturesController::g_das_autopilot_state = 0;
uint8_t FeaturesController::g_das_hands_on_state = 0xFF;

static bool isTorqueOverrideApState() {
    // Only allow injection while Autopilot is in the active state range.
    return FeaturesController::g_das_autopilot_state >= 3 &&
           FeaturesController::g_das_autopilot_state <= 6;
}

static bool shouldPauseTorqueForSteeringAngle() {
    // Safety gate retained for future use, currently disabled by request.
    // A port may re-enable this if it wants to suppress injection above
    // a steering-angle threshold.
    return false;
}

static bool isHandsState1GraceActive() {
    // State 1 is idle/no-injection, but holds the previous injected value
    // for 500ms only when it has just transitioned down from an active state.
    return FeaturesController::g_das_hands_on_state == 1 &&
           g_handsState1EnterMs != 0 &&
           millis() - g_handsState1EnterMs < 500UL;
}

static bool isStrongHandsOnDemand(uint32_t state) {
    // States 3, 4, and 5 use the same strong ramp-and-hold torque pattern.
    // Moving between 3/4/5 does not restart the initial 1-second pause.
    return state == 3 || state == 4 || state == 5;
}

static bool isHandsState3InitialPause() {
    // Strong demand starts with a 1-second no-injection pause.
    return isStrongHandsOnDemand(FeaturesController::g_das_hands_on_state) &&
           g_handsState3EnterMs != 0 &&
           millis() - g_handsState3EnterMs < 1000UL;
}

static bool isHandsState2DelayActive() {
    // State 2 starts with a 2-second no-injection delay.
    return FeaturesController::g_das_hands_on_state == 2 &&
           g_handsState2EnterMs != 0 &&
           millis() - g_handsState2EnterMs < 2000UL;
}

void FeaturesController::setTorqueOverrideEnabled(bool enabled) {
    g_torqueOverrideEnabled = enabled;
    if (!enabled) {
        // Turning the feature off must clear all synthetic output memory.
        g_shouldSpoofHandsOn = false;
        g_lastSpoofedHandsOnLevel = 0;
        g_targetHandsOnLevelForState3 = 1;
        g_lastGeneratedTorqueRaw = 2048;
    }
    syncControlSignals();
}

uint32_t FeaturesController::onDasAutopilotState(uint32_t val) {
    // Global AP state gate. Injection is allowed only in AP states 3..6.
    g_das_autopilot_state = val;
    return val;
}

uint32_t FeaturesController::onDasHandsOnState(uint32_t val) {
    // State 1 transition:
    // Capture the last generated torque/HandsOnLevel so the output can decay
    // through a short 500ms grace period instead of cutting instantly.
    if (g_torqueOverrideEnabled && g_das_hands_on_state != 1 && val == 1) {
        g_handsState1EnterMs = millis();
        g_state1HoldTorqueRaw = g_lastGeneratedTorqueRaw;
        g_state1HoldHandsLevel = g_lastSpoofedHandsOnLevel;
    }
    if (val != 1) {
        g_handsState1EnterMs = 0;
        g_state1HoldTorqueRaw = 2048;
        g_state1HoldHandsLevel = 0;
    }

    // State 2 transition:
    // Start a 2-second idle delay before mild organic torque begins.
    if (g_torqueOverrideEnabled && g_das_hands_on_state != 2 && val == 2) {
        g_handsState2EnterMs = millis();
    }
    if (val != 2) {
        g_handsState2EnterMs = 0;
        g_state2HoldUntilMs = 0;
        g_state2HoldTorqueRaw = 2048;
        g_state2HoldHandsLevel = 0;
        g_state2Level2WasActive = false;
    }

    // Strong-demand transition:
    // States 3/4/5 are treated as one group. Entering the group starts a
    // 1-second pause. Moving inside the group does not reset the timer.
    if (g_torqueOverrideEnabled && !isStrongHandsOnDemand(g_das_hands_on_state) && isStrongHandsOnDemand(val)) {
        g_targetHandsOnLevelForState3 = (g_lastSpoofedHandsOnLevel == 0) ? 1 : 2;
        g_handsState3EnterMs = millis();
    }
    if (!isStrongHandsOnDemand(val)) {
        g_targetHandsOnLevelForState3 = 1;
        g_handsState3EnterMs = 0;
    }

    g_das_hands_on_state = val;
    return val;
}

uint32_t FeaturesController::patchHandsOnLevel(uint32_t originalRaw) {
    // Global fail-open-to-original conditions.
    if (!g_torqueOverrideEnabled || !isTorqueOverrideApState()) {
        g_shouldSpoofHandsOn = false;
        g_lastSpoofedHandsOnLevel = 0;
        return originalRaw;
    }

    // Optional driver-feedback bypass.
    // In this project this is controlled by g_handsOverrideBypassEnabled.
    // Other projects can replace this with their own driver-presence rule.
    if (g_handsOverrideBypassEnabled && g_lastEpasHandsOnRaw != 0) {
        g_shouldSpoofHandsOn = false;
        g_lastGeneratedTorqueRaw = 2048;
        return originalRaw;
    }

    // Delay/pause windows pass original values through.
    if (shouldPauseTorqueForSteeringAngle() || isHandsState2DelayActive() || isHandsState3InitialPause()) {
        g_shouldSpoofHandsOn = false;
        g_lastGeneratedTorqueRaw = 2048;
        return originalRaw;
    }

    // State 1: idle after a short grace hold.
    if (g_das_hands_on_state == 1) {
        if (isHandsState1GraceActive()) {
            g_shouldSpoofHandsOn = true;
            return g_state1HoldHandsLevel;
        }
        g_shouldSpoofHandsOn = false;
        g_lastGeneratedTorqueRaw = 2048;
        return originalRaw;
    }

    // State 2: mild torque. HandsOnLevel follows generated torque magnitude.
    if (g_das_hands_on_state == 2) {
        g_shouldSpoofHandsOn = true;
        uint32_t level = 0;
        if (millis() < g_state2HoldUntilMs) {
            level = g_state2HoldHandsLevel;
        } else {
            int32_t absRaw = abs(static_cast<int32_t>(g_lastGeneratedTorqueRaw) - 2048);
            if (absRaw >= 200) level = 2;      // abs torque >= 2.0 Nm
            else if (absRaw >= 100) level = 1; // abs torque >= 1.0 Nm
        }
        g_lastSpoofedHandsOnLevel = static_cast<uint8_t>(level);
        return level;
    }

    // States 3/4/5: strong ramp-and-hold torque. HandsOnLevel follows
    // the same magnitude thresholds used by state 2.
    if (isStrongHandsOnDemand(g_das_hands_on_state)) {
        g_shouldSpoofHandsOn = true;
        int32_t absRaw = abs(static_cast<int32_t>(g_lastGeneratedTorqueRaw) - 2048);
        uint32_t level = 0;
        if (absRaw >= 200) level = 2;
        else if (absRaw >= 100) level = 1;
        g_lastSpoofedHandsOnLevel = static_cast<uint8_t>(level);
        return level;
    }

    g_shouldSpoofHandsOn = false;
    return originalRaw;
}

uint32_t FeaturesController::patchHandsOnLevelCanA(uint32_t originalRaw) {
    // Optional alternate EPAS path.
    return g_epasChannelAEnabled ? patchHandsOnLevel(originalRaw) : originalRaw;
}

uint32_t FeaturesController::patchHandsOnLevelCanB(uint32_t originalRaw) {
    // Default EPAS path.
    return g_epasChannelAEnabled ? originalRaw : patchHandsOnLevel(originalRaw);
}

uint32_t FeaturesController::observeEpasHandsOnLevelCanA(uint32_t val) {
    // Observe real driver feedback only on the active EPAS path.
    if (g_epasChannelAEnabled) g_lastEpasHandsOnRaw = val;
    return val;
}

uint32_t FeaturesController::observeEpasHandsOnLevelCanB(uint32_t val) {
    // Observe real driver feedback only on the active EPAS path.
    if (!g_epasChannelAEnabled) g_lastEpasHandsOnRaw = val;
    return val;
}

uint32_t FeaturesController::generateOrganicTorque(uint32_t originalRaw) {
    // Mild state-2 random-walk torque keeps its value across calls.
    static int16_t mildWalkRaw = 2048;

    // Strong state 3/4/5 currently only needs this to mark pattern continuity.
    static bool state3Active = false;

    // Global no-injection conditions.
    if (!g_torqueOverrideEnabled || !isTorqueOverrideApState() ||
        g_das_hands_on_state == 0 ||
        g_das_hands_on_state == 8 || g_das_hands_on_state == 15) {
        g_lastGeneratedTorqueRaw = originalRaw;
        return originalRaw;
    }

    // Optional driver-feedback bypass.
    if (g_handsOverrideBypassEnabled && g_lastEpasHandsOnRaw != 0) {
        g_lastGeneratedTorqueRaw = originalRaw;
        return originalRaw;
    }

    // Delay/pause windows pass original torque through.
    if (shouldPauseTorqueForSteeringAngle() || isHandsState2DelayActive()) {
        state3Active = false;
        g_lastGeneratedTorqueRaw = originalRaw;
        return originalRaw;
    }

    // State 1: hold briefly only if the state-1 grace window is active.
    if (g_das_hands_on_state == 1) {
        state3Active = false;
        if (isHandsState1GraceActive()) {
            g_lastGeneratedTorqueRaw = g_state1HoldTorqueRaw;
            return g_lastGeneratedTorqueRaw;
        }
        g_lastGeneratedTorqueRaw = originalRaw;
        return originalRaw;
    }

    // State 2: mild random-walk torque in the direction opposite steering.
    if (g_das_hands_on_state == 2) {
        state3Active = false;
        unsigned long now = millis();

        // Hold the first level-2 torque for one second once the threshold is crossed.
        if (now < g_state2HoldUntilMs) {
            g_lastGeneratedTorqueRaw = g_state2HoldTorqueRaw;
            return g_lastGeneratedTorqueRaw;
        }

        // Positive steering angle -> negative injected torque.
        // Non-positive steering angle -> positive injected torque.
        int16_t minRaw = 2098; // +0.5 Nm
        int16_t maxRaw = 2248; // +2.0 Nm
        if (g_sccmSteeringAngleDeg > 0.0f) {
            minRaw = 1848; // -2.0 Nm
            maxRaw = 1998; // -0.5 Nm
        }

        // Keep the random walk inside the currently selected range.
        if (mildWalkRaw < minRaw || mildWalkRaw > maxRaw) {
            mildWalkRaw = static_cast<int16_t>((minRaw + maxRaw) / 2);
        }
        mildWalkRaw += random(-12, 13);
        if (mildWalkRaw < minRaw) mildWalkRaw = minRaw;
        if (mildWalkRaw > maxRaw) mildWalkRaw = maxRaw;
        g_lastGeneratedTorqueRaw = static_cast<uint32_t>(mildWalkRaw);

        // If state 2 first crosses the level-2 threshold, latch it for 1 second.
        int32_t absRaw = abs(static_cast<int32_t>(g_lastGeneratedTorqueRaw) - 2048);
        bool level2Active = absRaw >= 200; // 2.0 Nm
        if (level2Active && !g_state2Level2WasActive) {
            g_state2HoldUntilMs = now + 1000UL;
            g_state2HoldTorqueRaw = g_lastGeneratedTorqueRaw;
            g_state2HoldHandsLevel = 2;
        }
        g_state2Level2WasActive = level2Active;
        return g_lastGeneratedTorqueRaw;
    }

    // States 3/4/5: strong ramp-and-hold torque.
    if (isStrongHandsOnDemand(g_das_hands_on_state)) {
        if (isHandsState3InitialPause()) {
            state3Active = false;
            g_lastGeneratedTorqueRaw = originalRaw;
            return originalRaw;
        }

        if (!state3Active) state3Active = true;

        // After the 1-second pause:
        // - Ramp from 0.0 Nm to 2.1 Nm over 500 ms.
        // - Hold 2.1 Nm for the remaining 1000 ms.
        // - Repeat every 1500 ms while state remains 3/4/5.
        unsigned long activeMs = (g_handsState3EnterMs == 0) ? 0 : (millis() - g_handsState3EnterMs - 1000UL);
        uint16_t phase = static_cast<uint16_t>(activeMs % 1500UL);
        int16_t magnitudeRaw = 210; // 2.1 Nm
        if (phase < 500) {
            magnitudeRaw = static_cast<int16_t>((static_cast<int32_t>(phase) * 210) / 500);
        }

        int16_t direction = (g_sccmSteeringAngleDeg > 0.0f) ? -1 : 1;
        int16_t offsetRaw = static_cast<int16_t>(direction * magnitudeRaw);
        int16_t raw = static_cast<int16_t>(2048 + offsetRaw);
        g_lastGeneratedTorqueRaw = static_cast<uint32_t>(raw);
        return g_lastGeneratedTorqueRaw;
    }

    state3Active = false;
    g_lastGeneratedTorqueRaw = originalRaw;
    return originalRaw;
}

uint32_t FeaturesController::generateOrganicTorqueCanA(uint32_t originalRaw) {
    // Optional alternate EPAS path.
    return g_epasChannelAEnabled ? generateOrganicTorque(originalRaw) : originalRaw;
}

uint32_t FeaturesController::generateOrganicTorqueCanB(uint32_t originalRaw) {
    // Default EPAS path.
    return g_epasChannelAEnabled ? originalRaw : generateOrganicTorque(originalRaw);
}

uint32_t FeaturesController::observeSccmSteeringAngle(uint32_t val) {
    // DBC: SCCM_steeringAngle : 16|14@1+ (0.1,-819.2)
    g_sccmSteeringAngleDeg = (static_cast<float>(val) * 0.1f) - 819.2f;
    return val;
}
```
