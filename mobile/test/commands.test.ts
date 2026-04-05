import { commands } from '../lib/protocol/commands';

describe('commands', () => {
  describe('system', () => {
    it('ping', () => expect(commands.ping()).toBe('ping'));
    it('status', () => expect(commands.status()).toBe('status'));
    it('variant', () => expect(commands.variant('hw3')).toBe('variant:hw3'));
  });

  describe('FSD', () => {
    it('fsd on', () => expect(commands.fsd(true)).toBe('fsd:on'));
    it('fsd off', () => expect(commands.fsd(false)).toBe('fsd:off'));
    it('fsdToggle', () => expect(commands.fsdToggle()).toBe('fsd:toggle'));
  });

  describe('Nag', () => {
    it('nag on', () => expect(commands.nag(true)).toBe('nag:on'));
    it('nag off', () => expect(commands.nag(false)).toBe('nag:off'));
    it('nagToggle', () => expect(commands.nagToggle()).toBe('nag:toggle'));
  });

  describe('Speed profile', () => {
    it('profile 0', () => expect(commands.profile(0)).toBe('profile:0'));
    it('profile 4', () => expect(commands.profile(4)).toBe('profile:4'));
    it('profileAuto', () => expect(commands.profileAuto()).toBe('profile:auto'));
  });

  describe('Speed offset', () => {
    it('positive', () => expect(commands.offset(5)).toBe('offset:5'));
    it('negative', () => expect(commands.offset(-10)).toBe('offset:-10'));
    it('zero', () => expect(commands.offset(0)).toBe('offset:0'));
    it('offsetAuto', () => expect(commands.offsetAuto()).toBe('offset:auto'));
  });

  describe('ISA chime', () => {
    it('on', () => expect(commands.isaChime(true)).toBe('isa-chime:on'));
    it('off', () => expect(commands.isaChime(false)).toBe('isa-chime:off'));
    it('toggle', () => expect(commands.isaChimeToggle()).toBe('isa-chime:toggle'));
  });

  describe('Summon', () => {
    it('summon', () => expect(commands.summon()).toBe('summon'));
    it('forward', () => expect(commands.summonForward()).toBe('summon:forward'));
    it('reverse', () => expect(commands.summonReverse()).toBe('summon:reverse'));
    it('stop', () => expect(commands.summonStop()).toBe('summon:stop'));
  });

  describe('Streaming', () => {
    it('stream on', () => expect(commands.stream(true)).toBe('stream:on'));
    it('stream off', () => expect(commands.stream(false)).toBe('stream:off'));
    it('rawCan on', () => expect(commands.rawCan(true)).toBe('can:raw:on'));
    it('rawCan off', () => expect(commands.rawCan(false)).toBe('can:raw:off'));
  });

  describe('Mirror', () => {
    it('fold', () => expect(commands.mirrorFold()).toBe('mirror:fold'));
    it('unfold', () => expect(commands.mirrorUnfold()).toBe('mirror:unfold'));
  });

  describe('Lock', () => {
    it('lock', () => expect(commands.lock()).toBe('lock'));
    it('unlock', () => expect(commands.unlock()).toBe('unlock'));
    it('childLock', () => expect(commands.lockChild()).toBe('lock:child'));
    it('horn', () => expect(commands.horn()).toBe('horn'));
  });

  describe('Trunk/Frunk', () => {
    it('frunkOpen', () => expect(commands.frunkOpen()).toBe('frunk:open'));
    it('trunkOpen', () => expect(commands.trunkOpen()).toBe('trunk:open'));
    it('trunkClose', () => expect(commands.trunkClose()).toBe('trunk:close'));
    it('glovebox', () => expect(commands.glovebox()).toBe('glovebox'));
  });

  describe('Lights', () => {
    it('fogFront', () => expect(commands.lightFogFront()).toBe('light:fog:front'));
    it('domeOn', () => expect(commands.lightDomeOn()).toBe('light:dome:on'));
    it('domeAuto', () => expect(commands.lightDomeAuto()).toBe('light:dome:auto'));
  });

  describe('Wiper', () => {
    it('off', () => expect(commands.wiperOff()).toBe('wiper:off'));
    it('1', () => expect(commands.wiper1()).toBe('wiper:1'));
    it('3', () => expect(commands.wiper3()).toBe('wiper:3'));
  });

  describe('Seats', () => {
    it('seatFL', () => expect(commands.seatFL(3)).toBe('seat:fl:3'));
    it('seatFR', () => expect(commands.seatFR(0)).toBe('seat:fr:0'));
    it('seatRL', () => expect(commands.seatRL(2)).toBe('seat:rl:2'));
  });

  describe('Power', () => {
    it('accOn', () => expect(commands.powerAccOn()).toBe('power:acc:on'));
    it('accOff', () => expect(commands.powerAccOff()).toBe('power:acc:off'));
    it('off', () => expect(commands.powerOff()).toBe('power:off'));
    it('ready', () => expect(commands.powerReady()).toBe('power:ready'));
  });

  describe('Windows/Sentry', () => {
    it('ventOpen', () => expect(commands.ventOpen()).toBe('vent:open'));
    it('ventClose', () => expect(commands.ventClose()).toBe('vent:close'));
    it('sentryOn', () => expect(commands.sentryOn()).toBe('sentry:on'));
    it('sentryOff', () => expect(commands.sentryOff()).toBe('sentry:off'));
  });

  describe('Climate', () => {
    it('keep', () => expect(commands.climateKeep()).toBe('climate:keep'));
    it('off', () => expect(commands.climateOff()).toBe('climate:off'));
  });

  describe('Charge', () => {
    it('start', () => expect(commands.chargeStart()).toBe('charge:start'));
    it('stop', () => expect(commands.chargeStop()).toBe('charge:stop'));
    it('port', () => expect(commands.chargePort()).toBe('chargeport'));
  });

  describe('Drive config', () => {
    it('pedalSport', () => expect(commands.pedalSport()).toBe('pedal:sport'));
    it('regenMax', () => expect(commands.regenMax()).toBe('regen:max'));
    it('stopHold', () => expect(commands.stopHold()).toBe('stop:hold'));
  });

  describe('Display', () => {
    it('mainDisplay', () => expect(commands.mainDisplay(50)).toBe('maindisplay:50'));
  });
});
