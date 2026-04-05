/** Shared dark theme — matches the web app's color scheme. */

export const colors = {
  bg: '#0a0c10',
  bgSecondary: '#14171f',
  bgTertiary: '#1a1d27',
  surface: '#14171f',
  surfaceHover: '#1a1d27',
  border: '#2a2e3a',
  text: '#e4e6eb',
  textMuted: '#9ca3af',
  textDim: '#6b7280',

  accent: '#e82127',
  accentHover: '#c91d22',
  accentSoft: 'rgba(232, 33, 39, 0.1)',

  success: '#34d399',
  successSoft: 'rgba(52, 211, 153, 0.1)',
  warning: '#f59e0b',
  error: '#ef4444',

  blue: '#3b82f6',
} as const;

export const spacing = {
  xs: 4,
  sm: 8,
  md: 16,
  lg: 24,
  xl: 32,
} as const;

export const radius = {
  sm: 4,
  md: 8,
  lg: 12,
} as const;

export const fonts = {
  mono: 'monospace',
} as const;

export const touchMin = 44;
