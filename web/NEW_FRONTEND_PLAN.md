# TeslaCANModder - New Frontend Plan

## Goals
- **Simple & Clean** - No complexity, just what's needed
- **Fast** - Minimal re-renders, efficient updates
- **Usable** - Clear controls, obvious status
- **Mobile-friendly** - Works on phone and desktop

## Tech Stack
- React 18 + Vite
- Web Serial API for USB/Bluetooth
- CSS Grid for layout
- No UI framework dependencies

## Layout Structure

### Desktop (3-panel layout)
```
┌─────────────────────────────────────────┐
│ Header: Connection + Quick Actions      │
├──────────────────┬──────────────────────┤
│ Left (60%)       │ Right (40%)          │
│ CAN Frames       │ Controls             │
│ - Live table     │ - Hardware info      │
│ - Scrollable     │ - Feature toggles    │
│                  │ - Status indicators  │
├──────────────────┴──────────────────────┤
│ Bottom: Console (full width)            │
└─────────────────────────────────────────┘
```

### Mobile (tabbed)
```
┌─────────────────────────────────────────┐
│ Header: Connection Status               │
├─────────────────────────────────────────┤
│ [Control] [Monitor] [Console]           │
├─────────────────────────────────────────┤
│                                         │
│ Active Tab Content                      │
│                                         │
└─────────────────────────────────────────┘
```

## Components

### Core Components
1. **App.jsx** - Main app shell, routing
2. **Dashboard.jsx** - Main dashboard view
3. **ConnectionBar.jsx** - Connection status + controls
4. **FrameTable.jsx** - Live CAN frame display
5. **ControlPanel.jsx** - Feature controls
6. **Console.jsx** - Command input/output

### Hooks
1. **useSerial.js** - Web Serial API wrapper
2. **useBoardState.js** - Board state management

## Features

### Connection
- USB Serial (primary)
- Bluetooth Serial (optional)
- Auto-reconnect
- Status indicator

### Hardware Info
- Board model
- CAN driver
- Variant (HW4/HW3/Legacy)
- Uptime
- Message rate

### Feature Controls (All OFF by default)
- **FSD** - Enable/Disable toggle
- **Nag Suppression** - Enable/Disable toggle
- **Speed Profile** - 0-4 buttons
- **Speed Offset** - 0-100% (HW3 only)
- **ISA Chime** - Suppress/Original (HW4 only)

### CAN Monitoring
- Live frame table
- Direction (RX/TX)
- Frame ID, DLC, Data
- Byte diff highlighting
- Stream on/off

### Console
- Command input
- Message history
- Auto-scroll
- Clear button

## State Management

### Board State
```js
{
  connected: false,
  variant: 'hw4',
  hardware: 'ArduinoUnoR3CH340',
  driver: 'MCP2515',
  transport: 'usb',
  uptime: 0,
  rate: 0,
  
  // Features
  fsd: false,
  nag: false,
  profile: 1,
  offset: 0,
  isaChime: false,
  
  // Streaming
  streaming: false,
  frames: [],
  
  // Console
  messages: []
}
```

## Protocol (Same as hardware)

### Commands
```
ping
status
variant:hw4|hw3|legacy
fsd:on|off|toggle
nag:on|off|toggle
profile:0-4
offset:0-100
isa-chime:on|off|toggle
stream:on|off
can:raw:on|off
```

### Messages
```json
{"t":"boot","hw":"...","variant":"hw4",...}
{"t":"status","fsd":0,"sp":1,"nag":0,...}
{"t":"frame","id":1021,"dlc":8,"d":"...",...}
{"t":"ack","cmd":"fsd:on"}
{"t":"error","msg":"..."}
{"t":"pong","v":1}
```

## File Structure

```
web/
├── src/
│   ├── components/
│   │   ├── ConnectionBar.jsx
│   │   ├── FrameTable.jsx
│   │   ├── ControlPanel.jsx
│   │   ├── Console.jsx
│   │   └── Dashboard.jsx
│   ├── hooks/
│   │   ├── useSerial.js
│   │   └── useBoardState.js
│   ├── utils/
│   │   ├── protocol.js
│   │   └── commands.js
│   ├── styles/
│   │   ├── reset.css
│   │   ├── variables.css
│   │   ├── layout.css
│   │   └── components.css
│   ├── App.jsx
│   └── main.jsx
├── index.html
├── vite.config.js
└── package.json
```

## Design System

### Colors
- Background: Dark (#0a0c10)
- Surface: #14171f
- Border: #2a2e3a
- Text: #e4e6eb
- Accent: #e82127 (Tesla red)
- Success: #34d399
- Warning: #f59e0b
- Error: #ef4444

### Typography
- UI: Inter, system-ui
- Mono: 'Fira Code', monospace

### Spacing
- Base: 4px
- Small: 8px
- Medium: 16px
- Large: 24px

## Implementation Steps

1. ✅ Create new file structure
2. ✅ Setup Vite + React
3. ✅ Implement useSerial hook
4. ✅ Implement useBoardState hook
5. ✅ Create ConnectionBar component
6. ✅ Create ControlPanel component
7. ✅ Create FrameTable component
8. ✅ Create Console component
9. ✅ Create Dashboard layout
10. ✅ Add mobile responsive
11. ✅ Test with hardware
12. ✅ Polish & optimize

## Next Steps

Shall I proceed with creating the new frontend from scratch?
