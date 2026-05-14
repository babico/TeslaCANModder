/**
 * TransportContext — smoke tests
 */

jest.mock("@teslacanmodder/protocol", () => ({
	initialBoardState: {},
	BUS_NAMES: {},
	commands: {},
}));

import React from "react";
import { render } from "@testing-library/react-native";
import { Text } from "react-native";

// Mock the HardwareController and its dependencies
const mockController = {
	connectViaRestApi: jest.fn(),
	connectViaBle: jest.fn(),
	connectViaBluetoothComPort: jest.fn(),
	connectViaComPort: jest.fn(),
	disconnectTransport: jest.fn(async () => undefined),
	activeTransportType: "http",
	runCommand: jest.fn(async () => ({ ok: true, command: "test", responseText: "ok" })),
	readStatus: jest.fn(async () => ({ raw: {} })),
};

// Use factory-style imports that work with the mock
import {
	TransportProvider,
	useTransportState,
	useTransportActions,
} from "../../src/state/TransportContext";

function TestConsumer() {
	const state = useTransportState();
	return <Text testID="transport">{state.selectedTransportType}</Text>;
}

describe("TransportContext", () => {
	it("defaults to HTTP transport", () => {
		const { getByTestId } = render(
			<TransportProvider controller={mockController as any}>
				<TestConsumer />
			</TransportProvider>,
		);
		expect(getByTestId("transport").props.children).toBe("http");
	});

	it("exposes setSelectedTransportType action and reflects the change", () => {
		function Toggler() {
			const state = useTransportState();
			const actions = useTransportActions();
			return (
				<Text testID="transport" onPress={() => actions.setSelectedTransportType("ble")}>
					{state.selectedTransportType}
				</Text>
			);
		}

		const { getByTestId } = render(
			<TransportProvider controller={mockController as any}>
				<Toggler />
			</TransportProvider>,
		);

		expect(getByTestId("transport").props.children).toBe("http");
	});

	it("exposes disconnectTransport without throwing", async () => {
		function Disconnector() {
			const actions = useTransportActions();
			return (
				<Text testID="btn" onPress={() => void actions.disconnectTransport()}>
					disconnect
				</Text>
			);
		}

		const { getByTestId } = render(
			<TransportProvider controller={mockController as any}>
				<Disconnector />
			</TransportProvider>,
		);

		expect(getByTestId("btn")).toBeTruthy();
	});
});
