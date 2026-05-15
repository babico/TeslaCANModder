# teslamotors/vehicle-command

**Submodule:** `legacy/teslamotors-vehicle-command`
**URL:** https://github.com/teslamotors/vehicle-command
**License:** Apache-2.0
**Language:** Go

## Overview

Official Tesla vehicle-command protocol implementation. The canonical reference for Tesla's BLE vehicle API and command protocol. Provides gRPC services for issuing vehicle commands and receiving telemetry.

## Architecture

```
teslamotors-vehicle-command/
├── cmd/                # CLI tools (tesla-command, tesla-key)
├── pkg/                # Protocol packages
│   ├── protocol/       # Core protocol definitions
│   ├── proto/          # gRPC proto files
│   └── ...
├── internal/          # Internal implementations
├── doc/                # Protocol documentation
├── examples/           # Usage examples
├── Dockerfile
├── docker-compose.yml
├── Makefile
├── .golangci.yml       # Linting config
├── go.mod/go.sum
└── check-all.sh
```

## Key Components

### Protocol (pkg/protocol/)
- **Protocol Buffers:** gRPC service definitions
- **Cryptography:** Vehicle authentication (VCSEC)
- **BLE Frames:** Frame encoding/decoding
- **Commands:** HVAC, locks, trunks, charging, etc.

### CLI Tools
- `tesla-command` — Issue vehicle commands
- `tesla-key` — Key management/authentication

## Protocol Details

### Authentication Flow (VCSEC)
1. Client initiates session with vehicle
2. Ephemeral key exchange
3. Command signing with shared secret
4. Encrypted command transmission

### Supported Commands
- Lock/unlock
- Climate control ( HVAC)
- Charge port open/close
- Trunk/frunk
- Speed limit
- And more...

## Comparison with TeslaCANModder

| Aspect | teslamotors/vehicle-command | TeslaCANModder |
|--------|----------------------------|----------------|
| Purpose | Vehicle control API | Autopilot CAN modification |
| Transport | BLE (encrypted) | CAN bus (raw frames) |
| Authentication | VCSEC cryptographic | None (passive CAN) |
| Modification | Issues commands | Intercepts/modifies CAN frames |

## Canonical Reference

This repo is the **primary reference** for:
1. Tesla protocol frame formats
2. gRPC service definitions
3. Authentication protocols
4. Command request/response schemas

## Relevant Files

- `pkg/protocol/` — Frame formats, checksums
- `doc/` — Protocol documentation
- `pkg/proto/` — gRPC definitions