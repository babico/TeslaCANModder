import { StyleSheet, View } from "react-native";
import { colors, spacing } from "../design/tokens";

export interface DividerProps {
  variant?: "default" | "dark";
  vertical?: boolean;
}

export function Divider({ variant = "default", vertical = false }: DividerProps) {
  return (
    <View
      style={[
        vertical ? styles.vertical : styles.horizontal,
        variant === "dark" ? styles.dark : styles.light,
      ]}
    />
  );
}

const styles = StyleSheet.create({
  horizontal: {
    height: 1,
    width: "100%",
    marginVertical: spacing.xs,
  },
  vertical: {
    width: 1,
    alignSelf: "stretch",
    marginHorizontal: spacing.xs,
  },
  light: {
    backgroundColor: colors.border,
  },
  dark: {
    backgroundColor: colors.dashCardBorder,
  },
});
