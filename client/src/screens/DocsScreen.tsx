import { useEffect, useMemo, useState } from "react";
import { Linking, Pressable, ScrollView, StyleSheet, Text, View, useWindowDimensions } from "react-native";

import { colors, font, radius, spacing } from "../design/tokens";
import {
  BUNDLED_DOCS_BY_ROUTE,
  DEFAULT_DOC_ROUTE,
  DOC_TREE,
  type BundledDocContent,
  type BundledDocTreeNode,
} from "../generated/docsContent";
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
        ...buildTreeRows(node.children, expandedFolders, depth + 1, [...hasNextSiblings, !isLast]),
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

function resolveDocRoute(target: string, currentDocPath: string): string | undefined {
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

  if (BUNDLED_DOCS_BY_ROUTE[normalizedTarget]) {
    return normalizedTarget;
  }

  const targetAsRoute = normalizedTarget.replace(/\.md$/i, "");
  if (BUNDLED_DOCS_BY_ROUTE[targetAsRoute]) {
    return targetAsRoute;
  }

  const currentDir = currentDocPath.includes("/")
    ? currentDocPath.slice(0, currentDocPath.lastIndexOf("/") + 1)
    : "";
  const resolvedPath = normalizeDocPath(
    normalizedTarget.startsWith("docs/") ? normalizedTarget.slice(5) : `${currentDir}${normalizedTarget}`,
  );

  if (BUNDLED_DOCS_BY_ROUTE[resolvedPath]) {
    return resolvedPath;
  }

  const resolvedRoute = resolvedPath.replace(/\.md$/i, "");
  if (BUNDLED_DOCS_BY_ROUTE[resolvedRoute]) {
    return resolvedRoute;
  }

  const fileName = resolvedPath.split("/").pop();
  if (!fileName) {
    return undefined;
  }

  const matches = Object.values(BUNDLED_DOCS_BY_ROUTE).filter(
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
  const fallbackContent = BUNDLED_DOCS_BY_ROUTE[DEFAULT_DOC_ROUTE] ?? Object.values(BUNDLED_DOCS_BY_ROUTE)[0];
  const activeContent = BUNDLED_DOCS_BY_ROUTE[activeDocRoute] ?? fallbackContent;
  const [expandedFolders, setExpandedFolders] = useState<Set<string>>(
    () => new Set(collectInitialExpandedFolders(DOC_TREE)),
  );

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

  const treeRows = useMemo(() => buildTreeRows(DOC_TREE.children ?? [], expandedFolders), [expandedFolders]);

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

    const resolvedRoute = resolveDocRoute(url, activeContent?.path ?? "");
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
      <View style={[styles.sidebar, isDesktop ? styles.sidebarDesktop : styles.sidebarMobile]}>
        <Pressable style={styles.rootRow} onPress={() => onNavigateDoc(DEFAULT_DOC_ROUTE)}>
          <View style={styles.rootDot} />
          <Text style={[styles.rootTitle, activeContent?.routePath === DEFAULT_DOC_ROUTE ? styles.rootTitleActive : undefined]}>
            {DOC_TREE.title}
          </Text>
        </Pressable>

        <ScrollView showsVerticalScrollIndicator={false} contentContainerStyle={styles.sidebarList}>
          {treeRows.map((row) => {
            const active = row.kind === "doc" && row.routePath === activeContent?.routePath;
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
        <Text style={styles.kicker}>{activeContent?.path ?? "docs"}</Text>
        {activeContent?.description ? <Text style={styles.summary}>{activeContent.description}</Text> : null}
        <MarkdownRenderer markdown={activeContent?.body ?? "No document selected."} onLinkPress={onMarkdownLinkPress} />
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
