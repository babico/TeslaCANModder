import React from "react";
import TestRenderer from "react-test-renderer";

(globalThis as { IS_REACT_ACT_ENVIRONMENT?: boolean }).IS_REACT_ACT_ENVIRONMENT = true;

jest.mock("react-native", () => {
	const View = (props: any) => React.createElement("View", props, props.children);
	return {
		View,
		StyleSheet: { create: <T extends Record<string, unknown>>(o: T) => o, absoluteFillObject: {} },
	};
});

import { Divider } from "../../src/ui/Divider";

describe("Divider", () => {
	it("renders horizontal by default", () => {
		let r!: TestRenderer.ReactTestRenderer;
		TestRenderer.act(() => {
			r = TestRenderer.create(<Divider />);
		});
		expect(r.root.findAll((n) => (n.type as any) === "View")).toHaveLength(1);
	});
	it("renders vertical when requested", () => {
		let r!: TestRenderer.ReactTestRenderer;
		TestRenderer.act(() => {
			r = TestRenderer.create(<Divider vertical />);
		});
		expect(r.root.findAll((n) => (n.type as any) === "View")).toHaveLength(1);
	});
	it("renders dark variant", () => {
		let r!: TestRenderer.ReactTestRenderer;
		TestRenderer.act(() => {
			r = TestRenderer.create(<Divider variant="dark" />);
		});
		expect(r.root.findAll((n) => (n.type as any) === "View")).toHaveLength(1);
	});
});
