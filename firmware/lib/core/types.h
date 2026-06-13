#pragma once

/**
 * @file firmware/lib/core/types.h
 * @brief Core type definitions, enums, and the global State struct for the Tesla CAN Mod firmware
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// BUS_MAX mirrors the definition in core/can/bus.h so CanDiag.bus[] is sized
// correctly regardless of include order.
#ifndef BUS_MAX
#define BUS_MAX 3
#endif

enum TeslaModel : uint8_t;
enum HWGeneration : uint8_t;
enum FsdProtocol : uint8_t;
enum SwCompatLevel : uint8_t;

/**
 * @brief Raw CAN frame with arbitration ID, data length code, and payload
 */
struct Frame
{
	uint32_t id;	 // 11-bit or 29-bit CAN arbitration ID
	uint8_t dlc;	 // Data length code (0-8)
	uint8_t data[8]; // Payload bytes
};

/**
 * @brief Set or clear a single bit in a CAN frame using a flat bit index
 * @param f Target CAN frame
 * @param bit Flat bit index (0 = LSB of byte 0)
 * @param val True to set, false to clear
 */
inline void setBit(Frame &f, int bit, bool val)
{
	int byteIdx = bit / 8;
	int bitIdx = bit % 8;
	if (byteIdx >= f.dlc)
		return;
	uint8_t mask = 1 << bitIdx;
	if (val)
		f.data[byteIdx] |= mask;
	else
		f.data[byteIdx] &= ~mask;
}

/**
 * @brief Read the 3-bit mux/counter from byte 0 bits [2:0]
 * @param f Source CAN frame
 * @return Mux value (0-7), or 0 if frame has no data
 */
inline uint8_t readMuxID(const Frame &f)
{
	return f.dlc >= 1 ? (f.data[0] & 0x07) : 0; // mask lowest 3 bits
}

/**
 * @brief Autopilot hardware variant used for handler dispatch
 */
enum Variant
{
	HW4,   // HW4 / AI4 autopilot computer
	HW3,   // HW3 autopilot computer
	LEGACY // Pre-HW3 (AP1/AP2)
};

/**
 * @brief Return a human-readable name for a Variant enum value
 * @param v Variant to name
 * @return Null-terminated string ("hw4", "hw3", or "legacy")
 */
inline const char *variantName(Variant v)
{
	switch (v)
	{
	case HW4:
		return "hw4";
	case HW3:
		return "hw3";
	case LEGACY:
		return "legacy";
	default:
		return "hw4";
	}
}

/**
 * @brief Parse a string into a Variant enum value
 * @param name Input string (e.g. "hw4", "hw3", "legacy")
 * @param out Output variant on success
 * @return True if parsing succeeded
 */
inline bool parseVariant(const char *name, Variant &out)
{
	if (strcmp(name, "hw4") == 0)
	{
		out = HW4;
		return true;
	}
	if (strcmp(name, "hw3") == 0)
	{
		out = HW3;
		return true;
	}
	if (strcmp(name, "legacy") == 0)
	{
		out = LEGACY;
		return true;
	}
	return false;
}

/**
 * @brief Feature availability flags determined by hardware variant
 */
struct Features
{
	bool fsd;
	bool fsdForce;
	bool offset;
	bool profile;
	bool nag;
	bool isaChime;
	bool summon;
};

/**
 * @brief Determine which features are available for a given hardware variant
 * @param v Hardware variant
 * @return Features struct with per-feature availability flags
 */
inline Features getFeatures(Variant v)
{
	Features f = {true, true, true, true, true, true, true};
	switch (v)
	{
	case HW4:
		break;
	case HW3:
		f.isaChime = false; // ISA chime suppression only available on HW4
		break;
	case LEGACY:
		f.offset = false;	// No speed offset support on Legacy
		f.isaChime = false; // ISA chime suppression only available on HW4
		f.summon = false;	// Legacy lacks UI_VEHICLE_CTRL for summon
		break;
	}
	return f;
}

#define CAN_TIMEOUT_MS 10000	  // 10 s without frames triggers standby
#define CAN_REINIT_INTERVAL 5000  // Re-try MCP2515 init every 5 s in standby
#define LED_STANDBY_INTERVAL 2000 // Slow LED blink period in standby (ms)

/**
 * @brief Direction for summon injection on 0x273 UI_vehicleControl
 */
enum SummonDirection
{
	SUMMON_FORWARD = 0,
	SUMMON_REVERSE = 1
};

/**
 * @brief Summon activation state
 */
enum SummonMode
{
	SUMMON_STOP = 0,
	SUMMON_START = 1
};

/**
 * @brief Nag alert suppression mode selecting which CAN mutations to perform
 *
 * Each mode controls bit-19 clearing on 0x3FD mux=1 (UI_autopilotControl)
 * and/or EPAS torque echo strategy on 0x370 (EPAS3P_sysStatus).
 */
enum NagMode
{
	NAG_MODE_OFF = 0,	  // No suppression
	NAG_MODE_BIT19 = 1,	  // Clear bit-19 on 0x3FD only
	NAG_MODE_LEGACY = 2,  // EPAS echo with fixed zero torque, always-on
	NAG_MODE_SAFE = 3,	  // EPAS echo only when DAS requests hands-on
	NAG_MODE_NATURAL = 4, // EPAS echo with Gaussian jitter 0.08-0.18 Nm
	NAG_MODE_ORGANIC = 5, // EPAS echo with DAS state machine + grip excursions
	NAG_MODE_FULL = 6	  // Bit-19 + organic EPAS echo combined
};

/**
 * @brief Return a human-readable name for a NagMode value
 * @param m Nag mode
 * @return Null-terminated mode name string
 */
inline const char *nagModeName(NagMode m)
{
	switch (m)
	{
	case NAG_MODE_OFF:
		return "off";
	case NAG_MODE_BIT19:
		return "bit19";
	case NAG_MODE_LEGACY:
		return "legacy";
	case NAG_MODE_SAFE:
		return "safe";
	case NAG_MODE_NATURAL:
		return "natural";
	case NAG_MODE_ORGANIC:
		return "organic";
	case NAG_MODE_FULL:
		return "full";
	default:
		return "off";
	}
}

/**
 * @brief Parse a string into a NagMode enum value
 * @param name Input string (e.g. "off", "bit19", "organic")
 * @param out Output NagMode on success
 * @return True if parsing succeeded
 */
inline bool parseNagMode(const char *name, NagMode &out)
{
	if (strcmp(name, "off") == 0)
	{
		out = NAG_MODE_OFF;
		return true;
	}
	if (strcmp(name, "bit19") == 0)
	{
		out = NAG_MODE_BIT19;
		return true;
	}
	if (strcmp(name, "legacy") == 0)
	{
		out = NAG_MODE_LEGACY;
		return true;
	}
	if (strcmp(name, "safe") == 0)
	{
		out = NAG_MODE_SAFE;
		return true;
	}
	if (strcmp(name, "natural") == 0)
	{
		out = NAG_MODE_NATURAL;
		return true;
	}
	if (strcmp(name, "organic") == 0)
	{
		out = NAG_MODE_ORGANIC;
		return true;
	}
	if (strcmp(name, "full") == 0)
	{
		out = NAG_MODE_FULL;
		return true;
	}
	return false;
}

/**
 * @brief Check if a nag mode emits 0x370 EPAS torque echo frames
 * @param m Nag mode to check
 * @return True for legacy, safe, natural, organic, and full modes
 */
inline bool nagModeUsesEpasEcho(NagMode m)
{
	return m == NAG_MODE_LEGACY || m == NAG_MODE_SAFE || m == NAG_MODE_NATURAL || m == NAG_MODE_ORGANIC ||
		   m == NAG_MODE_FULL;
}

/**
 * @brief Check if a nag mode modifies UI_autopilotControl mux=1 bit-19
 * @param m Nag mode to check
 * @return True for bit19 and full modes
 */
inline bool nagModeUsesBit19(NagMode m)
{
	return m == NAG_MODE_BIT19 || m == NAG_MODE_FULL;
}

/**
 * @brief Check if a nag mode is actively suppressing alerts
 * @param m Nag mode to check
 * @return True when mode is not off
 */
inline bool nagModeActive(NagMode m)
{
	return m != NAG_MODE_OFF;
}

/**
 * @brief Check if the echo side of a nag mode requires DAS gating
 * @param m Nag mode to check
 * @return True for safe, natural, organic, and full (legacy is always-on)
 */
inline bool nagModeNeedsDasGate(NagMode m)
{
	return m == NAG_MODE_SAFE || m == NAG_MODE_NATURAL || m == NAG_MODE_ORGANIC || m == NAG_MODE_FULL;
}

/**
 * @brief Per-bus CAN frame statistics with rolling 1-second rate window
 */
struct CanBusStat
{
	uint32_t frames;		// Total frames received (cumulative)
	uint16_t hz;			// Current frame rate x10 (e.g. 456 = 45.6 Hz)
	uint16_t hzMin;			// Minimum Hz x10 observed (0xFFFF until first window)
	uint16_t hzMax;			// Maximum Hz x10 observed
	uint16_t windowCount;	// Frames counted in current 1-second window
	uint32_t windowStartMs; // millis() when current window opened
	CanBusStat() : frames(0), hz(0), hzMin(0xFFFF), hzMax(0), windowCount(0), windowStartMs(0) {}
};

/**
 * @brief Aggregated CAN-layer diagnostic counters across all buses and handlers
 */
struct CanDiag
{
	CanBusStat bus[BUS_MAX]; // Per-bus stats: 0=Chassis, 1=Vehicle, 2=Body
	uint32_t nagEchoCount;	 // Nag-killer echoes sent on 0x370 (all modes)
	uint32_t eapModCount;	 // EAP frames modified for nag-suppress (mux=1)
	uint32_t txFailCount;	 // MCP2515 sendMessage() errors (accumulated)
	uint32_t busOffCount;	 // CAN bus-off events auto-recovered
	CanDiag() : nagEchoCount(0), eapModCount(0), txFailCount(0), busOffCount(0) {}
};

/**
 * @brief Global firmware state holding all runtime configuration, telemetry, and feature flags
 */
struct State
{
	Variant variant;
	bool fsdEnabled;
	bool fsdForceEnabled; // Apply FSD edits even when UI FSD bit is not set
	int speedProfile;
	bool profileOverride; // True = user-pinned, false = track CAN stalk
	int speedOffset;	  // Unified: 0-63 on HW4, 0-100 on HW3
	bool offsetOverride;  // True = user-pinned, false = track CAN (HW3 UI)
	bool isaChimeSuppress;
	bool summonInject; // True = summon injection allowed
	bool streamEnabled;
	unsigned long streamCount;
	bool rawCanListen;

	// CAN bus health tracking
	unsigned long lastFrameMs;	// millis() of last received CAN frame
	bool chassisOnline;			// True when CAN frames are flowing
	bool standby;				// True when in standby (no CAN traffic)
	unsigned long lastReinitMs; // Last MCP2515 reinit attempt timestamp

	// Summon burst state (0x273 UI_vehicleControl injection)
	uint8_t summonRemaining;
	unsigned long summonLastMs;
	uint8_t lastCtrl[8];
	bool hasCtrl;
	SummonDirection summonDirection;
	SummonMode summonMode;

	// CAN frame caching for advanced features
	uint8_t lastClimate[5];
	bool hasClimate;
	uint8_t lastCharge[5];
	bool hasCharge;
	uint8_t lastDrive[8];
	bool hasDrive;

	// BMS battery telemetry (read-only, decoded from CAN)
	float bmsVoltage;  // Pack voltage (V)
	float bmsCurrent;  // Pack current (A, negative = discharging)
	float bmsPower;	   // Pack power (kW)
	float bmsSoc;	   // State of charge (%)
	int8_t bmsTempMin; // Min cell temp (deg C)
	int8_t bmsTempMax; // Max cell temp (deg C)
	float bmsWhPerKm;  // Energy consumption (Wh/km)
	bool hasBms;	   // At least one BMS frame received

	// Enhanced BMS telemetry (Vehicle CAN)
	float bmsNominalFullPack;	// Current full capacity (kWh) from 0x352
	float bmsNominalRemaining;	// Energy remaining (kWh)
	float bmsIdealRemaining;	// Ideal energy remaining (kWh)
	float bmsCellVoltageMax;	// Max cell voltage (V) from 0x332
	float bmsCellVoltageMin;	// Min cell voltage (V)
	float bmsMaxRegenPower;		// Max regen power (kW) from 0x252
	float bmsMaxDischargePower; // Max discharge power (kW)
	bool hasEnhancedBms;		// Enhanced BMS frames received

	// Expanded SoC (0x292)
	float bmsSocUI;			  // SoC shown to user
	float bmsSocMax;		  // Maximum SoC
	float bmsSocAvg;		  // Average SoC
	float bmsInitialFullPack; // Factory capacity (kWh)

	// Expanded energy/range (0x33A)
	float bmsExpectedRange;	   // Expected range (km)
	float bmsIdealRange;	   // Ideal range (km)
	float bmsRatedConsumption; // Rated Wh/km
	uint8_t bmsActualSocInt;   // Displayed SoC integer
	uint8_t bmsUsableSocInt;   // Usable SoC integer

	// Expanded thermal (0x312)
	float bmsPowerDissipation; // Thermal power (kW)
	float bmsFlowRequest;	   // Coolant flow (LPM)
	float bmsCoolTarget;	   // Active cooling target (deg C)
	float bmsPassiveTarget;	   // Passive target (deg C)
	float bmsHeatTarget;	   // Active heating target (deg C)
	float bmsPackTMin;		   // Pack min temp (deg C)
	float bmsPackTMax;		   // Pack max temp (deg C)

	// Expanded power (0x252)
	float bmsStationaryHeatPower; // Stationary heating budget (kW)
	float bmsHvacPowerBudget;	  // HVAC budget (kW)

	// BMS_status (0x212)
	bool bmsPrecondAllowed;	   // BMS allows preconditioning
	bool bmsHeatingWorthwhile; // Heating would help
	uint8_t bmsContactorState; // Contactor state (0-7)
	uint8_t bmsHvState;		   // HV bus state (0-7)

	// Drive limits (0x2D2)
	float bmsMinBusVoltage;		  // Min bus voltage (V)
	float bmsMaxBusVoltage;		  // Max bus voltage (V)
	float bmsMaxChargeCurrent;	  // Max charge current (A)
	float bmsMaxDischargeCurrent; // Max discharge current (A)

	// 0x352 mux=1 energy status
	float bmsExpectedRemaining; // Expected remaining (kWh)
	float bmsEnergyBuffer;		// Buffer energy (kWh)
	float bmsEnergyToCharge;	// Energy to charge complete (kWh)
	bool bmsFullyCharged;		// Fully charged flag

	// Lifetime counters (0x3D2)
	float bmsKwhDischargeTotal; // Total discharged (kWh)
	float bmsKwhChargeTotal;	// Total charged (kWh)

	// Multiplexed counters (0x3F2)
	float bmsAcChargeTotal;		  // AC charge total (kWh)
	float bmsDcChargeTotal;		  // DC charge total (kWh)
	float bmsRegenTotal;		  // Regen charge total (kWh)
	float bmsDriveDischargeTotal; // Drive discharge total (kWh)

	// 0x332 mux=0 thermistor temps
	float bmsThermistorTMax; // Thermistor max (deg C)
	float bmsThermistorTMin; // Thermistor min (deg C)
	float bmsModelTMax;		 // Modeled max (deg C)
	float bmsModelTMin;		 // Modeled min (deg C)

	// 0x132 expanded
	float bmsChargeTimeToFull; // Hours to full charge

	// Steering mode monitoring (read-only from 0x370 EPAS_sysStatus)
	uint8_t steeringMode; // 0=FAIL_SAFE, 1=COMFORT, 2=STANDARD, 3=SPORT
	bool hasSteeringMode; // At least one 0x370 frame decoded

	// Nag alert suppression state
	NagMode nagMode;			 // Persisted (NVS key "nagMode")
	bool nagOrganicDriverBypass; // Persisted: skip injection when real EPAS handsOnLevel != 0
	uint8_t dasHandsOnState;	 // DAS_autopilotHandsOnState (0x39B byte 5 bits[5:2])
	uint8_t dasApState;			 // DAS_autopilotState (0x39B byte 1 bits[7:4])
	bool dasSeen;				 // True once first 0x39B frame decoded

	// Natural-strategy interval state (runtime only)
	unsigned long naturalNagLastMs; // Last injection timestamp
	uint16_t naturalNagIntervalMs;	// Current non-linear interval (150-350 ms)

	// Organic-strategy runtime state (DAS state machine + grip excursions)
	uint8_t nagOrganicRealHandsOn; // Last observed incoming EPAS3P_handsOnLevel (0-3)
	uint8_t nagOrganicPrevState;   // Previous dasHandsOnState for transition detection

	// Organic state 1 (grace hold)
	unsigned long nagOrg1EnterMs;
	int16_t nagOrg1HoldRaw; // Raw torque to hold during 500 ms grace (center 2048)
	uint8_t nagOrg1HoldLevel;

	// Organic state 2 (mild random walk)
	unsigned long nagOrg2EnterMs;
	int16_t nagOrg2WalkRaw;			  // Persistent random-walk value (center 2048)
	unsigned long nagOrg2HoldUntilMs; // Level-2 latch expiry
	int16_t nagOrg2HoldRaw;
	uint8_t nagOrg2HoldLevel;
	bool nagOrg2Level2Active;

	// Organic states 3-5 (strong ramp-and-hold)
	unsigned long nagOrgStrongEnterMs;

	// Grip excursion (anti-detection pulses)
	uint16_t nagOrgFramesUntilExc; // Frames until next grip pulse (125-225)
	uint8_t nagOrgExcFrames;	   // Frames remaining in current pulse (0-5)

	// Last generated organic output
	int16_t nagOrgLastRaw; // Center 2048
	uint8_t nagOrgLastLevel;

	// Auto Lane Change (ALC) — persisted
	bool alcAutoConfirmEnabled;		// Enable automatic lane change confirmation
	uint8_t dasLaneChangeState;		// DAS_laneChangeState from 0x39B (5-bit, 0-31)
	unsigned long alcLastConfirmMs; // Last ALC confirmation injection time

	// Safety cues (read-only from CAN)
	bool turnSignalLeft;   // Turn indicator active (from 0x3F5)
	bool turnSignalRight;  // Turn indicator active (from 0x3F5)
	uint8_t bsmLeftLevel;  // 0=none, 1=warning1, 2=warning2 (from 0x399)
	uint8_t bsmRightLevel; // 0=none, 1=warning1, 2=warning2 (from 0x399)

	// Open-state cues (read-only from CAN)
	bool doorFrontLeftOpen;
	bool doorFrontRightOpen;
	bool doorRearLeftOpen;
	bool doorRearRightOpen;
	bool driverDoorOpen;
	bool anyDoorOpen;
	bool frunkOpen;
	bool trunkOpen;

	// Cruise/speed-limit context (read-only from CAN)
	float cruiseSetSpeedKph;
	float accSpeedLimitKph;
	float mapSpeedLimitKph;
	float maxSpeedKph;

	// AP Injection Gate
	bool apInjectionGateEnabled; // True = gate writes until AP/Park/Summon conditions met
	bool apGateApActive;		 // Runtime AP-active signal
	bool apGateParked;			 // Runtime parked signal
	bool apGateSummoning;		 // Runtime summon signal

	// Auto HW detection (from 0x398)
	uint8_t detectedHW; // 0=unknown, 2=HW3, 3=HW4
	bool hwAutoDetected;
	bool variantAutoDetect; // True = auto-switch variant on CAN detection

	// MCP2515 CAN crystal profile (runtime)
	uint8_t canClockReqMHz; // Requested profile: 0=auto, otherwise 8/12/16/20
	uint8_t canClockMHz;	// Active profile after fallback: 8/16/20

	// GTW autopilot tier readback (from 0x7FF mux=2)
	int8_t gtwAutopilotTier; // -1=unknown, 0=NONE, 1=HIGHWAY, 2=ENHANCED, 3=SELF_DRIVING, 4=BASIC
	bool gtwAutopilotSeen;

	// Preconditioning — persisted
	bool preconditionEnabled;
	unsigned long precondLastMs;

	// Track mode — persisted
	bool trackModeEnabled;

	// Burst send queue (non-blocking one-shot CAN pattern)
	Frame burstFrame;
	uint8_t burstBus;
	uint8_t burstRemaining;
	uint8_t burstDelayMs;
	unsigned long burstLastMs;

	// Bus on which last 0x273 UI_VEHICLE_CTRL frame was received
	uint8_t ctrlBus;

	// Ban Shield — experimental telemetry monitoring
	bool banShieldEnabled;		// True = monitor for ban threat patterns
	uint8_t banThreatLevel;		// 0=none, 1-5=escalating threat levels
	uint16_t banDetectionCount; // Cumulative ban threat events
	unsigned long banThreatMs;	// Timestamp of last threat detected

	// GTW Shield — 0x7FF snapshot defense (captures healthy GTW_carConfig
	// mux variants and retransmits them when gateway pushes modified frames)
	uint8_t gtwSnapshot[8][8]; // [mux][byte0..7] — 64 bytes total
	bool gtwSnapshotValid[8];  // Per-mux: has this mux been captured?
	bool gtwShieldArmed;	   // True = actively blocking any change
	uint32_t gtwShieldBlocks;  // Counter: frames blocked since arm

	// Enhanced Autopilot — unlocks EAP/Summon by setting bit46 on mux=1
	bool enhancedAutopilot; // Persisted

	// Emergency Vehicle Detection — HW4 only, sets bit59 on mux=0
	bool evdEnabled; // Persisted

	// TLSSC Restore — spoof DAS_autopilot tier to SELF_DRIVING on 0x331
	bool tlsscRestore; // Persisted

	// TPMS — Tire Pressure Monitoring (read-only from 0x219)
	float tpmsPressure[4]; // FL, FR, RL, RR (bar)
	int8_t tpmsTemp[4];	   // FL, FR, RL, RR (deg C)
	bool hasTpms;		   // At least one TPMS frame decoded

	// Drive mode override ("Ghost Mode") — persisted
	uint8_t driveModeOverride; // 0=none, 1=chill, 2=standard, 3=performance
	uint8_t currentDriveMode;  // Current readback from DI_steer (0-3)
	unsigned long driveModeLastMs;

	// Region detection (from 0x398)
	uint8_t regionCode;		 // 0=unknown, 1=NA, 2=EU, 3=CN, 4=APAC, 5=ME
	uint8_t regionSpoofCode; // 0=off (use real), 1-5=spoof to that region
	bool hasRegion;
	bool chineseGatewayLocked; // True = CN market, FSD blocked at GTW level

	// ECE R79 bypass — persisted
	bool eceR79Bypass; // True = clear ECE R79 restriction bit in EU

	// Left-Hand Drive mode — persisted
	bool lhdEnabled; // True = clear UI_drivingSide bit on 0x3F8

	// AP-First mode (2026.14.x compatibility) — persisted
	bool apFirstEnabled; // True = delay 0x3FD injection until AP/TACC is active

	// Driver assist parity toggles — persisted
	bool assistNavEnable;	 // Bits 13+48+49 on 0x3F8
	bool assistHandsOff;	 // Bit 14 on 0x3F8
	bool assistDevMode;		 // Bit 5 on 0x3F8
	bool laneGraphEnable;	 // Bit 45 on 0x3FD mux1
	bool assistTelemetryOff; // Bit 43 cleared on 0x3F8

	// Steering input for natural nag-killer modulation
	float steeringAngle; // Degrees, positive = right (from 0x129)

	// CAN Diagnostics (runtime, not persisted)
	CanDiag canDiag; // Per-bus stats + all intercept/error counters

	// Rear seatbelt buckle emulation
	bool seatbeltEmulation; // True = suppress rear seatbelt warnings
	unsigned long seatbeltLastMs;

	// Wiper speed persistence
	bool wiperPersistEnabled; // True = persist wiper speed across restarts
	uint8_t savedWiperSpeed;  // Last wiper speed set (0-3)

	// Mirror auto-fold on lock
	bool mirrorAutoFoldEnabled; // True = auto fold on lock, unfold on unlock
	bool vehicleLockedState;	// Current known lock state

	// Powertrain telemetry (read-only decode)
	float vehicleSpeed;		 // km/h (signed, from 0x257)
	uint8_t gearState;		 // 0=inv, 1=P, 2=R, 3=N, 4=D (from 0x118)
	uint8_t accelPedal;		 // 0-100% (from 0x118)
	uint8_t brakePedalState; // 0=off, 1=on (bits[20:19] of 0x118 DI_STATE)
	int16_t rearMotorRpm;	 // RPM (from 0x106)
	int16_t frontMotorRpm;	 // RPM (from 0x115)
	bool hasPowertrain;		 // At least one powertrain frame decoded

	// Wheel speeds (from 0x175, ChassisBus — 13-bit LE, scale 0.04 km/h)
	float wheelSpeedFL; // km/h front-left
	float wheelSpeedFR; // km/h front-right
	float wheelSpeedRL; // km/h rear-left
	float wheelSpeedRR; // km/h rear-right
	bool hasWheelSpeeds;

	// Motor / inverter temperatures
	int8_t rearInvTemp;		  // deg C rear inverter (0x315 byte1 - 40)
	int8_t rearStatorTemp;	  // deg C rear stator (0x315 byte2 - 40)
	int8_t rearHeatsinkTemp;  // deg C rear heatsink (0x315 byte4 - 40)
	int8_t frontInvTemp;	  // deg C front inverter (0x376 byte1 - 40, dual-motor)
	int8_t frontStatorTemp;	  // deg C front stator (0x376 byte2 - 40)
	int8_t frontHeatsinkTemp; // deg C front heatsink (0x376 byte4 - 40)
	bool hasMotorTemps;

	// CAN simulation mode
	bool canSimEnabled; // True = generating synthetic CAN frames
	unsigned long canSimLastMs;
	uint16_t canSimCounter;

	// Single-shot TX mode
	bool singleShotTx; // True = use one-shot TX, no retries on failure

	// Firmware version compatibility
	uint16_t fwYear;   // Vehicle firmware year (e.g. 2026)
	uint8_t fwRelease; // Release number (e.g. 2, 8)
	uint8_t fwMinor;   // Minor version
	uint32_t fwBuild;  // Build number
	uint8_t fwCompat;  // FwCompatLevel enum
	bool hasFwVersion; // At least one 0x392 frame decoded

	// MQTT telemetry bridge
	bool mqttEnabled;	   // True = publish telemetry to MQTT
	char mqttHost[64];	   // Broker hostname
	uint16_t mqttPort;	   // Broker port (default 1883)
	uint16_t mqttInterval; // Publish interval ms (default 2000)
	unsigned long mqttLastPublishMs;
	bool mqttConnected; // True = currently connected to broker

	// Vehicle-specific config (from 0x398)
	uint8_t vehicleModel;  // VehicleModel enum
	uint16_t vehicleYear;  // Detected model year
	bool hasVehicleConfig; // At least one config frame decoded

	// Vehicle platform identity (Model -> HW -> SW)
	uint8_t platformModel;	   // TeslaModel enum
	uint8_t platformHwGen;	   // HWGeneration enum
	uint16_t platformSwYear;   // Software year (YYYY)
	uint8_t platformSwWeek;	   // Software week (WW)
	uint8_t platformSwRelease; // Software release
	uint8_t platformSwPatch;   // Software patch
	uint8_t platformFsdProto;  // FsdProtocol enum
	uint8_t platformSwCompat;  // SwCompatLevel enum
	bool platformResolved;	   // True = model + hw known

	// WiFi API authentication (NVS-persisted)
	char apiKey[33];	 // 32-char hex key + NUL, generated on first boot
	bool apiKeyRequired; // True = require X-API-Key header on mutable endpoints

	State()
		: variant(HW4), fsdEnabled(false), fsdForceEnabled(false), speedProfile(1), profileOverride(false),
		  speedOffset(0), offsetOverride(false), isaChimeSuppress(false), summonInject(false), streamEnabled(false),
		  streamCount(0), rawCanListen(false), lastFrameMs(0), chassisOnline(false), standby(false), lastReinitMs(0),
		  summonRemaining(0), summonLastMs(0), hasCtrl(false), summonDirection(SUMMON_FORWARD), summonMode(SUMMON_STOP),
		  hasClimate(false), hasCharge(false), hasDrive(false), bmsVoltage(0), bmsCurrent(0), bmsPower(0), bmsSoc(0),
		  bmsTempMin(0), bmsTempMax(0), bmsWhPerKm(0), hasBms(false), bmsNominalFullPack(0), bmsNominalRemaining(0),
		  bmsIdealRemaining(0), bmsCellVoltageMax(0), bmsCellVoltageMin(0), bmsMaxRegenPower(0),
		  bmsMaxDischargePower(0), hasEnhancedBms(false), bmsSocUI(0), bmsSocMax(0), bmsSocAvg(0),
		  bmsInitialFullPack(0), bmsExpectedRange(0), bmsIdealRange(0), bmsRatedConsumption(0), bmsActualSocInt(0),
		  bmsUsableSocInt(0), bmsPowerDissipation(0), bmsFlowRequest(0), bmsCoolTarget(0), bmsPassiveTarget(0),
		  bmsHeatTarget(0), bmsPackTMin(0), bmsPackTMax(0), bmsStationaryHeatPower(0), bmsHvacPowerBudget(0),
		  bmsPrecondAllowed(false), bmsHeatingWorthwhile(false), bmsContactorState(0), bmsHvState(0),
		  bmsMinBusVoltage(0), bmsMaxBusVoltage(0), bmsMaxChargeCurrent(0), bmsMaxDischargeCurrent(0),
		  bmsExpectedRemaining(0), bmsEnergyBuffer(0), bmsEnergyToCharge(0), bmsFullyCharged(false),
		  bmsKwhDischargeTotal(0), bmsKwhChargeTotal(0), bmsAcChargeTotal(0), bmsDcChargeTotal(0), bmsRegenTotal(0),
		  bmsDriveDischargeTotal(0), bmsThermistorTMax(0), bmsThermistorTMin(0), bmsModelTMax(0), bmsModelTMin(0),
		  bmsChargeTimeToFull(0), steeringMode(0), hasSteeringMode(false), nagMode(NAG_MODE_OFF),
		  nagOrganicDriverBypass(false), dasHandsOnState(0), dasApState(0), dasSeen(false), naturalNagLastMs(0),
		  naturalNagIntervalMs(200), nagOrganicRealHandsOn(0), nagOrganicPrevState(0xFF), nagOrg1EnterMs(0),
		  nagOrg1HoldRaw(2048), nagOrg1HoldLevel(0), nagOrg2EnterMs(0), nagOrg2WalkRaw(2048), nagOrg2HoldUntilMs(0),
		  nagOrg2HoldRaw(2048), nagOrg2HoldLevel(0), nagOrg2Level2Active(false), nagOrgStrongEnterMs(0),
		  nagOrgFramesUntilExc(175), nagOrgExcFrames(0), nagOrgLastRaw(2048), nagOrgLastLevel(0),
		  alcAutoConfirmEnabled(false), dasLaneChangeState(0), alcLastConfirmMs(0), turnSignalLeft(false),
		  turnSignalRight(false), bsmLeftLevel(0), bsmRightLevel(0), doorFrontLeftOpen(false),
		  doorFrontRightOpen(false), doorRearLeftOpen(false), doorRearRightOpen(false), driverDoorOpen(false),
		  anyDoorOpen(false), frunkOpen(false), trunkOpen(false), cruiseSetSpeedKph(0), accSpeedLimitKph(0),
		  mapSpeedLimitKph(0), maxSpeedKph(0), apInjectionGateEnabled(false), apGateApActive(false), apGateParked(true),
		  apGateSummoning(false), detectedHW(0), hwAutoDetected(false), variantAutoDetect(true),
		  canClockReqMHz(BOARD_CAN_CLOCK_MHZ), canClockMHz(BOARD_CAN_CLOCK_MHZ), gtwAutopilotTier(-1),
		  gtwAutopilotSeen(false), preconditionEnabled(false), precondLastMs(0), trackModeEnabled(false), burstBus(0),
		  burstRemaining(0), burstDelayMs(0), burstLastMs(0), ctrlBus(0), banShieldEnabled(false), banThreatLevel(0),
		  banDetectionCount(0), banThreatMs(0), gtwShieldArmed(false), gtwShieldBlocks(0), enhancedAutopilot(false),
		  evdEnabled(false), tlsscRestore(false), hasTpms(false), driveModeOverride(0), currentDriveMode(0),
		  driveModeLastMs(0), regionCode(0), regionSpoofCode(0), hasRegion(false), chineseGatewayLocked(false),
		  eceR79Bypass(false), lhdEnabled(false), apFirstEnabled(false), assistNavEnable(false), assistHandsOff(false),
		  assistDevMode(false), laneGraphEnable(false), assistTelemetryOff(false), seatbeltEmulation(false),
		  seatbeltLastMs(0), wiperPersistEnabled(false), savedWiperSpeed(0), mirrorAutoFoldEnabled(false),
		  vehicleLockedState(false), vehicleSpeed(0), gearState(0), accelPedal(0), brakePedalState(0), rearMotorRpm(0),
		  frontMotorRpm(0), hasPowertrain(false), wheelSpeedFL(0), wheelSpeedFR(0), wheelSpeedRL(0), wheelSpeedRR(0),
		  hasWheelSpeeds(false), rearInvTemp(0), rearStatorTemp(0), rearHeatsinkTemp(0), frontInvTemp(0),
		  frontStatorTemp(0), frontHeatsinkTemp(0), hasMotorTemps(false), canSimEnabled(false), canSimLastMs(0),
		  canSimCounter(0), singleShotTx(false), fwYear(0), fwRelease(0), fwMinor(0), fwBuild(0), fwCompat(0),
		  hasFwVersion(false), mqttEnabled(false), mqttPort(1883), mqttInterval(2000), mqttLastPublishMs(0),
		  mqttConnected(false), vehicleModel(0), vehicleYear(0), hasVehicleConfig(false), platformModel(0),
		  platformHwGen(0), platformSwYear(0), platformSwWeek(0), platformSwRelease(0), platformSwPatch(0),
		  platformFsdProto(0), platformSwCompat(0), platformResolved(false), apiKeyRequired(false), steeringAngle(0),
		  canDiag()
	{
		for (uint8_t i = 0; i < 8; i++)
			lastCtrl[i] = 0;
		for (uint8_t i = 0; i < 5; i++)
			lastClimate[i] = 0;
		for (uint8_t i = 0; i < 5; i++)
			lastCharge[i] = 0;
		for (uint8_t i = 0; i < 8; i++)
			lastDrive[i] = 0;
		burstFrame.id = 0;
		burstFrame.dlc = 0;
		for (uint8_t i = 0; i < 8; i++)
			burstFrame.data[i] = 0;
		for (uint8_t i = 0; i < 8; i++)
		{
			gtwSnapshotValid[i] = false;
			for (uint8_t j = 0; j < 8; j++)
				gtwSnapshot[i][j] = 0;
		}
		for (uint8_t i = 0; i < 4; i++)
		{
			tpmsPressure[i] = 0;
			tpmsTemp[i] = 0;
		}
		mqttHost[0] = '\0';
		apiKey[0] = '\0';
	}

	/**
	 * @brief Get the feature availability flags for the current variant
	 * @return Features struct based on the active hardware variant
	 */
	Features features() const
	{
		return getFeatures(variant);
	}

	/**
	 * @brief Check if the AP injection gate allows transmission
	 * @return True if gate is disabled or any gate condition is met
	 */
	bool apGateOpen() const
	{
		if (!apInjectionGateEnabled)
			return true;
		return apGateApActive || apGateParked || apGateSummoning;
	}
};
