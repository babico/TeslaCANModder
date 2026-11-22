/** Shared path-containment helper for tools that read or write local files. */

import { isAbsolute, relative, resolve, sep } from "node:path";

/**
 * Resolve a user-supplied path and refuse anything that escapes `cwd`.
 *
 * Lexical containment only: symlinks that already exist on disk are not
 * resolved. Returns the absolute path, or `null` for invalid/escaping input.
 */
export function resolveInsideCwd(value, cwd = process.cwd()) {
	if (typeof value !== "string" || value.length === 0 || value.includes("\0")) {
		return null;
	}

	const base = resolve(cwd);
	const target = resolve(base, value);
	if (target === base) return null;

	const rel = relative(base, target);
	if (rel === "" || rel === ".." || rel.startsWith(`..${sep}`) || isAbsolute(rel)) {
		return null;
	}

	return target;
}
