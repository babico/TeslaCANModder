/** @type {import('jest').Config} */
export default {
	preset: "ts-jest/presets/default-esm",
	testMatch: ["<rootDir>/test/**/*.test.ts"],
	testPathIgnorePatterns: ["<rootDir>/dist/", "<rootDir>/coverage/"],
	watchPathIgnorePatterns: ["<rootDir>/dist/", "<rootDir>/coverage/"],
	moduleNameMapper: {
		"^(\\.{1,2}/.*)\\.js$": "$1",
	},
	transform: {
		"^.+\\.ts$": ["ts-jest", { useESM: true }],
	},
	extensionsToTreatAsEsm: [".ts"],
	collectCoverageFrom: ["src/**/*.ts", "!src/**/*.d.ts"],
	coverageThreshold: {
		global: {
			branches: 70,
			functions: 55,
			lines: 70,
			statements: 70,
		},
	},
};
