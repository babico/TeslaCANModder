import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

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

jest.mock("react-native", () => {
	const View = (props: any) => React.createElement("View", props, props.children);
	const Text = (props: any) => React.createElement("Text", props, props.children);
	return {
		View,
		Text,
		StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFillObject: {} },
	};
});

import { IntegrationPanel } from "../../src/components/IntegrationPanel";

function render(node: React.ReactElement) {
	let r!: TestRenderer.ReactTestRenderer;
	TestRenderer.act(() => {
		r = TestRenderer.create(node);
	});
	return r;
}

function texts(r: TestRenderer.ReactTestRenderer) {
	return r.root
		.findAll((n) => (n.type as any) === "Text")
		.map((n) => String(n.props.children))
		.join(" ");
}

describe("IntegrationPanel", () => {
	it("renders all sections from default state as Disabled/Unavailable", () => {
		const r = render(<IntegrationPanel state={makeState()} />);
		const t = texts(r);
		expect(t).toContain("MQTT");
		expect(t).toContain("Tesla BLE");
		expect(t).toContain("Home Assistant");
		expect(t).toContain("Disabled");
	});

	it("shows MQTT connected state when enabled+connected", () => {
		const r = render(<IntegrationPanel state={makeState({ mqtt: true, mqttConnected: true })} />);
		expect(texts(r)).toContain("Connected");
	});

	it("shows ESP-NOW peer count when enabled", () => {
		const r = render(
			<IntegrationPanel state={makeState({ espNow: true, espNowChannel: 6, espNowPeers: 3 })} />,
		);
		const t = texts(r);
		expect(t).toContain("CH");
		expect(t).toContain("6");
		expect(t).toContain("3");
	});
});
