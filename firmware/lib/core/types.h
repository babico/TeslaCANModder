#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Forward-declare platform types (full header included after State)
enum TeslaModel : uint8_t;
enum HWGeneration : uint8_t;
enum FsdProtocol : uint8_t;
enum SwCompatLevel : uint8_t;

// ── CAN Frame ────────────────────────────────────────────────────────────────
struct Frame
{
	uint32_t id;
	uint8_t dlc;
	uint8_t data[8];
};

// ── Variant ──────────────────────────────────────────────────────────────────
enum Variant
{
	HW4,
	HW3,
	LEGACY
};

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

// ── Features ─────────────────────────────────────────────────────────────────
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

inline Features getFeatures(Variant v)
{
	// Base: all features enabled. Disable variant-specific ones below.
	Features f = {true, true, true, true, true, true, true};
	switch (v)
	{
	case HW4:
		break;
	case HW3:
		f.isaChime = false; // ISA chime only on HW4
		break;
	case LEGACY:
		f.offset = false;	// No offset support on Legacy
		f.isaChime = false; // ISA chime only on HW4
		f.summon = false;	// Legacy can't summon (no UI_VEHICLE_CTRL)
		break;
	}
	return f;
}

// ── CAN Bus Health ───────────────────────────────────────────────────────────
#define CAN_TIMEOUT_MS 10000	  // 10s without frames → standby
#define CAN_REINIT_INTERVAL 5000  // Re-try MCP2515 init every 5s in standby
#define LED_STANDBY_INTERVAL 2000 // Slow LED blink period in standby

// ── Summon Enums ─────────────────────────────────────────────────────────────
enum SummonDirection
{
	SUMMON_FORWARD = 0,
	SUMMON_REVERSE = 1
};

enum SummonMode
{
	SUMMON_STOP = 0,
	SUMMON_START = 1
};

enum NagKillerMode
{
	NAG_KILLER_LEGACY = 0,
	NAG_KILLER_SAFE = 1,
	NAG_KILLER_NATURAL = 2
};

inline const char *nagKillerModeName(NagKillerMode m)
{
	switch (m)
	{
	case NAG_KILLER_LEGACY:
		return "legacy";
	case NAG_KILLER_SAFE:
		return "safe";
	case NAG_KILLER_NATURAL:
		return "natural";
	default:
		return "legacy";
	}
}

inline bool parseNagKillerMode(const char *name, NagKillerMode &out)
{
	if (strcmp(name, "legacy") == 0)
	{
		out = NAG_KILLER_LEGACY;
		return true;
	}
	if (strcmp(name, "safe") == 0)
	{
		out = NAG_KILLER_SAFE;
		return true;
	}
	if (strcmp(name, "natural") == 0)
	{
		out = NAG_KILLER_NATURAL;
		return true;
	}
	return false;
}

// ── CAN Diagnostics ─────────────────────────────────────────────────────────

// Per-bus CAN statistics. Fixed-size; no dynamic allocation. Safe across all
// ride lengths (uint32_t frames wraps only after ~596 hours at peak bus load).
struct CanBusStat
{
	uint32_t frames;		// total frames received (cumulative)
	uint16_t hz;			// current frame rate × 10  (e.g. 456 → 45.6 Hz)
	uint16_t hzMin;			// minimum Hz × 10 observed (0xFFFF until first window)
	uint16_t hzMax;			// maximum Hz × 10 observed
	uint16_t windowCount;	// frames counted in current 1-second window
	uint32_t windowStartMs; // millis() when current window opened
	CanBusStat() : frames(0), hz(0), hzMin(0xFFFF), hzMax(0), windowCount(0), windowStartMs(0) {}
};

// All CAN-layer diagnostic counters in one place — three per-bus stat blocks
// plus global intercept counters collected from every handler path.
struct CanDiag
{
	CanBusStat bus[3];	   // index: 0 = Chassis, 1 = Vehicle, 2 = Body
	uint32_t nagEchoCount; // nag-killer echoes sent (0x370, all modes)
	uint32_t eapModCount;  // EAP frames modified for nag-suppress (mux=1)
	uint32_t txFailCount;  // MCP2515 sendMessage() errors (accumulated)
	uint32_t busOffCount;  // CAN bus-off events auto-recovered
	CanDiag() : nagEchoCount(0), eapModCount(0), txFailCount(0), busOffCount(0) {}
};

// ── State ────────────────────────────────────────────────────────────────────
struct State
{
	Variant variant;
	bool fsdEnabled;
	bool fsdForceEnabled; // true = apply FSD edits even when UI FSD bit is not set
	bool nagSuppress;
	int speedProfile;
	bool profileOverride; // true = user-pinned, false = track CAN (stalk)
	int speedOffset;	  // unified: 0-63 on HW4, 0-100 on HW3
	bool offsetOverride;  // true = user-pinned, false = track CAN (HW3 UI)
	bool isaChimeSuppress;
	bool summonInject; // true = summon injection allowed
	bool streamEnabled;
	unsigned long streamCount;
	bool rawCanListen;

	// CAN bus health tracking
	unsigned long lastFrameMs;	// millis() of last received CAN frame
	bool chassisOnline;			// true when CAN frames are flowing
	bool standby;				// true when in standby (no CAN traffic)
	unsigned long lastReinitMs; // last MCP2515 reinit attempt

	// Summon burst state (0x273 UI_vehicleControl injection)
	uint8_t summonRemaining;
	unsigned long summonLastMs;
	uint8_t lastCtrl[8];
	bool hasCtrl;
	SummonDirection summonDirection;
	SummonMode summonMode;

	// Additional CAN frame caching for advanced features
	uint8_t lastClimate[5];
	bool hasClimate;
	uint8_t lastCharge[5];
	bool hasCharge;
	uint8_t lastDrive[8];
	bool hasDrive;

	// BMS battery telemetry (read-only, decoded from CAN)
	float bmsVoltage;  // Pack voltage (V)
	float bmsCurrent;  // Pack current (A, negative=discharging)
	float bmsPower;	   // Pack power (kW)
	float bmsSoc;	   // State of charge (%)
	int8_t bmsTempMin; // Min cell temp (°C)
	int8_t bmsTempMax; // Max cell temp (°C)
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
	float bmsCoolTarget;	   // Active cooling target (°C)
	float bmsPassiveTarget;	   // Passive target (°C)
	float bmsHeatTarget;	   // Active heating target (°C)
	float bmsPackTMin;		   // Pack min temp (°C)
	float bmsPackTMax;		   // Pack max temp (°C)

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
	float bmsThermistorTMax; // Thermistor max (°C)
	float bmsThermistorTMin; // Thermistor min (°C)
	float bmsModelTMax;		 // Modeled max (°C)
	float bmsModelTMin;		 // Modeled min (°C)

	// 0x132 expanded
	float bmsChargeTimeToFull; // Hours to full charge

	// Steering mode monitoring (read-only from 0x370 EPAS_sysStatus)
	uint8_t steeringMode; // 0=FAIL_SAFE, 1=COMFORT, 2=STANDARD, 3=SPORT
	bool hasSteeringMode; // At least one 0x370 frame decoded

	// Nag killer (EPAS torque spoofing) — persisted
	bool nagKillerEnabled;
	NagKillerMode nagKillerMode;
	uint8_t dasHandsOnState; // DAS_autopilotHandsOnState (0x39B byte5 bits[5:2])
	uint8_t dasApState;		 // DAS_autopilotState (0x39B byte1 bits[7:4]): 0=UNAVAIL 1=AVAIL 2=ACTIVE_NOMINAL 3+=active
	bool dasSeen;
	unsigned long naturalNagLastMs; // Last natural nag injection timestamp
	uint16_t naturalNagIntervalMs;	// Current non-linear interval between injections

	// Auto Lane Change (ALC) — persisted
	bool alcAutoConfirmEnabled;		// Enable automatic lane change confirmation
	uint8_t dasLaneChangeState;		// DAS_laneChangeState from 0x39B (5-bit, 0-31)
	unsigned long alcLastConfirmMs; // Last ALC confirmation injection time

	// D-05 safety cues (read-only from CAN)
	bool turnSignalLeft;   // Turn indicator active (from 0x3F5)
	bool turnSignalRight;  // Turn indicator active (from 0x3F5)
	uint8_t bsmLeftLevel;  // 0=none, 1=warning1, 2=warning2 (from 0x399)
	uint8_t bsmRightLevel; // 0=none, 1=warning1, 2=warning2 (from 0x399)

	// D-11 open-state cues (read-only from CAN)
	bool doorFrontLeftOpen;
	bool doorFrontRightOpen;
	bool doorRearLeftOpen;
	bool doorRearRightOpen;
	bool driverDoorOpen;
	bool anyDoorOpen;
	bool frunkOpen;
	bool trunkOpen;

	// D-13 cruise/speed-limit context (read-only from CAN)
	float cruiseSetSpeedKph;
	float accSpeedLimitKph;
	float mapSpeedLimitKph;
	float maxSpeedKph;

	// OTA safety check
	bool otaInProgress; // true when Tesla OTA detected on 0x318
	bool txPaused;		// true = all TX paused during OTA

	// AP Injection Gate (phase 1 scaffold)
	bool apInjectionGateEnabled; // true = gate writes until AP/Park/Summon conditions are met
	bool apGateApActive;		 // runtime AP-active signal
	bool apGateParked;			 // runtime parked signal
	bool apGateSummoning;		 // runtime summon signal

	// Auto HW detection (from 0x398)
	uint8_t detectedHW; // 0=unknown, 2=HW3, 3=HW4
	bool hwAutoDetected;
	bool variantAutoDetect; // true = auto-switch variant on CAN detection

	// MCP2515 CAN crystal profile (runtime)
	uint8_t canClockReqMHz; // requested profile: 0=auto, otherwise 8/12/16/20
	uint8_t canClockMHz;	// active profile after fallback: 8/16/20

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
	bool banShieldEnabled;		// true = monitor for ban threat patterns
	uint8_t banThreatLevel;		// 0=none, 1-5=escalating threat levels
	uint16_t banDetectionCount; // cumulative ban threat events

	unsigned long banThreatMs; // timestamp of last threat detected

	// GTW Shield — 0x7FF snapshot defense (hypery11 pattern)
	// Captures each of the 8 mux variants of GTW_carConfig in a healthy
	// state, then (when armed) retransmits the snapshot any time the
	// gateway tries to push a modified frame (e.g. a server-side ban push).
	uint8_t gtwSnapshot[8][8]; // [mux][byte0..7] — 64 bytes
	bool gtwSnapshotValid[8];  // per-mux: has this mux been captured?
	bool gtwShieldArmed;	   // true = actively blocking any change
	uint32_t gtwShieldBlocks;  // counter: frames blocked since arm

	// Enhanced Autopilot — unlocks EAP/Summon by setting bit46 on mux=1
	bool enhancedAutopilot; // persisted

	// Emergency Vehicle Detection — HW4 only, sets bit59 on mux=0
	bool evdEnabled; // persisted

	// TLSSC Restore — spoof DAS_autopilot tier to SELF_DRIVING on 0x331
	bool tlsscRestore; // persisted

	// TPMS — Tire Pressure Monitoring (read-only from 0x219)
	float tpmsPressure[4]; // FL, FR, RL, RR (bar)
	int8_t tpmsTemp[4];	   // FL, FR, RL, RR (°C)
	bool hasTpms;		   // At least one TPMS frame decoded

	// Drive mode override ("Ghost Mode") — persisted
	uint8_t driveModeOverride; // 0=none, 1=chill, 2=standard, 3=performance
	uint8_t currentDriveMode;  // Current readback from DI_steer (0-3)
	unsigned long driveModeLastMs;

	// Region detection (from 0x398)
	uint8_t regionCode;		 // 0=unknown, 1=NA, 2=EU, 3=CN, 4=APAC, 5=ME
	uint8_t regionSpoofCode; // 0=off (use real), 1-5=spoof to that region
	bool hasRegion;
	bool chineseGatewayLocked; // true = CN market, FSD blocked at GTW level

	// ECE R79 bypass — persisted
	bool eceR79Bypass; // true = clear ECE R79 restriction bit in EU

	// Left-Hand Drive mode — persisted
	bool lhdEnabled; // true = clear UI_drivingSide bit on 0x3F8 (Beta/LHD.json)

	// AP-First mode (2026.14.x compatibility) — persisted
	// Suppresses 0x3FD injection until DAS_autopilotState >= 2 (AP is already running)
	bool apFirstEnabled; // true = delay 0x3FD injection until AP/TACC is active

	// Driver assist parity toggles (P2-01 to P2-05) — persisted
	// All bits apply to 0x3F8 (UI_driverAssistControl) and 0x3FD mux1
	bool assistNavEnable;	 // P2-01: bits 13+48+49 — UI_driveOnMapsEnable + UI_hasDriveOnNav + UI_followNavRouteEnable
	bool assistHandsOff;	 // P2-02: bit 14 — UI_handsOnRequirementDisable
	bool assistDevMode;		 // P2-03: bit 5  — UI_dasDeveloper
	bool laneGraphEnable;	 // P2-04: bit 45 on 0x3FD mux1 — lane visualization
	bool assistTelemetryOff; // P2-05: bit 43 cleared — UI_enableTripTelemetry off

	// Steering input for natural nag-killer modulation
	float steeringAngle; // degrees, + = right (from 0x129)

	// ── CAN Diagnostics (runtime, not persisted) ────────────────────────────
	CanDiag canDiag; // per-bus stats + all intercept/error counters

	// Turn signals (3-blink lane change)
	// No persistent state — momentary burst only

	// Rear seatbelt buckle emulation
	bool seatbeltEmulation; // true = suppress rear seatbelt warnings
	unsigned long seatbeltLastMs;

	// Automatic air recirculation (0x2AA)
	// No persistent state — momentary command only

	// Wiper speed persistence
	bool wiperPersistEnabled; // true = persist wiper speed across restarts
	uint8_t savedWiperSpeed;  // last wiper speed set (0-3)

	// Mirror auto-fold on lock
	bool mirrorAutoFoldEnabled; // true = auto fold on lock, unfold on unlock
	bool vehicleLockedState;	// current known lock state

	// Powertrain telemetry (read-only decode)
	float vehicleSpeed;		 // km/h (signed, from 0x257)
	uint8_t gearState;		 // 0=inv, 1=P, 2=R, 3=N, 4=D (from 0x118)
	uint8_t accelPedal;		 // 0-100% (from 0x118)
	uint8_t brakePedalState; // 0=off 1=on (bits[20:19] of 0x118 DI_STATE)
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
	int8_t rearInvTemp;		  // °C rear inverter (from 0x315 byte1 − 40)
	int8_t rearStatorTemp;	  // °C rear stator   (from 0x315 byte2 − 40)
	int8_t rearHeatsinkTemp;  // °C rear heatsink (from 0x315 byte4 − 40)
	int8_t frontInvTemp;	  // °C front inverter (from 0x376 byte1 − 40, dual-motor only)
	int8_t frontStatorTemp;	  // °C front stator   (from 0x376 byte2 − 40)
	int8_t frontHeatsinkTemp; // °C front heatsink (from 0x376 byte4 − 40)
	bool hasMotorTemps;

	// CAN simulation mode
	bool canSimEnabled; // true = generating synthetic CAN frames
	unsigned long canSimLastMs;
	uint16_t canSimCounter;

	// Single-shot TX mode (1.6)
	bool singleShotTx; // true = use one-shot TX, no retries on failure

	// Firmware version compatibility (1.7)
	uint16_t fwYear;   // Vehicle firmware year (e.g. 2026)
	uint8_t fwRelease; // Release number (e.g. 2, 8)
	uint8_t fwMinor;   // Minor version
	uint32_t fwBuild;  // Build number
	uint8_t fwCompat;  // FwCompatLevel enum
	bool hasFwVersion; // At least one 0x392 frame decoded

	// MQTT telemetry bridge (3.6)
	bool mqttEnabled;	   // true = publish telemetry to MQTT
	char mqttHost[64];	   // Broker hostname
	uint16_t mqttPort;	   // Broker port (default 1883)
	uint16_t mqttInterval; // Publish interval ms (default 2000)
	unsigned long mqttLastPublishMs;
	bool mqttConnected; // true = currently connected to broker

	// Vehicle-specific config (5.8)
	uint8_t vehicleModel;  // VehicleModel enum
	uint16_t vehicleYear;  // Detected model year
	bool hasVehicleConfig; // At least one config frame decoded

	// Vehicle platform identity (Model → HW → SW)
	uint8_t platformModel;	   // TeslaModel enum
	uint8_t platformHwGen;	   // HWGeneration enum
	uint16_t platformSwYear;   // Software year (YYYY)
	uint8_t platformSwWeek;	   // Software week (WW)
	uint8_t platformSwRelease; // Software release
	uint8_t platformSwPatch;   // Software patch
	uint8_t platformFsdProto;  // FsdProtocol enum
	uint8_t platformSwCompat;  // SwCompatLevel enum
	bool platformResolved;	   // true = model + hw known

	// WiFi API authentication (NVS-persisted)
	char apiKey[33];	 // 32-char hex key + NUL, generated on first boot
	bool apiKeyRequired; // true = require X-API-Key header on mutable endpoints

	State()
		: variant(HW4), fsdEnabled(false), fsdForceEnabled(false), nagSuppress(false), speedProfile(1),
		  profileOverride(false), speedOffset(0), offsetOverride(false), isaChimeSuppress(false), summonInject(false),
		  streamEnabled(false), streamCount(0), rawCanListen(false), lastFrameMs(0), chassisOnline(false),
		  standby(false), lastReinitMs(0), summonRemaining(0), summonLastMs(0), hasCtrl(false),
		  summonDirection(SUMMON_FORWARD), summonMode(SUMMON_STOP), hasClimate(false), hasCharge(false),
		  hasDrive(false), bmsVoltage(0), bmsCurrent(0), bmsPower(0), bmsSoc(0), bmsTempMin(0), bmsTempMax(0),
		  bmsWhPerKm(0), hasBms(false), bmsNominalFullPack(0), bmsNominalRemaining(0), bmsIdealRemaining(0),
		  bmsCellVoltageMax(0), bmsCellVoltageMin(0), bmsMaxRegenPower(0), bmsMaxDischargePower(0),
		  hasEnhancedBms(false), bmsSocUI(0), bmsSocMax(0), bmsSocAvg(0), bmsInitialFullPack(0), bmsExpectedRange(0),
		  bmsIdealRange(0), bmsRatedConsumption(0), bmsActualSocInt(0), bmsUsableSocInt(0), bmsPowerDissipation(0),
		  bmsFlowRequest(0), bmsCoolTarget(0), bmsPassiveTarget(0), bmsHeatTarget(0), bmsPackTMin(0), bmsPackTMax(0),
		  bmsStationaryHeatPower(0), bmsHvacPowerBudget(0), bmsPrecondAllowed(false), bmsHeatingWorthwhile(false),
		  bmsContactorState(0), bmsHvState(0), bmsMinBusVoltage(0), bmsMaxBusVoltage(0), bmsMaxChargeCurrent(0),
		  bmsMaxDischargeCurrent(0), bmsExpectedRemaining(0), bmsEnergyBuffer(0), bmsEnergyToCharge(0),
		  bmsFullyCharged(false), bmsKwhDischargeTotal(0), bmsKwhChargeTotal(0), bmsAcChargeTotal(0),
		  bmsDcChargeTotal(0), bmsRegenTotal(0), bmsDriveDischargeTotal(0), bmsThermistorTMax(0), bmsThermistorTMin(0),
		  bmsModelTMax(0), bmsModelTMin(0), bmsChargeTimeToFull(0), steeringMode(0), hasSteeringMode(false),
		  nagKillerEnabled(false), nagKillerMode(NAG_KILLER_LEGACY), dasHandsOnState(0), dasSeen(false),
		  naturalNagLastMs(0), naturalNagIntervalMs(200), alcAutoConfirmEnabled(false), dasLaneChangeState(0),
		  alcLastConfirmMs(0), turnSignalLeft(false), turnSignalRight(false), bsmLeftLevel(0), bsmRightLevel(0),
		  doorFrontLeftOpen(false), doorFrontRightOpen(false), doorRearLeftOpen(false), doorRearRightOpen(false),
		  driverDoorOpen(false), anyDoorOpen(false), frunkOpen(false), trunkOpen(false), cruiseSetSpeedKph(0),
		  accSpeedLimitKph(0), mapSpeedLimitKph(0), maxSpeedKph(0), otaInProgress(false), txPaused(false),
		  apInjectionGateEnabled(false), apGateApActive(false), apGateParked(true), apGateSummoning(false),
		  detectedHW(0), hwAutoDetected(false), variantAutoDetect(true), canClockReqMHz(BOARD_CAN_CLOCK_MHZ),
		  canClockMHz(BOARD_CAN_CLOCK_MHZ), gtwAutopilotTier(-1), gtwAutopilotSeen(false), preconditionEnabled(false),
		  precondLastMs(0), trackModeEnabled(false), burstBus(0), burstRemaining(0), burstDelayMs(0), burstLastMs(0),
		  ctrlBus(0), banShieldEnabled(false), banThreatLevel(0), banDetectionCount(0), banThreatMs(0),
		  gtwShieldArmed(false), gtwShieldBlocks(0), enhancedAutopilot(false), evdEnabled(false), tlsscRestore(false),
		  hasTpms(false), driveModeOverride(0), currentDriveMode(0), driveModeLastMs(0), regionCode(0),
		  regionSpoofCode(0), hasRegion(false), chineseGatewayLocked(false), eceR79Bypass(false), lhdEnabled(false),
		  dasApState(0), apFirstEnabled(false), assistNavEnable(false), assistHandsOff(false), assistDevMode(false),
		  laneGraphEnable(false), assistTelemetryOff(false), seatbeltEmulation(false), seatbeltLastMs(0),
		  wiperPersistEnabled(false), savedWiperSpeed(0), mirrorAutoFoldEnabled(false), vehicleLockedState(false),
		  vehicleSpeed(0), gearState(0), accelPedal(0), brakePedalState(0), rearMotorRpm(0), frontMotorRpm(0),
		  hasPowertrain(false), wheelSpeedFL(0), wheelSpeedFR(0), wheelSpeedRL(0), wheelSpeedRR(0),
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

	Features features() const
	{
		return getFeatures(variant);
	}

	bool apGateOpen() const
	{
		if (!apInjectionGateEnabled)
			return true;
		return apGateApActive || apGateParked || apGateSummoning;
	}
};
