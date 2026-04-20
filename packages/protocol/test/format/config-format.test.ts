/**
 * Config Format Tests
 * Tests formatFwCompat and formatVehicleModel helpers.
 */

import { formatFwCompat, formatVehicleModel } from '../../src/format.js';

describe('formatFwCompat', () => {
  it('returns UNKNOWN for 0', () => expect(formatFwCompat(0)).toBe('UNKNOWN'));
  it('returns OK for 1', () => expect(formatFwCompat(1)).toBe('OK'));
  it('returns WARN for 2', () => expect(formatFwCompat(2)).toBe('WARN'));
  it('returns FAIL for 3', () => expect(formatFwCompat(3)).toBe('FAIL'));
  it('returns LEVEL_N for unknown', () => expect(formatFwCompat(99)).toBe('LEVEL_99'));
});

describe('formatVehicleModel', () => {
  it('returns UNKNOWN for 0', () => expect(formatVehicleModel(0)).toBe('UNKNOWN'));
  it('returns Model 3 for 1', () => expect(formatVehicleModel(1)).toBe('Model 3'));
  it('returns Model Y for 2', () => expect(formatVehicleModel(2)).toBe('Model Y'));
  it('returns Model S for 3', () => expect(formatVehicleModel(3)).toBe('Model S'));
  it('returns Model X for 4', () => expect(formatVehicleModel(4)).toBe('Model X'));
  it('returns Cybertruck for 5', () => expect(formatVehicleModel(5)).toBe('Cybertruck'));
  it('returns MODEL_N for unknown', () => expect(formatVehicleModel(42)).toBe('MODEL_42'));
});

