import { View, Text, ScrollView, StyleSheet } from 'react-native';
import { colors, spacing, radius } from '../styles/theme';

interface Props {
  content: string;
  title: string;
}

/** Simple markdown-to-text viewer for documentation. Renders headings, paragraphs, lists, and code blocks. */
export default function DocViewer({ content, title }: Props) {
  const blocks = parseMarkdown(content);

  return (
    <ScrollView style={styles.container} contentContainerStyle={styles.content}>
      <Text style={styles.title}>{title}</Text>
      {blocks.map((block, i) => {
        switch (block.type) {
          case 'h1': return <Text key={i} style={styles.h1}>{block.text}</Text>;
          case 'h2': return <Text key={i} style={styles.h2}>{block.text}</Text>;
          case 'h3': return <Text key={i} style={styles.h3}>{block.text}</Text>;
          case 'code': return <View key={i} style={styles.codeBlock}><Text style={styles.codeText}>{block.text}</Text></View>;
          case 'list': return <Text key={i} style={styles.listItem}>  •  {block.text}</Text>;
          case 'p': default: return <Text key={i} style={styles.paragraph}>{block.text}</Text>;
        }
      })}
    </ScrollView>
  );
}

interface Block { type: 'h1' | 'h2' | 'h3' | 'p' | 'code' | 'list'; text: string; }

function parseMarkdown(md: string): Block[] {
  const lines = md.split('\n');
  const blocks: Block[] = [];
  let inCode = false;
  let codeBuf = '';

  for (const line of lines) {
    if (line.startsWith('```')) {
      if (inCode) { blocks.push({ type: 'code', text: codeBuf.trim() }); codeBuf = ''; }
      inCode = !inCode;
      continue;
    }
    if (inCode) { codeBuf += line + '\n'; continue; }
    const trimmed = line.trim();
    if (!trimmed) continue;
    if (trimmed.startsWith('### ')) blocks.push({ type: 'h3', text: trimmed.slice(4) });
    else if (trimmed.startsWith('## ')) blocks.push({ type: 'h2', text: trimmed.slice(3) });
    else if (trimmed.startsWith('# ')) blocks.push({ type: 'h1', text: trimmed.slice(2) });
    else if (trimmed.startsWith('- ') || trimmed.startsWith('* ')) blocks.push({ type: 'list', text: trimmed.slice(2) });
    else blocks.push({ type: 'p', text: trimmed });
  }
  if (codeBuf) blocks.push({ type: 'code', text: codeBuf.trim() });
  return blocks;
}

const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: colors.bg },
  content: { padding: spacing.lg },
  title: { color: colors.text, fontSize: 22, fontWeight: '700', marginBottom: spacing.lg },
  h1: { color: colors.text, fontSize: 20, fontWeight: '700', marginTop: spacing.lg, marginBottom: spacing.sm },
  h2: { color: colors.text, fontSize: 17, fontWeight: '600', marginTop: spacing.md, marginBottom: spacing.xs },
  h3: { color: colors.text, fontSize: 15, fontWeight: '600', marginTop: spacing.sm, marginBottom: spacing.xs },
  paragraph: { color: colors.textMuted, fontSize: 14, lineHeight: 21, marginBottom: spacing.sm },
  listItem: { color: colors.textMuted, fontSize: 14, lineHeight: 21, marginBottom: 4 },
  codeBlock: { backgroundColor: colors.bgSecondary, padding: spacing.md, borderRadius: radius.md, marginBottom: spacing.sm },
  codeText: { color: colors.text, fontSize: 12, fontFamily: 'monospace', lineHeight: 18 },
});
