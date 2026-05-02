import React from "react";
import { render } from "@testing-library/react-native";
import { IntegrationPanel } from "../../src/components/IntegrationPanel";

const makeState = (overrides: Record<string, any> = {}) =>
	({
		mqtt: false,
		mqttConnected: false,
		teslaBle: false,
		teslaBleConnected: false,
		teslaBleAuth: false,
		homeAssistant: false,
		haConnected: false,
		haEntities: 0,
		espNow: false,
		espNowChannel: 0,
		espNowPeers: 0,
		gvret: false,
		gvretPort: 0,
		gvretClients: 0,
		scanMyTesla: false,
		elm327: false,
		...overrides,
	}) as any;

describe("IntegrationPanel", () => {
	it("renders all sections from default state", () => {
		const { getAllByText } = render(<IntegrationPanel state={makeState()} />);
		expect(getAllByText("MQTT").length).toBeGreaterThan(0);
		expect(getAllByText("Tesla BLE").length).toBeGreaterThan(0);
		expect(getAllByText(/Home Assistant/).length).toBeGreaterThan(0);
	});

	it("shows MQTT connected state when enabled+connected", () => {
		const { getAllByText } = render(
			<IntegrationPanel state={makeState({ mqtt: true, mqttConnected: true })} />,
		);
		expect(getAllByText(/Connected/).length).toBeGreaterThan(0);
	});

	it("shows ESP-NOW peer count when enabled", () => {
		const { getAllByText } = render(
			<IntegrationPanel
				state={makeState({ espNow: true, espNowChannel: 6, espNowPeers: 3 })}
			/>,
		);
		expect(getAllByText(/6/).length).toBeGreaterThan(0);
		expect(getAllByText(/3/).length).toBeGreaterThan(0);
	});
});
