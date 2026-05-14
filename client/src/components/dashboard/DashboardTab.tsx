import { DashboardScreen } from "../../screens/DashboardScreen";
import { useBoardInstanceState } from "../../state/BoardStateContext";

export function DashboardTab() {
	const { boardState } = useBoardInstanceState();

	return <DashboardScreen boardState={boardState} />;
}
