import { Tabs } from 'expo-router';
import { Platform, StatusBar } from 'react-native';
import { colors } from '../styles/theme';

export default function RootLayout() {
  return (
    <>
      <StatusBar barStyle="light-content" backgroundColor={colors.bg} />
      <Tabs
        screenOptions={{
          headerShown: false,
          tabBarStyle: {
            backgroundColor: colors.surface,
            borderTopColor: colors.border,
            borderTopWidth: 1,
          },
          tabBarActiveTintColor: colors.accent,
          tabBarInactiveTintColor: colors.textMuted,
          tabBarLabelStyle: { fontSize: 12 },
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
        {Platform.OS === 'web' && (
          <Tabs.Screen
            name="flasher"
            options={{ title: 'Flasher', tabBarLabel: 'Flasher' }}
          />
        )}
      </Tabs>
    </>
  );
}
