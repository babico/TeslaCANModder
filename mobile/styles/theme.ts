/** Shared dark theme — matches the web app's color scheme. */

export const colors = {
  bg: '#0a0c10',
  bgSecondary: '#14171f',
  bgTertiary: '#1a1d27',
  surface: '#14171f',
  surfaceHover: '#1a1d27',
  border: '#2a2e3a',
  borderLight: '#22252e',
  text: '#e4e6eb',
  textMuted: '#9ca3af',
  textDim: '#6b7280',

  accent: '#e82127',
  accentHover: '#c91d22',
  accentSoft: 'rgba(232, 33, 39, 0.1)',

  success: '#34d399',
  successSoft: 'rgba(52, 211, 153, 0.1)',
  warning: '#f59e0b',
  warningSoft: 'rgba(245, 158, 11, 0.1)',
  error: '#ef4444',
  errorSoft: 'rgba(239, 68, 68, 0.1)',

  blue: '#3b82f6',
  blueSoft: 'rgba(59, 130, 246, 0.1)',
} as const;

export const spacing = {
  xs: 4,
  sm: 8,
  md: 16,
  lg: 24,
  xl: 32,
  xxl: 48,
} as const;

export const radius = {
  sm: 4,
  md: 8,
  lg: 12,
  xl: 16,
  full: 9999,
} as const;

export const fonts = {
  mono: 'monospace',
} as const;

export const typography = {
  h1: { fontSize: 24, fontWeight: '700' as const, color: colors.text },
  h2: { fontSize: 20, fontWeight: '600' as const, color: colors.text },
  h3: { fontSize: 16, fontWeight: '600' as const, color: colors.text },
  body: { fontSize: 14, fontWeight: '400' as const, color: colors.text },
  caption: { fontSize: 12, fontWeight: '400' as const, color: colors.textMuted },
  label: { fontSize: 11, fontWeight: '600' as const, color: colors.textMuted },
  mono: { fontSize: 12, fontFamily: 'monospace' as const, color: colors.text },
} as const;

export const shadows = {
  sm: {
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 1 },
    shadowOpacity: 0.2,
    shadowRadius: 2,
    elevation: 2,
  },
  md: {
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.25,
    shadowRadius: 4,
    elevation: 4,
  },
  lg: {
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 4 },
    shadowOpacity: 0.3,
    shadowRadius: 8,
    elevation: 8,
  },
} as const;

export const touchMin = 44;

export const animation = {
  fast: 150,
  normal: 250,
  slow: 400,
} as const;
