// @teslacanmodder/protocol — barrel export

// Types
export type {
  BoardFeatures,
  BootMessage,
  StatusMessage,
  FrameMessage,
  AckMessage,
  ErrorMessage,
  LogMessage,
  PongMessage,
  BoardMessage,
  CanFrame,
  ConsoleMessage,
  BoardState,
  TransportEvents,
  Transport,
  ScannedDevice,
  ParsedEvent,
} from './types.js';

// Commands
export { commands, PROFILE_LABELS } from './commands.js';

// Decoder
export {
  buildDecoderIndex,
  describeDecodedFrame,
  getCanIdLabel,
  KNOWN_CAN_IDS,
} from './decoder.js';
export type {
  DecoderSignal,
  DecoderFrame,
  DecoderDataset,
  DecoderIndex,
  DecodedEntry,
} from './decoder.js';

// Parser
export { parseSerialLine, parseSerialChunk } from './parser.js';
