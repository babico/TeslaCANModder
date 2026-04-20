/**
 * useBreakpoint — responsive layout hook (C-03).
 *
 * Returns a breakpoint descriptor derived from the current window width.
 * All breakpoint values originate from the design token dictionary so
 * layout rules stay in sync with visual tokens.
 */

import { useWindowDimensions } from "react-native";
import { breakpoints } from "../design/tokens";

export type Breakpoint = "phone" | "phoneLandscape" | "tablet" | "desktop" | "cluster";

export interface BreakpointInfo {
  /** Active breakpoint tier */
  bp: Breakpoint;
  /** Current window width in logical pixels */
  width: number;
  /** Current window height in logical pixels */
  height: number;
  /** True when width ≥ tablet breakpoint (768) */
  isTablet: boolean;
  /** True when width ≥ desktop breakpoint (1024) */
  isDesktop: boolean;
  /** True when width < phone-landscape breakpoint (phone portrait, ≤ 479) */
  isPhonePortrait: boolean;
  /** True when this is a narrow phone (portrait or landscape) */
  isPhone: boolean;
  /** True when width ≥ cluster breakpoint (320) but < phone-landscape (480) */
  isCluster: boolean;
}

export function useBreakpoint(): BreakpointInfo {
  const { width, height } = useWindowDimensions();

  let bp: Breakpoint;
  if (width >= breakpoints.desktop) {
    bp = "desktop";
  } else if (width >= breakpoints.tablet) {
    bp = "tablet";
  } else if (width >= breakpoints.phoneLS) {
    bp = "phoneLandscape";
  } else if (width >= breakpoints.cluster) {
    bp = "cluster";
  } else {
    bp = "phone";
  }

  return {
    bp,
    width,
    height,
    isTablet: width >= breakpoints.tablet,
    isDesktop: width >= breakpoints.desktop,
    isPhonePortrait: width < breakpoints.phoneLS,
    isPhone: width < breakpoints.tablet,
    isCluster: width >= breakpoints.cluster && width < breakpoints.phoneLS,
  };
}
