/** Monitor tab — live CAN frame table + serial console. */

import { View, StyleSheet } from 'react-native';
import FrameTable from '../components/FrameTable';
import Console from '../components/Console';
import { useTransport } from '../hooks/useTransport';
import { useBoardState } from '../hooks/useBoardState';
import { useFrameHistory } from '../hooks/useFrameHistory';
import { colors, spacing } from '../styles/theme';

export default function MonitorScreen() {
  const transport = useTransport();
  const board = useBoardState();
  const history = useFrameHistory();

  return (
    <View style={styles.container}>
      <View style={styles.framesSection}>
        <FrameTable
          frames={board.state.frames}
          frameCount={board.state.frameCount}
          onClear={board.clearFrames}
          recording={history.recording}
          onToggleRecord={() => history.recording ? history.stop() : history.start()}
        />
      </View>
      <View style={styles.consoleSection}>
        <Console
          messages={board.state.messages}
          connected={transport.connected}
          onCommand={transport.send}
          onClear={board.clearMessages}
        />
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: colors.bg },
  framesSection: { flex: 1, padding: spacing.md },
  consoleSection: { height: 250, padding: spacing.md, borderTopWidth: 1, borderTopColor: colors.borderLight },
});
