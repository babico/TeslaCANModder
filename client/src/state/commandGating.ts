import {
	getCommandGate as getProtocolCommandGate,
	type BoardState,
	type CommandGate,
	type CommandName,
} from "@teslacanmodder/protocol";

export type { CommandGate };

export function getCommandGate(name: CommandName, state: BoardState): CommandGate {
	return getProtocolCommandGate(name, state);
}
