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
