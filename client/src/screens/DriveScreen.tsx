/**
 * DriveScreen — Tesla CAN phone instrument cluster.
 *
 * Landscape: TopBar → [LeftPanel | SpeedDial | RightPanel] → BottomStrip
 * Portrait:  TopBar → [HudInfo?] → SpeedDial → [LeftPanel | RightPanel] → BottomStrip
 */

import { useEffect, useRef, useState } from "react";
import {
	Animated,
	Platform,
	Pressable,
	StyleSheet,
	Text,
	View,
	useWindowDimensions,
} from "react-native";
import type { BoardState } from "@teslacanmodder/protocol";
import { formatAutopilotTier, formatDriveMode, formatGear } from "@teslacanmodder/protocol";
import { colors, selectDashColors, font, radius, spacing, motion } from "../design/tokens";
import { useBreakpoint } from "../state/useBreakpoint";
import { useSpeedUnit } from "../state/useSpeedUnit";
import { useSocDisplay } from "../state/useSocDisplay";
import { usePowerCapture } from "../state/usePowerCapture";
import { useTheme } from "../state/useTheme";
import { useThemeState } from "../state/ThemeContext";
import { useGaugeMode } from "../state/useGaugeMode";
import type { GaugeMode } from "../state/useGaugeMode";
import { Badge } from "../ui/shadcn/badge";

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

function apBadgeVariant(s: ApClusterState): "default" | "destructive" | "outline" {
	if (s === "hands_warning") return "destructive";
	return "outline";
}

function apStateLabel(s: ApClusterState, tier: number): string {
	if (s === "unavailable") return "AP N/A";
	if (s === "hands_warning") return "Hands On";
	return formatAutopilotTier(tier);
}

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
		<Animated.Text style={[{ color, transform: [{ scale }], opacity }, grS.letter]}>
			{label}
		</Animated.Text>
	);
}

function GearRow({ gear }: { gear: number }) {
	return (
		<View className="flex-row items-center" style={{ gap: spacing.lg }}>
			{GEAR_DEFS.map(([id]) => (
				<GearLetter key={id} id={id} active={id === gear} />
			))}
		</View>
	);
}

const grS = StyleSheet.create({
	letter: { fontSize: font.size.xl2, fontWeight: font.weight.extrabold, letterSpacing: 2 },
});

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
		<View className="gap-0.5">
			<View className="flex-row justify-between">
				<Text style={[glS.label]}>{label}</Text>
				<Text style={[glS.value, muted ? glS.muted : undefined]}>{value}</Text>
			</View>
			<View style={[glS.track]}>
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
	label: {
		color: C.lbl,
		fontSize: font.size.xs,
		fontWeight: "700",
		textTransform: "uppercase",
		letterSpacing: 0.5,
	},
	value: { color: C.sub, fontSize: font.size.xs, fontWeight: "600" },
	muted: { color: C.bord },
	track: { height: 4, borderRadius: radius.full, backgroundColor: C.barBg, overflow: "hidden" },
	fill: { height: "100%", borderRadius: radius.full },
});

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
		<View className="gap-0.5">
			<View style={[pbS.track]}>
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
			<View className="flex-row justify-between items-center">
				<Text style={[{ color: isReg ? C.cyan : C.grn }, pbS.labelStyle]}>
					{isReg
						? `${"\u25BC"} ${Math.abs(power).toFixed(0)} kW  regen`
						: `${"\u25B2"} ${power.toFixed(0)} kW`}
				</Text>
				{onReset ? (
					<Pressable onPress={onReset} hitSlop={8}>
						<Text
							style={{
								color: C.lbl,
								fontSize: font.size.xs,
								fontWeight: "700",
								opacity: 0.5,
							}}
						>
							RST
						</Text>
					</Pressable>
				) : null}
			</View>
		</View>
	);
}

const pbS = StyleSheet.create({
	track: {
		height: 6,
		borderRadius: radius.full,
		backgroundColor: C.barBg,
		overflow: "hidden",
		position: "relative",
	},
	fill: { height: "100%", borderRadius: radius.full },
	mark: { position: "absolute", top: 0, width: 2, height: "100%", opacity: 0.8 },
	labelStyle: { fontSize: font.size.xs, fontWeight: "600" },
});

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
		: "\u2014";
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
			className="items-center justify-center border-3 bg-card"
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
				<View className="items-center gap-0.5 w-[68%]">
					<Text style={[sdS.powerNum, { color: pwr < 0 ? C.cyan : C.grn }]}>
						{Number.isFinite(pwr) ? `${pwr > 0 ? "+" : ""}${pwr.toFixed(1)}` : "\u2014"}
					</Text>
					<Text style={sdS.subLbl}>kW</Text>
				</View>
			) : gaugeMode === "perf" ? (
				<View className="items-center gap-0.5 w-[68%]">
					<View className="w-full h-[5px] rounded-full bg-[#1e293b] overflow-hidden relative">
						<View className="absolute left-1/2 w-px h-full bg-muted" />
						<View
							className="absolute w-2 h-full rounded-full bg-cyan"
							style={{
								left: `${50 + steerNorm * 44}%` as `${number}%`,
								marginLeft: -4,
							}}
						/>
					</View>
					<Text style={sdS.subLbl}>
						{(state.steeringAngle ?? 0).toFixed(0)}° · {pedal.toFixed(0)}%
					</Text>
				</View>
			) : (
				<View className="items-center gap-0.5 w-[68%]">
					<View className="w-full h-[5px] rounded-full bg-[#1e293b] overflow-hidden">
						<View
							className="h-full rounded-full bg-cyan"
							style={{ width: `${pedal}%` as `${number}%` }}
						/>
					</View>
					<Text style={sdS.subLbl}>PEDAL {pedal.toFixed(0)}%</Text>
				</View>
			)}
		</View>
	);
}

const sdS = StyleSheet.create({
	ring: { gap: 4, elevation: 16 },
	unit: {
		color: C.dim,
		fontSize: font.size.xs,
		fontWeight: "700",
		letterSpacing: 2,
		textTransform: "uppercase",
	},
	speed: { color: C.val, fontWeight: "900", letterSpacing: -3 },
	subLbl: {
		color: C.dim,
		fontSize: font.size.xs,
		fontWeight: "700",
		letterSpacing: 0.5,
		textTransform: "uppercase",
	},
	powerNum: { fontSize: font.size.xl3, fontWeight: "800", letterSpacing: -1 },
});

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
		<View className="flex-1 px-2 py-2 justify-center" style={{ gap: spacing.xs2 + 2 }}>
			<Pressable onPress={cycleMode} className="flex-row items-baseline gap-1" hitSlop={8}>
				<Text style={[lpS.socNum, { color: socColor }]}>{primaryValue}</Text>
				<Text style={lpS.socMode}>{modeLabel}</Text>
			</Pressable>
			<View className="h-2 rounded-full bg-[#1e293b] overflow-hidden">
				<View
					className="h-full rounded-full"
					style={{ width: `${soc}%` as `${number}%`, backgroundColor: socColor }}
				/>
			</View>
			{range !== null ? (
				<Text className="text-sm font-semibold text-muted">{range.toFixed(0)} km est</Text>
			) : null}
			{Number.isFinite(pwr) ? (
				<Text className="text-sm font-bold" style={{ color: pwrColor }}>
					{pwr < 0 ? "\u25BC" : "\u25B2"} {Math.abs(pwr).toFixed(1)} kW
				</Text>
			) : null}
			<View className="h-px bg-border my-0.5" />
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
					value={`${state.bmsTempMin.toFixed(0)}\u2013${state.bmsTempMax.toFixed(0)} \u00B0C`}
					pct={Math.min(Math.max(0, state.bmsTempMax - state.bmsTempMin) / 60, 1)}
					color={C.amb}
				/>
			) : null}
		</View>
	);
}

const lpS = StyleSheet.create({
	socNum: { fontSize: font.size.xl3, fontWeight: "900", letterSpacing: -1 },
	socMode: {
		color: C.lbl,
		fontSize: font.size.xs,
		fontWeight: "700",
		letterSpacing: 1,
		textTransform: "uppercase",
	},
});

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
		<View
			className="flex-row items-center gap-1 px-2 py-1 border rounded-md bg-card"
			style={{ borderColor: color, opacity: active ? 1 : 0.28 }}
		>
			<Text className="text-lg font-black" style={{ color }}>
				{side === "L" ? "\u25C4" : "\u25BA"}
			</Text>
			<Text className="text-xs font-bold uppercase tracking-wider text-muted">
				{side === "L" ? "LEFT" : "RIGHT"}
			</Text>
			{label ? (
				<Text className="text-xs font-extrabold uppercase ml-auto" style={{ color }}>
					{label}
				</Text>
			) : null}
			{bsmLevel > 0 ? (
				<Text className="text-xs font-bold opacity-80" style={{ color }}>
					BSM {bsmLevel}
				</Text>
			) : null}
		</View>
	);
}

function DoorStateCar({ state }: { state: BoardState }) {
	const partStyle = (open: boolean) => ({
		backgroundColor: open ? C.cyan : C.card,
		borderColor: open ? C.grn : C.bord,
	});
	return (
		<View className="w-[54px] h-[82px] items-center justify-center">
			<View
				className="absolute bottom-1 w-9 h-1 rounded-full opacity-50"
				style={{ backgroundColor: C.bord }}
			/>
			<View
				className="w-10 h-[74px] rounded-xl border overflow-hidden relative"
				style={{ borderColor: C.sub, backgroundColor: C.card }}
			>
				<View
					className="absolute left-[9px] top-5 w-[22px] h-[34px] border rounded-sm"
					style={{ borderColor: C.bord }}
				/>
				<View
					className="absolute left-2 top-[3px] w-6 h-2 border rounded-sm"
					style={partStyle(state.frunkOpen)}
				/>
				<View
					className="absolute left-2 bottom-[3px] w-6 h-[9px] border rounded-sm"
					style={partStyle(state.trunkOpen)}
				/>
				<View
					className="absolute left-0 top-5 w-[5px] h-[14px] border rounded-sm"
					style={partStyle(state.driverDoorOpen)}
				/>
				<View
					className="absolute right-0 top-5 w-[5px] h-[14px] border rounded-sm"
					style={partStyle(state.doorFrontRightOpen)}
				/>
				<View
					className="absolute left-0 top-[38px] w-[5px] h-[14px] border rounded-sm"
					style={partStyle(state.doorRearLeftOpen)}
				/>
				<View
					className="absolute right-0 top-[38px] w-[5px] h-[14px] border rounded-sm"
					style={partStyle(state.doorRearRightOpen)}
				/>
				<View className="absolute left-[7px] top-[1px] w-[6px] h-[2px] rounded-full bg-amber" />
				<View className="absolute right-[7px] top-[1px] w-[6px] h-[2px] rounded-full bg-amber" />
				<View className="absolute left-[7px] bottom-[1px] w-[6px] h-[2px] rounded-full bg-red" />
				<View className="absolute right-[7px] bottom-[1px] w-[6px] h-[2px] rounded-full bg-red" />
			</View>
		</View>
	);
}

function RightPanel({ state }: { state: BoardState }) {
	const openDoors: string[] = [];
	if (state.driverDoorOpen) openDoors.push("Driver");
	if (state.doorFrontRightOpen) openDoors.push("FR");
	if (state.doorRearLeftOpen) openDoors.push("RL");
	if (state.doorRearRightOpen) openDoors.push("RR");
	if (state.frunkOpen) openDoors.push("Frunk");
	if (state.trunkOpen) openDoors.push("Trunk");
	const hasOpen = openDoors.length > 0;
	const fmtKph = (v: number) => (v > 0 ? `${v.toFixed(0)} kph` : "\u2014");
	const cruiseActive = state.cruiseSetSpeedKph > 0 || state.maxSpeedKph > 0;

	return (
		<View className="flex-1 px-2 py-2 justify-center gap-1">
			<TurnChip side="L" bsmLevel={state.bsmLeftLevel} turnOn={state.turnSignalLeft} />
			<View className="h-px bg-border" />
			<View className="flex-row items-center gap-2">
				<DoorStateCar state={state} />
				<View className="flex-1 gap-0.5">
					<Text className="text-xs font-bold uppercase tracking-wider text-muted">
						DOORS
					</Text>
					<Text
						className="text-sm font-extrabold uppercase"
						style={{ color: hasOpen ? C.amb : C.grn }}
					>
						{hasOpen ? `${openDoors.length} OPEN` : "CLOSED"}
					</Text>
					{hasOpen ? (
						<Text className="text-xs text-muted" numberOfLines={2}>
							{openDoors.join(" \u00B7 ")}
						</Text>
					) : null}
				</View>
			</View>
			<View className="h-px bg-border" />
			<View className="gap-0.5">
				<Text className="text-xs font-bold uppercase tracking-wider text-muted">
					CRUISE
				</Text>
				<Text
					className="text-sm font-extrabold uppercase"
					style={{ color: cruiseActive ? C.amb : C.lbl }}
				>
					{cruiseActive ? "SET" : "IDLE"}
				</Text>
				<Text className="text-xs font-semibold text-muted">
					CC {fmtKph(state.cruiseSetSpeedKph)}
				</Text>
				<Text className="text-xs font-semibold text-muted">
					Max {fmtKph(state.maxSpeedKph)}
				</Text>
			</View>
			<View className="h-px bg-border" />
			<TurnChip side="R" bsmLevel={state.bsmRightLevel} turnOn={state.turnSignalRight} />
		</View>
	);
}

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
		<View className="flex-row items-center justify-between px-3 py-2 bg-card border-b border-border min-h-[40px]">
			<View className="flex-row items-center gap-1 flex-1 overflow-hidden">
				<View
					className={`w-1.5 h-1.5 rounded-full ${state.chassisOnline ? "bg-green-500" : "bg-red-500"}`}
				/>
				<Text className="text-xs font-semibold text-muted">
					{state.chassisOnline ? "ONLINE" : "OFFLINE"}
					{state.vehicleOnline ? " \u00B7 V" : ""}
					{state.bodyOnline ? " \u00B7 B" : ""}
				</Text>
				<Text className="text-xs text-muted">\u00B7</Text>
				<Text className="text-xs font-bold text-foreground">
					{formatGear(state.gearState)}
				</Text>
				<Text className="text-xs text-muted">\u00B7</Text>
				<Text className="text-xs font-semibold uppercase text-muted flex-shrink">
					{formatDriveMode(state.currentDriveMode ?? state.driveMode)}
				</Text>
			</View>
			<View className="flex-row items-center gap-1">
				{state.fsd ? <Badge label="FSD" variant="default" /> : null}
				{state.nag ? <Badge label="NAG" variant="destructive" /> : null}
				{state.dasHandsOn >= 1 ? (
					<Badge label="\u26A0 HANDS" variant="destructive" />
				) : null}
				{apState !== "unavailable" ? (
					<Badge
						label={apStateLabel(apState, state.gtwAutopilotTier)}
						variant={apBadgeVariant(apState)}
					/>
				) : null}
				{onToggleHud ? (
					<Pressable
						onPress={onToggleHud}
						className={`px-1 py-0.5 rounded-sm border ${
							showHud ? "border-cyan bg-muted" : "border-border bg-transparent"
						}`}
						hitSlop={8}
					>
						<Text
							className={`text-xs font-bold ${showHud ? "text-cyan" : "text-muted"}`}
						>
							{showHud ? "\u25B2" : "\u25BC"}
						</Text>
					</Pressable>
				) : null}
				{source !== "force" ? (
					<Pressable
						onPress={() => setOverride(isDark ? "light" : "dark")}
						hitSlop={8}
						className="p-1"
					>
						<Text className="text-lg text-muted">{isDark ? "\u2600" : "\u263E"}</Text>
					</Pressable>
				) : null}
			</View>
		</View>
	);
}

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
		<View className="flex-row flex-wrap gap-2 px-3 py-1.5 bg-card border-b border-border">
			{(
				[
					["Board", state.hardware ?? "--"],
					["Driver", state.driver ?? "--"],
					["Uptime", formatUptime(state.uptime)],
					["Variant", (state.variant ?? "--").toUpperCase()],
					["Rate", state.rate > 0 ? `${state.rate} Hz` : "--"],
				] as [string, string][]
			).map(([lbl, val]) => (
				<View key={lbl} className="min-w-[72px] gap-0.5">
					<Text className="text-xs uppercase tracking-wider text-muted">{lbl}</Text>
					<Text className="text-xs font-semibold text-muted">{val}</Text>
				</View>
			))}
		</View>
	);
}

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
		<View className="px-3 py-2 gap-2 bg-muted border-t border-border">
			<PowerBar
				power={state.bmsPower}
				maxDrive={maxDrive}
				maxRegen={maxRegen}
				onReset={onReset}
			/>
			<View className="flex-row flex-wrap gap-1">
				<StatChip label="Mode" value={driveMode} accent={driveMode !== "OFF"} />
				<StatChip label="FSD" value={state.fsd ? "ON" : "OFF"} accent={state.fsd} />
				{state.hasBms ? (
					<>
						<StatChip
							label="Regen"
							value={state.bmsMaxRegenPower.toFixed(0)}
							unit="kW"
						/>
						<StatChip
							label="Temp"
							value={`${state.bmsTempMin.toFixed(0)}\u2013${state.bmsTempMax.toFixed(0)}`}
							unit="\u00B0C"
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
							accent={low}
						/>
					);
				})}
			</View>
		</View>
	);
}

function StatChip({
	label,
	value,
	unit,
	accent,
}: {
	label: string;
	value: string;
	unit?: string;
	accent?: boolean;
}) {
	return (
		<View className="min-w-[112px] flex-1 rounded-lg border border-border bg-card px-2 py-2 gap-0.5">
			<Text className="text-xs font-semibold uppercase tracking-wide text-muted">
				{label}
			</Text>
			<View className="flex-row items-end gap-0.5">
				<Text
					className={`text-sm font-bold ${accent ? "text-primary" : "text-foreground"}`}
				>
					{value}
				</Text>
				{unit ? <Text className="text-xs font-medium text-muted">{unit}</Text> : null}
			</View>
		</View>
	);
}

export interface DriveScreenProps {
	boardState: BoardState;
}

export function DriveScreen({ boardState: s }: DriveScreenProps) {
	const { isDark: _isDark } = useThemeState();
	const _colors = selectDashColors(_isDark);
	const { width, height } = useWindowDimensions();
	const bp = useBreakpoint();
	const isLand = bp.bp === "phoneLandscape" || (bp.isPhone && width > height);
	const { mode: gaugeMode, cycleMode } = useGaugeMode();
	const [showHud, setShowHud] = useState(false);
	const { maxDrive, maxRegen, reset } = usePowerCapture(s.bmsPower);

	if (isLand) {
		return (
			<View className="flex-1 flex-col bg-background">
				<TopBar state={s} />
				<View className="flex-1 flex-row items-stretch">
					<View className="w-[178px] border-r border-border">
						<LeftPanel state={s} />
					</View>
					<View className="flex-1 items-center justify-center py-2">
						<SpeedDial state={s} gaugeMode={gaugeMode} onCycle={cycleMode} size={220} />
					</View>
					<View className="w-[178px] border-l border-border">
						<RightPanel state={s} />
					</View>
				</View>
				<BottomStrip state={s} maxDrive={maxDrive} maxRegen={maxRegen} onReset={reset} />
			</View>
		);
	}

	return (
		<View className="flex-1 flex-col bg-background">
			<TopBar state={s} showHud={showHud} onToggleHud={() => setShowHud((v) => !v)} />
			{showHud ? <HudInfo state={s} /> : null}
			<View className="items-center justify-center py-3 border-b border-border">
				<SpeedDial state={s} gaugeMode={gaugeMode} onCycle={cycleMode} size={220} />
			</View>
			<View className="flex-1 flex-row border-b border-border">
				<View className="flex-1 border-r border-border">
					<LeftPanel state={s} />
				</View>
				<View className="flex-1">
					<RightPanel state={s} />
				</View>
			</View>
			<BottomStrip state={s} maxDrive={maxDrive} maxRegen={maxRegen} onReset={reset} />
		</View>
	);
}
