import { useMemo, useState } from "react";
import { Modal, Pressable, ScrollView, StyleSheet, Text, View } from "react-native";
import type { BoardState } from "@teslacanmodder/protocol";

import { DriveScreen } from "./DriveScreen";
import { TelemetryPanel } from "../components/TelemetryPanel";
import { IntegrationPanel } from "../components/IntegrationPanel";
import { UtilityPanel } from "../components/UtilityPanel";
import { colors, font, radius, spacing } from "../design/tokens";

type DashboardSection = "overview" | "drive";

export interface DashboardScreenProps {
  boardState: BoardState;
}

export function DashboardScreen({ boardState }: DashboardScreenProps) {
  const [section, setSection] = useState<DashboardSection>("overview");
  const [drawerOpen, setDrawerOpen] = useState(false);

  const sectionLabel = section === "overview" ? "Overview" : "Drive";

  const headline = useMemo(() => {
    const status = boardState.chassisOnline || boardState.vehicleOnline || boardState.bodyOnline
      ? "CAN Active"
      : "Offline";
    const speed = Number.isFinite(boardState.vehicleSpeed) ? `${boardState.vehicleSpeed.toFixed(0)} km/h` : "--";
    return { status, speed };
  }, [boardState]);

  if (section === "drive") {
    return (
      <View style={styles.container}>
        <View style={styles.selectorBar}>
          <Pressable onPress={() => setDrawerOpen(true)} style={styles.selectorTrigger}>
            <Text style={styles.selectorTitle}>Dashboard</Text>
            <Text style={styles.selectorMeta}>Section: {sectionLabel}</Text>
          </Pressable>
        </View>

        <DriveScreen boardState={boardState} />

        <Modal transparent animationType="fade" visible={drawerOpen} onRequestClose={() => setDrawerOpen(false)}>
          <View style={styles.drawerRoot}>
            <Pressable style={styles.drawerBackdrop} onPress={() => setDrawerOpen(false)} />
            <View style={styles.drawerPanel}>
              <Text style={styles.drawerTitle}>Dashboard Sections</Text>
              <Pressable
                style={[styles.drawerItem]}
                onPress={() => {
                  setSection("overview");
                  setDrawerOpen(false);
                }}
              >
                <Text style={styles.drawerItemLabel}>Overview</Text>
              </Pressable>
              <Pressable
                style={[styles.drawerItem, styles.drawerItemActive]}
                onPress={() => {
                  setSection("drive");
                  setDrawerOpen(false);
                }}
              >
                <Text style={styles.drawerItemLabel}>Drive</Text>
              </Pressable>
            </View>
          </View>
        </Modal>
      </View>
    );
  }

  return (
    <View style={styles.container}>
      <View style={styles.selectorBar}>
        <Pressable onPress={() => setDrawerOpen(true)} style={styles.selectorTrigger}>
          <Text style={styles.selectorTitle}>Dashboard</Text>
          <Text style={styles.selectorMeta}>Section: {sectionLabel}</Text>
        </Pressable>
      </View>

      <ScrollView contentContainerStyle={styles.content}>
        <View style={styles.heroCard}>
          <Text style={styles.heroTitle}>Dashboard</Text>
          <Text style={styles.heroMeta}>{headline.status} · {headline.speed}</Text>
        </View>

        <TelemetryPanel state={boardState} />
        <IntegrationPanel state={boardState} />
        <UtilityPanel state={boardState} />
      </ScrollView>

      <Modal transparent animationType="fade" visible={drawerOpen} onRequestClose={() => setDrawerOpen(false)}>
        <View style={styles.drawerRoot}>
          <Pressable style={styles.drawerBackdrop} onPress={() => setDrawerOpen(false)} />
          <View style={styles.drawerPanel}>
            <Text style={styles.drawerTitle}>Dashboard Sections</Text>
            <Pressable
              style={[styles.drawerItem, styles.drawerItemActive]}
              onPress={() => {
                setSection("overview");
                setDrawerOpen(false);
              }}
            >
              <Text style={styles.drawerItemLabel}>Overview</Text>
            </Pressable>
            <Pressable
              style={[styles.drawerItem]}
              onPress={() => {
                setSection("drive");
                setDrawerOpen(false);
              }}
            >
              <Text style={styles.drawerItemLabel}>Drive</Text>
            </Pressable>
          </View>
        </View>
      </Modal>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: colors.dashBackground,
  },
  selectorBar: {
    paddingHorizontal: spacing.md,
    paddingTop: spacing.sm,
    paddingBottom: spacing.sm,
    borderBottomWidth: 1,
    borderBottomColor: colors.dashCardBorder,
    backgroundColor: colors.dashBackground,
  },
  selectorTrigger: {
    backgroundColor: colors.dashCard,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: radius.md,
    minHeight: 46,
    paddingHorizontal: spacing.md,
    justifyContent: "center",
    gap: 2,
  },
  selectorTitle: {
    color: colors.dashValue,
    fontSize: font.size.md,
    fontWeight: font.weight.bold,
  },
  selectorMeta: {
    color: colors.dashSecondary,
    fontSize: font.size.xs,
    fontWeight: font.weight.semibold,
  },
  drawerRoot: {
    flex: 1,
    flexDirection: "row",
  },
  drawerBackdrop: {
    flex: 1,
    backgroundColor: "rgba(3, 7, 18, 0.45)",
  },
  drawerPanel: {
    width: 260,
    backgroundColor: colors.dashCard,
    borderLeftWidth: 1,
    borderLeftColor: colors.dashCardBorder,
    paddingTop: spacing.xl,
    paddingHorizontal: spacing.md,
    gap: spacing.sm,
  },
  drawerTitle: {
    color: colors.dashValue,
    fontSize: font.size.md,
    fontWeight: font.weight.bold,
    marginBottom: spacing.xs,
  },
  drawerItem: {
    minHeight: 42,
    borderRadius: radius.md,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    backgroundColor: colors.dashBackground,
    justifyContent: "center",
    paddingHorizontal: spacing.md,
  },
  drawerItemActive: {
    borderColor: colors.dashPrimary,
    backgroundColor: colors.backgroundDarkSubtle,
  },
  drawerItemLabel: {
    color: colors.dashLabel,
    fontSize: font.size.sm,
    fontWeight: font.weight.semibold,
  },
  content: {
    padding: spacing.md,
    gap: spacing.md,
    paddingBottom: spacing.xl2,
  },
  heroCard: {
    backgroundColor: colors.dashCard,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: radius.lg,
    padding: spacing.md,
    gap: spacing.xs,
  },
  heroTitle: {
    color: colors.dashValue,
    fontSize: font.size.lg,
    fontWeight: font.weight.bold,
  },
  heroMeta: {
    color: colors.dashSecondary,
    fontSize: font.size.sm,
    fontWeight: font.weight.semibold,
  },
});

export default DashboardScreen;
