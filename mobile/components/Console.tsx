import { useState, useRef, useEffect } from 'react';
import { View, Text, TextInput, FlatList, StyleSheet } from 'react-native';
import { colors, spacing, radius, shadows } from '../styles/theme';
import { Button, Badge } from './ui';

interface LogMsg { id: string; ts: string; text: string; type: string; }

interface Props {
  messages: LogMsg[];
  connected: boolean;
  onCommand: (cmd: string) => void;
  onClear: () => void;
}

const TYPE_COLORS: Record<string, string> = {
  error: colors.error,
  log: colors.textMuted,
  ack: colors.success,
  warn: colors.warning,
};

export default function Console({ messages, connected, onCommand, onClear }: Props) {
  const [input, setInput] = useState('');
  const listRef = useRef<FlatList>(null);

  useEffect(() => {
    if (messages.length > 0) listRef.current?.scrollToEnd({ animated: true });
  }, [messages.length]);

  const handleSend = () => {
    if (!input.trim() || !connected) return;
    onCommand(input.trim());
    setInput('');
  };

  return (
    <View style={styles.panel}>
      <View style={styles.header}>
        <View style={styles.headerLeft}>
          <Text style={styles.title}>Console</Text>
          <Badge label={connected ? 'Online' : 'Offline'} variant={connected ? 'success' : 'default'} />
        </View>
        <Button label="Clear" variant="ghost" compact onPress={onClear} />
      </View>

      <FlatList
        ref={listRef}
        data={messages}
        keyExtractor={m => m.id}
        style={styles.body}
        renderItem={({ item }) => (
          <View style={styles.line}>
            <Text style={styles.ts}>{item.ts}</Text>
            <Text style={[styles.text, { color: TYPE_COLORS[item.type] ?? colors.text }]}>{item.text}</Text>
          </View>
        )}
        ListEmptyComponent={<Text style={styles.empty}>Board messages will appear here</Text>}
      />

      <View style={styles.inputRow}>
        <TextInput
          style={styles.input}
          placeholder={connected ? 'Type command...' : 'Connect to send commands'}
          placeholderTextColor={colors.textDim}
          value={input}
          onChangeText={setInput}
          onSubmitEditing={handleSend}
          editable={connected}
          returnKeyType="send"
        />
        <Button label="Send" variant="primary" compact onPress={handleSend} disabled={!connected || !input.trim()} />
      </View>
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
  headerLeft: { flexDirection: 'row', alignItems: 'center', gap: spacing.sm },
  title: { color: colors.text, fontWeight: '600', fontSize: 14 },
  body: { flex: 1, padding: spacing.sm },
  line: { flexDirection: 'row', gap: spacing.sm, paddingVertical: 2 },
  ts: { color: colors.textDim, fontSize: 11, fontFamily: 'monospace', width: 65 },
  text: { fontSize: 12, fontFamily: 'monospace', flex: 1 },
  empty: { color: colors.textDim, fontSize: 13, textAlign: 'center', marginTop: spacing.lg },
  inputRow: {
    flexDirection: 'row',
    alignItems: 'center',
    borderTopWidth: 1,
    borderTopColor: colors.borderLight,
    paddingRight: spacing.xs,
  },
  input: { flex: 1, padding: spacing.sm, color: colors.text, fontSize: 13, fontFamily: 'monospace' },
});
