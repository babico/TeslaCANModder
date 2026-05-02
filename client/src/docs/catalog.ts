import { Asset } from "expo-asset";
import * as FileSystem from "expo-file-system";

export interface BundledDocContent {
	path: string;
	routePath: string;
	category: string;
	title: string;
	description: string;
	order: number;
	body: string;
}

export interface BundledDocTreeNode {
	kind: "folder" | "doc";
	path: string;
	routePath?: string;
	title: string;
	order: number;
	children?: BundledDocTreeNode[];
}

export interface DocsCatalog {
	defaultDocRoute: string;
	docsByRoute: Record<string, BundledDocContent>;
	docTree: BundledDocTreeNode;
}

type MarkdownAssetModule = number;

type MarkdownRequireContext = {
	(key: string): MarkdownAssetModule;
	keys(): string[];
};

const docsContext = require.context("../../../docs", true, /\.md$/) as MarkdownRequireContext;

const docPaths = docsContext
	.keys()
	.map((key) => key.replace(/^\.\//, "").replace(/\\/g, "/"))
	.sort((left, right) => left.localeCompare(right));

export const DEFAULT_DOC_ROUTE = (() => {
	if (docPaths.some((docPath) => toRoutePath(docPath) === "")) {
		return "";
	}

	return docPaths[0] ? toRoutePath(docPaths[0]) : "";
})();

export const EMPTY_DOC_TREE: BundledDocTreeNode = {
	kind: "folder",
	path: "",
	routePath: DEFAULT_DOC_ROUTE,
	title: "Documentation",
	order: 0,
	children: [],
};

let docsCatalogPromise: Promise<DocsCatalog> | null = null;

function prettifyTitle(value: string): string {
	return value
		.replace(/[-_]+/g, " ")
		.replace(/\s+/g, " ")
		.trim()
		.replace(/\b\w/g, (match) => match.toUpperCase());
}

function toRoutePath(relativePath: string): string {
	const withoutExtension = relativePath.replace(/\.md$/i, "");
	if (withoutExtension.toLowerCase() === "readme") {
		return "";
	}
	if (withoutExtension.toLowerCase().endsWith("/readme")) {
		return withoutExtension.slice(0, -"/README".length);
	}
	return withoutExtension;
}

function parseFrontmatter(raw: string, relativePath: string): BundledDocContent {
	const normalizedRaw = raw.replace(/^\uFEFF/, "");
	const routePath = toRoutePath(relativePath);
	const category = relativePath.includes("/") ? relativePath.split("/")[0] : "root";
	const frontmatterMatch = normalizedRaw.match(/^---\r?\n([\s\S]*?)\r?\n---\r?\n?/);
	const metadata = new Map<string, string>();

	let bodySource = normalizedRaw;
	if (frontmatterMatch) {
		const frontmatterBlock = frontmatterMatch[1].trim();
		bodySource = normalizedRaw.slice(frontmatterMatch[0].length).trim();

		for (const line of frontmatterBlock.split(/\r?\n/)) {
			const match = line.match(/^([a-zA-Z0-9_]+):\s*(.+)$/);
			if (!match) {
				continue;
			}
			metadata.set(match[1], match[2].replace(/^['"]|['"]$/g, "").trim());
		}
	}

	const firstHeading = bodySource.match(/^#\s+(.+)$/m)?.[1]?.trim();
	const fallbackTitle = prettifyTitle(relativePath.split("/").pop()?.replace(/\.md$/i, "") ?? "");
	const orderValue = Number.parseInt(metadata.get("order") ?? "9999", 10);

	return {
		path: relativePath,
		routePath,
		category: metadata.get("category") || category,
		title: metadata.get("title") || firstHeading || fallbackTitle,
		description: metadata.get("description") || "",
		order: Number.isNaN(orderValue) ? 9999 : orderValue,
		body: bodySource.trim(),
	};
}

function createFolderNode(
	pathValue: string,
	title: string,
	order = 9999,
	routePath: string | undefined = undefined,
): BundledDocTreeNode {
	return {
		kind: "folder",
		path: pathValue,
		routePath,
		title,
		order,
		children: [],
	};
}

function createDocNode(doc: BundledDocContent): BundledDocTreeNode {
	return {
		kind: "doc",
		path: doc.path,
		routePath: doc.routePath,
		title: doc.title,
		order: doc.order,
	};
}

function buildDocTree(docs: BundledDocContent[]): BundledDocTreeNode {
	const root = createFolderNode(
		"",
		"Documentation",
		0,
		docs.find((doc) => doc.routePath === "")?.routePath,
	);
	const folderMap = new Map<string, BundledDocTreeNode>([["", root]]);

	for (const doc of docs) {
		const pathParts = doc.path.split("/");
		const fileName = pathParts[pathParts.length - 1];
		const folderParts = pathParts.slice(0, -1);
		let currentFolder = root;
		let currentPath = "";

		for (const folderPart of folderParts) {
			currentPath = currentPath ? `${currentPath}/${folderPart}` : folderPart;
			let nextFolder = folderMap.get(currentPath);
			if (!nextFolder) {
				nextFolder = createFolderNode(currentPath, prettifyTitle(folderPart));
				folderMap.set(currentPath, nextFolder);
				currentFolder.children?.push(nextFolder);
			}
			currentFolder = nextFolder;
		}

		if (fileName.toLowerCase() === "readme.md") {
			currentFolder.routePath = doc.routePath;
			currentFolder.title = doc.title;
			currentFolder.order = doc.order;
			continue;
		}

		currentFolder.children?.push(createDocNode(doc));
	}

	function sortNodes(node: BundledDocTreeNode): BundledDocTreeNode {
		if (node.kind !== "folder") {
			return node;
		}

		node.children = (node.children ?? []).map(sortNodes).sort((left, right) => {
			if (left.kind !== right.kind) {
				return left.kind === "folder" ? -1 : 1;
			}
			if (left.order !== right.order) {
				return left.order - right.order;
			}
			return left.title.localeCompare(right.title);
		});

		return node;
	}

	return sortNodes(root);
}

async function readMarkdownFile(docPath: string): Promise<string> {
	const asset = Asset.fromModule(docsContext(`./${docPath}`));
	await asset.downloadAsync();

	const uri = asset.localUri ?? asset.uri;
	if (!uri) {
		throw new Error(`Missing asset URI for ${docPath}`);
	}

	try {
		return await FileSystem.readAsStringAsync(uri);
	} catch {
		const response = await fetch(uri);
		if (!response.ok) {
			throw new Error(`Failed to read ${docPath}`);
		}
		return response.text();
	}
}

async function buildDocsCatalog(): Promise<DocsCatalog> {
	const docs = await Promise.all(
		docPaths.map(async (docPath) => parseFrontmatter(await readMarkdownFile(docPath), docPath)),
	);

	const docsByRoute = Object.fromEntries(docs.map((doc) => [doc.routePath, doc]));
	const docTree = buildDocTree(docs);

	return {
		defaultDocRoute: DEFAULT_DOC_ROUTE,
		docsByRoute,
		docTree,
	};
}

export function loadDocsCatalog(): Promise<DocsCatalog> {
	if (!docsCatalogPromise) {
		docsCatalogPromise = buildDocsCatalog().catch((error) => {
			docsCatalogPromise = null;
			throw error;
		});
	}

	return docsCatalogPromise;
}
