/**
 * useReducedMotion — respects the platform "reduce motion" accessibility setting.
 *
 * On platforms that support AccessibilityInfo.isReduceMotionEnabled (iOS/Android),
 * subscribes to the system preference and returns true when the user has requested
 * reduced motion. Falls back to false on web/unsupported platforms.
 *
 * All animation hooks in this project should gate loops and transitions behind this.
 */
import { useEffect, useState } from "react";
import { AccessibilityInfo } from "react-native";

export function useReducedMotion(): boolean {
  const [reduced, setReduced] = useState(false);

  useEffect(() => {
    let cancelled = false;

    // Initial query
    AccessibilityInfo.isReduceMotionEnabled().then((value) => {
      if (!cancelled) setReduced(value);
    }).catch(() => {
      // Platform may not support this — leave as false
    });

    // Subscribe to changes
    const subscription = AccessibilityInfo.addEventListener(
      "reduceMotionChanged",
      (value) => {
        if (!cancelled) setReduced(value);
      }
    );

    return () => {
      cancelled = true;
      subscription.remove();
    };
  }, []);

  return reduced;
}
