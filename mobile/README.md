# TeslaCANModder Mobile

React Native + Expo mobile app for controlling Tesla CAN bus modifications via BLE.

## Setup

```bash
npm install          # from repo root (workspace install)
cd mobile
npx expo start       # start dev server
```

## Architecture

- **Expo ~54.0.0** with React Native 0.81.5
- **expo-router ~6.0.23** — file-based tab navigation
- **@teslacanmodder/protocol** — shared types, commands, decoder

### Screens

| Tab | File | Description |
|-----|------|-------------|
| Dashboard | `app/index.tsx` | Connection bar + control panel |
| Vehicle | `app/vehicle.tsx` | Mirror, lock, light, climate, charge, drive commands |
| Monitor | `app/monitor.tsx` | Live CAN frame table + serial console |
| Docs | `app/docs.tsx` | Built-in documentation viewer |
| Flasher | `app/flasher.tsx` | Firmware flashing (web only) |

### UI Components

Shared primitives in `components/ui/`:
- `Button` — variant (primary/secondary/ghost/danger), active, disabled, compact
- `Card` — title, right slot (ReactNode), warning banner
- `Badge` — colored pill (default/success/warning/error/accent/blue)
- `Section` — titled section with header
- `StatusDot` — connection status indicator

### Hooks

- `useTransport` — unified BLE + Serial transport with auto-detection
- `useBoardState` — firmware state management from JSON messages
- `useFrameHistory` — CAN frame recording/playback

## Testing

```bash
npm test             # run all 138 tests
```

Tests use Jest with `jest-expo` preset and `@testing-library/react-native`.

| Suite | Tests |
|-------|-------|
| boardState | useBoardState hook |
| commands | command string builders |
| decoder | CAN frame decoder |
| frameHistory | frame recording |
| protocol | serial parser |
| components/* | UI component rendering |

## Theme

Dark theme defined in `styles/theme.ts` with design tokens:
- Colors (bg, surface, text, accent, status colors, soft variants)
- Spacing scale (xs through xxl)
- Border radius (sm, md, lg, xl, full)
- Shadows (sm, md, lg with Android elevation)
- Typography scale (h1-h3, body, caption, label, mono)
