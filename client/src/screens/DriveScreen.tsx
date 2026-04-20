/**
 * DriveScreen — Tesla CAN phone instrument cluster.
 *
 * Landscape: TopBar → [LeftPanel | SpeedDial | RightPanel] → BottomStrip
 * Portrait:  TopBar → [HudInfo?] → SpeedDial → [LeftPanel | RightPanel] → BottomStrip
 */

import { useEffect, useRef, useState } from "react";
import { Animated, Platform, Pressable, StyleSheet, Text, View } from "react-native";
import type { BoardState } from "@teslacanmodder/protocol";
import { formatAutopilotTier, formatDriveMode, formatGear } from "@teslacanmodder/protocol";
import { colors, font, radius, spacing, motion } from "../design/tokens";
import { useBreakpoint } from "../state/useBreakpoint";
import { useSpeedUnit } from "../state/useSpeedUnit";
import { useSocDisplay } from "../state/useSocDisplay";
import { usePowerCapture } from "../state/usePowerCapture";
import { useTheme } from "../state/useTheme";
import { useGaugeMode } from "../state/useGaugeMode";
import type { GaugeMode } from "../state/useGaugeMode";
import { Badge, StatChip } from "../ui";

// ── Palette ───────────────────────────────────────────────────────────────────

const C = {
	bg: colors.dashBackground,
	card: colors.dashCard,
	bord: colors.dashCardBorder,
	strip: colors.backgroundDarkSubtle,
	val: colors.dashValue,
	lbl: colors.dashMuted,
	sub: colors.dashSecondary,
	dim: colors.dashUnit,
	cyan: colors.dashPrimary,
	blue: colors.primary,
	grn: colors.powerPositive,
	amb: colors.alarmWarning,
	red: colors.alarmCritical,
	barBg: colors.backgroundDarkCard,
} as const;

// ── Glow helper ───────────────────────────────────────────────────────────────

function hexAlpha(color: string, alpha: number): string {
	const n = color.trim();
	if (n.startsWith("#")) {
		let hex = n.slice(1);
		if (hex.length === 3)
			hex = hex
				.split("")
				.map((ch) => `${ch}${ch}`)
				.join("");
		if (hex.length === 6) {
			const r = Number.parseInt(hex.slice(0, 2), 16);
			const g = Number.parseInt(hex.slice(2, 4), 16);
			const b = Number.parseInt(hex.slice(4, 6), 16);
			return `rgba(${r}, ${g}, ${b}, ${alpha})`;
		}
	}
	return n;
}

function buildGlow(color: string, opacity: number, blur: number) {
	if (Platform.OS === "web") return { boxShadow: `0 0 ${blur}px ${hexAlpha(color, opacity)}` };
	return {
		shadowColor: color,
		shadowOffset: { width: 0, height: 0 },
		shadowOpacity: opacity,
		shadowRadius: blur,
	};
}

// ── AP state ──────────────────────────────────────────────────────────────────

export type ApClusterState = "unavailable" | "inactive" | "active" | "hands_warning";

export function resolveApState(
	tier: number,
	dasHandsOn: number,
	chassisOnline: boolean,
): ApClusterState {
	if (!chassisOnline || tier === 0) return "unavailable";
	if (dasHandsOn >= 1) return "hands_warning";
	return "inactive";
}

function apBadgeVariant(s: ApClusterState): "success" | "warning" | "destructive" | "muted" {
	if (s === "active") return "success";
	if (s === "hands_warning") return "warning";
	return "muted";
}

function apStateLabel(s: ApClusterState, tier: number): string {
	if (s === "unavailable") return "AP N/A";
	if (s === "hands_warning") return "Hands On";
	return formatAutopilotTier(tier);
}

// ── Gear row ──────────────────────────────────────────────────────────────────

const GEAR_DEFS: [number, string][] = [
	[1, "P"],
	[3, "N"],
	[4, "D"],
	[2, "R"],
];

function GearLetter({ id, active }: { id: number; active: boolean }) {
	const useNativeDriver = Platform?.OS !== "web";
	const scale = useRef(new Animated.Value(active ? 1.22 : 1)).current;
	const opacity = useRef(new Animated.Value(active ? 1 : 0.18)).current;

	useEffect(() => {
		Animated.parallel([
			Animated.timing(scale, {
				toValue: active ? 1.22 : 1,
				duration: motion.duration.fast,
				useNativeDriver,
			}),
			Animated.timing(opacity, {
				toValue: active ? 1 : 0.18,
				duration: motion.duration.fast,
				useNativeDriver,
			}),
		]).start();
	}, [active, scale, opacity]);

	const label = GEAR_DEFS.find(([g]) => g === id)?.[1] ?? "?";
	const color = !active
		? C.dim
		: id === 2
			? colors.gearReverse
			: id === 1
				? colors.gearPark
				: id === 3
					? colors.gearInactive
					: C.val;

	return (
		<Animated.Text style={[grS.letter, { color, transform: [{ scale }], opacity }]}>
			{label}
		</Animated.Text>
	);
}

function GearRow({ gear }: { gear: number }) {
	return (
		<View style={grS.row}>
			{GEAR_DEFS.map(([id]) => (
				<GearLetter key={id} id={id} active={id === gear} />
			))}
		</View>
	);
}

const grS = StyleSheet.create({
	row: { flexDirection: "row", gap: spacing.lg, alignItems: "center" },
	letter: { fontSize: font.size.xl2, fontWeight: font.weight.extrabold, letterSpacing: 2 },
});

// ── Signal helpers ────────────────────────────────────────────────────────────

type SignalTone = "idle" | "warn" | "alert" | "ok";

function signalColor(tone: SignalTone): string {
	if (tone === "alert") return C.red;
	if (tone === "warn") return C.amb;
	if (tone === "ok") return C.grn;
	return C.bord;
}

function sideTone(bsm: number, turn: boolean): SignalTone {
	if (bsm >= 2) return "alert";
	if (turn || bsm >= 1) return "warn";
	return "idle";
}

function sideLabel(bsm: number, turn: boolean): string {
	if (bsm >= 2) return "BSM!";
	if (turn) return "TURN";
	if (bsm >= 1) return "BSM";
	return "";
}

// ── Gauge line ────────────────────────────────────────────────────────────────

function GaugeLine({
	label,
	value,
	pct,
	color,
	muted,
}: {
	label: string;
	value: string;
	pct: number;
	color: string;
	muted?: boolean;
}) {
	const w = `${Math.floor(Math.max(0, Math.min(1, pct)) * 100)}%` as `${number}%`;
	return (
		<View style={glS.wrap}>
			<View style={glS.header}>
				<Text style={glS.label}>{label}</Text>
				<Text style={[glS.value, muted ? glS.muted : undefined]}>{value}</Text>
			</View>
			<View style={glS.track}>
				<View
					style={[
						glS.fill,
						{ width: w, backgroundColor: color, opacity: muted ? 0.2 : 1 },
					]}
				/>
			</View>
		</View>
	);
}

const glS = StyleSheet.create({
	wrap: { gap: 2 },
	header: { flexDirection: "row", justifyContent: "space-between" },
	label: {
		color: C.lbl,
		fontSize: font.size.xs,
		fontWeight: font.weight.bold,
		textTransform: "uppercase",
		letterSpacing: 0.5,
	},
	value: { color: C.sub, fontSize: font.size.xs, fontWeight: font.weight.semibold },
	muted: { color: C.bord },
	track: { height: 4, borderRadius: radius.full, backgroundColor: C.barBg, overflow: "hidden" },
	fill: { height: "100%", borderRadius: radius.full },
});

// ── Power bar ─────────────────────────────────────────────────────────────────

function PowerBar({
	power,
	maxKw = 200,
	maxDrive,
	maxRegen,
	onReset,
}: {
	power: number;
	maxKw?: number;
	maxDrive?: number;
	maxRegen?: number;
	onReset?: () => void;
}) {
	const abs = Math.min(Math.abs(power), maxKw);
	const pct = abs / maxKw;
	const isReg = power < 0;
	const dPct =
		maxDrive !== null && maxDrive !== undefined ? Math.min(maxDrive / maxKw, 1) * 100 : null;
	const rPct =
		maxRegen !== null && maxRegen !== undefined ? Math.min(maxRegen / maxKw, 1) * 100 : null;

	return (
		<View style={pbS.wrap}>
			<View style={pbS.track}>
				<View
					style={[
						pbS.fill,
						{
							width: `${Math.floor(pct * 100)}%` as `${number}%`,
							backgroundColor: isReg ? C.cyan : C.grn,
						},
					]}
				/>
				{dPct !== null && dPct > 0 ? (
					<View
						style={[
							pbS.mark,
							{
								left: `${Math.floor(dPct)}%` as `${number}%`,
								backgroundColor: C.grn,
							},
						]}
					/>
				) : null}
				{rPct !== null && rPct > 0 ? (
					<View
						style={[
							pbS.mark,
							{
								left: `${Math.floor(rPct)}%` as `${number}%`,
								backgroundColor: C.cyan,
							},
						]}
					/>
				) : null}
			</View>
			<View style={pbS.row}>
				<Text style={[pbS.label, { color: isReg ? C.cyan : C.grn }]}>
					{isReg
						? `▼ ${Math.abs(power).toFixed(0)} kW  regen`
						: `▲ ${power.toFixed(0)} kW`}
				</Text>
				{onReset ? (
					<Pressable onPress={onReset} hitSlop={8}>
						<Text style={pbS.reset}>RST</Text>
					</Pressable>
				) : null}
			</View>
		</View>
	);
}

const pbS = StyleSheet.create({
	wrap: { gap: 3 },
	track: {
		height: 6,
		borderRadius: radius.full,
		backgroundColor: C.barBg,
		overflow: "hidden",
		position: "relative",
	},
	fill: { height: "100%", borderRadius: radius.full },
	mark: { position: "absolute", top: 0, width: 2, height: "100%", opacity: 0.8 },
	row: { flexDirection: "row", justifyContent: "space-between", alignItems: "center" },
	label: { fontSize: font.size.xs, fontWeight: font.weight.semibold },
	reset: { color: C.lbl, fontSize: font.size.xs, fontWeight: font.weight.bold, opacity: 0.5 },
});

// ── Speed dial ────────────────────────────────────────────────────────────────

function SpeedDial({
	state,
	gaugeMode,
	onCycle,
	size = 220,
}: {
	state: BoardState;
	gaugeMode: GaugeMode;
	onCycle: () => void;
	size?: number;
}) {
	const { cycleUnit, convert, label: unitLabel } = useSpeedUnit();
	const speed = Number.isFinite(state.vehicleSpeed)
		? convert(state.vehicleSpeed).toFixed(0)
		: "—";
	const speedFs = size < 200 ? 52 : 68;
	const pedal = Math.max(0, Math.min(100, state.accelPedal));
	const pwr = state.bmsPower;

	const ringColor =
		Number.isFinite(pwr) && pwr > 5
			? C.grn
			: Number.isFinite(pwr) && pwr < -5
				? C.cyan
				: C.bord;
	const glowOpacity = Number.isFinite(pwr) ? Math.min(0.75, 0.2 + Math.abs(pwr) / 180) : 0.2;
	const steerNorm = Math.max(-1, Math.min(1, (state.steeringAngle ?? 0) / 180));

	return (
		<View
			style={[
				sdS.ring,
				{
					width: size,
					height: size,
					borderRadius: size / 2,
					borderColor: ringColor,
					...buildGlow(ringColor, glowOpacity, 28),
				},
			]}
		>
			<Pressable onPress={cycleUnit} hitSlop={12}>
				<Text style={sdS.unit}>{unitLabel}</Text>
			</Pressable>
			<Pressable onPress={onCycle} hitSlop={8}>
				<Text style={[sdS.speed, { fontSize: speedFs, lineHeight: speedFs + 4 }]}>
					{speed}
				</Text>
			</Pressable>
			<GearRow gear={state.gearState} />

			{gaugeMode === "power" ? (
				<View style={sdS.sub}>
					<Text style={[sdS.powerNum, { color: pwr < 0 ? C.cyan : C.grn }]}>
						{Number.isFinite(pwr) ? `${pwr > 0 ? "+" : ""}${pwr.toFixed(1)}` : "—"}
					</Text>
					<Text style={sdS.subLbl}>kW</Text>
				</View>
			) : gaugeMode === "perf" ? (
				<View style={sdS.sub}>
					<View style={sdS.miniTrack}>
						<View style={sdS.miniCenter} />
						<View
							style={[
								sdS.miniDot,
								{ left: `${50 + steerNorm * 44}%` as `${number}%` },
							]}
						/>
					</View>
					<Text style={sdS.subLbl}>
						{(state.steeringAngle ?? 0).toFixed(0)}° · {pedal.toFixed(0)}%
					</Text>
				</View>
			) : (
				<View style={sdS.sub}>
					<View style={sdS.miniTrack}>
						<View style={[sdS.miniFill, { width: `${pedal}%` as `${number}%` }]} />
					</View>
					<Text style={sdS.subLbl}>PEDAL {pedal.toFixed(0)}%</Text>
				</View>
			)}
		</View>
	);
}

const sdS = StyleSheet.create({
	ring: {
		alignItems: "center",
		justifyContent: "center",
		gap: 4,
		borderWidth: 3,
		backgroundColor: C.card,
		elevation: 16,
	},
	unit: {
		color: C.dim,
		fontSize: font.size.xs,
		fontWeight: font.weight.bold,
		letterSpacing: 2,
		textTransform: "uppercase",
	},
	speed: { color: C.val, fontWeight: font.weight.black, letterSpacing: -3 },
	sub: { alignItems: "center", gap: 3, width: "68%" },
	miniTrack: {
		width: "100%",
		height: 5,
		borderRadius: radius.full,
		backgroundColor: C.barBg,
		overflow: "hidden",
		position: "relative",
	},
	miniFill: { height: "100%", backgroundColor: C.cyan, borderRadius: radius.full },
	miniCenter: {
		position: "absolute",
		left: "50%",
		width: 1,
		height: "100%",
		backgroundColor: C.lbl,
	},
	miniDot: {
		position: "absolute",
		width: 8,
		height: "100%",
		marginLeft: -4,
		backgroundColor: C.cyan,
		borderRadius: radius.full,
	},
	subLbl: {
		color: C.dim,
		fontSize: font.size.xs,
		fontWeight: font.weight.bold,
		letterSpacing: 0.5,
		textTransform: "uppercase",
	},
	powerNum: { fontSize: font.size.xl3, fontWeight: font.weight.extrabold, letterSpacing: -1 },
});

// ── Left panel — battery + motors ─────────────────────────────────────────────

function LeftPanel({ state }: { state: BoardState }) {
	const { primaryValue, modeLabel, cycleMode, socAlarm } = useSocDisplay(state);
	const soc = Math.min(100, Math.max(0, state.bmsSoc));
	const socColor = socAlarm === "critical" ? C.red : socAlarm === "warning" ? C.amb : C.grn;
	const range =
		state.bmsIdealRemaining > 0
			? state.bmsIdealRemaining
			: state.bmsExpectedRange > 0
				? state.bmsExpectedRange
				: null;
	const pwr = state.bmsPower;
	const pwrColor =
		Number.isFinite(pwr) && pwr < 0 ? C.cyan : Number.isFinite(pwr) && pwr > 5 ? C.grn : C.sub;

	return (
		<View style={lpS.panel}>
			<Pressable onPress={cycleMode} style={lpS.socHeader} hitSlop={8}>
				<Text style={[lpS.socNum, { color: socColor }]}>{primaryValue}</Text>
				<Text style={lpS.socMode}>{modeLabel}</Text>
			</Pressable>
			<View style={lpS.socTrack}>
				<View
					style={[
						lpS.socFill,
						{ width: `${soc}%` as `${number}%`, backgroundColor: socColor },
					]}
				/>
			</View>
			{range !== null ? <Text style={lpS.range}>{range.toFixed(0)} km est</Text> : null}
			{Number.isFinite(pwr) ? (
				<Text style={[lpS.pwr, { color: pwrColor }]}>
					{pwr < 0 ? "▼" : "▲"} {Math.abs(pwr).toFixed(1)} kW
				</Text>
			) : null}
			<View style={lpS.divider} />
			{state.hasPowertrain ? (
				<>
					<GaugeLine
						label="REAR"
						value={`${Math.abs(state.rearMotorRpm).toFixed(0)} rpm`}
						pct={Math.min(Math.abs(state.rearMotorRpm) / 16000, 1)}
						color={C.grn}
					/>
					<GaugeLine
						label="FRONT"
						value={
							Math.abs(state.frontMotorRpm) > 5
								? `${Math.abs(state.frontMotorRpm).toFixed(0)} rpm`
								: "RWD"
						}
						pct={Math.min(Math.abs(state.frontMotorRpm) / 16000, 1)}
						color={C.cyan}
						muted={Math.abs(state.frontMotorRpm) <= 5}
					/>
				</>
			) : null}
			{state.hasBms ? (
				<GaugeLine
					label="PACK"
					value={`${state.bmsTempMin.toFixed(0)}–${state.bmsTempMax.toFixed(0)} °C`}
					pct={Math.min(Math.max(0, state.bmsTempMax - state.bmsTempMin) / 60, 1)}
					color={C.amb}
				/>
			) : null}
		</View>
	);
}

const lpS = StyleSheet.create({
	panel: {
		flex: 1,
		paddingHorizontal: spacing.sm,
		paddingVertical: spacing.sm,
		gap: spacing.xs2 + 2,
		justifyContent: "center",
	},
	socHeader: { flexDirection: "row", alignItems: "baseline", gap: spacing.xs },
	socNum: { fontSize: font.size.xl3, fontWeight: font.weight.black, letterSpacing: -1 },
	socMode: {
		color: C.lbl,
		fontSize: font.size.xs,
		fontWeight: font.weight.bold,
		letterSpacing: 1,
		textTransform: "uppercase",
	},
	socTrack: {
		height: 8,
		borderRadius: radius.full,
		backgroundColor: C.barBg,
		overflow: "hidden",
	},
	socFill: { height: "100%", borderRadius: radius.full },
	range: { color: C.sub, fontSize: font.size.sm, fontWeight: font.weight.semibold },
	pwr: { fontSize: font.size.md, fontWeight: font.weight.bold },
	divider: { height: 1, backgroundColor: C.bord, marginVertical: spacing.xs2 },
});

// ── Turn chip ─────────────────────────────────────────────────────────────────

function TurnChip({
	side,
	bsmLevel,
	turnOn,
}: {
	side: "L" | "R";
	bsmLevel: number;
	turnOn: boolean;
}) {
	const tone = sideTone(bsmLevel, turnOn);
	const color = signalColor(tone);
	const label = sideLabel(bsmLevel, turnOn);
	const active = turnOn || bsmLevel > 0;

	return (
		<View style={[tcS.chip, { borderColor: color, opacity: active ? 1 : 0.28 }]}>
			<Text style={[tcS.arrow, { color }]}>{side === "L" ? "◄" : "►"}</Text>
			<Text style={tcS.side}>{side === "L" ? "LEFT" : "RIGHT"}</Text>
			{label ? <Text style={[tcS.label, { color }]}>{label}</Text> : null}
			{bsmLevel > 0 ? <Text style={[tcS.bsm, { color }]}>BSM {bsmLevel}</Text> : null}
		</View>
	);
}

const tcS = StyleSheet.create({
	chip: {
		flexDirection: "row",
		alignItems: "center",
		gap: spacing.xs,
		paddingHorizontal: spacing.sm,
		paddingVertical: spacing.xs2 + 2,
		borderWidth: 1,
		borderRadius: radius.md,
		backgroundColor: C.card,
	},
	arrow: { fontSize: font.size.lg, fontWeight: font.weight.black },
	side: {
		color: C.lbl,
		fontSize: font.size.xs,
		fontWeight: font.weight.bold,
		letterSpacing: 1,
		textTransform: "uppercase",
	},
	label: {
		fontSize: font.size.xs,
		fontWeight: font.weight.extrabold,
		letterSpacing: 0.5,
		textTransform: "uppercase",
		marginLeft: "auto",
	},
	bsm: { fontSize: font.size.xs, fontWeight: font.weight.bold, opacity: 0.8 },
});

// ── Door state car ────────────────────────────────────────────────────────────

function DoorStateCar({ state }: { state: BoardState }) {
	const partStyle = (open: boolean) => [dcS.part, open ? dcS.partOpen : dcS.partClosed];
	return (
		<View style={dcS.wrap}>
			<View style={dcS.shadow} />
			<View style={dcS.body}>
				<View style={[dcS.frunk, ...partStyle(state.frunkOpen)]} />
				<View style={[dcS.trunk, ...partStyle(state.trunkOpen)]} />
				<View style={dcS.roof} />
				<View style={[dcS.doorFL, ...partStyle(state.driverDoorOpen)]} />
				<View style={[dcS.doorFR, ...partStyle(state.doorFrontRightOpen)]} />
				<View style={[dcS.doorRL, ...partStyle(state.doorRearLeftOpen)]} />
				<View style={[dcS.doorRR, ...partStyle(state.doorRearRightOpen)]} />
				<View style={dcS.hlL} />
				<View style={dcS.hlR} />
				<View style={dcS.tlL} />
				<View style={dcS.tlR} />
			</View>
		</View>
	);
}

const dcS = StyleSheet.create({
	wrap: { width: 54, height: 82, alignItems: "center", justifyContent: "center" },
	shadow: {
		position: "absolute",
		bottom: 4,
		width: 36,
		height: 4,
		borderRadius: radius.full,
		backgroundColor: C.bord,
		opacity: 0.5,
	},
	body: {
		width: 40,
		height: 74,
		borderRadius: radius.lg,
		borderWidth: 1,
		borderColor: C.sub,
		backgroundColor: C.card,
		position: "relative",
		overflow: "hidden",
	},
	roof: {
		position: "absolute",
		left: 9,
		top: 20,
		width: 22,
		height: 34,
		borderWidth: 1,
		borderColor: C.bord,
		borderRadius: radius.sm,
	},
	part: { position: "absolute", borderWidth: 1, borderRadius: 2 },
	partOpen: { backgroundColor: C.cyan, borderColor: C.grn },
	partClosed: { backgroundColor: C.card, borderColor: C.bord },
	frunk: { left: 8, top: 3, width: 24, height: 8 },
	trunk: { left: 8, bottom: 3, width: 24, height: 9 },
	doorFL: { left: 0, top: 20, width: 5, height: 14 },
	doorFR: { right: 0, top: 20, width: 5, height: 14 },
	doorRL: { left: 0, top: 38, width: 5, height: 14 },
	doorRR: { right: 0, top: 38, width: 5, height: 14 },
	hlL: {
		position: "absolute",
		left: 7,
		top: 1,
		width: 6,
		height: 2,
		borderRadius: radius.full,
		backgroundColor: C.amb,
	},
	hlR: {
		position: "absolute",
		right: 7,
		top: 1,
		width: 6,
		height: 2,
		borderRadius: radius.full,
		backgroundColor: C.amb,
	},
	tlL: {
		position: "absolute",
		left: 7,
		bottom: 1,
		width: 6,
		height: 2,
		borderRadius: radius.full,
		backgroundColor: C.red,
	},
	tlR: {
		position: "absolute",
		right: 7,
		bottom: 1,
		width: 6,
		height: 2,
		borderRadius: radius.full,
		backgroundColor: C.red,
	},
});

// ── Right panel — safety ──────────────────────────────────────────────────────

function RightPanel({ state }: { state: BoardState }) {
	const openDoors: string[] = [];
	if (state.driverDoorOpen) openDoors.push("Driver");
	if (state.doorFrontRightOpen) openDoors.push("FR");
	if (state.doorRearLeftOpen) openDoors.push("RL");
	if (state.doorRearRightOpen) openDoors.push("RR");
	if (state.frunkOpen) openDoors.push("Frunk");
	if (state.trunkOpen) openDoors.push("Trunk");
	const hasOpen = openDoors.length > 0;
	const fmtKph = (v: number) => (v > 0 ? `${v.toFixed(0)} kph` : "—");
	const cruiseActive = state.cruiseSetSpeedKph > 0 || state.maxSpeedKph > 0;

	return (
		<View style={rpS.panel}>
			<TurnChip side="L" bsmLevel={state.bsmLeftLevel} turnOn={state.turnSignalLeft} />
			<View style={rpS.divider} />

			<View style={rpS.doorsRow}>
				<DoorStateCar state={state} />
				<View style={rpS.doorsText}>
					<Text style={rpS.secTitle}>DOORS</Text>
					<Text style={[rpS.doorsState, { color: hasOpen ? C.amb : C.grn }]}>
						{hasOpen ? `${openDoors.length} OPEN` : "CLOSED"}
					</Text>
					{hasOpen ? (
						<Text style={rpS.doorsList} numberOfLines={2}>
							{openDoors.join(" · ")}
						</Text>
					) : null}
				</View>
			</View>
			<View style={rpS.divider} />

			<View style={rpS.cruise}>
				<Text style={rpS.secTitle}>CRUISE</Text>
				<Text style={[rpS.cruiseState, { color: cruiseActive ? C.amb : C.lbl }]}>
					{cruiseActive ? "SET" : "IDLE"}
				</Text>
				<Text style={rpS.cruiseVal}>CC {fmtKph(state.cruiseSetSpeedKph)}</Text>
				<Text style={rpS.cruiseVal}>Max {fmtKph(state.maxSpeedKph)}</Text>
			</View>
			<View style={rpS.divider} />

			<TurnChip side="R" bsmLevel={state.bsmRightLevel} turnOn={state.turnSignalRight} />
		</View>
	);
}

const rpS = StyleSheet.create({
	panel: {
		flex: 1,
		paddingHorizontal: spacing.sm,
		paddingVertical: spacing.sm,
		gap: spacing.xs,
		justifyContent: "center",
	},
	divider: { height: 1, backgroundColor: C.bord },
	doorsRow: { flexDirection: "row", alignItems: "center", gap: spacing.sm },
	doorsText: { flex: 1, gap: 2 },
	secTitle: {
		color: C.lbl,
		fontSize: font.size.xs,
		fontWeight: font.weight.bold,
		letterSpacing: 1,
		textTransform: "uppercase",
	},
	doorsState: {
		fontSize: font.size.md,
		fontWeight: font.weight.extrabold,
		textTransform: "uppercase",
		letterSpacing: 0.3,
	},
	doorsList: { color: C.sub, fontSize: font.size.xs },
	cruise: { gap: 2 },
	cruiseState: {
		fontSize: font.size.md,
		fontWeight: font.weight.extrabold,
		textTransform: "uppercase",
		letterSpacing: 0.3,
	},
	cruiseVal: { color: C.sub, fontSize: font.size.sm, fontWeight: font.weight.semibold },
});

// ── Top bar ───────────────────────────────────────────────────────────────────

function TopBar({
	state,
	showHud,
	onToggleHud,
}: {
	state: BoardState;
	showHud?: boolean;
	onToggleHud?: () => void;
}) {
	const { isDark, setOverride, source } = useTheme();
	const apState = resolveApState(state.gtwAutopilotTier, state.dasHandsOn, state.chassisOnline);

	return (
		<View style={tbS.bar}>
			<View style={tbS.left}>
				<View style={[tbS.dot, state.chassisOnline ? tbS.dotOn : tbS.dotOff]} />
				<Text style={tbS.conn}>
					{state.chassisOnline ? "ONLINE" : "OFFLINE"}
					{state.vehicleOnline ? " · V" : ""}
					{state.bodyOnline ? " · B" : ""}
				</Text>
				<Text style={tbS.sep}>·</Text>
				<Text style={tbS.gear}>{formatGear(state.gearState)}</Text>
				<Text style={tbS.sep}>·</Text>
				<Text style={tbS.mode}>
					{formatDriveMode(state.currentDriveMode ?? state.driveMode)}
				</Text>
			</View>
			<View style={tbS.right}>
				{state.fsd ? <Badge variant="dark">FSD</Badge> : null}
				{state.nag ? <Badge variant="warning">NAG</Badge> : null}
				{state.dasHandsOn >= 1 ? <Badge variant="destructive">⚠ HANDS</Badge> : null}
				<Badge variant={apBadgeVariant(apState)}>
					{apStateLabel(apState, state.gtwAutopilotTier)}
				</Badge>
				{onToggleHud ? (
					<Pressable
						onPress={onToggleHud}
						style={[tbS.hudBtn, showHud ? tbS.hudBtnOn : tbS.hudBtnOff]}
						hitSlop={8}
					>
						<Text style={[tbS.hudTxt, showHud ? tbS.hudTxtOn : tbS.hudTxtOff]}>
							{showHud ? "▲" : "▼"}
						</Text>
					</Pressable>
				) : null}
				{source !== "force" ? (
					<Pressable
						onPress={() => setOverride(isDark ? "light" : "dark")}
						hitSlop={8}
						style={tbS.themeBtn}
					>
						<Text style={tbS.themeIcon}>{isDark ? "☀" : "☾"}</Text>
					</Pressable>
				) : null}
			</View>
		</View>
	);
}

const tbS = StyleSheet.create({
	bar: {
		flexDirection: "row",
		alignItems: "center",
		justifyContent: "space-between",
		paddingHorizontal: spacing.md,
		paddingVertical: spacing.sm,
		backgroundColor: C.card,
		borderBottomWidth: 1,
		borderBottomColor: C.bord,
		minHeight: 40,
	},
	left: {
		flexDirection: "row",
		alignItems: "center",
		gap: spacing.xs,
		flex: 1,
		overflow: "hidden",
	},
	right: { flexDirection: "row", alignItems: "center", gap: spacing.xs },
	dot: { width: 6, height: 6, borderRadius: 99 },
	dotOn: { backgroundColor: C.grn },
	dotOff: { backgroundColor: C.red },
	conn: { color: C.sub, fontSize: font.size.xs, fontWeight: font.weight.semibold },
	sep: { color: C.lbl, fontSize: font.size.xs },
	gear: { color: C.val, fontSize: font.size.xs, fontWeight: font.weight.bold },
	mode: {
		color: C.sub,
		fontSize: font.size.xs,
		fontWeight: font.weight.semibold,
		textTransform: "uppercase",
		flexShrink: 1,
	},
	hudBtn: {
		paddingHorizontal: spacing.xs,
		paddingVertical: 2,
		borderRadius: radius.sm,
		borderWidth: 1,
	},
	hudBtnOn: { borderColor: C.cyan, backgroundColor: C.barBg },
	hudBtnOff: { borderColor: C.bord, backgroundColor: "transparent" },
	hudTxt: { fontSize: font.size.xs, fontWeight: font.weight.bold },
	hudTxtOn: { color: C.cyan },
	hudTxtOff: { color: C.lbl },
	themeBtn: { padding: spacing.xs },
	themeIcon: { fontSize: font.size.lg, color: C.sub },
});

// ── HUD info row (portrait collapsible) ───────────────────────────────────────

function formatUptime(ms: number): string {
	if (!ms) return "--";
	const s = Math.floor(ms / 1000),
		m = Math.floor(s / 60),
		h = Math.floor(m / 60);
	if (h > 0) return `${h}h ${m % 60}m`;
	if (m > 0) return `${m}m ${s % 60}s`;
	return `${s}s`;
}

function HudInfo({ state }: { state: BoardState }) {
	return (
		<View style={hiS.row}>
			{(
				[
					["Board", state.hardware ?? "--"],
					["Driver", state.driver ?? "--"],
					["Uptime", formatUptime(state.uptime)],
					["Variant", (state.variant ?? "--").toUpperCase()],
					["Rate", state.rate > 0 ? `${state.rate} Hz` : "--"],
				] as [string, string][]
			).map(([lbl, val]) => (
				<View key={lbl} style={hiS.cell}>
					<Text style={hiS.lbl}>{lbl}</Text>
					<Text style={hiS.val}>{val}</Text>
				</View>
			))}
		</View>
	);
}

const hiS = StyleSheet.create({
	row: {
		flexDirection: "row",
		flexWrap: "wrap",
		gap: spacing.sm,
		paddingHorizontal: spacing.md,
		paddingVertical: spacing.sm2,
		backgroundColor: C.card,
		borderBottomWidth: 1,
		borderBottomColor: C.bord,
	},
	cell: { minWidth: 72, gap: 1 },
	lbl: { color: C.lbl, fontSize: font.size.xs, textTransform: "uppercase", letterSpacing: 0.5 },
	val: { color: C.sub, fontSize: font.size.sm, fontWeight: font.weight.semibold },
});

// ── Bottom strip ──────────────────────────────────────────────────────────────

function BottomStrip({
	state,
	maxDrive,
	maxRegen,
	onReset,
}: {
	state: BoardState;
	maxDrive?: number;
	maxRegen?: number;
	onReset: () => void;
}) {
	const driveMode = formatDriveMode(state.currentDriveMode ?? state.driveMode);
	const tires = state.hasTpms
		? [
				["FL", state.tpmsPressureFL] as [string, number],
				["FR", state.tpmsPressureFR] as [string, number],
				["RL", state.tpmsPressureRL] as [string, number],
				["RR", state.tpmsPressureRR] as [string, number],
			]
		: [];

	return (
		<View style={bsS.strip}>
			<PowerBar
				power={state.bmsPower}
				maxDrive={maxDrive}
				maxRegen={maxRegen}
				onReset={onReset}
			/>
			<View style={bsS.chips}>
				<StatChip
					label="Mode"
					value={driveMode}
					variant="dark"
					accent={driveMode !== "OFF"}
				/>
				<StatChip
					label="FSD"
					value={state.fsd ? "ON" : "OFF"}
					variant="dark"
					accent={state.fsd}
				/>
				{state.hasBms ? (
					<>
						<StatChip
							label="Regen"
							value={state.bmsMaxRegenPower.toFixed(0)}
							unit="kW"
							variant="dark"
						/>
						<StatChip
							label="Temp"
							value={`${state.bmsTempMin.toFixed(0)}–${state.bmsTempMax.toFixed(0)}`}
							unit="°C"
							variant="dark"
						/>
					</>
				) : null}
				{tires.map(([lbl, p]) => {
					const missing = p <= 0;
					const low = !missing && p < 2.3;
					return (
						<StatChip
							key={lbl}
							label={lbl}
							value={missing ? "--" : p.toFixed(1)}
							unit={missing ? "" : "b"}
							variant="dark"
							accent={low}
						/>
					);
				})}
			</View>
		</View>
	);
}

const bsS = StyleSheet.create({
	strip: {
		paddingHorizontal: spacing.md,
		paddingVertical: spacing.sm,
		gap: spacing.sm,
		backgroundColor: C.strip,
		borderTopWidth: 1,
		borderTopColor: C.bord,
	},
	chips: { flexDirection: "row", flexWrap: "wrap", gap: spacing.xs },
});

// ── DriveScreen ───────────────────────────────────────────────────────────────

export interface DriveScreenProps {
	boardState: BoardState;
}

export function DriveScreen({ boardState: s }: DriveScreenProps) {
	const bp = useBreakpoint();
	const isLand = bp.bp === "phoneLandscape" || (bp.isPhone && bp.width > bp.height);
	const { mode: gaugeMode, cycleMode } = useGaugeMode();
	const [showHud, setShowHud] = useState(false);
	const { maxDrive, maxRegen, reset } = usePowerCapture(s.bmsPower);

	// ── Landscape ──────────────────────────────────────────────────────────────
	if (isLand) {
		return (
			<View style={ds.root}>
				<TopBar state={s} />
				<View style={ds.lsBody}>
					<View style={ds.lsLeft}>
						<LeftPanel state={s} />
					</View>
					<View style={ds.lsCenter}>
						<SpeedDial state={s} gaugeMode={gaugeMode} onCycle={cycleMode} size={220} />
					</View>
					<View style={ds.lsRight}>
						<RightPanel state={s} />
					</View>
				</View>
				<BottomStrip state={s} maxDrive={maxDrive} maxRegen={maxRegen} onReset={reset} />
			</View>
		);
	}

	// ── Portrait ───────────────────────────────────────────────────────────────
	return (
		<View style={ds.root}>
			<TopBar state={s} showHud={showHud} onToggleHud={() => setShowHud((v) => !v)} />
			{showHud ? <HudInfo state={s} /> : null}
			<View style={ds.ptCenter}>
				<SpeedDial state={s} gaugeMode={gaugeMode} onCycle={cycleMode} size={220} />
			</View>
			<View style={ds.ptRow}>
				<View style={ds.ptLeft}>
					<LeftPanel state={s} />
				</View>
				<View style={ds.ptRight}>
					<RightPanel state={s} />
				</View>
			</View>
			<BottomStrip state={s} maxDrive={maxDrive} maxRegen={maxRegen} onReset={reset} />
		</View>
	);
}

// ── Styles ────────────────────────────────────────────────────────────────────

const ds = StyleSheet.create({
	root: { flex: 1, flexDirection: "column", backgroundColor: C.bg },

	// landscape
	lsBody: { flex: 1, flexDirection: "row", alignItems: "stretch" },
	lsLeft: { width: 178, borderRightWidth: 1, borderRightColor: C.bord },
	lsCenter: {
		flex: 1,
		alignItems: "center",
		justifyContent: "center",
		paddingVertical: spacing.sm,
	},
	lsRight: { width: 178, borderLeftWidth: 1, borderLeftColor: C.bord },

	// portrait
	ptCenter: {
		alignItems: "center",
		justifyContent: "center",
		paddingVertical: spacing.md,
		borderBottomWidth: 1,
		borderBottomColor: C.bord,
	},
	ptRow: { flex: 1, flexDirection: "row", borderBottomWidth: 1, borderBottomColor: C.bord },
	ptLeft: { flex: 1, borderRightWidth: 1, borderRightColor: C.bord },
	ptRight: { flex: 1 },
});
