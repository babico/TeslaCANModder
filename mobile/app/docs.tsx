/** Docs tab — bilingual documentation viewer (EN/TR). */

import { useState } from 'react';
import { View, ScrollView, Text, TouchableOpacity, StyleSheet } from 'react-native';
import DocViewer from '../components/DocViewer';
import { colors, spacing, radius } from '../styles/theme';

const DOC_SECTIONS = [
  { id: 'setup', labelEn: 'Setup Guide', labelTr: 'Kurulum Rehberi' },
  { id: 'wiring', labelEn: 'Wiring', labelTr: 'Kablolama' },
  { id: 'commands', labelEn: 'Commands', labelTr: 'Komutlar' },
  { id: 'can-protocol', labelEn: 'CAN Protocol', labelTr: 'CAN Protokolü' },
  { id: 'troubleshooting', labelEn: 'Troubleshooting', labelTr: 'Sorun Giderme' },
  { id: 'hardware-variants', labelEn: 'Hardware Variants', labelTr: 'Donanım Varyantları' },
  { id: 'firmware-flashing', labelEn: 'Firmware Flashing', labelTr: 'Firmware Yükleme' },
];

export default function DocsScreen() {
  const [lang, setLang] = useState<'en' | 'tr'>('en');
  const [activeDoc, setActiveDoc] = useState('setup');

  return (
    <View style={styles.container}>
      {/* Language toggle */}
      <View style={styles.langBar}>
        <TouchableOpacity
          style={[styles.langBtn, lang === 'en' && styles.langBtnActive]}
          onPress={() => setLang('en')}
        >
          <Text style={[styles.langText, lang === 'en' && styles.langTextActive]}>English</Text>
        </TouchableOpacity>
        <TouchableOpacity
          style={[styles.langBtn, lang === 'tr' && styles.langBtnActive]}
          onPress={() => setLang('tr')}
        >
          <Text style={[styles.langText, lang === 'tr' && styles.langTextActive]}>Türkçe</Text>
        </TouchableOpacity>
      </View>

      {/* Section tabs */}
      <ScrollView horizontal showsHorizontalScrollIndicator={false} style={styles.tabBar}>
        {DOC_SECTIONS.map((sec) => (
          <TouchableOpacity
            key={sec.id}
            style={[styles.tab, activeDoc === sec.id && styles.tabActive]}
            onPress={() => setActiveDoc(sec.id)}
          >
            <Text style={[styles.tabText, activeDoc === sec.id && styles.tabTextActive]}>
              {lang === 'en' ? sec.labelEn : sec.labelTr}
            </Text>
          </TouchableOpacity>
        ))}
      </ScrollView>

      {/* Content */}
      <ScrollView style={styles.content}>
        <DocViewer section={activeDoc} lang={lang} />
      </ScrollView>
    </View>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: colors.bg },
  langBar: { flexDirection: 'row', padding: spacing.sm, gap: spacing.xs, backgroundColor: colors.surface, borderBottomWidth: 1, borderBottomColor: colors.border },
  langBtn: { paddingHorizontal: spacing.md, paddingVertical: spacing.sm, borderRadius: radius.md },
  langBtnActive: { backgroundColor: colors.accentSoft },
  langText: { color: colors.textMuted, fontSize: 14 },
  langTextActive: { color: colors.accent },
  tabBar: { backgroundColor: colors.surface, borderBottomWidth: 1, borderBottomColor: colors.border, maxHeight: 48 },
  tab: { paddingHorizontal: spacing.md, paddingVertical: spacing.sm, justifyContent: 'center' },
  tabActive: { borderBottomWidth: 2, borderBottomColor: colors.accent },
  tabText: { color: colors.textMuted, fontSize: 13 },
  tabTextActive: { color: colors.accent },
  content: { flex: 1, padding: spacing.md },
});
