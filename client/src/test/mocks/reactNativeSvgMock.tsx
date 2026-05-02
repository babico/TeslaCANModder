import React from "react";

interface MockProps {
	children?: React.ReactNode;
}

function createMock(name: string) {
	return function SvgMockComponent({ children }: MockProps) {
		return React.createElement(name, null, children);
	};
}

const Svg = createMock("Svg");

export const Circle = createMock("Circle");
export const Path = createMock("Path");
export const Rect = createMock("Rect");

export default Svg;
