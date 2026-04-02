import { BOARD_COMMANDS, BoardCommands } from '../../board/commands';

export const BASE_COMMANDS = {
  ping: BOARD_COMMANDS.ping,
  status: BOARD_COMMANDS.status,
  streamOn: BOARD_COMMANDS.streamOn,
  streamOff: BOARD_COMMANDS.streamOff,
  variantHw4: BOARD_COMMANDS.variantHw4,
  variantHw3: BOARD_COMMANDS.variantHw3,
  variantLegacy: BOARD_COMMANDS.variantLegacy,
};

export function getStreamingCommand(nextEnabled) {
  return BoardCommands.stream(nextEnabled);
}

export function getVariantCommand(variant) {
  return BoardCommands.variant(variant);
}
