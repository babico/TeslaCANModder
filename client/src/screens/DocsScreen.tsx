import { useEffect, useMemo, useState } from "react";
import {
	Linking,
	Pressable,
	ScrollView,
	StyleSheet,
	Text,
	View,
	useWindowDimensions,
} from "react-native";

import { colors, font, spacing } from "../design/tokens";
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

		void Linking.openURL(url).catch(() => undefined);
		return false;
	};

	return (
		<View style={[styles.screen, isDesktop ? styles.screenDesktop : styles.screenMobile]}>
			<View
				style={[styles.sidebar, isDesktop ? styles.sidebarDesktop : styles.sidebarMobile]}
			>
				<Pressable style={styles.rootRow} onPress={() => onNavigateDoc(DEFAULT_DOC_ROUTE)}>
					<View style={styles.rootDot} />
					<Text
						style={[
							styles.rootTitle,
							activeContent?.routePath === DEFAULT_DOC_ROUTE
								? styles.rootTitleActive
								: undefined,
						]}
					>
						{docTree.title}
					</Text>
				</Pressable>

				<ScrollView
					showsVerticalScrollIndicator={false}
					contentContainerStyle={styles.sidebarList}
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

			<ScrollView style={styles.content} contentContainerStyle={styles.contentInner}>
				<Text style={styles.kicker}>
					{activeContent?.path ?? (catalog ? "docs" : "loading")}
				</Text>
				{activeContent?.description ? (
					<Text style={styles.summary}>{activeContent.description}</Text>
				) : catalogError ? (
					<Text style={styles.summary}>{catalogError}</Text>
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

const styles = StyleSheet.create({
	screen: {
		flex: 1,
		backgroundColor: colors.dashBackground,
	},
	screenDesktop: {
		flexDirection: "row",
	},
	screenMobile: {
		flexDirection: "column",
	},
	sidebar: {
		backgroundColor: colors.dashCard,
		padding: spacing.md,
		gap: spacing.md,
	},
	sidebarDesktop: {
		width: 390,
		borderRightWidth: 1,
		borderRightColor: colors.dashCardBorder,
	},
	sidebarMobile: {
		width: "100%",
		maxHeight: 330,
		borderBottomWidth: 1,
		borderBottomColor: colors.dashCardBorder,
	},
	rootRow: {
		flexDirection: "row",
		alignItems: "center",
		gap: spacing.sm,
		paddingBottom: spacing.sm,
		borderBottomWidth: 1,
		borderBottomColor: colors.dashCardBorder,
	},
	rootDot: {
		width: 8,
		height: 8,
		borderRadius: 4,
		backgroundColor: colors.dashPrimary,
	},
	rootTitle: {
		color: colors.dashValue,
		fontSize: font.size.lg,
		fontWeight: font.weight.extrabold,
	},
	rootTitleActive: {
		color: colors.dashPrimary,
	},
	sidebarList: {
		paddingBottom: spacing.lg,
		gap: spacing.xs2,
	},
	content: {
		flex: 1,
	},
	contentInner: {
		padding: spacing.lg2,
		gap: spacing.md,
		paddingBottom: spacing.xl3,
	},
	kicker: {
		color: colors.dashMuted,
		fontSize: font.size.sm,
		letterSpacing: 0.7,
		textTransform: "uppercase",
	},
	summary: {
		color: colors.dashLabel,
		fontSize: font.size.md,
		lineHeight: 22,
	},
});

export default DocsScreen;
