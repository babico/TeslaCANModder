import React from "react";
import { render } from "@testing-library/react-native";
import { Divider } from "../../src/ui/Divider";

describe("Divider", () => {
	it("renders horizontal by default", () => {
		const { toJSON } = render(<Divider />);
		expect(toJSON()).not.toBeNull();
	});
	it("renders vertical when requested", () => {
		const { toJSON } = render(<Divider vertical />);
		expect(toJSON()).not.toBeNull();
	});
	it("renders dark variant", () => {
		const { toJSON } = render(<Divider variant="dark" />);
		expect(toJSON()).not.toBeNull();
	});
});
