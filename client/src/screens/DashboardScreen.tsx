import { useMemo, useState } from "react";
import { Pressable, ScrollView, Text, View } from "react-native";

import { DriveScreen } from "./DriveScreen";
import { TelemetryPanel } from "../components/TelemetryPanel";
import { IntegrationPanel } from "../components/IntegrationPanel";
import { UtilityPanel } from "../components/UtilityPanel";
import { useBreakpoint } from "../state/useBreakpoint";
import { useBoardInstanceState } from "../state/BoardStateContext";
import { Tabs, TabsList, TabsTrigger, TabsContent } from "../ui/shadcn/tabs";
import { Sheet } from "../ui/shadcn/sheet";

import { Card, CardHeader, CardTitle, CardContent } from "../ui/shadcn/card";

type DashboardSection = "overview" | "drive";

export function DashboardScreen() {
	const { boardState } = useBoardInstanceState();
	const [section, setSection] = useState<DashboardSection>("overview");
	const [drawerOpen, setDrawerOpen] = useState(false);
	const bp = useBreakpoint();

	const sectionLabel = section === "overview" ? "Overview" : "Drive";

	const headline = useMemo(() => {
		const status =
			boardState.chassisOnline || boardState.vehicleOnline || boardState.bodyOnline
				? "CAN Active"
				: "Offline";
		const speed = Number.isFinite(boardState.vehicleSpeed)
			? `${boardState.vehicleSpeed.toFixed(0)} km/h`
			: "--";
		return { status, speed };
	}, [boardState]);

	const isDesktop = bp.isDesktop;

	if (!isDesktop && section === "drive") {
		return (
			<View className="flex-1 bg-background">
				<Pressable
					onPress={() => setDrawerOpen(true)}
					className="px-3 pt-2 pb-2 border-b border-border bg-background"
				>
					<View className="bg-card border border-border rounded-lg px-3 min-h-[46px] justify-center gap-0.5">
						<Text className="text-card-foreground text-sm font-bold">Dashboard</Text>
						<Text className="text-muted-foreground text-xs font-semibold">
							Section: {sectionLabel}
						</Text>
					</View>
				</Pressable>

				<DriveScreen boardState={boardState} />

				<Sheet
					open={drawerOpen}
					onClose={() => setDrawerOpen(false)}
					title="Dashboard Sections"
				>
					<Pressable
						className="min-h-[42px] rounded-lg border border-border bg-muted justify-center px-3"
						onPress={() => {
							setSection("overview");
							setDrawerOpen(false);
						}}
					>
						<Text className="text-sm font-semibold text-foreground">Overview</Text>
					</Pressable>
					<Pressable
						className="min-h-[42px] rounded-lg border border-primary bg-background justify-center px-3"
						onPress={() => {
							setSection("drive");
							setDrawerOpen(false);
						}}
					>
						<Text className="text-sm font-semibold text-foreground">Drive</Text>
					</Pressable>
				</Sheet>
			</View>
		);
	}

	if (!isDesktop) {
		return (
			<View className="flex-1 bg-background">
				<Pressable
					onPress={() => setDrawerOpen(true)}
					className="px-3 pt-2 pb-2 border-b border-border bg-background"
				>
					<View className="bg-card border border-border rounded-lg px-3 min-h-[46px] justify-center gap-0.5">
						<Text className="text-card-foreground text-sm font-bold">Dashboard</Text>
						<Text className="text-muted-foreground text-xs font-semibold">
							Section: {sectionLabel}
						</Text>
					</View>
				</Pressable>

				<ScrollView
					className="flex-1 px-3 gap-3"
					contentContainerStyle={{ paddingBottom: 40 }}
				>
					<Card className="mt-3">
						<CardHeader>
							<CardTitle>Dashboard</CardTitle>
						</CardHeader>
						<CardContent className="gap-1">
							<Text className="text-sm font-semibold text-muted-foreground">
								{headline.status} · {headline.speed}
							</Text>
							<Text className="text-xs text-muted-foreground leading-4">
								Telemetry shows vehicle health, Integrations tracks external links,
								and Utility covers safety/feature helper states.
							</Text>
							<Text className="text-xs text-warning font-semibold leading-4">
								High-risk actions should only be executed while parked or in a
								controlled test environment.
							</Text>
						</CardContent>
					</Card>

					<TelemetryPanel state={boardState} />
					<IntegrationPanel state={boardState} />
					<UtilityPanel state={boardState} />
				</ScrollView>

				<Sheet
					open={drawerOpen}
					onClose={() => setDrawerOpen(false)}
					title="Dashboard Sections"
				>
					<Pressable
						className="min-h-[42px] rounded-lg border border-primary bg-background justify-center px-3"
						onPress={() => {
							setSection("overview");
							setDrawerOpen(false);
						}}
					>
						<Text className="text-sm font-semibold text-foreground">Overview</Text>
					</Pressable>
					<Pressable
						className="min-h-[42px] rounded-lg border border-border bg-muted justify-center px-3"
						onPress={() => {
							setSection("drive");
							setDrawerOpen(false);
						}}
					>
						<Text className="text-sm font-semibold text-foreground">Drive</Text>
					</Pressable>
				</Sheet>
			</View>
		);
	}

	return (
		<View className="flex-1 bg-background">
			<Tabs value={section} onChange={(v) => setSection(v as DashboardSection)}>
				<TabsList className="px-4 pt-2">
					<TabsTrigger value="overview" label="Overview" />
					<TabsTrigger value="drive" label="Drive" />
				</TabsList>
				<TabsContent value="overview">
					<ScrollView
						className="flex-1 px-4 py-3 gap-3"
						contentContainerStyle={{ paddingBottom: 40 }}
					>
						<Card>
							<CardHeader>
								<CardTitle>Dashboard</CardTitle>
							</CardHeader>
							<CardContent className="gap-1">
								<Text className="text-sm font-semibold text-muted-foreground">
									{headline.status} · {headline.speed}
								</Text>
								<Text className="text-xs text-muted-foreground leading-4">
									Telemetry shows vehicle health, Integrations tracks external
									links, and Utility covers safety/feature helper states.
								</Text>
								<Text className="text-xs text-warning font-semibold leading-4">
									High-risk actions should only be executed while parked or in a
									controlled test environment.
								</Text>
							</CardContent>
						</Card>

						<TelemetryPanel state={boardState} />
						<IntegrationPanel state={boardState} />
						<UtilityPanel state={boardState} />
					</ScrollView>
				</TabsContent>
				<TabsContent value="drive">
					<DriveScreen boardState={boardState} />
				</TabsContent>
			</Tabs>
		</View>
	);
}
