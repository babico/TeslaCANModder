/**
 * Sheet — bottom sheet overlay primitive.
 *
 * Animates up from the bottom of the screen using Animated.Value.
 * Renders a translucent backdrop that dismisses on tap.
 * Uses React Native's Modal for correct z-order and safe-area behavior.
 *
 * NOTE: Modal is mocked in Jest (see environment-notes.md).
 */
import React, { useEffect, useRef } from "react";
import {
  Animated,
  Modal,
  Platform,
  Pressable,
  StyleSheet,
  Text,
  View,
  type ViewStyle,
} from "react-native";
import { colors, font, radius, spacing } from "../design/tokens";

export interface SheetProps {
  /** Controls visibility */
  visible: boolean;
  /** Called when the backdrop or close button is pressed */
  onClose: () => void;
  /** Optional title rendered in the handle area */
  title?: string;
  /** Content rendered inside the sheet */
  children?: React.ReactNode;
  /** Override the sheet container style */
  style?: ViewStyle;
  /** Approximate sheet height used for slide animation (default 360) */
  estimatedHeight?: number;
}

export function Sheet({
  visible,
  onClose,
  title,
  children,
  style,
  estimatedHeight = 360,
}: SheetProps) {
  const useNativeDriver = Platform.OS !== "web";
  const translateY = useRef(new Animated.Value(estimatedHeight)).current;
  const backdropOpacity = useRef(new Animated.Value(0)).current;

  useEffect(() => {
    if (visible) {
      Animated.parallel([
        Animated.timing(translateY, {
          toValue: 0,
          duration: 280,
          useNativeDriver,
        }),
        Animated.timing(backdropOpacity, {
          toValue: 1,
          duration: 200,
          useNativeDriver,
        }),
      ]).start();
    } else {
      Animated.parallel([
        Animated.timing(translateY, {
          toValue: estimatedHeight,
          duration: 220,
          useNativeDriver,
        }),
        Animated.timing(backdropOpacity, {
          toValue: 0,
          duration: 180,
          useNativeDriver,
        }),
      ]).start();
    }
  }, [visible, translateY, backdropOpacity, estimatedHeight]);

  return (
    <Modal
      visible={visible}
      transparent
      animationType="none"
      onRequestClose={onClose}
      statusBarTranslucent
    >
      {/* Backdrop */}
      <Animated.View
        style={[styles.backdrop, { opacity: backdropOpacity }]}
        pointerEvents={visible ? "auto" : "none"}
      >
        <Pressable style={StyleSheet.absoluteFill} onPress={onClose} />
      </Animated.View>

      {/* Sheet panel */}
      <Animated.View
        style={[
          styles.sheet,
          { transform: [{ translateY }] },
          style,
        ]}
      >
        {/* Drag handle */}
        <View style={styles.handleRow}>
          <View style={styles.handle} />
        </View>

        {title ? (
          <Text style={styles.title}>{title}</Text>
        ) : null}

        <View style={styles.content}>{children}</View>
      </Animated.View>
    </Modal>
  );
}

const styles = StyleSheet.create({
  backdrop: {
    position: "absolute",
    top: 0,
    left: 0,
    right: 0,
    bottom: 0,
    backgroundColor: "rgba(0,0,0,0.55)",
  },
  sheet: {
    position: "absolute",
    bottom: 0,
    left: 0,
    right: 0,
    backgroundColor: colors.dashCard,
    borderTopLeftRadius: radius.xl,
    borderTopRightRadius: radius.xl,
    borderTopWidth: 1,
    borderColor: colors.dashCardBorder,
    minHeight: 120,
    paddingBottom: spacing.xl4,
  },
  handleRow: {
    alignItems: "center",
    paddingTop: spacing.sm,
    paddingBottom: spacing.sm2,
  },
  handle: {
    width: 40,
    height: 4,
    borderRadius: radius.full,
    backgroundColor: colors.dashCardBorder,
  },
  title: {
    color: colors.dashValue,
    fontSize: font.size.lg,
    fontWeight: font.weight.semibold,
    paddingHorizontal: spacing.lg,
    paddingBottom: spacing.md,
  },
  content: {
    paddingHorizontal: spacing.lg,
  },
});
