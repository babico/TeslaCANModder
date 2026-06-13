// @teslacanmodder/protocol — barrel export

// Types
export type {
	BoardFeatures,
	BootMessage,
	StatusMessage,
	PlatformMessage,
	StatusLiveMessage,
	StatusMetaMessage,
	StatusFeaturesMessage,
	StatusCanMessage,
	StatusStateMessage,
	StatusCompactMessage,
	FrameMessage,
	AckMessage,
	ErrorMessage,
	LogMessage,
	PongMessage,
	BmsMessage,
	TpmsMessage,
	PowertrainMessage,
	FwCompatMessage,
	VehicleConfigMessage,
	BoardMessage,
	CanFrame,
	ConsoleMessage,
	BoardState,
	TransportEvents,
	Transport,
	ScannedDevice,
	ParsedEvent,
} from "./types.js";

// Commands
export {
	commands,
	PROFILE_LABELS,
	COMMAND_RANGES,
	VALID_VARIANTS,
	lhdOn,
	lhdOff,
	apFirstOn,
	apFirstOff,
	laneGraphOn,
	laneGraphOff,
	assistDevOn,
	assistDevOff,
	assistNavOn,
	assistNavOff,
	assistHofOn,
	assistHofOff,
	assistTelOn,
	assistTelOff,
	tlsscOn,
	tlsscOff,
	evdOn,
	evdOff,
	dasArm,
	dasDisarm,
	dasStatus,
	teslaKeyGen,
	teslaKeyShow,
	teslaKeyRoleOwner,
	teslaKeyRoleChargingManager,
	teslaKeySend,
	teslaWake,
	teslaChargeStart,
	teslaChargeStop,
	teslaClimateOn,
	teslaClimateOff,
	pedalStd,
	regenStandard,
	frunk,
} from "./commands.js";
export type { Variant, NagMode, Command } from "./commands.js";

// Feature settings specs
export {
	FEATURE_IDS,
	FEATURE_SETTINGS_BY_ID,
	ALL_FEATURE_SETTINGS_SPECS,
	getFeatureSettingsSpecById,
} from "./featureSettings.js";
export type {
	FeatureId,
	CommandBuilderName,
	FeatureSettingControlType,
	FeatureSettingSpec,
	FeatureSpecKind,
	FeatureSettingsSpec,
} from "./featureSettings.js";

// Command gating
export { getCommandGate } from "./gating.js";
export type { CommandGate, CommandName } from "./gating.js";

// Decoder
export {
	buildDecoderIndex,
	describeDecodedFrame,
	getCanIdLabel,
	KNOWN_CAN_IDS,
} from "./decoder.js";
export type {
	DecoderSignal,
	DecoderFrame,
	DecoderDataset,
	DecoderIndex,
	DecodedEntry,
} from "./decoder.js";

// Reducer
export {
	initialBoardState,
	reduceBoardMessage,
	addNotification,
	detectBoard,
	BUS_NAMES,
} from "./reducer.js";

// Format helpers
export {
	formatAutopilotTier,
	formatSteeringMode,
	formatUptime,
	formatDriveMode,
	formatRegion,
	formatPressureBar,
	formatPressurePsi,
	formatGear,
	formatFwCompat,
	formatVehicleModel,
} from "./format.js";

// Selectors
export {
	selectConnectionSummary,
	selectDriveSnapshot,
	selectChargeSnapshot,
	selectAutopilotIndicatorState,
} from "./selectors.js";
export type {
	ConnectionSummary,
	DriveSnapshot,
	ChargeSnapshot,
	ApClusterState,
	IndicatorVariant,
	AutopilotIndicatorState,
} from "./selectors.js";

// Transport lifecycle
export {
	initialTransportLifecycleState,
	resolveTransportSelection,
	reduceTransportLifecycle,
} from "./transportLifecycle.js";
export type {
	TransportKind,
	TransportCapabilities,
	TransportSelection,
	TransportLifecycleState,
	TransportLifecycleEvent,
} from "./transportLifecycle.js";

// Parser
export { parseSerialLine, parseSerialChunk } from "./parser.js";
