#pragma once

/**
 * @file firmware/lib/transport/can/handler/bus/vehicle.h
 * @brief Bus 1 (Vehicle) frame handler for control-frame caching and telemetry decode
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "transport/can/bus.h"
#include "core/log/ring.h"
#include "transport/can/esp32.h"
#include "core/platform.h"
#include "vehicle/can/ids.h"
#include "vehicle/can/feature/telemetry/bms.h"
#include "vehicle/can/feature/fsd/nag.h"
#include "vehicle/can/feature/das/das_drive.h"
#include "vehicle/can/feature/telemetry/tpms.h"
#include "vehicle/can/feature/fsd/region.h"
#include "vehicle/can/feature/body/turn_signal.h"
#include "vehicle/can/feature/drive/drive_context.h"
#include "vehicle/can/feature/comfort/seatbelt.h"
#include "vehicle/can/feature/telemetry/powertrain.h"
#include "vehicle/can/feature/telemetry/motor_temps.h"
#include "vehicle/can/feature/telemetry/fw_compat.h"
#include "vehicle/can/feature/telemetry/vehicle_config.h"
#include "vehicle/can/feature/safety/ban_shield.h"
#include "vehicle/can/feature/safety/ban_detect.h"
#include "vehicle/can/feature/fsd/auto_lane_change.h"
#include "vehicle/can/feature/das/tlssc.h"
#include "../helpers.h"
#include "../filters.h"

/**
 * @brief Handle an incoming frame on the vehicle bus (X179 pins 9-10).
 *
 * Processes control-frame caching, BMS telemetry, DAS status with ALC,
 * blind-spot/turn signals, doors, EPAS nag-killer echo, TPMS, region
 * spoofing, drive mode, powertrain, motor temps, HW auto-detect,
 * GTW shield/ban detection, TLSSC restore, firmware version,
 * and vehicle config piggyback.
 *
 * @param f Reference to the received CAN frame.
 * @param s Reference to the shared vehicle state.
 */
inline void handleVehicleBus(Frame &f, State &s)
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

	// BMS high-voltage bus telemetry (read-only decode)
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

		// ALC auto-confirm: inject stalk/button when lane change is prompted
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

	// Turn signal status from VCFRONT lights (D-05 safety cues)
	if (f.id == CAN_ID_VCFRONT_LIGHTS && f.dlc >= 7)
	{
		s.turnSignalLeft = decodeTurnSignalLeftActive(f);
		s.turnSignalRight = decodeTurnSignalRightActive(f);
		return;
	}

	// Blind-spot monitoring levels (D-05 safety cues)
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

	// Enhanced BMS: degradation/capacity (mux=0) + energy status (mux=1)
	if (f.id == CAN_ID_BMS_ENERGY_ST && f.dlc >= 8)
	{
		uint8_t mux = f.data[0] & 0x0F; // Lower nibble selects sub-message
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
		uint8_t mux = f.data[0] & 0x0F; // Lower nibble selects sub-message
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

	// BMS precondition flags, HV state, and contactor status
	if (f.id == CAN_ID_BMS_STATUS && f.dlc >= 3)
	{
		s.bmsPrecondAllowed = decodeBmsPrecondAllowed(f.data);
		s.bmsHeatingWorthwhile = decodeBmsHeatingWorthwhile(f.data);
		s.bmsContactorState = decodeBmsContactorState(f.data);
		s.bmsHvState = decodeBmsHvState(f.data);
		s.hasEnhancedBms = true;
		return;
	}

	// BMS drive limits: bus voltage and current boundaries
	if (f.id == CAN_ID_BMS_DRIVE_LIM && f.dlc >= 8)
	{
		s.bmsMinBusVoltage = decodeBmsMinBusVoltage(f.data);
		s.bmsMaxBusVoltage = decodeBmsMaxBusVoltage(f.data);
		s.bmsMaxChargeCurrent = decodeBmsMaxChargeCurrent(f.data);
		s.bmsMaxDischargeCurrent = decodeBmsMaxDischargeCurrent(f.data);
		s.hasEnhancedBms = true;
		return;
	}

	// BMS lifetime energy counters (kWh discharged/charged)
	if (f.id == CAN_ID_BMS_KWH_CNT && f.dlc >= 8)
	{
		s.bmsKwhDischargeTotal = decodeBmsKwhDischargeTotal(f.data);
		s.bmsKwhChargeTotal = decodeBmsKwhChargeTotal(f.data);
		s.hasEnhancedBms = true;
		return;
	}

	// BMS multiplexed kWh counters (AC charge, DC charge, regen, drive discharge)
	if (f.id == CAN_ID_BMS_KWH_MUX && f.dlc >= 5)
	{
		uint8_t mux = f.data[0]; // Mux index selects counter type
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
		// EPAS_currentTuneMode = byte[0] bits[7:4]
		s.steeringMode = (f.data[0] >> 4) & 0x0F;
		s.hasSteeringMode = true;
		// Capture real handsOnLevel for organic mode driver bypass (byte[4] bits[7:6])
		s.nagOrganicRealHandsOn = (f.data[4] >> 6) & 0x03;

		if (!s.apGateOpen() || !nagModeUsesEpasEcho(s.nagMode))
			return;

		const unsigned long nowMs = millis();
		const NagMode mode = s.nagMode;

		// Organic/full mode: full DAS state machine driving byte 6 layout
		if (mode == NAG_MODE_ORGANIC || mode == NAG_MODE_FULL)
		{
			if (s.nagOrganicPrevState != s.dasHandsOnState)
			{
				nagOrganicOnStateChange(s, nowMs);
				s.nagOrganicPrevState = s.dasHandsOnState;
			}
			if (nagOrganicTick(s, nowMs))
			{
				Frame echo = f;
				nagOrganicApply(echo, s);
				driverSend(echo, BUS_VEHICLE);
				s.canDiag.nagEchoCount++;
			}
			return;
		}

		// Natural mode: Gaussian jitter with non-linear interval, byte 1 counter
		if (mode == NAG_MODE_NATURAL)
		{
			if (!nagFixedOrNaturalShouldEcho(s))
				return;
			if (!nagNaturalIntervalReady(s, nowMs))
				return;
			Frame echo = f;
			nagApplyNaturalTorque(echo, nagNaturalTorque(s.steeringAngle, s.dasHandsOnState));
			driverSend(echo, BUS_VEHICLE);
			s.canDiag.nagEchoCount++;
			return;
		}

		// Legacy/safe mode: zero torque echo with byte 1 counter
		if (nagFixedOrNaturalShouldEcho(s))
		{
			Frame echo = f;
			nagApplyZeroTorque(echo);
			driverSend(echo, BUS_VEHICLE);
			s.canDiag.nagEchoCount++;
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
		if (s.regionSpoofCode != 0 && s.apGateOpen())
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
		// Gear 1=Park, 0=Invalid, 5=Neutral treated as parked for AP gate
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

	// Rear inverter/stator/heatsink temperatures (all models)
	if (f.id == CAN_ID_REAR_INV_TEMPS && f.dlc >= 5)
	{
		s.rearInvTemp = decodeRearInvTemp(f.data);
		s.rearStatorTemp = decodeRearStatorTemp(f.data);
		s.rearHeatsinkTemp = decodeRearHeatsinkTemp(f.data);
		s.hasMotorTemps = true;
		return;
	}

	// Front inverter/stator/heatsink temperatures (dual-motor only)
	if (f.id == CAN_ID_FRONT_INV_TEMPS && f.dlc >= 5)
	{
		s.frontInvTemp = decodeFrontInvTemp(f.data);
		s.frontStatorTemp = decodeFrontStatorTemp(f.data);
		s.frontHeatsinkTemp = decodeFrontHeatsinkTemp(f.data);
		s.hasMotorTemps = true;
		return;
	}

	// Auto HW detection from GTW_carConfig das_hw field
	// das_hw: 0/1=Legacy (MCU2/HW1/HW2 retrofit), 2=HW3, 3=HW4
	if (f.id == CAN_ID_GTW_CAR_CFG && f.dlc >= 1)
	{
		// Reject all-zero 0x398 frames — Bus 6 noise on Juniper/Giga Shanghai HW4
		// An all-zero 8-byte 0x398 is never valid Tesla config data
		{
			bool allZero = true;
			for (uint8_t i = 0; i < f.dlc; i++)
			{
				if (f.data[i] != 0) { allZero = false; break; }
			}
			if (allZero) return;
		}

		uint8_t hw = (f.data[0] >> 6) & 0x03; // das_hw = byte[0] bits[7:6]
		s.detectedHW = hw;
		s.hwAutoDetected = true;

		// Auto-switch variant if enabled
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

	// GTW autopilot tier readback + GTW shield defense
	if (f.id == CAN_ID_GTW_CONFIG_ETH)
	{
		// GTW shield: learn snapshot or retransmit healthy frame when armed
		if (handleGtwShield(f, s))
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

	// TLSSC Restore: spoof DAS_autopilotConfig to SELF_DRIVING
	if (f.id == CAN_ID_DAS_AP_CONFIG)
	{
		if (s.apGateOpen() && handleTlssc(f, s))
		{
			driverSend(f, BUS_VEHICLE);
		}
		return;
	}

	// Firmware version decode
	if (f.id == CAN_ID_GTW_VERSION && f.dlc >= 5)
	{
		decodeFwVersion(f, s);
		// Re-resolve platform when software version updates
		dispatchPlatform.resolveFromState(s);
		syncPlatformToState(dispatchPlatform, s);
		return;
	}

	// Vehicle-specific config decode — piggyback on GTW_carConfig
	if (f.id == CAN_ID_GTW_CAR_CFG && f.dlc >= 3)
	{
		decodeVehicleConfig(f, s);
		// Re-resolve platform when vehicle model updates
		dispatchPlatform.resolveFromState(s);
		syncPlatformToState(dispatchPlatform, s);
	}
}
