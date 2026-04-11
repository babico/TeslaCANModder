import { Tabs } from 'expo-router';
import { Platform, StatusBar } from 'react-native';
import { SafeAreaProvider, SafeAreaView } from 'react-native-safe-area-context';
import { colors, shadows } from '../styles/theme';

const isWeb = Platform.OS === 'web';

export default function RootLayout() {
  return (
    <SafeAreaProvider>
      <SafeAreaView style={{ flex: 1, backgroundColor: colors.bg }} edges={['top']}>
        <StatusBar barStyle="light-content" backgroundColor={colors.bg} />
        <Tabs
          screenOptions={{
            headerShown: false,
          tabBarStyle: {
            backgroundColor: colors.surface,
            borderTopColor: colors.borderLight,
            borderTopWidth: 1,
            ...shadows.sm,
          },
          tabBarActiveTintColor: colors.accent,
          tabBarInactiveTintColor: colors.textMuted,
          tabBarLabelStyle: { fontSize: 12, fontWeight: '500' },
        }}
      >
        <Tabs.Screen
          name="index"
          options={{ title: 'Dashboard', tabBarLabel: 'Dashboard' }}
        />
        <Tabs.Screen
          name="vehicle"
          options={{ title: 'Vehicle', tabBarLabel: 'Vehicle' }}
        />
        <Tabs.Screen
          name="monitor"
          options={{ title: 'Monitor', tabBarLabel: 'Monitor' }}
        />
        <Tabs.Screen
          name="docs"
          options={{ title: 'Docs', tabBarLabel: 'Docs' }}
        />
        <Tabs.Screen
          name="flasher"
          options={{
            title: 'Flasher',
            tabBarLabel: 'Flasher',
            href: isWeb ? '/flasher' : null,
          }}
        />
      </Tabs>
      </SafeAreaView>
    </SafeAreaProvider>
  );
}
