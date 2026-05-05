#pragma once
#include "core/forward.h"
#include "core/can/bus.h"
#include "vehicle/can/ids.h"
#include <Preferences.h>

// ── DAS Drive: openpilot-style autopilot CAN injection ───────────────────────
// Sends three frames on BUS_CHASSIS (autopilot party CAN, X179 pins 13-14):
//
//   DAS_control         0x2B9 (697)  — longitudinal ACC, 25 Hz
//   DAS_steeringControl 0x488 (1160) — steering angle,   50 Hz
//   APS_eacMonitor      0x27D (637)  — EPAS steer-allow, 10 Hz
//
// REQUIRES: X179 connector physically wired to Tesla autopilot harness.
//
// HW variant mapping (matches openpilot carcontroller.py):
//   HW3  → DAS_steeringControlType = ANGLE_CONTROL (1)
//   HW4  → DAS_steeringControlType = LANE_KEEP_ASSIST (2)  [FSD14 semantics]
//   LEGACY → ANGLE_CONTROL (1)  (HW2.5, steering support uncertain)
//
// Safety gates (openpilot Tesla CarController port — opendbc/car/tesla):
//   accel_max  ≤  2.0 m/s²        (CarControllerParams.ACCEL_MAX)
//   accel_min  ≥ -3.48 m/s²       (CarControllerParams.ACCEL_MIN)
//   speed cap  ≤ runtime dasSpeedCapKph (NVS, default 25, max 200 kph)
//   jerk       ±4.9 m/s³          (JERK_LIMIT_MAX/MIN, ACC faults at ±5.0)
//   angle rate ≤ 5°/20 ms frame   (MAX_ANGLE_RATE — EPS faults at 12)
//   max angle  ≤ 360° absolute, additionally clamped by speed-aware
//              bicycle-model lateral-accel cap (≤ 3.0 m/s²) above 8 kph
//   standstill brake-hold = -0.4 m/s² when no input near zero speed
//   dead-man   frames stop if dasSetControl() not called within 150 ms
//   cancel     5× DAS_ACC_CANCEL frames sent before going silent,
//              counter pre-synced to last observed AP frame +1

// ── DAS CAN IDs (defined here, also registered in ids.h) ─────────────────────
// These IDs already exist in ids.h; they are listed here for clarity only.
// #define CAN_ID_DAS_CONTROL      0x2B9  (see ids.h)
// #define CAN_ID_DAS_STEERING_CTRL 0x488 (see ids.h)
// #define CAN_ID_APS_EAC_MONITOR  0x27D  (see ids.h)

// ── Constants ─────────────────────────────────────────────────────────────────
#define DAS_ACC_ON              4
#define DAS_ACC_CANCEL          13

#define DAS_STEER_NONE          0
#define DAS_STEER_ANGLE_CTRL    1   // HW3 / LEGACY
#define DAS_STEER_LKA           2   // HW4 FSD14+

#define DAS_CTRL_INTERVAL_MS    40   // 25 Hz
#define DAS_STEER_INTERVAL_MS   20   // 50 Hz
#define DAS_EAC_INTERVAL_MS     100  // 10 Hz
#define DAS_CANCEL_FRAMES       5    // cancel burst length

#define DAS_ACCEL_MAX_MS2       2.0f
#define DAS_ACCEL_MIN_MS2      -3.48f
#define DAS_SPEED_CAP_DEFAULT   25.0f   // default safety cap on first boot (overridden by NVS)
#define DAS_SPEED_CAP_MIN_KPH   1.0f    // never below this
#define DAS_SPEED_CAP_MAX_KPH   200.0f  // absolute compile-time ceiling (DAS_control byte limit)
#define DAS_SPEED_LIMIT_DEFAULT 25.0f   // default user speed limit (overridden by NVS)
#define DAS_JERK_MAX_MS3        4.9f
#define DAS_JERK_MIN_MS3       -4.9f
#define DAS_DEADMAN_MS          150
// openpilot Tesla CarControllerParams (opendbc/car/tesla/values.py):
//   MAX_ANGLE_RATE = 5°/20ms (steering frame interval). EPS faults at 12.
//   STEER_STEP = 2 → angle frame at 50 Hz (matches our DAS_STEER_INTERVAL_MS).
#define DAS_MAX_ANGLE_RATE_DEG  5.0f    // per 20 ms steering frame
#define DAS_MAX_ANGLE_DEG       360.0f  // EPAS hard fault above this
// Speed-aware angle clamp: openpilot uses VehicleModel + MAX_LATERAL_ACCEL.
// We approximate with constant lateral-accel cap a_y_max ≈ 3.0 m/s² and
// Tesla Model 3 wheelbase 2.875 m. max_angle_rad ≈ a_y_max * L / v² for
// small angles. Below DAS_LOW_SPEED_KPH the clamp is bypassed (full lock).
#define DAS_LOW_SPEED_KPH       8.0f
#define DAS_LAT_ACCEL_MAX_MS2   3.0f
#define DAS_WHEELBASE_M         2.875f
// Standstill brake-hold: if no longitudinal input and we're near stop,
// inject a small negative accel so the car doesn't roll. openpilot uses a
// similar "creep prevention" / brake-hold strategy on Tesla port.
#define DAS_STANDSTILL_KPH      1.5f
#define DAS_STANDSTILL_HOLD_MS2 -0.4f

// ── Static state ──────────────────────────────────────────────────────────────
static bool          dasDriveEnabled   = false;
static bool          dasActive         = false;   // has live control input
static float         dasSteerAngle     = 0.0f;    // degrees, signed
static float         dasAccelMin       = 0.0f;    // m/s²  (≤ 0, brake)
static float         dasAccelMax       = 0.0f;    // m/s²  (≥ 0, accel)
static float         dasSetSpeedKph    = DAS_SPEED_CAP_DEFAULT;
static float         dasSpeedCapKph    = DAS_SPEED_CAP_DEFAULT;   // runtime safety cap (NVS-backed)
static float         dasSpeedLimitKph  = DAS_SPEED_LIMIT_DEFAULT; // user-configured speed limit
static uint8_t       dasCounter3       = 0;       // DAS_controlCounter  [0..7]
static uint8_t       dasCounter4       = 0;       // steeringControlCounter [0..15]
static uint8_t       dasEacCounter     = 0;       // APS_eacMonitorCounter [0..15]
static unsigned long dasCtrlLastMs     = 0;
static unsigned long dasSteerLastMs    = 0;
static unsigned long dasEacLastMs      = 0;
static unsigned long dasLastUpdateMs   = 0;       // dead-man timer
static uint8_t       dasCancelCount    = 0;       // cancel frames left to send
static float         dasAppliedAngle   = 0.0f;    // last commanded steer angle (post rate-limit)
static Preferences   dasPrefs;

// ── AP Computer observation ────────────────────────────────────────────────────
// REMOVED. The previous implementation passively sniffed AP DAS frames and
// implemented YIELD/COPILOT/OVERRIDE co-existence modes. That feature has
// been deleted: this firmware is the sole authority on the chassis bus and
// the user drives the car with a gamepad — no AP computer participates.

// Force a 5-frame DAS cancel burst (tells the car to disengage AP cruise).
// Safe to call any time; only takes effect if dasDriveEnabled is true so we
// actually own the bus right now.
static void dasSendCancelBurst()
{
    dasActive      = false;
    dasCancelCount = DAS_CANCEL_FRAMES;
}

// ── Tesla CAN checksum ────────────────────────────────────────────────────────
// Matches teslacan.py: sum = (addr & 0xFF) + (addr >> 8), then add all bytes
// except the checksum byte, result & 0xFF.
static uint8_t dasChecksum(uint32_t canId, const uint8_t *data, uint8_t len, uint8_t checksumByte)
{
    uint8_t sum = (uint8_t)(canId & 0xFF) + (uint8_t)((canId >> 8) & 0xFF);
    for (uint8_t i = 0; i < len; i++)
        if (i != checksumByte)
            sum += data[i];
    return sum & 0xFF;
}

// ── Frame builders ────────────────────────────────────────────────────────────

// DAS_control (0x2B9, 8 bytes) — little-endian (Intel) bit order:
//   bits  0-11: DAS_setSpeed    (factor 0.1)
//   bits 12-15: DAS_accState    (4=ACC_ON, 13=CANCEL)
//   bits 16-17: DAS_aebEvent    (0)
//   bits 18-26: DAS_jerkMin     (factor 0.018, offset -9.1)
//   bits 27-34: DAS_jerkMax     (factor 0.034)
//   bits 35-43: DAS_accelMin    (factor 0.04, offset -15)
//   bits 44-52: DAS_accelMax    (factor 0.04, offset -15)
//   bits 53-55: DAS_controlCounter
//   bits 56-63: DAS_controlChecksum
static void buildDasControlFrame(uint8_t *d, float speed_kph, float accel_min,
                                  float accel_max, uint8_t counter, bool active)
{
    memset(d, 0, 8);

    // Hard clamp
    if (speed_kph < 0)               speed_kph = 0;
    if (speed_kph > dasSpeedCapKph) speed_kph = dasSpeedCapKph;
    if (accel_min < DAS_ACCEL_MIN_MS2) accel_min = DAS_ACCEL_MIN_MS2;
    if (accel_min > 0.0f)              accel_min = 0.0f;
    if (accel_max < 0.0f)              accel_max = 0.0f;
    if (accel_max > DAS_ACCEL_MAX_MS2) accel_max = DAS_ACCEL_MAX_MS2;

    uint16_t setSpeedRaw = (uint16_t)(speed_kph / 0.1f + 0.5f);
    uint8_t  accState    = active ? DAS_ACC_ON : DAS_ACC_CANCEL;
    // Fixed jerk limits (safe operating range, below 5.0 m/s³ fault threshold)
    uint16_t jerkMinRaw  = (uint16_t)((DAS_JERK_MIN_MS3 + 9.1f) / 0.018f + 0.5f); // ≈233
    uint8_t  jerkMaxRaw  = (uint8_t) (DAS_JERK_MAX_MS3 / 0.034f + 0.5f);          // ≈144
    uint16_t accelMinRaw = (uint16_t)((accel_min + 15.0f) / 0.04f + 0.5f);
    uint16_t accelMaxRaw = (uint16_t)((accel_max + 15.0f) / 0.04f + 0.5f);

    // Pack LE fields
    d[0]  = (uint8_t)(setSpeedRaw & 0xFF);              // bits  0-7
    d[1] |= (uint8_t)((setSpeedRaw >> 8) & 0x0F);       // bits  8-11
    d[1] |= (uint8_t)((accState & 0x0F) << 4);          // bits 12-15
    // bits 16-17: aebEvent = 0 (already zero)
    d[2] |= (uint8_t)((jerkMinRaw & 0x3F) << 2);        // bits 18-23
    d[3] |= (uint8_t)((jerkMinRaw >> 6) & 0x07);        // bits 24-26
    d[3] |= (uint8_t)((jerkMaxRaw & 0x1F) << 3);        // bits 27-31
    d[4] |= (uint8_t)((jerkMaxRaw >> 5) & 0x07);        // bits 32-34
    d[4] |= (uint8_t)((accelMinRaw & 0x1F) << 3);       // bits 35-39
    d[5] |= (uint8_t)((accelMinRaw >> 5) & 0x0F);       // bits 40-43
    d[5] |= (uint8_t)((accelMaxRaw & 0x0F) << 4);       // bits 44-47
    d[6] |= (uint8_t)((accelMaxRaw >> 4) & 0x1F);       // bits 48-52
    d[6] |= (uint8_t)((counter & 0x07) << 5);           // bits 53-55
    d[7]  = dasChecksum(CAN_ID_DAS_CONTROL, d, 8, 7);   // bits 56-63
}

// DAS_steeringControl (0x488, 4 bytes) — mixed endianness:
//   DAS_steeringAngleRequest    : bit  6, len 15, @0+ (Motorola/BE, factor 0.1, offset -1638.35)
//   DAS_steeringHapticRequest   : bit  7, len  1, @0+ (Motorola/BE) — bit 7 of byte 0
//   DAS_steeringControlCounter  : bit 16, len  4, @1+ (Intel/LE)    — byte 2 bits[3:0]
//   DAS_steeringControlType     : bit 23, len  2, @0+ (Motorola/BE) — byte 2 bits[7:6]
//   DAS_steeringControlChecksum : bit 24, len  8, @1+ (Intel/LE)    — byte 3
//
// Motorola start bit 6, length 15:
//   byte[0] bits[6:0] = raw[14:8]  (7 MSBs of 15-bit value)
//   byte[1]           = raw[7:0]   (8 LSBs)
static void buildDasSteeringFrame(uint8_t *d, float angle_deg, bool enabled,
                                   uint8_t counter, bool hw4)
{
    memset(d, 0, 4);

    if (angle_deg > 360.0f)  angle_deg = 360.0f;
    if (angle_deg < -360.0f) angle_deg = -360.0f;

    // openpilot negates the requested angle (sign convention difference)
    float req_angle = -angle_deg;
    float raw_f = (req_angle + 1638.35f) / 0.1f;
    if (raw_f < 0)     raw_f = 0;
    if (raw_f > 32767) raw_f = 32767;
    uint16_t raw = (uint16_t)(raw_f + 0.5f);

    // byte[0]: haptic(b7)=0, angle[14:8] in bits[6:0]
    d[0] = (uint8_t)((raw >> 8) & 0x7F);
    // byte[1]: angle[7:0]
    d[1] = (uint8_t)(raw & 0xFF);
    // byte[2]: steeringControlType[7:6] | counter[3:0]
    uint8_t ctrl_type = 0;
    if (enabled)
        ctrl_type = hw4 ? DAS_STEER_LKA : DAS_STEER_ANGLE_CTRL;
    d[2] = (uint8_t)((ctrl_type << 6) | (counter & 0x0F));
    // byte[3]: checksum
    d[3] = dasChecksum(CAN_ID_DAS_STEERING_CTRL, d, 4, 3);
}

// APS_eacMonitor (0x27D, 3 bytes) — all little-endian:
//   bits  0-1: APS_eacAllow       (1 = ALLOW EPAS to accept steer commands)
//   bits  8-11: APS_eacMonitorCounter
//   bits 16-23: APS_eacMonitorChecksum
static void buildApsEacFrame(uint8_t *d, uint8_t counter)
{
    memset(d, 0, 3);
    d[0] = 0x01;            // APS_eacAllow = 1 (ALLOW)
    d[1] = counter & 0x0F; // APS_eacMonitorCounter
    d[2] = dasChecksum(CAN_ID_APS_EAC_MONITOR, d, 3, 2);
}

// ── NVS ───────────────────────────────────────────────────────────────────────
static void dasLoadNvs()
{
    dasPrefs.begin("tcm_das", true);
    dasDriveEnabled  = dasPrefs.getBool("en", false);
    // Runtime safety cap (was DAS_SPEED_CAP_KPH compile-time constant; now
    // changeable from API/serial/dashboard so users with a closed track or
    // private property can raise it. Hard ceiling is DAS_SPEED_CAP_MAX_KPH.)
    uint16_t cap     = dasPrefs.getUShort("cap", (uint16_t)DAS_SPEED_CAP_DEFAULT);
    if (cap < (uint16_t)DAS_SPEED_CAP_MIN_KPH) cap = (uint16_t)DAS_SPEED_CAP_MIN_KPH;
    if (cap > (uint16_t)DAS_SPEED_CAP_MAX_KPH) cap = (uint16_t)DAS_SPEED_CAP_MAX_KPH;
    dasSpeedCapKph   = (float)cap;
    uint8_t spd      = dasPrefs.getUChar("spd", (uint8_t)DAS_SPEED_LIMIT_DEFAULT);
    if (spd > (uint8_t)dasSpeedCapKph) spd = (uint8_t)dasSpeedCapKph;
    dasSpeedLimitKph = (float)spd;
    dasPrefs.end();
}

static void dasSaveNvs()
{
    dasPrefs.begin("tcm_das", false);
    dasPrefs.putBool("en", dasDriveEnabled);
    dasPrefs.putUShort("cap", (uint16_t)dasSpeedCapKph);
    dasPrefs.putUChar("spd", (uint8_t)dasSpeedLimitKph);
    dasPrefs.end();
}

// ── Public API ────────────────────────────────────────────────────────────────

static void dasInit()
{
    dasLoadNvs();
}

// Enable/disable drive mode — persisted to NVS.
// Disabling immediately queues 5 cancel frames.
static void dasDriveSetEnabled(bool en)
{
    dasDriveEnabled = en;
    if (!en)
    {
        dasActive       = false;
        dasCancelCount  = DAS_CANCEL_FRAMES;
        dasAppliedAngle = 0.0f;
    }
    dasSaveNvs();
}

static bool dasDriveIsEnabled() { return dasDriveEnabled; }
static bool dasDriveIsActive()  { return dasActive; }

// Update live control values. Call every ~20 ms (gamepad axis tick).
// Dead-man: if not called for DAS_DEADMAN_MS, auto-cancel.
static void dasSetControl(float steer_deg, float accel_min_ms2, float accel_max_ms2,
                           float speed_kph, unsigned long now)
{
    if (!dasDriveEnabled) return;
    dasSteerAngle  = steer_deg;
    dasAccelMin    = accel_min_ms2;
    dasAccelMax    = accel_max_ms2;
    dasSetSpeedKph = speed_kph;
    dasLastUpdateMs = now;
    dasActive       = true;
}

// ── Safety shaping (openpilot Tesla CarController port) ──────────────────────
// Speed-aware steering angle clamp. Below DAS_LOW_SPEED_KPH the full ±360°
// is permitted (parking-lot maneuvers). Above that we cap to whichever
// produces lateral accel ≤ DAS_LAT_ACCEL_MAX_MS2 at the given speed using
// the bicycle-model small-angle approximation max_angle = a_y_max·L / v².
static float dasMaxSteerAtSpeed(float v_kph)
{
    if (v_kph < DAS_LOW_SPEED_KPH) return DAS_MAX_ANGLE_DEG;
    float v_ms  = v_kph / 3.6f;
    float lim_rad = (DAS_LAT_ACCEL_MAX_MS2 * DAS_WHEELBASE_M) / (v_ms * v_ms);
    float lim_deg = lim_rad * 57.29578f;
    if (lim_deg > DAS_MAX_ANGLE_DEG) lim_deg = DAS_MAX_ANGLE_DEG;
    if (lim_deg < 1.0f)              lim_deg = 1.0f;
    return lim_deg;
}

// Per-frame angle rate limit (openpilot MAX_ANGLE_RATE = 5°/20ms frame).
// `target` is the externally-requested angle, `last` is what we sent last
// frame (dasAppliedAngle). Returns the new commanded angle.
static float dasRateLimitAngle(float target, float last)
{
    float delta = target - last;
    if (delta >  DAS_MAX_ANGLE_RATE_DEG) delta =  DAS_MAX_ANGLE_RATE_DEG;
    if (delta < -DAS_MAX_ANGLE_RATE_DEG) delta = -DAS_MAX_ANGLE_RATE_DEG;
    return last + delta;
}

// Standstill brake-hold: if we are commanded "no input" near zero speed,
// inject a small negative accel so the car does not creep/roll. Mirrors
// openpilot's brake-hold behaviour for Tesla long control.
static void dasApplyStandstillHold(float v_kph, float &accel_min, float &accel_max)
{
    bool noInput = (accel_max < 0.05f) && (accel_min > -0.05f);
    if (noInput && fabsf(v_kph) < DAS_STANDSTILL_KPH)
    {
        if (accel_min > DAS_STANDSTILL_HOLD_MS2)
            accel_min = DAS_STANDSTILL_HOLD_MS2;
    }
}

// Rate-limited CAN frame sender. Call every loop iteration.
static void dasTick(unsigned long now, const State &s)
{
    if (s.txPaused) return;
    if (!dasDriveEnabled && dasCancelCount == 0) return;

    // Dead-man: auto-cancel if no update received recently
    if (dasActive && (now - dasLastUpdateMs) > DAS_DEADMAN_MS)
    {
        dasActive      = false;
        dasCancelCount = DAS_CANCEL_FRAMES;
    }

    // AP co-existence has been removed — we are the sole authority on the
    // chassis bus when dasDriveEnabled is true. No per-frame AP gating.

    const bool active = dasActive;
    const bool hw4    = (s.variant == HW4);

    // APS_eacMonitor — 10 Hz
    if (now - dasEacLastMs >= DAS_EAC_INTERVAL_MS)
    {
        dasEacLastMs = now;
        if (dasDriveEnabled && (active || dasCancelCount > 0))
        {
            uint8_t d[3];
            buildApsEacFrame(d, dasEacCounter);
            Frame f;
            f.id  = CAN_ID_APS_EAC_MONITOR;
            f.dlc = 3;
            memcpy(f.data, d, 3);
            driverSend(f, BUS_CHASSIS);
            dasEacCounter = (dasEacCounter + 1) & 0x0F;
        }
    }

    // DAS_steeringControl — 50 Hz
    if (now - dasSteerLastMs >= DAS_STEER_INTERVAL_MS)
    {
        dasSteerLastMs = now;
        if (dasDriveEnabled)
        {
            // openpilot Tesla port: speed-aware angle clamp + 5°/frame rate limit.
            float vKph    = fabsf(s.vehicleSpeed);
            float maxAng  = dasMaxSteerAtSpeed(vKph);
            float target  = dasSteerAngle;
            if (target >  maxAng) target =  maxAng;
            if (target < -maxAng) target = -maxAng;
            float commanded = active ? dasRateLimitAngle(target, dasAppliedAngle)
                                     : 0.0f;  // disengaged → relax to centre
            dasAppliedAngle = commanded;

            uint8_t d[4];
            buildDasSteeringFrame(d, commanded, active, dasCounter4, hw4);
            Frame f;
            f.id  = CAN_ID_DAS_STEERING_CTRL;
            f.dlc = 4;
            memcpy(f.data, d, 4);
            driverSend(f, BUS_CHASSIS);
            dasCounter4 = (dasCounter4 + 1) & 0x0F;
        }
    }

    // DAS_control — 25 Hz
    if (now - dasCtrlLastMs >= DAS_CTRL_INTERVAL_MS)
    {
        dasCtrlLastMs = now;
        if (dasDriveEnabled && (active || dasCancelCount > 0))
        {
            // Standstill brake-hold so car doesn't creep when the user lets
            // go of both triggers. Only applied while actively driving.
            float aMin = dasAccelMin, aMax = dasAccelMax;
            if (active) dasApplyStandstillHold(s.vehicleSpeed, aMin, aMax);

            uint8_t d[8];
            buildDasControlFrame(d, dasSetSpeedKph, aMin, aMax,
                                  dasCounter3, active);
            Frame f;
            f.id  = CAN_ID_DAS_CONTROL;
            f.dlc = 8;
            memcpy(f.data, d, 8);
            driverSend(f, BUS_CHASSIS);
            dasCounter3 = (dasCounter3 + 1) & 0x07;
            if (!active && dasCancelCount > 0)
                dasCancelCount--;
        }
    }
}

// ── Command Execution ─────────────────────────────────────────────────────────
// Handles: drive:on  drive:off
// Caller (dispatch.h) handles sendAck/sendLog/sendStatus.
static bool executeDasCmd(const char *cmd, State &)
{
    if (strncmp(cmd, "drive:", 6) != 0) return false;
    const char *sub = cmd + 6;

    if (strcmp(sub, "on") == 0)
    {
        dasDriveSetEnabled(true);
        return true;
    }
    if (strcmp(sub, "off") == 0)
    {
        dasDriveSetEnabled(false);
        return true;
    }
    if (strncmp(sub, "speed:", 6) == 0)
    {
        int v = atoi(sub + 6);
        if (v < 1) v = 1;
        if (v > (int)dasSpeedCapKph) v = (int)dasSpeedCapKph;
        dasSpeedLimitKph = (float)v;
        dasSaveNvs();
        return true;
    }
    if (strncmp(sub, "cap:", 4) == 0)
    {
        // Runtime safety cap. Bounded by DAS_SPEED_CAP_MIN/MAX_KPH so users
        // can't accidentally lock themselves out (min) or send a value that
        // overflows the DAS_control speed byte (max).
        int v = atoi(sub + 4);
        if (v < (int)DAS_SPEED_CAP_MIN_KPH) v = (int)DAS_SPEED_CAP_MIN_KPH;
        if (v > (int)DAS_SPEED_CAP_MAX_KPH) v = (int)DAS_SPEED_CAP_MAX_KPH;
        dasSpeedCapKph = (float)v;
        // If the user-facing limit is now above the new cap, pull it down.
        if (dasSpeedLimitKph > dasSpeedCapKph) dasSpeedLimitKph = dasSpeedCapKph;
        dasSaveNvs();
        return true;
    }
    return false;
}
