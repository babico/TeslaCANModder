/** Docs tab — documentation viewer loading markdown from root docs/ folder. */

import { useState } from 'react';
import { View, ScrollView, Text, TouchableOpacity, StyleSheet } from 'react-native';
import DocViewer from '../components/DocViewer';
import { colors, spacing } from '../styles/theme';

const DOC_SECTIONS = [
  { id: 'getting-started', label: 'Getting Started' },
  { id: 'hardware-setup', label: 'Hardware Setup' },
  { id: 'commands', label: 'Commands' },
  { id: 'can-protocol', label: 'CAN Protocol' },
  { id: 'firmware-variants', label: 'Firmware Variants' },
  { id: 'ble', label: 'Bluetooth (BLE)' },
  { id: 'wifi-api', label: 'WiFi API' },
  { id: 'vehicle-features', label: 'Vehicle Features' },
  { id: 'troubleshooting', label: 'Troubleshooting' },
];

export default function DocsScreen() {
  const [activeDoc, setActiveDoc] = useState('getting-started');

  return (
    <View style={styles.container}>
      {/* Section tabs */}
      <ScrollView horizontal showsHorizontalScrollIndicator={false} style={styles.tabBar}>
        {DOC_SECTIONS.map((sec) => (
          <TouchableOpacity
            key={sec.id}
            style={[styles.tab, activeDoc === sec.id && styles.tabActive]}
            onPress={() => setActiveDoc(sec.id)}
          >
            <Text style={[styles.tabText, activeDoc === sec.id && styles.tabTextActive]}>
              {sec.label}
            </Text>
          </TouchableOpacity>
        ))}
      </ScrollView>

      {/* Content */}
      <ScrollView style={styles.content}>
        <DocViewer section={activeDoc} />
      </ScrollView>
    </View>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: colors.bg },
  tabBar: { backgroundColor: colors.surface, borderBottomWidth: 1, borderBottomColor: colors.border, maxHeight: 48 },
  tab: { paddingHorizontal: spacing.md, paddingVertical: spacing.sm, justifyContent: 'center' },
  tabActive: { borderBottomWidth: 2, borderBottomColor: colors.accent },
  tabText: { color: colors.textMuted, fontSize: 13 },
  tabTextActive: { color: colors.accent },
  content: { flex: 1, padding: spacing.md },
});
