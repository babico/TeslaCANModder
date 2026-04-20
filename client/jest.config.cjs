module.exports = {
  preset: 'ts-jest',
  testEnvironment: 'node',
  roots: ['<rootDir>/tests'],
  testMatch: ['**/*.test.ts', '**/*.test.tsx'],
  moduleFileExtensions: ['ts', 'tsx', 'js', 'jsx', 'json'],
  transform: {
    '^.+\\.(ts|tsx)$': ['ts-jest', {
      tsconfig: {
        jsx: 'react-jsx',
      },
    }],
  },
  moduleNameMapper: {
    '^react-native-svg$': '<rootDir>/src/test/mocks/reactNativeSvgMock.tsx',
  },
  collectCoverageFrom: [
    'src/**/*.{ts,tsx}',
    '!src/generated/**',
    '!src/test/**',
    '!src/**/*.d.ts',
  ],
  coverageThreshold: {
    global: {
      branches: 35,
      functions: 40,
      lines: 45,
      statements: 45,
    },
  },
};
