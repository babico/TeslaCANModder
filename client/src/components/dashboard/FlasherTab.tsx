import { Suspense, lazy } from "react";
const FlasherScreen = lazy(() =>
	import("../../screens/FlasherScreen").then((m) => ({ default: m.FlasherScreen })),
);

/**
 * FlasherTab connector — renders FlasherScreen which uses
 * useBoardConnection internally for its own state needs.
 */
export function FlasherTab() {
	return (
		<Suspense fallback={null}>
			<FlasherScreen />
		</Suspense>
	);
}
