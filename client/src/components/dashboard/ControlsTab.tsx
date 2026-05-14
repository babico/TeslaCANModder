import { ControlsScreen } from "../../screens/ControlsScreen";
import { useBoardInstanceState } from "../../state/BoardStateContext";
import { useCommandActions } from "../../state/CommandContext";

export function ControlsTab() {
	const { boardState } = useBoardInstanceState();
	const { runCommand } = useCommandActions();

	const handleRunCommand = (name: Parameters<typeof runCommand>[0], args?: string) => {
		void runCommand(name, args);
	};

	return <ControlsScreen boardState={boardState} onRunCommand={handleRunCommand} />;
}
