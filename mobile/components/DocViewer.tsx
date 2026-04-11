import React from 'react';
import { View, Text, ScrollView, StyleSheet } from 'react-native';
import { Asset } from 'expo-asset';
import { colors, spacing, radius } from '../styles/theme';

const DOC_MAP: Record<string, { labelEn: string; labelTr: string }> = {
  'getting-started': { labelEn: 'Getting Started', labelTr: 'Başlarken' },
  'hardware-setup': { labelEn: 'Hardware Setup', labelTr: 'Donanım Kurulumu' },
  'commands': { labelEn: 'Commands', labelTr: 'Komutlar' },
  'can-protocol': { labelEn: 'CAN Protocol', labelTr: 'CAN Protokolü' },
  'firmware-variants': { labelEn: 'Firmware Variants', labelTr: 'Firmware Varyantları' },
  'ble': { labelEn: 'Bluetooth (BLE)', labelTr: 'Bluetooth (BLE)' },
  'wifi-api': { labelEn: 'WiFi API', labelTr: 'WiFi API' },
  'vehicle-features': { labelEn: 'Vehicle Features', labelTr: 'Araç Özellikleri' },
  'troubleshooting': { labelEn: 'Troubleshooting', labelTr: 'Sorun Giderme' },
};

/* eslint-disable @typescript-eslint/no-require-imports */
// Load markdown files from the root docs/ folder (shared across all apps)
const docAssets: Record<string, number> = {
  'getting-started': require('../../docs/getting-started.md'),
  'hardware-setup': require('../../docs/hardware-setup.md'),
  'commands': require('../../docs/commands.md'),
  'can-protocol': require('../../docs/can-protocol.md'),
  'firmware-variants': require('../../docs/firmware-variants.md'),
  'ble': require('../../docs/ble.md'),
  'wifi-api': require('../../docs/wifi-api.md'),
  'vehicle-features': require('../../docs/vehicle-features.md'),
  'troubleshooting': require('../../docs/troubleshooting.md'),
};
/* eslint-enable @typescript-eslint/no-require-imports */

async function loadDoc(section: string): Promise<{ text: string; label: string }> {
  const map = DOC_MAP[section];
  const label = map?.labelEn || section;
  try {
    const assetId = docAssets[section];
    if (assetId == null) return { text: `No content found for ${section}.`, label };
    const [asset] = await Asset.loadAsync(assetId);
    const uri = asset.localUri || asset.uri;
    const res = await fetch(uri);
    return { text: await res.text(), label };
  } catch {
    return { text: `Failed to load ${section}.`, label };
  }
}

interface Props {
  content?: string;
  title?: string;
  section?: string;
  lang?: 'en' | 'tr';
}

/** Simple markdown-to-text viewer for documentation. Renders headings, paragraphs, lists, and code blocks. */
export default function DocViewer({ content, title, section, lang }: Props) {
  const [resolved, setResolved] = React.useState(content || '');
  const [resolvedTitle, setResolvedTitle] = React.useState(title || '');

  React.useEffect(() => {
    if (content) { setResolved(content); return; }
    if (!section) return;
    loadDoc(section).then(({ text, label }) => {
      setResolved(text);
      if (!title) setResolvedTitle(label);
    });
  }, [content, section, title]);

  const blocks = parseMarkdown(resolved);

  return (
    <ScrollView style={styles.container} contentContainerStyle={styles.content}>
      <Text style={styles.title}>{resolvedTitle || title}</Text>
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
