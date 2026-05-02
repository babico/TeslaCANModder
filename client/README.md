# Tesla CAN Modder Client

Expo client for Tesla CAN Modder on web, iOS, and Android.

## What It Includes

- Dashboard and controls for live board state and command execution
- Console/monitor surface for frame streaming, decoder views, and diagnostics
- Browser flasher for release firmware assets over Web Serial
- In-app documentation rendered directly from the repo `docs/` tree

## Development

Start the client:

```bash
npm run start -w @teslacanmodder/client
```

Run the browser target:

```bash
npm run web -w @teslacanmodder/client
```

Other targets:

```bash
npm run ios -w @teslacanmodder/client
npm run android -w @teslacanmodder/client
```

## Validation

```bash
npm run typecheck -w @teslacanmodder/client
npm test -w @teslacanmodder/client
npm run web:build -w @teslacanmodder/client
```

## Transports

The client talks to the board through the transport layer in `src/hardware/`.

Primary paths:

- Web Serial / COM serial
- Bluetooth COM
- BLE
- REST API / WiFi bridge
- Socket-style bridge adapters

Default HTTP routes:

- `POST /api/command` with `{ "cmd": "status" }`
- `GET /api/status`

## Docs Flow

The docs screen no longer depends on a generated TypeScript bundle. Raw markdown files under the repo `docs/` directory are bundled as assets, frontmatter is parsed in the client, and markdown is rendered with the existing `markdown-it` plugin stack.# Tesla CAN Modder Client

Unified Expo client for browser, iOS, and Android targets.

## Start

```bash
npm install
npm run start -w @teslacanmodder/client
```

## Run In Browser

```bash
npm run web -w @teslacanmodder/client
```

## Hardware Control

The app sends firmware commands through a transport layer. Use the endpoint settings in the app to point to your board bridge API.

Default command endpoint:

- `POST /api/command` with JSON body `{ "cmd": "status" }`

Default status endpoint:

- `GET /api/status`
