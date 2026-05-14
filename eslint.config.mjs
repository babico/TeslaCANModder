import js from "@eslint/js";
import tseslint from "typescript-eslint";
import globals from "globals";

export default tseslint.config(
  js.configs.recommended,
  ...tseslint.configs.recommended,
  {
    languageOptions: {
      globals: {
        ...globals.browser,
        ...globals.node,
      },
    },
    rules: {
      "@typescript-eslint/no-unused-vars": [
        "warn",
        { argsIgnorePattern: "^_", varsIgnorePattern: "^_" },
      ],
      "@typescript-eslint/no-explicit-any": "warn",
      "no-console": ["warn", { allow: ["warn", "error"] }],
      eqeqeq: ["error", "always"],
      "no-eval": "error",
      "no-implied-eval": "error",
    },
  },
  {
    // CLI tooling intentionally writes machine-readable JSON / status
    // directly to stdout via console.log.
    files: ["tools/**/*.js", "tools/**/*.mjs"],
    rules: { "no-console": "off" },
  },
  {
    // Test files often need `any` to satisfy structural test fixtures
    // (mock factories, jest spies, partial-shape assertions). Allowing
    // `any` here keeps test ergonomics without weakening production types.
    files: [
      "**/test/**/*.{ts,tsx,js,jsx,mjs,cjs}",
      "**/tests/**/*.{ts,tsx,js,jsx,mjs,cjs}",
      "**/*.test.{ts,tsx,js,jsx,mjs,cjs}",
    ],
    rules: { "@typescript-eslint/no-explicit-any": "off" },
  },
  {
    ignores: [
      "**/node_modules/**",
      "**/dist/**",
      "**/dist-web/**",
      "**/dist-ios/**",
      "**/build/**",
      "**/legacy/**",
      "**/.pio/**",
      "**/coverage/**",
    ],
  },
);
