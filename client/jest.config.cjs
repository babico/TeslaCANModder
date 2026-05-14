module.exports = {
  preset: "jest-expo",
  roots: ["<rootDir>/tests"],
  testMatch: ["**/*.test.ts", "**/*.test.tsx"],
  testPathIgnorePatterns: ["<rootDir>/dist-web/", "<rootDir>/dist-ios/", "<rootDir>/coverage/"],
  watchPathIgnorePatterns: ["<rootDir>/dist-web/", "<rootDir>/dist-ios/", "<rootDir>/coverage/"],
  moduleNameMapper: {
    "^react-native-svg$": "<rootDir>/src/test/mocks/reactNativeSvgMock.tsx",
  },
  collectCoverageFrom: ["src/**/*.{ts,tsx}", "!src/generated/**", "!src/test/**", "!src/**/*.d.ts"],
  coverageThreshold: {
    global: {
      branches: 35,
      functions: 40,
      lines: 45,
      statements: 45,
    },
  },
};
