module.exports = {
  maxWorkers: 4,
  workerIdleMemoryLimit: "512MB",
  projects: [
    "<rootDir>/client/jest.config.cjs",
    "<rootDir>/packages/protocol/jest.config.js",
    {
      displayName: "tools",
      rootDir: "<rootDir>/tools",
      testEnvironment: "node",
      testMatch: ["<rootDir>/test/**/*.test.js"],
      testPathIgnorePatterns: ["<rootDir>/coverage/"],
      watchPathIgnorePatterns: ["<rootDir>/coverage/"],
      transform: {
        "^.+\\.js$": ["babel-jest", { plugins: ["@babel/plugin-transform-modules-commonjs"] }],
      },
    },
  ],
};
