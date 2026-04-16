import { commands, COMMAND_RANGES, VALID_VARIANTS } from '../src/commands.js';

describe('commands', () => {
  describe('system', () => {
    it('ping', () => expect(commands.ping()).toBe('ping'));
    it('status', () => expect(commands.status()).toBe('status'));
    it('variant', () => expect(commands.variant('hw3')).toBe('variant:hw3'));
    it('variant hw4', () => expect(commands.variant('hw4')).toBe('variant:hw4'));
    it('variant legacy', () => expect(commands.variant('legacy')).toBe('variant:legacy'));
    it('rejects invalid variant', () => {
      expect(() => commands.variant('hw5')).toThrow(RangeError);
      expect(() => commands.variant('')).toThrow(RangeError);
    });
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

  describe('range validation', () => {
    it('profile rejects below min', () => {
      expect(() => commands.profile(-1)).toThrow(RangeError);
    });
    it('profile rejects above max', () => {
      expect(() => commands.profile(5)).toThrow(RangeError);
    });
    it('profile rejects non-integer', () => {
      expect(() => commands.profile(1.5)).toThrow(RangeError);
    });
    it('profile accepts all valid values', () => {
      for (let i = COMMAND_RANGES.profile.min; i <= COMMAND_RANGES.profile.max; i++) {
        expect(commands.profile(i)).toBe(`profile:${i}`);
      }
    });

    it('offset rejects below min', () => {
      expect(() => commands.offset(-11)).toThrow(RangeError);
    });
    it('offset rejects above max', () => {
      expect(() => commands.offset(11)).toThrow(RangeError);
    });
    it('offset rejects non-integer', () => {
      expect(() => commands.offset(0.5)).toThrow(RangeError);
    });
    it('offset accepts boundary values', () => {
      expect(commands.offset(-10)).toBe('offset:-10');
      expect(commands.offset(0)).toBe('offset:0');
      expect(commands.offset(10)).toBe('offset:10');
    });

    it('seat rejects out of range', () => {
      expect(() => commands.seatFL(-1)).toThrow(RangeError);
      expect(() => commands.seatFL(4)).toThrow(RangeError);
      expect(() => commands.seatFR(99)).toThrow(RangeError);
      expect(() => commands.seatRL(-1)).toThrow(RangeError);
      expect(() => commands.seatRR(4)).toThrow(RangeError);
      expect(() => commands.seatRC(4)).toThrow(RangeError);
    });
    it('seat rejects non-integer', () => {
      expect(() => commands.seatFL(1.5)).toThrow(RangeError);
    });
    it('seat accepts all valid levels', () => {
      for (let i = COMMAND_RANGES.seat.min; i <= COMMAND_RANGES.seat.max; i++) {
        expect(commands.seatFL(i)).toBe(`seat:fl:${i}`);
        expect(commands.seatFR(i)).toBe(`seat:fr:${i}`);
        expect(commands.seatRL(i)).toBe(`seat:rl:${i}`);
        expect(commands.seatRR(i)).toBe(`seat:rr:${i}`);
        expect(commands.seatRC(i)).toBe(`seat:rc:${i}`);
      }
    });

    it('mainDisplay rejects out of range', () => {
      expect(() => commands.mainDisplay(-1)).toThrow(RangeError);
      expect(() => commands.mainDisplay(101)).toThrow(RangeError);
    });
    it('mainDisplay accepts boundary values', () => {
      expect(commands.mainDisplay(0)).toBe('maindisplay:0');
      expect(commands.mainDisplay(50)).toBe('maindisplay:50');
      expect(commands.mainDisplay(100)).toBe('maindisplay:100');
    });
  });
});
