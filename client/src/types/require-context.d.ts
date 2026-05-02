interface MetroRequireContext<TModule = number> {
	(key: string): TModule;
	keys(): string[];
	resolve?(key: string): string;
}

interface MetroRequire {
	context(directory: string, useSubdirectories?: boolean, regExp?: RegExp): MetroRequireContext;
}

declare const require: MetroRequire;
