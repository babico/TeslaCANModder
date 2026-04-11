# TeslaCANModder Web UI

Simple, usable web interface for Tesla CAN bus modification control.

## Features

- **USB & Bluetooth Serial** - Connect via Web Serial API
- **Runtime Variant Switching** - HW4, HW3, Legacy
- **Live CAN Monitoring** - Real-time frame streaming with per-bus lane labels
- **CAN Frame Decoder** - 577 known Tesla frames with signal details
- **Per-Bus Vehicle Controls** - Controls gated by active bus (Vehicle/Body)
- **Feature Control** - FSD, nag, profile, offset, ISA chime
- **Firmware Flasher** - Build & flash with per-bus flag selection
- **Mobile Responsive** - Works on desktop and Android Chrome

## Quick Start

```bash
cd web
npm install
npm run dev
```

Open http://localhost:5173

## Build

```bash
npm run build
```

Output: `dist/`

## Protocol Compatibility

The web UI is 100% compatible with the new simplified hardware firmware. Both use the same JSON protocol:

### Messages
- `boot` - Board initialization
- `status` - State updates (every 500ms)
- `frame` - CAN frames (when streaming)
- `ack` - Command acknowledgment
- `error` - Error messages
- `pong` - Ping response

### Commands
- `ping`, `status`
- `variant:hw4`, `variant:hw3`, `variant:legacy`
- `fsd:on`, `fsd:off`, `fsd:toggle`
- `nag:on`, `nag:off`, `nag:toggle`
- `profile:0-4` or `sp:0-4`
- `offset:0-100` (HW3)
- `isa-chime:on`, `isa-chime:off`, `isa-chime:toggle` (HW4)
- `stream:on`, `stream:off`
- `can:raw:on`, `can:raw:off`

## Browser Support

- ✅ Desktop Chrome/Edge - Full support (flashing + control)
- ✅ Android Chrome - Runtime control via Bluetooth
- ⚠️ Other browsers - Guide mode only

## Structure

```
web/
├── src/
│   ├── components/     - ConnectionBar, ControlPanel, FrameTable, Flasher, Console
│   ├── hooks/          - useBoardState (board state + bus flags)
│   ├── lib/board/      - Serial client, protocol, commands
│   ├── pages/          - DashboardPage, FlasherPage, SetupPage
│   └── styles/         - CSS modules
├── public/
│   └── can_frames_mcu2.json  - CAN frame decoder data (577 frames)
└── index.html          - Entry point
```

## Development

The web UI automatically detects browser capabilities and adapts:
- Desktop: Full dashboard with drag-drop tiles
- Mobile: Tabbed interface (Control/Monitor/Advanced)
- No serial: Guide-only mode

## Testing Hardware Compatibility

1. Build and flash firmware: `cd hardware && .\pio.ps1 run -e uno -t upload`
2. Start web UI: `cd web && npm run dev`
3. Open http://localhost:5173
4. Click "Connect USB"
5. Select Arduino port
6. Verify boot message appears
7. Test commands: `ping`, `status`, `variant:hw4`, `fsd:on`, `stream:on`

All commands should work identically to the old firmware.
