import { getCommandGate } from '../../src/gating.js';
import { initialBoardState } from '../../src/reducer.js';

describe('gating: getCommandGate', () => {
  it('allows commands with no explicit gate rules', () => {
    const gate = getCommandGate('ping', initialBoardState);

    expect(gate.available).toBe(true);
    expect(gate.reason).toBeNull();
  });

  it('blocks chassis-gated commands when chassis bus is offline', () => {
    const gate = getCommandGate('trackMode', {
      ...initialBoardState,
      chassisOnline: false,
    });

    expect(gate.available).toBe(false);
    expect(gate.reason).toBe('Chassis CAN bus not available');
  });

  it('blocks feature-gated commands when firmware capability is missing', () => {
    const gate = getCommandGate('profile', {
      ...initialBoardState,
      chassisOnline: true,
      features: {
        ...initialBoardState.features,
        profile: false,
      },
    });

    expect(gate.available).toBe(false);
    expect(gate.reason).toBe('Speed Profile feature not supported by this firmware');
  });

  it('blocks state-gated commands when capability flag is disabled', () => {
    const gate = getCommandGate('tpms', {
      ...initialBoardState,
      hasTpms: false,
    });

    expect(gate.available).toBe(false);
    expect(gate.reason).toBe('TPMS hardware not available or not enabled');
  });

  it('returns available when all gating requirements pass', () => {
    const gate = getCommandGate('mirrorAutoFold', {
      ...initialBoardState,
      bodyOnline: true,
      mirrorAutoFold: true,
    });

    expect(gate.available).toBe(true);
    expect(gate.reason).toBeNull();
  });

  it('reports first failing requirement for multi-rule commands', () => {
    const gate = getCommandGate('canSimStart', {
      ...initialBoardState,
      chassisOnline: false,
      canSim: false,
    });

    expect(gate.available).toBe(false);
    expect(gate.reason).toBe('Chassis CAN bus not available');
  });
});
