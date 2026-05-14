import { Suspense, lazy } from "react";
const DocsScreen = lazy(() =>
	import("../../screens/DocsScreen").then((m) => ({ default: m.DocsScreen })),
);

interface DocsTabProps {
	activeDocRoute: string;
	onNavigateDoc: (docRoute: string) => void;
}

export function DocsTab({ activeDocRoute, onNavigateDoc }: DocsTabProps) {
	return (
		<Suspense fallback={null}>
			<DocsScreen activeDocRoute={activeDocRoute} onNavigateDoc={onNavigateDoc} />
		</Suspense>
	);
}
