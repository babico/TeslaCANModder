/**
 * useBlinkAnimation — drives a repeating opacity blink for turn signals and warnings.
 *
 * When `active` is true and reduced motion is NOT set, starts an Animated.loop
 * oscillating opacity between `minOpacity` and 1. When either condition makes it
 * stop, the opacity settles at 1 (fully visible).
 *
 * Blink cadence is controlled by the caller via `cycleDuration` (defaults to
 * motion.signalBlink = 600ms for turn signals, pass motion.warningPulse for
 * slower warning pulsing).
 *
 * Usage:
 *   const blinkAnim = useBlinkAnimation(isSignalActive, motion.signalBlink);
 *   <Animated.View style={{ opacity: blinkAnim }} />
 */
import { useEffect, useRef } from "react";
import { Animated, Platform } from "react-native";
import { motion } from "../design/tokens";
import { useReducedMotion } from "./useReducedMotion";

const USE_NATIVE_DRIVER = Platform.OS !== "web";

export function useBlinkAnimation(
	active: boolean,
	cycleDuration: number = motion.signalBlink,
	minOpacity: number = 0.15,
): Animated.Value {
	const anim = useRef(new Animated.Value(active ? 1 : 0)).current;
	const reduced = useReducedMotion();
	const loopRef = useRef<Animated.CompositeAnimation | null>(null);

	useEffect(() => {
		if (loopRef.current) {
			loopRef.current.stop();
			loopRef.current = null;
		}

		if (!active) {
			Animated.timing(anim, {
				toValue: 0,
				duration: motion.duration.fast,
				useNativeDriver: USE_NATIVE_DRIVER,
			}).start();
			return;
		}

		if (reduced) {
			// No animation — just show solid
			anim.setValue(1);
			return;
		}

		const half = cycleDuration / 2;
		const loop = Animated.loop(
			Animated.sequence([
				Animated.timing(anim, {
					toValue: 1,
					duration: half,
					useNativeDriver: USE_NATIVE_DRIVER,
				}),
				Animated.timing(anim, {
					toValue: minOpacity,
					duration: half,
					useNativeDriver: USE_NATIVE_DRIVER,
				}),
			]),
		);
		loopRef.current = loop;
		loop.start();

		return () => {
			loop.stop();
			loopRef.current = null;
		};
	}, [active, reduced, cycleDuration, minOpacity, anim]);

	return anim;
}

/**
 * useWarningPulse — slower pulses for persistent warning indicators.
 * Wraps useBlinkAnimation with motion.warningPulse defaults and higher min opacity.
 */
export function useWarningPulse(active: boolean): Animated.Value {
	return useBlinkAnimation(active, motion.warningPulse, 0.4);
}

/**
 * useAlarmPulse — fast pulses for critical alarms (critical SOC, system faults).
 * min opacity 0.2 for maximum contrast.
 */
export function useAlarmPulse(active: boolean): Animated.Value {
	return useBlinkAnimation(active, motion.alarmPulse, 0.2);
}

/**
 * useGaugeValue — interpolates a numeric value smoothly for gauge displays.
 * Returns an Animated.Value that chases the input value at gaugeTransition speed.
 *
 * Usage:
 *   const animVal = useGaugeValue(state.vehicleSpeed);
 *   // Map animVal to display coordinates via .interpolate()
 */
export function useGaugeValue(value: number): Animated.Value {
	const anim = useRef(new Animated.Value(value)).current;
	const reduced = useReducedMotion();

	useEffect(() => {
		if (reduced) {
			anim.setValue(value);
			return;
		}
		Animated.timing(anim, {
			toValue: value,
			duration: motion.gaugeTransition,
			useNativeDriver: false, // layout-driving animations need this false
		}).start();
	}, [value, reduced, anim]);

	return anim;
}
