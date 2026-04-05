import { useState, useRef, useEffect } from 'react';
import { View, Text, TextInput, TouchableOpacity, FlatList, StyleSheet } from 'react-native';
import { colors, spacing, radius } from '../styles/theme';

interface LogMsg { id: string; ts: string; text: string; type: string; }

interface Props {
  messages: LogMsg[];
  connected: boolean;
  onCommand: (cmd: string) => void;
  onClear: () => void;
}

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

  const typeColor = (t: string) => {
    if (t === 'error') return colors.error;
    if (t === 'log') return colors.textMuted;
    if (t === 'ack') return colors.success;
    return colors.text;
  };

  return (
    <View style={styles.panel}>
      <View style={styles.header}>
        <Text style={styles.title}>Console</Text>
        <TouchableOpacity onPress={onClear}><Text style={styles.clearBtn}>Clear</Text></TouchableOpacity>
      </View>

      <FlatList
        ref={listRef}
        data={messages}
        keyExtractor={m => m.id}
        style={styles.body}
        renderItem={({ item }) => (
          <View style={styles.line}>
            <Text style={styles.ts}>{item.ts}</Text>
            <Text style={[styles.text, { color: typeColor(item.type) }]}>{item.text}</Text>
          </View>
        )}
        ListEmptyComponent={<Text style={styles.empty}>Board messages will appear here</Text>}
      />

      <View style={styles.inputRow}>
        <TextInput
          style={styles.input}
          placeholder="Type command..."
          placeholderTextColor={colors.textDim}
          value={input}
          onChangeText={setInput}
          onSubmitEditing={handleSend}
          editable={connected}
          returnKeyType="send"
        />
        <TouchableOpacity style={styles.sendBtn} onPress={handleSend} disabled={!connected}>
          <Text style={styles.sendText}>Send</Text>
        </TouchableOpacity>
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  panel: { flex: 1, backgroundColor: colors.surface, borderRadius: radius.md, overflow: 'hidden' },
  header: { flexDirection: 'row', justifyContent: 'space-between', alignItems: 'center', padding: spacing.sm, borderBottomWidth: 1, borderBottomColor: colors.border },
  title: { color: colors.text, fontWeight: '600', fontSize: 14 },
  clearBtn: { color: colors.textMuted, fontSize: 12 },
  body: { flex: 1, padding: spacing.sm },
  line: { flexDirection: 'row', gap: spacing.sm, paddingVertical: 2 },
  ts: { color: colors.textDim, fontSize: 11, fontFamily: 'monospace', width: 65 },
  text: { fontSize: 12, fontFamily: 'monospace', flex: 1 },
  empty: { color: colors.textDim, fontSize: 13, textAlign: 'center', marginTop: spacing.lg },
  inputRow: { flexDirection: 'row', borderTopWidth: 1, borderTopColor: colors.border },
  input: { flex: 1, padding: spacing.sm, color: colors.text, fontSize: 13, fontFamily: 'monospace' },
  sendBtn: { padding: spacing.sm, justifyContent: 'center' },
  sendText: { color: colors.accent, fontWeight: '600', fontSize: 13 },
});
