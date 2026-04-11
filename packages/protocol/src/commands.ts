/** Command builders for all TeslaCANModder firmware commands. */

export const commands = {
  // System
  ping: () => 'ping',
  status: () => 'status',
  variant: (v: string) => `variant:${v}`,

  // FSD
  fsd: (on: boolean) => on ? 'fsd:on' : 'fsd:off',
  fsdToggle: () => 'fsd:toggle',

  // Nag
  nag: (on: boolean) => on ? 'nag:on' : 'nag:off',
  nagToggle: () => 'nag:toggle',

  // Speed Profile
  profile: (p: number) => `profile:${p}`,
  profileAuto: () => 'profile:auto',

  // Speed Offset (HW3)
  offset: (o: number) => `offset:${o}`,
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
  seatFL: (level: number) => `seat:fl:${level}`,
  seatFR: (level: number) => `seat:fr:${level}`,
  seatRL: (level: number) => `seat:rl:${level}`,
  seatRR: (level: number) => `seat:rr:${level}`,
  seatRC: (level: number) => `seat:rc:${level}`,

  // Display
  mainDisplay: (level: number) => `maindisplay:${level}`,

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
