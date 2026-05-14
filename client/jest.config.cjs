module.exports = {
  preset: "@react-native/jest-preset",
  setupFiles: ["<rootDir>/src/test/setup.ts"],
  roots: ["<rootDir>/tests"],
  testMatch: ["**/*.test.ts", "**/*.test.tsx"],
  testPathIgnorePatterns: ["<rootDir>/dist-web/", "<rootDir>/dist-ios/", "<rootDir>/coverage/"],
  watchPathIgnorePatterns: ["<rootDir>/dist-web/", "<rootDir>/dist-ios/", "<rootDir>/coverage/"],
  moduleNameMapper: {
    "^react$": "<rootDir>/../node_modules/react",
    "^react/jsx-runtime$": "<rootDir>/../node_modules/react/jsx-runtime",
    "^react/jsx-dev-runtime$": "<rootDir>/../node_modules/react/jsx-dev-runtime",
    "^react-native-svg$": "<rootDir>/src/test/mocks/reactNativeSvgMock.tsx",
    "^react-native-vector-icons$": "@expo/vector-icons",
    "^react-native-vector-icons/(.*)": "@expo/vector-icons/$1",
  },
  transformIgnorePatterns: [
    "node_modules/(?!(jest-)?react-native|@react-native|expo|@expo|@unimodules|nativewind|react-native-css-interop|react-native-web|@react-native-reusables)",
  ],
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
