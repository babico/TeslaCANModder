import { View, Text, TouchableOpacity, FlatList, StyleSheet } from 'react-native';
import { colors, spacing, radius, shadows } from '../styles/theme';
import { Badge } from './ui';
import type { CanFrame } from '@teslacanmodder/protocol';

interface Props {
  frames: CanFrame[];
  frameCount: number;
  onClear: () => void;
  recording?: boolean;
  onToggleRecord?: () => void;
}

export default function FrameTable({ frames, frameCount, onClear, recording, onToggleRecord }: Props) {
  return (
    <View style={styles.panel}>
      <View style={styles.header}>
        <Text style={styles.title}>Live CAN Frames</Text>
        <View style={styles.actions}>
          {onToggleRecord && (
            <TouchableOpacity onPress={onToggleRecord}>
              <Text style={[styles.actionBtn, recording && styles.recordActive]}>
                {recording ? '⏹ Stop' : '⏺ Record'}
              </Text>
            </TouchableOpacity>
          )}
          <TouchableOpacity onPress={onClear}>
            <Text style={styles.actionBtn}>Clear</Text>
          </TouchableOpacity>
          <Badge label={`${frameCount} frames`} />
        </View>
      </View>

      <View style={styles.headerRow}>
        <Text style={[styles.hCell, styles.colTime]}>Time</Text>
        <Text style={[styles.hCell, styles.colDir]}>Dir</Text>
        <Text style={[styles.hCell, styles.colId]}>ID</Text>
        <Text style={[styles.hCell, styles.colSeq]}>Seq</Text>
        <Text style={[styles.hCell, styles.colDlc]}>DLC</Text>
        <Text style={[styles.hCell, styles.colData]}>Data</Text>
      </View>

      {frames.length === 0 ? (
        <Text style={styles.empty}>Connect and start streaming to see CAN frames</Text>
      ) : (
        <FlatList
          data={frames}
          keyExtractor={f => f.key}
          style={styles.body}
          renderItem={({ item }) => (
            <View style={styles.row}>
              <Text style={[styles.cell, styles.colTime]}>{item.ts}</Text>
              <Text style={[styles.cell, styles.colDir, item.dir === 'tx' ? styles.dirTx : styles.dirRx]}>{item.dir.toUpperCase()}</Text>
              <Text style={[styles.cell, styles.colId, styles.mono]}>0x{item.id.toString(16).toUpperCase()}</Text>
              <Text style={[styles.cell, styles.colSeq, styles.mono]}>{item.seq ?? '—'}</Text>
              <Text style={[styles.cell, styles.colDlc]}>{item.dlc}</Text>
              <Text style={[styles.cell, styles.colData, styles.mono]} numberOfLines={1}>{item.data || '—'}</Text>
            </View>
          )}
        />
      )}
    </View>
  );
}

const styles = StyleSheet.create({
  panel: {
    flex: 1,
    backgroundColor: colors.surface,
    borderRadius: radius.lg,
    borderWidth: 1,
    borderColor: colors.borderLight,
    overflow: 'hidden',
    ...shadows.sm,
  },
  header: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    padding: spacing.sm,
    paddingHorizontal: spacing.md,
    borderBottomWidth: 1,
    borderBottomColor: colors.borderLight,
  },
  title: { color: colors.text, fontWeight: '600', fontSize: 14 },
  actions: { flexDirection: 'row', alignItems: 'center', gap: spacing.sm },
  actionBtn: { color: colors.textMuted, fontSize: 12 },
  recordActive: { color: colors.error },
  headerRow: {
    flexDirection: 'row',
    paddingHorizontal: spacing.sm,
    paddingVertical: 4,
    backgroundColor: colors.bgTertiary,
    borderBottomWidth: 1,
    borderBottomColor: colors.borderLight,
  },
  hCell: { color: colors.textDim, fontSize: 11, fontWeight: '600' },
  body: { flex: 1 },
  row: {
    flexDirection: 'row',
    paddingHorizontal: spacing.sm,
    paddingVertical: 3,
    borderBottomWidth: StyleSheet.hairlineWidth,
    borderBottomColor: colors.borderLight,
  },
  cell: { fontSize: 11, color: colors.text },
  mono: { fontFamily: 'monospace' },
  colTime: { width: 55 },
  colDir: { width: 30 },
  colId: { width: 60 },
  colSeq: { width: 35 },
  colDlc: { width: 28 },
  colData: { flex: 1 },
  dirTx: { color: colors.accent },
  dirRx: { color: colors.success },
  empty: { color: colors.textDim, fontSize: 13, textAlign: 'center', padding: spacing.lg },
});
