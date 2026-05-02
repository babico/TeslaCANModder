/**
 * Design token dictionary for TeslaCANModder unified client.
 *
 * Strict shadcn semantic parity, adapted for React Native (no CSS vars).
 * All color values are hex strings; spacing/radius values are numbers (dp/px).
 *
 * Token tiers:
 *  1. Primitive palette — raw values, not consumed directly by components
 *  2. Semantic tokens — role-named aliases consumed by components
 *  3. Dashboard-layer tokens — density, alarm, and data-specific overrides
 */

// ── 1. Primitive Palette ─────────────────────────────────────────────────────

const palette = {
	// Neutral
	white: "#ffffff",
	black: "#000000",
	slate50: "#f8fafc",
	slate100: "#f1f5f9",
	slate200: "#e2e8f0",
	slate300: "#cbd5e1",
	slate400: "#94a3b8",
	slate500: "#64748b",
	slate600: "#475569",
	slate700: "#334155",
	slate800: "#1e293b",
	slate900: "#0f172a",
	slate950: "#020617",

	// Brand blue
	blue50: "#eff6ff",
	blue100: "#dbeafe",
	blue200: "#bfdbfe",
	blue300: "#93c5fd",
	blue400: "#60a5fa",
	blue500: "#3b82f6",
	blue600: "#2563eb",
	blue700: "#1d4ed8",
	blue800: "#1e40af",
	blue900: "#1e3a8a",

	// Cyan (primary accent)
	cyan50: "#ecfeff",
	cyan100: "#cffafe",
	cyan200: "#a5f3fc",
	cyan300: "#67e8f9",
	cyan400: "#22d3ee",
	cyan500: "#06b6d4",
	cyan600: "#0891b2",
	cyan700: "#0e7490",
	cyan800: "#155e75",
	cyan900: "#164e63",

	// Green (success / on state)
	green50: "#f0fdf4",
	green100: "#dcfce7",
	green200: "#bbf7d0",
	green500: "#22c55e",
	green600: "#16a34a",
	green700: "#15803d",
	green900: "#14532d",

	// Yellow/Amber (warning)
	amber50: "#fffbeb",
	amber100: "#fef3c7",
	amber200: "#fde68a",
	amber400: "#fbbf24",
	amber500: "#f59e0b",
	amber600: "#d97706",
	amber900: "#78350f",

	// Red (destructive / error)
	red50: "#fef2f2",
	red100: "#fee2e2",
	red400: "#f87171",
	red500: "#ef4444",
	red600: "#dc2626",
	red700: "#b91c1c",
	red900: "#7f1d1d",

	// Dashboard darks
	navy900: "#0a1628",
	navy850: "#0d1e35",
	navy800: "#102243",
	navy700: "#16305a",
	navy600: "#1e4270",
	navy500: "#2a5a8e",
	navy400: "#3e7ab5",
} as const;

// ── 2. Semantic Tokens ───────────────────────────────────────────────────────

export const colors = {
	// Backgrounds
	background: palette.slate50,
	backgroundCard: palette.white,
	backgroundSubtle: palette.slate100,
	backgroundDark: palette.navy900,
	backgroundDarkCard: palette.navy850,
	backgroundDarkSubtle: palette.navy800,

	// Foregrounds
	foreground: palette.slate900,
	foregroundMuted: palette.slate500,
	foregroundSubtle: palette.slate400,
	foregroundInverse: palette.white,
	foregroundDark: palette.white,
	foregroundDarkMuted: "#8baec8",
	foregroundDarkSubtle: "#5c7ea0",

	// Primary (cyan/action)
	primary: palette.cyan600,
	primaryHover: palette.cyan700,
	primaryForeground: palette.white,
	primarySubtle: palette.cyan50,
	primaryBorder: palette.cyan200,

	// Secondary
	secondary: palette.slate200,
	secondaryHover: palette.slate300,
	secondaryForeground: palette.slate800,

	// Border
	border: palette.slate200,
	borderStrong: palette.slate300,
	borderFocus: palette.cyan500,

	// Semantic states
	success: palette.green600,
	successSubtle: palette.green50,
	successBorder: palette.green200,
	successForeground: palette.white,

	warning: palette.amber500,
	warningSubtle: palette.amber50,
	warningBorder: palette.amber200,
	warningForeground: palette.amber900,

	destructive: palette.red600,
	destructiveSubtle: palette.red50,
	destructiveBorder: palette.red100,
	destructiveForeground: palette.white,

	// Status rail colors (persistent connection/health strip)
	statusConnected: palette.green500,
	statusConnecting: palette.amber400,
	statusDisconnected: palette.slate400,
	statusError: palette.red500,

	// Muted/disabled
	muted: palette.slate100,
	mutedForeground: palette.slate400,
	disabled: palette.slate200,
	disabledForeground: palette.slate400,

	// Dashboard data density layer
	dashBackground: palette.navy900,
	dashCard: palette.navy850,
	dashCardBorder: palette.navy700,
	dashPrimary: palette.cyan400,
	dashSecondary: "#8baec8",
	dashMuted: "#4a6d8d",
	dashValue: palette.white,
	dashLabel: "#8baec8",
	dashUnit: "#5c7ea0",

	// Alarm states (dashboard)
	alarmWarning: palette.amber400,
	alarmCritical: palette.red500,
	alarmPulse: palette.red400,

	// Turn signal / BSM
	signalActive: "#fbbf24",
	bsmActive: "#f59e0b",
	bsmWarning: palette.red500,

	// Autopilot tiers
	apInactive: palette.slate500,
	apActive: palette.cyan400,
	apHandsWarning: palette.amber400,
	apUnavailable: palette.slate300,

	// Powertrain gauge
	powerPositive: palette.green500,
	powerNegative: palette.cyan400,
	powerPeak: palette.red400,

	// Gear indicator
	gearActive: palette.white,
	gearInactive: "#3e5a78",
	gearReverse: palette.amber400,
	gearPark: palette.green500,
} as const;

// ── Spacing ───────────────────────────────────────────────────────────────────

export const spacing = {
	/** 2dp */ xs2: 2,
	/** 4dp */ xs: 4,
	/** 6dp */ sm2: 6,
	/** 8dp */ sm: 8,
	/** 10dp */ md2: 10,
	/** 12dp */ md: 12,
	/** 14dp */ md3: 14,
	/** 16dp */ lg: 16,
	/** 20dp */ lg2: 20,
	/** 24dp */ xl: 24,
	/** 28dp */ xl2: 28,
	/** 32dp */ xl3: 32,
	/** 40dp */ xl4: 40,
	/** 48dp */ xl5: 48,
	/** 64dp */ xl6: 64,
} as const;

// ── Border radius ─────────────────────────────────────────────────────────────

export const radius = {
	/** 4dp  */ xs: 4,
	/** 6dp  */ sm: 6,
	/** 8dp  */ md: 8,
	/** 10dp */ md2: 10,
	/** 12dp */ lg: 12,
	/** 14dp */ lg2: 14,
	/** 16dp */ xl: 16,
	/** 999 */ full: 999,
} as const;

// ── Typography scale ──────────────────────────────────────────────────────────

export const font = {
	size: {
		/** 10 */ xs: 10,
		/** 11 */ sm2: 11,
		/** 12 */ sm: 12,
		/** 13 */ md2: 13,
		/** 14 */ md: 14,
		/** 16 */ lg: 16,
		/** 18 */ lg2: 18,
		/** 20 */ xl: 20,
		/** 24 */ xl2: 24,
		/** 28 */ xl3: 28,
		/** 32 */ xl4: 32,
		/** 40 */ xl5: 40,
		/** 48 */ xl6: 48,
		/** 64 */ xl7: 64,
	},
	weight: {
		regular: "400" as const,
		medium: "500" as const,
		semibold: "600" as const,
		bold: "700" as const,
		extrabold: "800" as const,
		black: "900" as const,
	},
	lineHeight: {
		tight: 1.2,
		snug: 1.35,
		normal: 1.5,
		relaxed: 1.65,
	},
} as const;

// ── Elevation / shadow ────────────────────────────────────────────────────────

export const elevation = {
	none: 0,
	low: 2,
	mid: 4,
	high: 8,
	overlay: 16,
} as const;

// ── Breakpoints ───────────────────────────────────────────────────────────────

export const breakpoints = {
	/** Phone portrait */ phone: 0,
	/** Phone landscape */ phoneLS: 568,
	/** Tablet */ tablet: 768,
	/** Desktop */ desktop: 980,
	/** Cluster display */ cluster: 480, // compact landscape embedded
} as const;

// ── Motion ────────────────────────────────────────────────────────────────────

export const motion = {
	duration: {
		instant: 0,
		fast: 100,
		normal: 200,
		slow: 350,
		deliberate: 500,
	},
	easing: {
		ease: "ease" as const,
		easeIn: "ease-in" as const,
		easeOut: "ease-out" as const,
		easeInOut: "ease-in-out" as const,
		linear: "linear" as const,
	},
	/** Turn signal blink cycle (ms) */
	signalBlink: 600,
	/** Warning pulse cycle (ms) */
	warningPulse: 1000,
	/** Alarm pulse cycle (ms — faster than warning) */
	alarmPulse: 600,
	/** Gauge interpolation (ms) */
	gaugeTransition: 120,
} as const;

// ── Data density tiers ────────────────────────────────────────────────────────

export const density = {
	/** Default — phone. Full labels, spacious padding. */
	default: {
		cardPadding: spacing.md,
		statMinWidth: 120,
		statPadH: spacing.md,
		statPadV: spacing.sm,
		labelSize: font.size.sm,
		valueSize: font.size.md,
	},
	/** Compact — tablet landscape / cluster. Tighter but still legible. */
	compact: {
		cardPadding: spacing.sm,
		statMinWidth: 96,
		statPadH: spacing.sm,
		statPadV: spacing.xs,
		labelSize: font.size.xs,
		valueSize: font.size.sm,
	},
	/** Dense — desktop or secondary panel. Very tight grid. */
	dense: {
		cardPadding: spacing.xs,
		statMinWidth: 80,
		statPadH: spacing.xs,
		statPadV: spacing.xs2,
		labelSize: font.size.xs,
		valueSize: font.size.sm2,
	},
} as const;

// ── SOC level thresholds & colours ───────────────────────────────────────────

/**
 * State-of-charge visual thresholds.
 *
 * Components should choose the color bucket whose `max` is the first value
 * ≥ the current SoC percentage.
 *
 * Example:
 *   const bucket = socThresholds.find(t => soc <= t.max) ?? socThresholds[socThresholds.length - 1];
 *   const barColor = bucket.color;
 */
export const socThresholds = [
	{ max: 10, label: "critical", color: "#ef4444" /* red-500    */ },
	{ max: 20, label: "low", color: "#f97316" /* orange-500 */ },
	{ max: 40, label: "moderate", color: "#f59e0b" /* amber-500  */ },
	{ max: 80, label: "good", color: "#22c55e" /* green-500  */ },
	{ max: 100, label: "full", color: "#06b6d4" /* cyan-500   */ },
] as const;

export type SocThresholdLabel = (typeof socThresholds)[number]["label"];

// ── Speedometer arc segments ──────────────────────────────────────────────────

/**
 * Colour zones for the speedometer arc.  Values are in km/h.
 * Each `upTo` marks the upper end of that colour band.
 * Anything above the last entry uses `overLimit.color`.
 */
export const speedometerArc = {
	/** Total sweep angle in degrees (centred at bottom). */
	totalDegrees: 240,
	/** Speed (km/h) at which the arc is full (needle at max visible). */
	maxSpeed: 200,
	zones: [
		{ upTo: 80, color: "#22d3ee" /* cyan-400 — normal    */ },
		{ upTo: 130, color: "#a3e635" /* lime-400 — spirited  */ },
		{ upTo: 160, color: "#fbbf24" /* amber-400 — fast     */ },
		{ upTo: 200, color: "#f87171" /* red-400 — very fast  */ },
	],
	overLimit: { color: "#ef4444" /* red-500 — over limit */ },
	needle: { color: "#ffffff", shadowColor: "#000000", shadowRadius: 4 },
	track: { color: "#1e293b" /* slate-800 background arc */ },
} as const;

// ── Chart / sparkline colour series ──────────────────────────────────────────

/**
 * Ordered palette for multi-series charts (SignalChart, Monitor).
 * Signal charts pick series[index % series.length].
 */
export const chartColors = {
	series: [
		"#22d3ee", // cyan-400  — primary
		"#a3e635", // lime-400
		"#fb923c", // orange-400
		"#c084fc", // purple-400
		"#34d399", // emerald-400
		"#f472b6", // pink-400
		"#60a5fa", // blue-400
		"#facc15", // yellow-400
	],
	grid: "#1e293b", // slate-800
	axis: "#334155", // slate-700
	tickLabel: "#5c7ea0",
	tooltipBg: "#0d1e35",
	tooltipText: "#e2e8f0",
} as const;

// ── Power meter colours ───────────────────────────────────────────────────────

export const powerMeter = {
	/** Positive power (discharging / accelerating). */
	regen: { color: "#22d3ee" /* cyan-400 */ },
	drive: { color: "#22c55e" /* green-500 */ },
	peak: { color: "#ef4444" /* red-500 */ },
	kWLabel: { color: "#8baec8" },
	zeroLine: { color: "#334155" },
} as const;

// ── Component-level dashboard token bundle ────────────────────────────────────

/**
 * `dashTokens` is a flat bundle of all dashboard-layer tokens.
 * Components should prefer importing from this object to avoid scattered
 * token imports across the design system.
 *
 * @example
 * import { dashTokens } from '@/design/tokens';
 * const style = { color: dashTokens.value };
 */
export const dashTokens = {
	// Backgrounds
	bg: colors.dashBackground,
	bgCard: colors.dashCard,
	bgCardBorder: colors.dashCardBorder,

	// Text roles
	value: colors.dashValue,
	label: colors.dashLabel,
	unit: colors.dashUnit,
	primary: colors.dashPrimary,
	secondary: colors.dashSecondary,
	muted: colors.dashMuted,

	// Alarm
	alarmWarning: colors.alarmWarning,
	alarmCritical: colors.alarmCritical,
	alarmPulse: colors.alarmPulse,

	// Turn signal / BSM
	signalActive: colors.signalActive,
	bsmActive: colors.bsmActive,
	bsmWarning: colors.bsmWarning,

	// AP tiers
	apInactive: colors.apInactive,
	apActive: colors.apActive,
	apHandsWarn: colors.apHandsWarning,

	// Powertrain
	powerPositive: colors.powerPositive,
	powerNegative: colors.powerNegative,
	powerPeak: colors.powerPeak,

	// Gear
	gearActive: colors.gearActive,
	gearInactive: colors.gearInactive,
	gearReverse: colors.gearReverse,
	gearPark: colors.gearPark,

	// Sub-systems
	soc: socThresholds,
	arc: speedometerArc,
	chart: chartColors,
	power: powerMeter,
	motion,
	density,
} as const;

export type DashTokens = typeof dashTokens;

// ── Convenience re-exports ─────────────────────────────────────────────────────

export const tokens = {
	colors,
	spacing,
	radius,
	font,
	elevation,
	breakpoints,
	motion,
	density,
	dashTokens,
} as const;

export type Tokens = typeof tokens;
