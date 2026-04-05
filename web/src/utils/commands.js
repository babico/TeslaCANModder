export const commands = {
  ping: () => 'ping',
  status: () => 'status',
  
  variant: (v) => `variant:${v}`,
  
  fsd: (enabled) => enabled ? 'fsd:on' : 'fsd:off',
  fsdToggle: () => 'fsd:toggle',
  
  nag: (enabled) => enabled ? 'nag:on' : 'nag:off',
  nagToggle: () => 'nag:toggle',
  
  profile: (p) => `profile:${p}`,
  profileAuto: () => 'profile:auto',

  offset: (o) => `offset:${o}`,
  offsetAuto: () => 'offset:auto',
  
  isaChime: (enabled) => enabled ? 'isa-chime:on' : 'isa-chime:off',
  isaChimeToggle: () => 'isa-chime:toggle',

  summon: () => 'summon',
  summonForward: () => 'summon:forward',
  summonReverse: () => 'summon:reverse',
  summonStop: () => 'summon:stop',
  
  stream: (enabled) => enabled ? 'stream:on' : 'stream:off',
  
  rawCan: (enabled) => enabled ? 'can:raw:on' : 'can:raw:off',

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
  seatFL: (level) => `seat:fl:${level}`,
  seatFR: (level) => `seat:fr:${level}`,
  seatRL: (level) => `seat:rl:${level}`,
  seatRR: (level) => `seat:rr:${level}`,
  seatRC: (level) => `seat:rc:${level}`,

  // Display
  mainDisplay: (level) => `maindisplay:${level}`,

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
  pedalStd: () => 'pedal:std',
  pedalChill: () => 'pedal:chill',
  pedalSport: () => 'pedal:sport',
  regenOff: () => 'regen:off',
  regenLow: () => 'regen:low',
  regenStandard: () => 'regen:standard',
  regenStd: () => 'regen:std',
  regenMax: () => 'regen:max',
  stopCreep: () => 'stop:creep',
  stopRoll: () => 'stop:roll',
  stopHold: () => 'stop:hold',
};
