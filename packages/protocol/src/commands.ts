/** Command builders for all TeslaCANModder firmware commands. */

/** Valid firmware variants. */
export const VALID_VARIANTS = ['hw3', 'hw4', 'legacy'] as const;
export type Variant = (typeof VALID_VARIANTS)[number];

/** Range limits for numeric command parameters. */
export const COMMAND_RANGES = {
  profile: { min: 0, max: 4 },
  offset: { min: -10, max: 10 },
  seat: { min: 0, max: 3 },
  mainDisplay: { min: 0, max: 100 },
} as const;

function assertRange(name: string, value: number, min: number, max: number): void {
  if (!Number.isInteger(value) || value < min || value > max) {
    throw new RangeError(`${name} must be an integer between ${min} and ${max}, got ${value}`);
  }
}

function assertVariant(v: string): asserts v is Variant {
  if (!(VALID_VARIANTS as readonly string[]).includes(v)) {
    throw new RangeError(`variant must be one of ${VALID_VARIANTS.join(', ')}, got "${v}"`);
  }
}

export const commands = {
  // System
  ping: () => 'ping',
  status: () => 'status',
  variant: (v: string) => { assertVariant(v); return `variant:${v}`; },

  // FSD
  fsd: (on: boolean) => on ? 'fsd:on' : 'fsd:off',
  fsdToggle: () => 'fsd:toggle',

  // Nag
  nag: (on: boolean) => on ? 'nag:on' : 'nag:off',
  nagToggle: () => 'nag:toggle',

  // Speed Profile
  profile: (p: number) => { assertRange('profile', p, COMMAND_RANGES.profile.min, COMMAND_RANGES.profile.max); return `profile:${p}`; },
  profileAuto: () => 'profile:auto',

  // Speed Offset (HW3)
  offset: (o: number) => { assertRange('offset', o, COMMAND_RANGES.offset.min, COMMAND_RANGES.offset.max); return `offset:${o}`; },
  offsetAuto: () => 'offset:auto',

  // ISA Chime (HW4)
  isaChime: (on: boolean) => on ? 'isa-chime:on' : 'isa-chime:off',
  isaChimeToggle: () => 'isa-chime:toggle',

  // Summon
  summonInject: (on: boolean) => on ? 'summon-inject:on' : 'summon-inject:off',
  summon: () => 'summon',
  summonForward: () => 'summon:forward',
  summonReverse: () => 'summon:reverse',
  summonStop: () => 'summon:stop',

  // Streaming
  stream: (on: boolean) => on ? 'stream:on' : 'stream:off',
  rawCan: (on: boolean) => on ? 'can:raw:on' : 'can:raw:off',

  // Mirror
  mirrorFold: () => 'mirror:fold',
  mirrorUnfold: () => 'mirror:unfold',
  mirrorHeat: () => 'mirror:heat',
  mirrorAutofold: () => 'mirror:autofold',
  mirrorDip: () => 'mirror:dip',

  // Lock
  lock: () => 'lock',
  unlock: () => 'unlock',
  lockChild: () => 'lock:child',
  horn: () => 'horn',

  // Trunk/Frunk
  frunkOpen: () => 'frunk:open',
  frunkClose: () => 'frunk:close',
  trunkOpen: () => 'trunk:open',
  trunkClose: () => 'trunk:close',
  glovebox: () => 'glovebox',

  // Lights
  lightFogFront: () => 'light:fog:front',
  lightFogRear: () => 'light:fog:rear',
  lightHighbeamAuto: () => 'light:highbeam:auto',
  lightAmbient: () => 'light:ambient',
  lightHome: () => 'light:home',
  lightDomeOff: () => 'light:dome:off',
  lightDomeOn: () => 'light:dome:on',
  lightDomeAuto: () => 'light:dome:auto',

  // Wiper
  wiperOff: () => 'wiper:off',
  wiper1: () => 'wiper:1',
  wiper2: () => 'wiper:2',
  wiper3: () => 'wiper:3',

  // Seat Heating
  seatFL: (level: number) => { assertRange('seatFL', level, COMMAND_RANGES.seat.min, COMMAND_RANGES.seat.max); return `seat:fl:${level}`; },
  seatFR: (level: number) => { assertRange('seatFR', level, COMMAND_RANGES.seat.min, COMMAND_RANGES.seat.max); return `seat:fr:${level}`; },
  seatRL: (level: number) => { assertRange('seatRL', level, COMMAND_RANGES.seat.min, COMMAND_RANGES.seat.max); return `seat:rl:${level}`; },
  seatRR: (level: number) => { assertRange('seatRR', level, COMMAND_RANGES.seat.min, COMMAND_RANGES.seat.max); return `seat:rr:${level}`; },
  seatRC: (level: number) => { assertRange('seatRC', level, COMMAND_RANGES.seat.min, COMMAND_RANGES.seat.max); return `seat:rc:${level}`; },

  // Display
  mainDisplay: (level: number) => { assertRange('mainDisplay', level, COMMAND_RANGES.mainDisplay.min, COMMAND_RANGES.mainDisplay.max); return `maindisplay:${level}`; },

  // Power
  powerAccOn: () => 'power:acc:on',
  powerAccOff: () => 'power:acc:off',
  powerOff: () => 'power:off',
  powerReady: () => 'power:ready',

  // Window
  ventOpen: () => 'vent:open',
  ventClose: () => 'vent:close',

  // Sentry
  sentryOn: () => 'sentry:on',
  sentryOff: () => 'sentry:off',

  // Climate
  climateKeep: () => 'climate:keep',
  climateOff: () => 'climate:off',

  // Charge
  chargeStart: () => 'charge:start',
  chargeStop: () => 'charge:stop',
  chargePort: () => 'chargeport',

  // Drive Config
  pedalStandard: () => 'pedal:standard',
  pedalChill: () => 'pedal:chill',
  pedalSport: () => 'pedal:sport',
  regenOff: () => 'regen:off',
  regenLow: () => 'regen:low',
  regenStd: () => 'regen:std',
  regenMax: () => 'regen:max',
  stopCreep: () => 'stop:creep',
  stopRoll: () => 'stop:roll',
  stopHold: () => 'stop:hold',
} as const;

/** Profile label map */
export const PROFILE_LABELS: Record<number, string> = {
  0: 'Chill',
  1: 'Normal',
  2: 'Hurry',
  3: 'Max',
  4: 'Sloth',
};
