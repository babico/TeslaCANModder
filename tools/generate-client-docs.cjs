const fs = require("fs");
const path = require("path");

const repoRoot = path.resolve(__dirname, "..");
const docsRoot = path.join(repoRoot, "docs");
const outputFile = path.join(repoRoot, "client", "src", "generated", "docsContent.ts");

function walkDocs(dirPath, relativePrefix = "") {
  const entries = fs.readdirSync(dirPath, { withFileTypes: true });
  const results = [];

  for (const entry of entries) {
    if (entry.name.startsWith(".")) {
      continue;
    }

    const fullPath = path.join(dirPath, entry.name);
    const relativePath = relativePrefix ? `${relativePrefix}/${entry.name}` : entry.name;

    if (entry.isDirectory()) {
      results.push(...walkDocs(fullPath, relativePath));
      continue;
    }

    if (!entry.name.toLowerCase().endsWith(".md")) {
      continue;
    }

    results.push(relativePath);
  }

  return results;
}

function prettifyTitle(value) {
  return value
    .replace(/[-_]+/g, " ")
    .replace(/\s+/g, " ")
    .trim()
    .replace(/\b\w/g, (match) => match.toUpperCase());
}

function toRoutePath(relativePath) {
  const withoutExtension = relativePath.replace(/\.md$/i, "");
  if (withoutExtension.toLowerCase() === "readme") {
    return "";
  }
  if (withoutExtension.toLowerCase().endsWith("/readme")) {
    return withoutExtension.slice(0, -"/README".length);
  }
  return withoutExtension;
}

function parseFrontmatter(raw, relativePath) {
  const normalizedRaw = raw.replace(/^\uFEFF/, "");
  const routePath = toRoutePath(relativePath);
  const category = relativePath.includes("/") ? relativePath.split("/")[0] : "root";
  const frontmatterMatch = normalizedRaw.match(/^---\r?\n([\s\S]*?)\r?\n---\r?\n?/);
  const metadata = new Map();

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
  const fallbackTitle = prettifyTitle(path.basename(relativePath, ".md"));
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

function createFolderNode(pathValue, title, order = 9999, routePath = undefined) {
  return {
    kind: "folder",
    path: pathValue,
    routePath,
    title,
    order,
    children: [],
  };
}

function createDocNode(doc) {
  return {
    kind: "doc",
    path: doc.path,
    routePath: doc.routePath,
    title: doc.title,
    order: doc.order,
  };
}

function buildDocTree(docs) {
  const root = createFolderNode("", "Documentation", 0, docs.find((doc) => doc.routePath === "")?.routePath);
  const folderMap = new Map([["", root]]);

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
        currentFolder.children.push(nextFolder);
      }
      currentFolder = nextFolder;
    }

    if (fileName.toLowerCase() === "readme.md") {
      currentFolder.routePath = doc.routePath;
      currentFolder.title = doc.title;
      currentFolder.order = doc.order;
      continue;
    }

    currentFolder.children.push(createDocNode(doc));
  }

  function sortNodes(node) {
    if (node.kind !== "folder") {
      return node;
    }

    node.children = node.children
      .map(sortNodes)
      .sort((left, right) => {
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

function main() {
  const docPaths = walkDocs(docsRoot)
    .map((relativePath) => relativePath.replace(/\\/g, "/"))
    .sort((left, right) => left.localeCompare(right));

  const docs = docPaths.map((relativePath) => {
    const fullPath = path.join(docsRoot, relativePath);
    const raw = fs.readFileSync(fullPath, "utf8");
    return parseFrontmatter(raw, relativePath);
  });

  const docsByRoute = Object.fromEntries(docs.map((doc) => [doc.routePath, doc]));
  const docTree = buildDocTree(docs);
  const defaultDocRoute = docsByRoute[""] ? "" : (docs[0]?.routePath ?? "");

  const output = [
    "export interface BundledDocContent {",
    "  path: string;",
    "  routePath: string;",
    "  category: string;",
    "  title: string;",
    "  description: string;",
    "  order: number;",
    "  body: string;",
    "}",
    "",
    "export interface BundledDocTreeNode {",
    "  kind: \"folder\" | \"doc\";",
    "  path: string;",
    "  routePath?: string;",
    "  title: string;",
    "  order: number;",
    "  children?: BundledDocTreeNode[];",
    "}",
    "",
    `export const DEFAULT_DOC_ROUTE = ${JSON.stringify(defaultDocRoute)};`,
    "",
    "export const BUNDLED_DOCS_BY_ROUTE: Record<string, BundledDocContent> = ",
    `${JSON.stringify(docsByRoute, null, 2)} as const;`,
    "",
    "export const DOC_TREE: BundledDocTreeNode = ",
    `${JSON.stringify(docTree, null, 2)} as const;`,
    "",
  ].join("\n");

  fs.mkdirSync(path.dirname(outputFile), { recursive: true });
  fs.writeFileSync(outputFile, output);
  console.log(`Generated ${docs.length} bundled docs -> ${outputFile}`);
}

main();
