# Tesla CAN Modder Client

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
