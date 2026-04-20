/**
 * Gear Format Tests
 * Tests the formatGear helper function.
 */

import { formatGear } from '../../src/format.js';

describe('formatGear', () => {
  it('returns P for gear 1', () => expect(formatGear(1)).toBe('P'));
  it('returns R for gear 2', () => expect(formatGear(2)).toBe('R'));
  it('returns N for gear 3', () => expect(formatGear(3)).toBe('N'));
  it('returns D for gear 4', () => expect(formatGear(4)).toBe('D'));
  it('returns ? for gear 0', () => expect(formatGear(0)).toBe('?'));
  it('returns ? for gear 5', () => expect(formatGear(5)).toBe('?'));
  it('returns ? for negative', () => expect(formatGear(-1)).toBe('?'));
});

