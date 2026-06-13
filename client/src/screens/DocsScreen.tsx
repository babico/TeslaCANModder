import { useEffect, useMemo, useState } from "react";
import { Linking, Pressable, ScrollView, Text, View, useWindowDimensions } from "react-native";

import {
	DEFAULT_DOC_ROUTE,
	EMPTY_DOC_TREE,
	loadDocsCatalog,
	type BundledDocContent,
	type BundledDocTreeNode,
} from "../docs/catalog";
import { DocTreeItem } from "../components/docs/DocTreeItem";
import { MarkdownRenderer } from "../components/docs/MarkdownRenderer";

export interface DocsScreenProps {
	activeDocRoute: string;
	onNavigateDoc: (docRoute: string) => void;
}

type TreeRow = {
	key: string;
	kind: "folder" | "doc";
	path: string;
	routePath?: string;
	title: string;
	depth: number;
	hasNextSiblings: boolean[];
	expanded?: boolean;
};

function collectInitialExpandedFolders(root: BundledDocTreeNode): string[] {
	if (!root.children) {
		return [];
	}

	return root.children
		.filter((child) => child.kind === "folder" && (child.children?.length ?? 0) <= 24)
		.map((child) => child.path);
}

function collectAncestorFolders(content: BundledDocContent | undefined): string[] {
	if (!content) {
		return [];
	}

	const pathParts = content.path.split("/").slice(0, -1);
	const ancestors: string[] = [];
	let current = "";

	for (const part of pathParts) {
		current = current ? `${current}/${part}` : part;
		ancestors.push(current);
	}

	return ancestors;
}

function buildTreeRows(
	nodes: BundledDocTreeNode[],
	expandedFolders: Set<string>,
	depth = 0,
	hasNextSiblings: boolean[] = [],
): TreeRow[] {
	const rows: TreeRow[] = [];

	nodes.forEach((node, index) => {
		const isLast = index === nodes.length - 1;

		rows.push({
			key: `${node.kind}:${node.path}`,
			kind: node.kind,
			path: node.path,
			routePath: node.routePath,
			title: node.kind === "folder" ? node.title : node.title,
			depth,
			hasNextSiblings,
			expanded: node.kind === "folder" ? expandedFolders.has(node.path) : undefined,
		});

		if (node.kind === "folder" && expandedFolders.has(node.path) && node.children?.length) {
			rows.push(
				...buildTreeRows(node.children, expandedFolders, depth + 1, [
					...hasNextSiblings,
					!isLast,
				]),
			);
		}
	});

	return rows;
}

function normalizeDocPath(path: string): string {
	const segments = path.replace(/\\/g, "/").split("/");
	const stack: string[] = [];

	for (const segment of segments) {
		if (!segment || segment === ".") {
			continue;
		}
		if (segment === "..") {
			stack.pop();
			continue;
		}
		stack.push(segment);
	}

	return stack.join("/");
}

function resolveDocRoute(
	target: string,
	currentDocPath: string,
	docsByRoute: Record<string, BundledDocContent>,
): string | undefined {
	const decoded = (() => {
		try {
			return decodeURIComponent(target);
		} catch {
			return target;
		}
	})();

	const withoutAngleBrackets = decoded.replace(/^<|>$/g, "");
	const cleaned = withoutAngleBrackets.split("#")[0].split("?")[0].trim();
	if (!cleaned) {
		return undefined;
	}

	const normalizedTarget = cleaned.replace(/\\/g, "/").replace(/^\//, "");

	if (docsByRoute[normalizedTarget]) {
		return normalizedTarget;
	}

	const targetAsRoute = normalizedTarget.replace(/\.md$/i, "");
	if (docsByRoute[targetAsRoute]) {
		return targetAsRoute;
	}

	const currentDir = currentDocPath.includes("/")
		? currentDocPath.slice(0, currentDocPath.lastIndexOf("/") + 1)
		: "";
	const resolvedPath = normalizeDocPath(
		normalizedTarget.startsWith("docs/")
			? normalizedTarget.slice(5)
			: `${currentDir}${normalizedTarget}`,
	);

	if (docsByRoute[resolvedPath]) {
		return resolvedPath;
	}

	const resolvedRoute = resolvedPath.replace(/\.md$/i, "");
	if (docsByRoute[resolvedRoute]) {
		return resolvedRoute;
	}

	const fileName = resolvedPath.split("/").pop();
	if (!fileName) {
		return undefined;
	}

	const matches = Object.values(docsByRoute).filter(
		(doc) => doc.path === fileName || doc.path.endsWith(`/${fileName}`),
	);
	if (matches.length === 1) {
		return matches[0].routePath;
	}

	return undefined;
}

export function DocsScreen({ activeDocRoute, onNavigateDoc }: DocsScreenProps) {
	const { width } = useWindowDimensions();
	const isDesktop = width >= 1140;
	const [catalogError, setCatalogError] = useState<string | null>(null);
	const [catalog, setCatalog] = useState<{
		docsByRoute: Record<string, BundledDocContent>;
		docTree: BundledDocTreeNode;
	} | null>(null);
	const [expandedFolders, setExpandedFolders] = useState<Set<string>>(() => new Set());
	const docsByRoute = catalog?.docsByRoute ?? {};
	const docTree = catalog?.docTree ?? EMPTY_DOC_TREE;
	const fallbackContent = docsByRoute[DEFAULT_DOC_ROUTE] ?? Object.values(docsByRoute)[0];
	const activeContent = docsByRoute[activeDocRoute] ?? fallbackContent;

	useEffect(() => {
		let cancelled = false;

		void loadDocsCatalog()
			.then((nextCatalog) => {
				if (cancelled) {
					return;
				}

				setCatalog(nextCatalog);
				setCatalogError(null);
				setExpandedFolders(new Set(collectInitialExpandedFolders(nextCatalog.docTree)));
			})
			.catch((error: unknown) => {
				if (cancelled) {
					return;
				}

				setCatalogError(
					error instanceof Error ? error.message : "Failed to load documentation.",
				);
			});

		return () => {
			cancelled = true;
		};
	}, []);

	useEffect(() => {
		const ancestors = collectAncestorFolders(activeContent);
		if (ancestors.length === 0) {
			return;
		}

		setExpandedFolders((current) => {
			const next = new Set(current);
			ancestors.forEach((ancestor) => next.add(ancestor));
			return next;
		});
	}, [activeContent]);

	const treeRows = useMemo(
		() => buildTreeRows(docTree.children ?? [], expandedFolders),
		[docTree.children, expandedFolders],
	);

	const toggleFolder = (folderPath: string) => {
		setExpandedFolders((current) => {
			const next = new Set(current);
			if (next.has(folderPath)) {
				next.delete(folderPath);
			} else {
				next.add(folderPath);
			}
			return next;
		});
	};

	const onMarkdownLinkPress = (url: string): boolean => {
		const externalLink = /^(https?:|mailto:|tel:)/i.test(url);
		if (externalLink) {
			void Linking.openURL(url);
			return false;
		}

		const resolvedRoute = resolveDocRoute(url, activeContent?.path ?? "", docsByRoute);
		if (resolvedRoute) {
			onNavigateDoc(resolvedRoute);
			return false;
		}

		if (/^#/.test(url)) {
			return false;
		}

		void Linking.openURL(url).catch((err) => {
			console.warn("Failed to open URL", url, err);
		});
		return false;
	};

	return (
		<View className={`flex-1 bg-background ${isDesktop ? "flex-row" : "flex-col"}`}>
			<View
				className={`bg-card p-3 gap-3 ${isDesktop ? "w-[390px] border-r border-border" : "max-h-[330px]"}`}
			>
				<Pressable
					className="flex-row items-center gap-2 pb-2 border-b border-border"
					onPress={() => onNavigateDoc(DEFAULT_DOC_ROUTE)}
				>
					<View className="w-2 h-2 rounded-full bg-primary" />
					<Text
						className={`text-sm font-bold ${activeContent?.routePath === DEFAULT_DOC_ROUTE ? "text-primary" : "text-card-foreground"}`}
					>
						{docTree.title}
					</Text>
				</Pressable>

				<ScrollView
					showsVerticalScrollIndicator={false}
					contentContainerStyle={{ paddingBottom: 20, gap: 2 }}
				>
					{treeRows.map((row) => {
						const active =
							row.kind === "doc" && row.routePath === activeContent?.routePath;
						return (
							<DocTreeItem
								key={row.key}
								title={row.kind === "folder" ? `${row.title}` : row.title}
								depth={row.depth}
								hasNextSiblings={row.hasNextSiblings}
								isFolder={row.kind === "folder"}
								expanded={row.expanded}
								active={active}
								onPress={() => {
									if (row.kind === "folder") {
										toggleFolder(row.path);
										return;
									}
									if (row.routePath !== undefined) {
										onNavigateDoc(row.routePath);
									}
								}}
							/>
						);
					})}
				</ScrollView>
			</View>

			<ScrollView
				className="flex-1"
				contentContainerStyle={{ padding: 16, paddingBottom: 40, gap: 8 }}
			>
				<Text className="text-xs font-bold text-muted-foreground uppercase tracking-wide">
					{activeContent?.path ?? (catalog ? "docs" : "loading")}
				</Text>
				{activeContent?.description ? (
					<Text className="text-sm text-muted-foreground leading-5 mb-2">
						{activeContent.description}
					</Text>
				) : catalogError ? (
					<Text className="text-sm text-destructive">{catalogError}</Text>
				) : null}
				<MarkdownRenderer
					markdown={
						catalogError
							? `# Documentation unavailable\n\n${catalogError}`
							: (activeContent?.body ?? "Loading documentation...")
					}
					onLinkPress={onMarkdownLinkPress}
				/>
			</ScrollView>
		</View>
	);
}

export default DocsScreen;
