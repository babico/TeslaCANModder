#pragma once
#include "infra/can.h"
#include "infra/can_recorder.h"
#include "infra/id_filter.h"
#include "infra/log_ring.h"
#include "infra/ring_buffer.h"
#include "feature/summon.h"
#include "feature/bms.h"
#include "feature/nag.h"
#include "feature/tpms.h"
#include "feature/region.h"
#include "feature/drive_mode.h"
#include "feature/turn_signal.h"
#include "feature/drive_context.h"
#include "feature/seatbelt.h"
#include "feature/air_recirc.h"
#include "feature/wiper.h"
#include "feature/mirror.h"
#include "feature/powertrain.h"
#include "feature/wheel_speeds.h"
#include "feature/motor_temps.h"
#include "feature/can_sim.h"
#include "feature/fw_compat.h"
#include "feature/vehicle_config.h"
#include "feature/single_shot.h"
#include "feature/ban_shield.h"
#include "feature/ban_detect.h"
#include "feature/auto_lane_change.h"
#include "feature/tlssc.h"
#include "core/platform.h"
#include "core/driver/esp32/board.h"
#include "handler/hw4.h"
#include "handler/hw3.h"
#include "handler/legacy.h"

// Module-level platform instance for re-resolution on CAN updates
static VehiclePlatform dispatchPlatform;

void resetHandlerLogFlags()
{
	resetHW4LogFlags();
	resetHW3LogFlags();
	resetLegacyLogFlags();
}

// ── Per-Bus Filter Setup (Hardcoded Tesla X179) ─────────────────────────────
void applyFilters(State &s)
{
	// Bus 0 (Chassis bus, X179 pins 13-14): dynamic filters based on enabled features
	// Toggle-inject pattern: feature ON → intercept + inject, feature OFF → don't intercept CAN line
	if (s.rawCanListen)
	{
		driverSetBusFilters(0, nullptr, 0);
	}
	else
	{
		uint32_t ids[13];
		uint8_t count = 0;
		ids[count++] = CAN_ID_DAS_CONTROL;
		ids[count++] = CAN_ID_DAS_STATUS2;
		ids[count++] = CAN_ID_UI_GPS_SPEED;
		ids[count++] = CAN_ID_WHEEL_SPEED; // wheel speed telemetry (read-only)
		switch (s.variant)
		{
		case HW4:
			if (s.isaChimeSuppress)
				ids[count++] = CAN_ID_ISA_SPEED;
			if (s.fsdEnabled)
				ids[count++] = CAN_ID_FOLLOW_DIST;
			if (s.fsdEnabled || s.nagSuppress)
				ids[count++] = CAN_ID_FSD_MUX;
			break;
		case HW3:
			if (s.fsdEnabled)
				ids[count++] = CAN_ID_FOLLOW_DIST;
			if (s.fsdEnabled || s.nagSuppress)
				ids[count++] = CAN_ID_FSD_MUX;
			break;
		case LEGACY:
			if (s.fsdEnabled)
				ids[count++] = CAN_ID_LEGACY_STALK;
			if (s.fsdEnabled || s.nagSuppress)
				ids[count++] = CAN_ID_LEGACY_FSD_MUX;
			break;
		}
		// P2-06: Fallback variant detection — when auto-detect is on and 0x398 not yet seen,
		// include discriminating frames so variant can be inferred from bus traffic presence.
		if (s.variantAutoDetect && !s.hwAutoDetected)
		{
			bool isaAlready = (s.variant == HW4 && s.isaChimeSuppress);
			bool legacyMuxAlready = (s.variant == LEGACY && (s.fsdEnabled || s.nagSuppress));
			if (!isaAlready)
				ids[count++] = CAN_ID_ISA_SPEED;      // HW4-only frame (ISA speed chime)
			if (!legacyMuxAlready)
				ids[count++] = CAN_ID_LEGACY_FSD_MUX; // Legacy-only frame
		}
		if (count > 0)
		{
			driverSetBusFilters(BUS_CHASSIS, ids, count);
		}
		else
		{
			static const uint32_t none[] = {0x000};
			driverSetBusFilters(BUS_CHASSIS, none, 1);
		}
	}

#if BUS_VEHICLE_ACTIVE
	// Bus 1 (Vehicle bus, X179 pins 9-10): vehicle control + BMS + new feature frames
	if (s.rawCanListen)
	{
		driverSetBusFilters(BUS_VEHICLE, nullptr, 0);
	}
	else
	{
		static const uint32_t vehIds[] = {CAN_ID_PRECONDITION,
										  CAN_ID_BMS_HV_BUS,
										  CAN_ID_UI_VEHICLE_CTRL,
										  CAN_ID_BMS_SOC,
										  CAN_ID_CLIMATE,
										  CAN_ID_BMS_THERMAL,
										  CAN_ID_TRACK_MODE,
										  CAN_ID_GTW_CAR_STATE,
										  CAN_ID_CHARGE,
										  CAN_ID_DRIVE_CONFIG,
										  CAN_ID_BMS_ENERGY,
										  CAN_ID_EPAS_TORQUE,
										  CAN_ID_GTW_CAR_CFG,
										  CAN_ID_DAS_STATUS,
										  CAN_ID_GTW_CONFIG_ETH,
										  CAN_ID_BLIND_SPOT,
										  CAN_ID_VCLEFT_DOOR_STATUS,
										  CAN_ID_VCRIGHT_DOOR_STATUS,
										  CAN_ID_VCFRONT_STATUS,
										  CAN_ID_VCFRONT_VEH_STATUS,
										  CAN_ID_BMS_ENERGY_ST,
										  CAN_ID_BMS_MIN_MAX,
										  CAN_ID_BMS_POWER_AV,
										  CAN_ID_BMS_STATUS,
										  CAN_ID_BMS_DRIVE_LIM,
										  CAN_ID_BMS_KWH_CNT,
										  CAN_ID_BMS_KWH_MUX,
										  CAN_ID_BMS_BRICK_V,
										  CAN_ID_TPMS,
										  CAN_ID_DI_STEER,
										  CAN_ID_VCFRONT_LIGHTS,
										  CAN_ID_VEHICLE_SPEED,
										  CAN_ID_DI_STATE,
										  CAN_ID_STEERING_ANGLE,
										  CAN_ID_REAR_MOTOR,
										  CAN_ID_FRONT_MOTOR,
										  CAN_ID_SEATBELT_STATUS,
										  CAN_ID_GTW_VERSION,
									  CAN_ID_DAS_AP_CONFIG,
									  CAN_ID_REAR_INV_TEMPS,
									  CAN_ID_FRONT_INV_TEMPS};
		driverSetBusFilters(BUS_VEHICLE, vehIds, sizeof(vehIds) / sizeof(vehIds[0]));
	}
#endif

#if BUS_BODY_ACTIVE
	// Bus 2 (Body bus, X179 pins 2-3): body control frames
	if (s.rawCanListen)
	{
		driverSetBusFilters(BUS_BODY, nullptr, 0);
	}
	else
	{
		static const uint32_t bodyIds[] = {CAN_ID_WINDOW_VENT, CAN_ID_SENTRY, CAN_ID_TRUNK_CTRL};
		driverSetBusFilters(BUS_BODY, bodyIds, 3);
	}
#endif
}

// ── Summon Tick ──────────────────────────────────────────────────────────────
void summonTick(State &s)
{
#if !BUS_VEHICLE_ACTIVE
	(void)s;
	return;
#else
	if (s.summonRemaining == 0 || !s.hasCtrl || !s.summonInject)
		return;
	if (!s.apGateOpen())
		return;
	if (s.txPaused)
	{
		s.summonRemaining = 0;
		return;
	}
	unsigned long now = millis();
	if (now - s.summonLastMs < 20)
		return;
	s.summonLastMs = now;

	Frame f;
	f.id = CAN_ID_UI_VEHICLE_CTRL;
	f.dlc = 8;
	memcpy(f.data, s.lastCtrl, 8);
	setSummonActive(f, true);
	setSummonDirection(f, s.summonDirection);
	setSummonMode(f, s.summonMode);
	driverSend(f, BUS_VEHICLE);
	s.summonRemaining--;
	if (s.summonRemaining == 0)
		sendLog(F("Summon burst complete"));
#endif
}

// ── Preconditioning Tick ──────────────────────────────────────────────────────
void preconditionTick(State &s)
{
#if !BUS_VEHICLE_ACTIVE
	(void)s;
	return;
#else
	if (!s.preconditionEnabled)
		return;
	if (!s.apGateOpen())
		return;
	if (s.txPaused)
		return;
	unsigned long now = millis();
	if (now - s.precondLastMs < 500)
		return;
	s.precondLastMs = now;
	Frame f;
	f.id = CAN_ID_PRECONDITION;
	f.dlc = 8;
	memset(f.data, 0, 8);
	f.data[0] = 0x05;
	driverSend(f, BUS_VEHICLE);
#endif
}

// ── Burst Tick (non-blocking one-shot sends) ────────────────────────────────────
void burstTick(State &s)
{
	if (s.burstRemaining == 0)
		return;
	if (!s.apGateOpen())
		return;
	if (s.txPaused)
	{
		s.burstRemaining = 0;
		return;
	}
	unsigned long now = millis();
	if (now - s.burstLastMs < s.burstDelayMs)
		return;
	s.burstLastMs = now;
	driverSend(s.burstFrame, s.burstBus);
	s.burstRemaining--;
}

// ── Drive Mode Tick ─────────────────────────────────────────────────────────
void driveModeTick_dispatch(State &s)
{
#if !BUS_VEHICLE_ACTIVE
	(void)s;
	return;
#else
	driveModeTick(s, millis());
#endif
}

// ── CAN Frame Rate Update (call once per received frame) ─────────────────────
// Updates rolling Hz (x10 fixed-point) + lifetime frame count for the given bus.
// Hz min/max are tracked per bus after the first complete 1-second window.
static inline void _updateCanFrameRate(State &s, uint8_t bus, uint32_t now)
{
	if (bus >= 3)
		return;
	CanBusStat &b = s.canDiag.bus[bus];
	b.frames++;
	b.windowCount++;
	uint32_t elapsed = now - b.windowStartMs;
	if (elapsed >= 1000UL)
	{
		// Compute Hz × 10 as integer; clamp to uint16_t max
		uint32_t hz10 = (b.windowCount * 10000UL) / elapsed;
		b.hz = (hz10 > 0xFFFFu) ? 0xFFFFu : (uint16_t)hz10;
		if (b.hz < b.hzMin) b.hzMin = b.hz;
		if (b.hz > b.hzMax) b.hzMax = b.hz;
		b.windowCount = 0;
		b.windowStartMs = now;
	}
}

// ── Message Dispatch ─────────────────────────────────────────────────────────
void handleMessage(Frame &f, uint8_t bus, State &s)
{
	canRecorderCapture(f, bus, millis());
	_updateCanFrameRate(s, bus, millis());

	// Bus 0 (Chassis): variant-specific FSD frame processing
	if (bus == BUS_CHASSIS)
	{
		if (f.id == CAN_ID_DAS_CONTROL && f.dlc >= 2)
		{
			s.cruiseSetSpeedKph = decodeCruiseSetSpeedKph(f);
			s.maxSpeedKph = s.cruiseSetSpeedKph;
			return;
		}
		if (f.id == CAN_ID_DAS_STATUS2 && f.dlc >= 2)
		{
			s.accSpeedLimitKph = decodeAccSpeedLimitKph(f);
			if (s.accSpeedLimitKph > s.maxSpeedKph)
				s.maxSpeedKph = s.accSpeedLimitKph;
			return;
		}
		if (f.id == CAN_ID_UI_GPS_SPEED && f.dlc >= 7)
		{
			s.mapSpeedLimitKph = decodeMapSpeedLimitKph(f);
			if (s.mapSpeedLimitKph > s.maxSpeedKph)
				s.maxSpeedKph = s.mapSpeedLimitKph;
			return;
		}
		// Wheel speeds — all four wheels packed in one 8-byte frame (read-only telemetry)
		if (f.id == CAN_ID_WHEEL_SPEED && f.dlc >= 7)
		{
			s.wheelSpeedFL = decodeWheelSpeedFL(f.data);
			s.wheelSpeedFR = decodeWheelSpeedFR(f.data);
			s.wheelSpeedRL = decodeWheelSpeedRL(f.data);
			s.wheelSpeedRR = decodeWheelSpeedRR(f.data);
			s.hasWheelSpeeds = true;
			return;
		}
		// P2-06: Fallback variant inference from distinctive frame presence (when 0x398 absent)
		if (s.variantAutoDetect && !s.hwAutoDetected)
		{
			if (f.id == CAN_ID_ISA_SPEED && s.variant != HW4)
			{
				// ISA speed chime (921) is HW4-only; infer variant from its presence
				bool fromLegacy = (s.variant == LEGACY);
				s.variant = HW4;
				if (fromLegacy) s.speedProfile = 1; // P2-07: clear stale legacy stalk value
				applyFilters(s);
				resetHandlerLogFlags();
				sendLog(F("Fallback: HW4 inferred from ISA_SPEED"));
			}
			else if (f.id == CAN_ID_LEGACY_FSD_MUX && s.variant != LEGACY)
			{
				// Legacy FSD mux (1006) is Legacy-only; infer variant from its presence
				s.variant = LEGACY;
				applyFilters(s);
				resetHandlerLogFlags();
				sendLog(F("Fallback: LEGACY inferred from legacy mux frame"));
			}
		}
		switch (s.variant)
		{
		case HW4:
			handleHW4(f, s);
			break;
		case HW3:
			handleHW3(f, s);
			break;
		case LEGACY:
			handleLegacy(f, s);
			break;
		}
		return;
	}

#if BUS_VEHICLE_ACTIVE
	// Bus 1 (Vehicle): cache control frames + new feature frames
	if (bus == BUS_VEHICLE)
	{
		if (f.id == CAN_ID_UI_VEHICLE_CTRL && f.dlc >= 8)
		{
			memcpy(s.lastCtrl, f.data, 8);
			s.hasCtrl = true;
			return;
		}
		if (f.id == CAN_ID_CLIMATE && f.dlc >= 5)
		{
			memcpy(s.lastClimate, f.data, 5);
			s.hasClimate = true;
			return;
		}
		if (f.id == CAN_ID_CHARGE && f.dlc >= 5)
		{
			memcpy(s.lastCharge, f.data, 5);
			s.hasCharge = true;
			return;
		}
		if (f.id == CAN_ID_DRIVE_CONFIG && f.dlc >= 8)
		{
			memcpy(s.lastDrive, f.data, 8);
			s.hasDrive = true;
			return;
		}

		// BMS battery telemetry (read-only decode)
		if (f.id == CAN_ID_BMS_HV_BUS && f.dlc >= 4)
		{
			s.bmsVoltage = decodeBmsVoltage(f.data);
			s.bmsCurrent = decodeBmsCurrent(f.data);
			s.bmsPower = decodeBmsPower(f.data);
			if (f.dlc >= 8)
				s.bmsChargeTimeToFull = decodeBmsChargeTimeToFull(f.data);
			s.hasBms = true;
			return;
		}
		if (f.id == CAN_ID_BMS_SOC && f.dlc >= 2)
		{
			s.bmsSoc = decodeBmsSoc(f.data);
			if (f.dlc >= 5)
			{
				s.bmsSocUI = decodeBmsSocUI(f.data);
				s.bmsSocMax = decodeBmsSocMax(f.data);
				s.bmsSocAvg = decodeBmsSocAvg(f.data);
			}
			if (f.dlc >= 7)
				s.bmsInitialFullPack = decodeBmsInitialFullPack(f.data);
			s.hasBms = true;
			return;
		}
		if (f.id == CAN_ID_BMS_THERMAL && f.dlc >= 2)
		{
			s.bmsTempMin = decodeBmsTempMin(f.data);
			s.bmsTempMax = decodeBmsTempMax(f.data);
			if (f.dlc >= 8)
			{
				s.bmsPowerDissipation = decodeBmsPowerDissipation(f.data);
				s.bmsFlowRequest = decodeBmsFlowRequest(f.data);
				s.bmsCoolTarget = decodeBmsCoolTarget(f.data);
				s.bmsPassiveTarget = decodeBmsPassiveTarget(f.data);
				s.bmsHeatTarget = decodeBmsHeatTarget(f.data);
				s.bmsPackTMin = decodeBmsPackTMin(f.data);
				s.bmsPackTMax = decodeBmsPackTMax(f.data);
			}
			s.hasBms = true;
			return;
		}
		if (f.id == CAN_ID_BMS_ENERGY && f.dlc >= 2)
		{
			s.bmsWhPerKm = decodeBmsWhPerKm(f.data);
			if (f.dlc >= 8)
			{
				s.bmsExpectedRange = decodeBmsExpectedRange(f.data);
				s.bmsIdealRange = decodeBmsIdealRange(f.data);
				s.bmsRatedConsumption = decodeBmsRatedConsumption(f.data);
				s.bmsActualSocInt = decodeBmsActualSocInt(f.data);
				s.bmsUsableSocInt = decodeBmsUsableSocInt(f.data);
			}
			s.hasBms = true;
			return;
		}

		if (f.id == CAN_ID_DAS_STATUS && f.dlc >= 6)
		{
			s.dasHandsOnState = readDasHandsOnState(f);
			s.dasLaneChangeState = readDasLaneChangeState(f);
			s.dasApState = readDASAutopilotState(f);
			s.apGateApActive = isDASAutopilotActive(readDASAutopilotStatus(f));
			s.dasSeen = true;
			// ALC auto-confirm: inject stalk/button when lane change prompted
			unsigned long now = millis();
			if (alcShouldConfirm(s, now) && s.apGateOpen())
			{
				int8_t dir = alcDirectionFromTurnSignal(s);
				if (dir != 0)
				{
					Frame confirm;
					if (s.variant == HW4)
					{
						buildPalladiumTurnFrame(confirm, dir < 0);
					}
					else
					{
						buildStalkFrame(confirm, dir < 0);
					}
					driverSend(confirm, BUS_VEHICLE);
					s.alcLastConfirmMs = now;
					sendLog(dir < 0 ? F("ALC: confirmed LEFT") : F("ALC: confirmed RIGHT"));
				}
			}
			return;
		}

		// D-05 safety cues: turn signal status from VCFRONT lights
		if (f.id == CAN_ID_VCFRONT_LIGHTS && f.dlc >= 7)
		{
			s.turnSignalLeft = decodeTurnSignalLeftActive(f);
			s.turnSignalRight = decodeTurnSignalRightActive(f);
			return;
		}

		// D-05 safety cues: blind-spot levels
		if (f.id == CAN_ID_BLIND_SPOT && f.dlc >= 1)
		{
			s.bsmLeftLevel = decodeBlindSpotLeftLevel(f);
			s.bsmRightLevel = decodeBlindSpotRightLevel(f);
			return;
		}

		if (f.id == CAN_ID_VCLEFT_DOOR_STATUS && f.dlc >= 2)
		{
			s.doorFrontLeftOpen = decodeDoorFrontLeftOpen(f);
			s.doorRearLeftOpen = decodeDoorRearLeftOpen(f);
			return;
		}

		if (f.id == CAN_ID_VCRIGHT_DOOR_STATUS && f.dlc >= 8)
		{
			s.doorFrontRightOpen = decodeDoorFrontRightOpen(f);
			s.doorRearRightOpen = decodeDoorRearRightOpen(f);
			s.trunkOpen = decodeTrunkOpen(f);
			return;
		}

		if (f.id == CAN_ID_VCFRONT_STATUS && f.dlc >= 8)
		{
			s.frunkOpen = decodeFrunkOpen(f);
			s.anyDoorOpen = decodeAnyDoorOpen(f);
			return;
		}

		if (f.id == CAN_ID_VCFRONT_VEH_STATUS && f.dlc >= 4)
		{
			s.driverDoorOpen = decodeDriverDoorOpen(f);
			return;
		}

		// Enhanced BMS: degradation / capacity (mux=0) + energy status (mux=1)
		if (f.id == CAN_ID_BMS_ENERGY_ST && f.dlc >= 8)
		{
			uint8_t mux = f.data[0] & 0x0F;
			if (mux == 0)
			{
				s.bmsNominalFullPack = decodeBmsNominalFullPack(f.data);
				s.bmsNominalRemaining = decodeBmsNominalRemaining(f.data);
				s.bmsIdealRemaining = decodeBmsIdealRemaining(f.data);
				s.hasEnhancedBms = true;
			}
			else if (mux == 1)
			{
				s.bmsEnergyBuffer = decodeBmsEnergyBuffer(f.data);
				s.bmsExpectedRemaining = decodeBmsExpectedRemaining(f.data);
				s.bmsEnergyToCharge = decodeBmsEnergyToChargeComplete(f.data);
				s.bmsFullyCharged = decodeBmsFullyCharged(f.data);
				s.hasEnhancedBms = true;
			}
			return;
		}
		// Enhanced BMS: cell voltage min/max (mux=1) + thermistor temps (mux=0)
		if (f.id == CAN_ID_BMS_MIN_MAX && f.dlc >= 4)
		{
			uint8_t mux = f.data[0] & 0x0F;
			if (mux == 1)
			{
				s.bmsCellVoltageMax = decodeBmsCellVoltageMax(f.data);
				s.bmsCellVoltageMin = decodeBmsCellVoltageMin(f.data);
				s.hasEnhancedBms = true;
			}
			else if (mux == 0 && f.dlc >= 6)
			{
				s.bmsThermistorTMax = decodeBmsThermistorTMax(f.data);
				s.bmsThermistorTMin = decodeBmsThermistorTMin(f.data);
				s.bmsModelTMax = decodeBmsModelTMax(f.data);
				s.bmsModelTMin = decodeBmsModelTMin(f.data);
				s.hasEnhancedBms = true;
			}
			return;
		}
		// Enhanced BMS: power limits + HVAC budget
		if (f.id == CAN_ID_BMS_POWER_AV && f.dlc >= 4)
		{
			s.bmsMaxRegenPower = decodeBmsMaxRegenPower(f.data);
			s.bmsMaxDischargePower = decodeBmsMaxDischargePower(f.data);
			if (f.dlc >= 8)
			{
				s.bmsStationaryHeatPower = decodeBmsStationaryHeatPower(f.data);
				s.bmsHvacPowerBudget = decodeBmsHvacPowerBudget(f.data);
			}
			s.hasEnhancedBms = true;
			return;
		}

		// BMS_status (0x212): precondition flags, HV state, contactor
		if (f.id == CAN_ID_BMS_STATUS && f.dlc >= 3)
		{
			s.bmsPrecondAllowed = decodeBmsPrecondAllowed(f.data);
			s.bmsHeatingWorthwhile = decodeBmsHeatingWorthwhile(f.data);
			s.bmsContactorState = decodeBmsContactorState(f.data);
			s.bmsHvState = decodeBmsHvState(f.data);
			s.hasEnhancedBms = true;
			return;
		}

		// BMS_driveLimits (0x2D2): bus voltage/current limits
		if (f.id == CAN_ID_BMS_DRIVE_LIM && f.dlc >= 8)
		{
			s.bmsMinBusVoltage = decodeBmsMinBusVoltage(f.data);
			s.bmsMaxBusVoltage = decodeBmsMaxBusVoltage(f.data);
			s.bmsMaxChargeCurrent = decodeBmsMaxChargeCurrent(f.data);
			s.bmsMaxDischargeCurrent = decodeBmsMaxDischargeCurrent(f.data);
			s.hasEnhancedBms = true;
			return;
		}

		// BMS_kwhCounter (0x3D2): lifetime counters
		if (f.id == CAN_ID_BMS_KWH_CNT && f.dlc >= 8)
		{
			s.bmsKwhDischargeTotal = decodeBmsKwhDischargeTotal(f.data);
			s.bmsKwhChargeTotal = decodeBmsKwhChargeTotal(f.data);
			s.hasEnhancedBms = true;
			return;
		}

		// BMS_kwhCountersMultiplexed (0x3F2)
		if (f.id == CAN_ID_BMS_KWH_MUX && f.dlc >= 5)
		{
			uint8_t mux = f.data[0];
			float val = decodeBmsKwhMuxCounter(f.data);
			switch (mux)
			{
			case 0:
				s.bmsAcChargeTotal = val;
				break;
			case 1:
				s.bmsDcChargeTotal = val;
				break;
			case 2:
				s.bmsRegenTotal = val;
				break;
			case 3:
				s.bmsDriveDischargeTotal = val;
				break;
			}
			s.hasEnhancedBms = true;
			return;
		}

		// Nag killer + steering mode: intercept EPAS torque frame
		if (f.id == CAN_ID_EPAS_TORQUE && f.dlc >= 8)
		{
			// Steering mode readback: EPAS_currentTuneMode = byte[0] bits[7:4]
			s.steeringMode = (f.data[0] >> 4) & 0x0F;
			s.hasSteeringMode = true;
			if (!s.txPaused && s.apGateOpen() && nagKillerShouldEcho(s))
			{
				Frame echo = f;
				if (s.nagKillerMode == NAG_KILLER_NATURAL && nagNaturalIntervalReady(s, millis()))
				{
					float torque = nagNaturalTorque(s.steeringAngle, s.dasHandsOnState);
					nagKillerModifyNatural(echo, torque);
					driverSend(echo, BUS_VEHICLE);
					s.canDiag.nagEchoCount++;
				}
				else if (s.nagKillerMode != NAG_KILLER_NATURAL)
				{
					nagKillerModify(echo);
					driverSend(echo, BUS_VEHICLE);
					s.canDiag.nagEchoCount++;
				}
			}
			return;
		}

		// TPMS tire pressure decode (read-only)
		if (f.id == CAN_ID_TPMS && f.dlc >= 8)
		{
			decodeTpms(f, s);
			return;
		}

		// Region awareness: decode gateway region code + optional spoofing
		if (f.id == CAN_ID_GTW_CAR_CFG && f.dlc >= 3)
		{
			// Apply region spoof before decoding (modifies frame in-place)
			if (s.regionSpoofCode != 0 && !s.txPaused && s.apGateOpen())
			{
				applySpoofRegion(f, s.regionSpoofCode);
				driverSend(f, BUS_VEHICLE);
			}
			uint8_t r = decodeRegionCode(f.data);
			if (r != 0)
			{
				s.regionCode = r;
				s.hasRegion = true;
				s.chineseGatewayLocked = isChineseMarket(r);
			}
		}

		// Drive mode readback from DI_steer
		if (f.id == CAN_ID_DI_STEER && f.dlc >= 1)
		{
			// DI_driveMode = byte[0] bits[6:5]
			s.currentDriveMode = (f.data[0] >> 5) & 0x03;
			return;
		}

		// Powertrain telemetry (read-only decode)
		if (f.id == CAN_ID_VEHICLE_SPEED && f.dlc >= 4)
		{
			s.vehicleSpeed = decodeVehicleSpeed(f.data);
			s.hasPowertrain = true;
			return;
		}
		if (f.id == CAN_ID_DI_STATE && f.dlc >= 3)
		{
			s.gearState = decodeGearState(f.data);
			s.accelPedal = decodeAccelPedal(f.data);
			s.brakePedalState = decodeBrakePedalState(f.data);
			s.apGateParked = (s.gearState == 1 || s.gearState == 0 || s.gearState == 5);
			s.hasPowertrain = true;
			return;
		}
		if (f.id == CAN_ID_STEERING_ANGLE && f.dlc >= 2)
		{
			s.steeringAngle = decodeSteeringAngle(f.data);
			s.hasPowertrain = true;
			return;
		}
		if (f.id == CAN_ID_REAR_MOTOR && f.dlc >= 6)
		{
			s.rearMotorRpm = decodeMotorRpm(f.data);
			s.hasPowertrain = true;
			return;
		}
		if (f.id == CAN_ID_FRONT_MOTOR && f.dlc >= 6)
		{
			s.frontMotorRpm = decodeMotorRpm(f.data);
			s.hasPowertrain = true;
			return;
		}
		// Rear inverter / stator / heatsink temperatures (all models)
		if (f.id == CAN_ID_REAR_INV_TEMPS && f.dlc >= 5)
		{
			s.rearInvTemp = decodeRearInvTemp(f.data);
			s.rearStatorTemp = decodeRearStatorTemp(f.data);
			s.rearHeatsinkTemp = decodeRearHeatsinkTemp(f.data);
			s.hasMotorTemps = true;
			return;
		}
		// Front inverter / stator / heatsink temperatures (dual-motor only)
		if (f.id == CAN_ID_FRONT_INV_TEMPS && f.dlc >= 5)
		{
			s.frontInvTemp = decodeFrontInvTemp(f.data);
			s.frontStatorTemp = decodeFrontStatorTemp(f.data);
			s.frontHeatsinkTemp = decodeFrontHeatsinkTemp(f.data);
			s.hasMotorTemps = true;
			return;
		}

		// OTA safety check: detect Tesla OTA in progress
		// GTW_updateInProgress: bits[1:0] of byte 6 (0=none,1=available,2=installing,3=scheduled)
		// Fix (hypery11 v2.11): only pause TX when value=2 (installing); value=1 (available)
		// caused false positives. Added 3-frame assert / 6-frame clear debounce.
		if (f.id == CAN_ID_GTW_CAR_STATE && f.dlc >= 7)
		{
			static uint8_t otaAssertCnt = 0;
			static uint8_t otaClearCnt  = 0;
			bool installing = ((f.data[6] & 0x03) == 2);
			if (installing)
			{
				otaAssertCnt = (otaAssertCnt < 3) ? otaAssertCnt + 1 : 3;
				otaClearCnt  = 0;
				if (otaAssertCnt >= 3 && !s.otaInProgress)
				{
					s.otaInProgress = true;
					s.txPaused = true;
					sendLog(F("OTA detected - TX paused"));
				}
			}
			else
			{
				otaClearCnt  = (otaClearCnt < 6) ? otaClearCnt + 1 : 6;
				otaAssertCnt = 0;
				if (otaClearCnt >= 6 && s.otaInProgress)
				{
					s.otaInProgress = false;
					s.txPaused = false;
					sendLog(F("OTA complete - TX resumed"));
				}
			}
			return;
		}

		// Auto HW detection from GTW_carConfig
		// das_hw: 0/1=Legacy(MCU2/HW1/HW2 retrofit), 2=HW3, 3=HW4
		// Fix (hypery11 v2.11): previously fell through for hw=0/1, silently
		// skipping Legacy auto-detect for an entire class of vehicles.
		if (f.id == CAN_ID_GTW_CAR_CFG && f.dlc >= 1)
		{
			uint8_t hw = (f.data[0] >> 6) & 0x03;
			s.detectedHW = hw;
			s.hwAutoDetected = true;
			// Auto-switch variant if enabled (inspired by hypery11 detection)
			if (s.variantAutoDetect)
			{
				Variant detected;
				if (hw == 3)
					detected = HW4;
				else if (hw == 2)
					detected = HW3;
				else
					detected = LEGACY; // hw==0 or hw==1: MCU2/HW3 retrofit
				if (s.variant != detected)
				{
					bool fromLegacy = (s.variant == LEGACY);
					s.variant = detected;
					if (fromLegacy && detected != LEGACY)
						s.speedProfile = 1; // P2-07: reset stale legacy stalk profile
					applyFilters(s);
					resetHandlerLogFlags();
					if (detected == HW4)
						sendLog(F("Auto-detected HW4"));
					else if (detected == HW3)
						sendLog(F("Auto-detected HW3"));
					else
						sendLog(F("Auto-detected Legacy"));
				}
			}
			return;
		}

		// GTW autopilot tier readback from mixed/Ethernet bridge frame
		// Also run GTW shield defense when armed (hypery11 pattern)
		if (f.id == CAN_ID_GTW_CONFIG_ETH)
		{
			// GTW shield: learn snapshot or retransmit healthy frame
			if (!s.txPaused && handleGtwShield(f, s))
			{
				driverSend(f, BUS_VEHICLE);
				sendLog(F("GTW shield: blocked frame retransmitted"));
			}
			int8_t tier = readGtwAutopilotTier(f);
			if (tier >= 0)
			{
				int8_t prev = s.gtwAutopilotTier;
				s.gtwAutopilotTier = tier;
				s.gtwAutopilotSeen = true;
				if (checkBanDetection(prev, tier, s))
				{
					sendLog(F("BAN DETECTED: AP tier dropped"));
				}
			}
			return;
		}

		// TLSSC Restore: spoof DAS_autopilotConfig to SELF_DRIVING (0x331)
		// Source: hypery11/flipper-tesla-fsd, community research issue #18
		if (f.id == CAN_ID_DAS_AP_CONFIG)
		{
			if (!s.txPaused && s.apGateOpen() && handleTlssc(f, s))
			{
				driverSend(f, BUS_VEHICLE);
			}
			return;
		}

		// Firmware version decode (1.7)
		if (f.id == CAN_ID_GTW_VERSION && f.dlc >= 5)
		{
			decodeFwVersion(f, s);
			// Re-resolve platform when software version updates
			dispatchPlatform.resolveFromState(s);
			syncPlatformToState(dispatchPlatform, s);
			return;
		}

		// Vehicle-specific config decode (5.8) — piggyback on GTW_carConfig
		if (f.id == CAN_ID_GTW_CAR_CFG && f.dlc >= 3)
		{
			decodeVehicleConfig(f, s);
			// Re-resolve platform when vehicle model updates
			dispatchPlatform.resolveFromState(s);
			syncPlatformToState(dispatchPlatform, s);
		}
	}

	// Bus 2 (Body): no caching needed, body commands generate fresh frames
#endif
}
