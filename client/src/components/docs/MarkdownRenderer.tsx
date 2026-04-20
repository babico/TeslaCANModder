import Markdown from "react-native-markdown-display";
// @ts-ignore -- module declarations are not resolved in this bundler setup
import MarkdownIt from "markdown-it";
// @ts-ignore -- module declarations are not resolved in this bundler setup
import emoji from "markdown-it-emoji";
// @ts-ignore -- module declarations are not resolved in this bundler setup
import footnote from "markdown-it-footnote";
// @ts-ignore -- module declarations are not resolved in this bundler setup
import sub from "markdown-it-sub";
// @ts-ignore -- module declarations are not resolved in this bundler setup
import sup from "markdown-it-sup";
// @ts-ignore -- module declarations are not resolved in this bundler setup
import deflist from "markdown-it-deflist";
// @ts-ignore -- module declarations are not resolved in this bundler setup
import abbr from "markdown-it-abbr";
// @ts-ignore -- module declarations are not resolved in this bundler setup
import ins from "markdown-it-ins";
// @ts-ignore -- module declarations are not resolved in this bundler setup
import mark from "markdown-it-mark";
// @ts-ignore -- module declarations are not resolved in this bundler setup
import taskLists from "markdown-it-task-lists";
import { StyleSheet } from "react-native";

import { colors, font, radius, spacing } from "../../design/tokens";

type MarkdownItParser = any;

function resolvePlugin(pluginModule: unknown): ((...args: unknown[]) => unknown) | undefined {
  const candidates = [
    pluginModule,
    (pluginModule as any)?.default,
    (pluginModule as any)?.full,
    (pluginModule as any)?.light,
    (pluginModule as any)?.bare,
    (pluginModule as any)?.default?.full,
    (pluginModule as any)?.default?.light,
    (pluginModule as any)?.default?.bare,
  ];

  return candidates.find((candidate) => typeof candidate === "function") as
    | ((...args: unknown[]) => unknown)
    | undefined;
}

function usePluginSafe(
  parser: MarkdownItParser,
  pluginModule: unknown,
  options?: Record<string, unknown>,
): MarkdownItParser {
  const plugin = resolvePlugin(pluginModule);
  if (!plugin) {
    return parser;
  }
  return options ? parser.use(plugin as any, options) : parser.use(plugin as any);
}

const markdownParser = (() => {
  const parser = new MarkdownIt({
    html: false,
    linkify: true,
    typographer: true,
  });

  usePluginSafe(parser, emoji);
  usePluginSafe(parser, footnote);
  usePluginSafe(parser, sub);
  usePluginSafe(parser, sup);
  usePluginSafe(parser, deflist);
  usePluginSafe(parser, abbr);
  usePluginSafe(parser, ins);
  usePluginSafe(parser, mark);
  usePluginSafe(parser, taskLists, { enabled: true, label: true });

  return parser;
})();

export function MarkdownRenderer({
  markdown,
  onLinkPress,
}: {
  markdown: string;
  onLinkPress?: (url: string) => boolean;
}) {
  return (
    <Markdown markdownit={markdownParser} style={markdownStyles as any} onLinkPress={onLinkPress}>
      {markdown || "No document selected."}
    </Markdown>
  );
}

const markdownStyles = StyleSheet.create({
  h1: {
    color: colors.dashValue,
    fontSize: font.size.xl3,
    fontWeight: font.weight.bold,
    lineHeight: 38,
    marginTop: spacing.sm,
    marginBottom: spacing.md,
  },
  h2: {
    color: colors.dashValue,
    fontSize: font.size.xl2,
    fontWeight: font.weight.bold,
    lineHeight: 34,
    marginTop: spacing.sm,
    marginBottom: spacing.sm,
  },
  h3: {
    color: colors.dashValue,
    fontSize: font.size.xl,
    fontWeight: font.weight.bold,
    lineHeight: 30,
    marginTop: spacing.xs,
    marginBottom: spacing.sm,
  },
  h4: {
    color: colors.dashValue,
    fontSize: font.size.lg,
    fontWeight: font.weight.bold,
    lineHeight: 26,
    marginBottom: spacing.xs,
  },
  body: {
    color: colors.foregroundDark,
    fontSize: font.size.md2,
    lineHeight: 24,
  },
  paragraph: {
    marginTop: 0,
    marginBottom: spacing.sm,
  },
  blockquote: {
    borderLeftWidth: 3,
    borderLeftColor: colors.primary,
    backgroundColor: colors.backgroundDarkSubtle,
    paddingVertical: spacing.sm,
    paddingHorizontal: spacing.md,
    borderRadius: radius.sm,
    marginTop: spacing.xs,
    marginBottom: spacing.md,
  },
  code_block: {
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    backgroundColor: colors.backgroundDark,
    borderRadius: radius.md,
    marginTop: spacing.xs,
    padding: spacing.md,
    marginBottom: spacing.md,
  },
  fence: {
    color: colors.dashPrimary,
    fontSize: font.size.sm2,
    lineHeight: 20,
    fontFamily: "Courier",
  },
  code_inline: {
    color: colors.dashPrimary,
    backgroundColor: colors.backgroundDark,
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: radius.sm,
    paddingHorizontal: spacing.xs,
    paddingVertical: 1,
    fontFamily: "Courier",
    fontSize: font.size.sm2,
  },
  bullet_list: {
    marginTop: spacing.xs,
    marginBottom: spacing.sm,
  },
  ordered_list: {
    marginTop: spacing.xs,
    marginBottom: spacing.sm,
  },
  list_item: {
    color: colors.foregroundDark,
    fontSize: font.size.md2,
    lineHeight: 22,
    marginBottom: spacing.xs2,
  },
  bullet_list_icon: {
    color: colors.dashMuted,
    fontSize: font.size.md,
  },
  ordered_list_icon: {
    color: colors.dashMuted,
    fontSize: font.size.md,
  },
  table: {
    borderWidth: 1,
    borderColor: colors.dashCardBorder,
    borderRadius: radius.md,
    marginTop: spacing.xs,
    marginBottom: spacing.md,
    overflow: "hidden",
  },
  thead: {
    backgroundColor: colors.backgroundDarkSubtle,
  },
  tbody: {
    backgroundColor: colors.dashCard,
  },
  th: {
    color: colors.dashPrimary,
    fontWeight: font.weight.bold,
    paddingHorizontal: spacing.sm,
    paddingVertical: spacing.sm2,
    borderRightWidth: 1,
    borderRightColor: colors.dashCardBorder,
  },
  td: {
    color: colors.foregroundDark,
    paddingHorizontal: spacing.sm,
    paddingVertical: spacing.sm2,
    borderTopWidth: 1,
    borderTopColor: colors.dashCardBorder,
    borderRightWidth: 1,
    borderRightColor: colors.dashCardBorder,
  },
  hr: {
    height: 1,
    backgroundColor: colors.dashCardBorder,
    marginVertical: spacing.sm,
  },
  link: {
    color: colors.primary,
    textDecorationLine: "underline",
  },
  inline: {
    color: colors.foregroundDark,
  },
  strong: {
    color: colors.dashValue,
    fontWeight: font.weight.bold,
  },
  em: {
    color: colors.foregroundDark,
  },
  footnote_reference: {
    color: colors.primary,
  },
});
