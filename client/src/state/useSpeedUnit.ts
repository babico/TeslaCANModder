/**
 * useSpeedUnit — D-02 speed unit hook.
 *
 * Provides a tap-to-cycle unit (km/h → mph → km/h) with in-process
 * persistence via a module-level cache (survives re-mounts).
 */

import { useCallback, useState } from "react";

export type SpeedUnit = "kmh" | "mph";

const KMH_TO_MPH = 0.621371;

// Module-level cache — survives component remounts within a session
let cachedUnit: SpeedUnit = "kmh";

function convertSpeed(speedKmh: number, unit: SpeedUnit): number {
  return unit === "mph" ? speedKmh * KMH_TO_MPH : speedKmh;
}

function unitLabel(unit: SpeedUnit): string {
  return unit === "mph" ? "mph" : "km/h";
}

interface SpeedUnitResult {
  unit: SpeedUnit;
  /** Cycles km/h → mph → km/h */
  cycleUnit: () => void;
  /** Convert a raw km/h value to the active unit */
  convert: (speedKmh: number) => number;
  /** Display label for the active unit */
  label: string;
}

export function useSpeedUnit(): SpeedUnitResult {
  const [unit, setUnit] = useState<SpeedUnit>(cachedUnit);

  const cycleUnit = useCallback(() => {
    setUnit((prev) => {
      const next: SpeedUnit = prev === "kmh" ? "mph" : "kmh";
      cachedUnit = next;
      return next;
    });
  }, []);

  return {
    unit,
    cycleUnit,
    convert: (speedKmh: number) => convertSpeed(speedKmh, unit),
    label: unitLabel(unit),
  };
}
