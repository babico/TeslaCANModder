export const BOARD_VARIANTS = [
  { id: 'hw4', label: 'HW4' },
  { id: 'hw3', label: 'HW3' },
  { id: 'legacy', label: 'Legacy' },
];

export class BoardCommands {
  static ping() {
    return 'ping';
  }

  static status() {
    return 'status';
  }

  static stream(nextEnabled) {
    return nextEnabled ? 'stream:on' : 'stream:off';
  }

  static fsd(nextEnabled) {
    return nextEnabled ? 'fsd:on' : 'fsd:off';
  }

  static toggleFsd() {
    return 'fsd:toggle';
  }

  static profile(profile) {
    return `profile:${profile}`;
  }

  static offset(offset) {
    return `offset:${offset}`;
  }

  static isaChime(nextEnabled) {
    return nextEnabled ? 'isa-chime:on' : 'isa-chime:off';
  }

  static toggleIsaChime() {
    return 'isa-chime:toggle';
  }

  static variant(variant) {
    return `variant:${variant}`;
  }
}

export const BOARD_COMMANDS = {
  ping: BoardCommands.ping(),
  status: BoardCommands.status(),
  streamOn: BoardCommands.stream(true),
  streamOff: BoardCommands.stream(false),
  fsdOn: BoardCommands.fsd(true),
  fsdOff: BoardCommands.fsd(false),
  fsdToggle: BoardCommands.toggleFsd(),
  isaChimeOn: BoardCommands.isaChime(true),
  isaChimeOff: BoardCommands.isaChime(false),
  variantHw4: BoardCommands.variant('hw4'),
  variantHw3: BoardCommands.variant('hw3'),
  variantLegacy: BoardCommands.variant('legacy'),
};
