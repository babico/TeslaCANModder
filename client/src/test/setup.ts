// Setup file for Jest tests with @react-native/jest-preset
// Sets required globals for RN 0.85+ test environment

declare const global: Record<string, unknown>;

global.__DEV__ = true;
global.IS_REACT_ACT_ENVIRONMENT = true;
global.IS_REACT_NATIVE_TEST_ENVIRONMENT = true;
