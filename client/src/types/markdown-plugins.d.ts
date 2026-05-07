// Lightweight ambient declarations for `markdown-it` and its plugins.
//
// `markdown-it` ships without its own TypeScript types, and the DefinitelyTyped
// `@types/markdown-it` package is not installed in this workspace. We declare
// just enough of the surface we use so that TS can typecheck the docs renderer
// without pulling in the heavier `any` shim that was here previously.
declare module "markdown-it" {
	interface MarkdownItOptions {
		html?: boolean;
		linkify?: boolean;
		typographer?: boolean;
		xhtmlOut?: boolean;
		breaks?: boolean;
		langPrefix?: string;
	}

	interface MarkdownItInstance {
		use(plugin: unknown, ...params: unknown[]): MarkdownItInstance;
		render(src: string, env?: unknown): string;
		renderInline(src: string, env?: unknown): string;
		parse(src: string, env?: unknown): unknown[];
		options: MarkdownItOptions;
	}

	interface MarkdownItConstructor {
		new (options?: MarkdownItOptions): MarkdownItInstance;
		new (preset: string, options?: MarkdownItOptions): MarkdownItInstance;
		(options?: MarkdownItOptions): MarkdownItInstance;
		(preset: string, options?: MarkdownItOptions): MarkdownItInstance;
	}

	const MarkdownIt: MarkdownItConstructor;
	export default MarkdownIt;
}

declare module "markdown-it-emoji";
declare module "markdown-it-footnote";
declare module "markdown-it-sub";
declare module "markdown-it-sup";
declare module "markdown-it-deflist";
declare module "markdown-it-abbr";
declare module "markdown-it-ins";
declare module "markdown-it-mark";
declare module "markdown-it-task-lists";
