/** Shared formatting helpers for board state display. */

export function formatAutopilotTier(tier: number): string {
  switch (tier) {
    case 0: return 'NONE';
    case 1: return 'HIGHWAY';
    case 2: return 'ENHANCED';
    case 3: return 'SELF_DRIVING';
    case 4: return 'BASIC';
    default: return 'UNKNOWN';
  }
}

export function formatSteeringMode(mode: number): string {
  switch (mode) {
    case 0: return 'FAIL_SAFE';
    case 1: return 'COMFORT';
    case 2: return 'STANDARD';
    case 3: return 'SPORT';
    case 4: return 'RWD_COMFORT';
    case 5: return 'RWD_STANDARD';
    case 6: return 'RWD_SPORT';
    default: return `MODE_${mode}`;
  }
}

export function formatUptime(ms?: number): string {
  if (!ms) return '\u2014';
  const s = Math.floor(ms / 1000);
  const m = Math.floor(s / 60);
  const h = Math.floor(m / 60);
  if (h > 0) return `${h}h ${m % 60}m`;
  if (m > 0) return `${m}m ${s % 60}s`;
  return `${s}s`;
}

export function formatDriveMode(mode: number): string {
  switch (mode) {
    case 0: return 'OFF';
    case 1: return 'CHILL';
    case 2: return 'STANDARD';
    case 3: return 'PERFORMANCE';
    default: return 'UNKNOWN';
  }
}

export function formatRegion(code: number): string {
  switch (code) {
    case 0: return 'UNKNOWN';
    case 1: return 'NA';
    case 2: return 'EU';
    case 3: return 'CN';
    case 4: return 'APAC';
    case 5: return 'ME';
    default: return 'UNKNOWN';
  }
}

export function formatPressureBar(bar: number): string {
  return `${bar.toFixed(2)} bar`;
}

export function formatPressurePsi(bar: number): string {
  return `${(bar * 14.5038).toFixed(1)} psi`;
}

export function formatGear(gear: number): string {
  switch (gear) {
    case 1: return 'P';
    case 2: return 'R';
    case 3: return 'N';
    case 4: return 'D';
    default: return '?';
  }
}

export function formatFwCompat(level: number): string {
  switch (level) {
    case 0: return 'UNKNOWN';
    case 1: return 'OK';
    case 2: return 'WARN';
    case 3: return 'FAIL';
    default: return `LEVEL_${level}`;
  }
}

export function formatVehicleModel(model: number): string {
  switch (model) {
    case 0: return 'UNKNOWN';
    case 1: return 'Model 3';
    case 2: return 'Model Y';
    case 3: return 'Model S';
    case 4: return 'Model X';
    case 5: return 'Cybertruck';
    default: return `MODEL_${model}`;
  }
}
