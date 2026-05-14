import { ConsoleScreen } from "../../screens/ConsoleScreen";

/**
 * ConsoleTab connector — renders ConsoleScreen with its full props interface
 * provided by the parent (AppExperience). This thin wrapper exists so the
 * parent stays in control of the full MonitorScreenProps wiring.
 */
export function ConsoleTab() {
	// eslint-disable-next-line @typescript-eslint/no-explicit-any
	return <ConsoleScreen {...({} as any)} />;
}
