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

## BLE connection layer

The BLE transport lives under `pkg/connector/` and the gRPC-over-BLE dispatch is in `pkg/protocol/`. While the official SDK does not expose RSSI-based distance estimation, it shows the connection lifecycle used by the phone key:

1. Scan / connect to the vehicle's BLE advertisement
2. Establish a VCSEC session (ephemeral key exchange)
3. Sign and send commands over the encrypted BLE link

RSSI is available from the underlying BLE host stack during this flow. Our firmware uses NimBLE-Arduino, so we can read peer RSSI directly via `NimBLEClient::getRssi()` and estimate key-to-car distance without depending on vehicle-side support.

## Relevant Files

| Path | Notes |
| ---- | ----- |
| `pkg/protocol/` | Frame formats, checksums, VCSEC session |
| `pkg/connector/` | BLE connector abstraction |
| `pkg/vehicle/` | High-level command builders |
| `doc/` | Protocol documentation |
| `pkg/proto/` | gRPC definitions |

## Potential improvements for TeslaCANModder

1. **BLE key distance estimator**
   Read RSSI from the active NimBLE connection and expose selectable modes (threshold / log-distance formula / Kalman-filtered). Add `blekey:distance:*` commands and client UI. This repo confirms the connection is purely BLE+VCSEC, so distance can be measured host-side.

2. **VCSEC command parity**
   Cross-check `pkg/vehicle/` command builders against our `teslable:*` and `tesla:*` commands. Missing commands (e.g., specific charge-limit behaviors, vent windows) can be added to the BLE command dispatcher.

3. **Session handshake hardening**
   The official repo implements full ephemeral-key exchange and command signing. Audit our `teslable:auth` flow against the VCSEC spec in `doc/` to ensure we are not skipping required handshake steps.

4. **Protobuf size constraints**
   The official protocol uses length-prefixed protobuf frames. Verify our BLE MTU and protobuf encode/decode boundaries match the reference, especially for larger carserver commands.

## Safety / legal notes

- This is an official Tesla repository under Apache-2.0. It is safe to read and cite, but per project policy do not copy code directly into shipping firmware.
- VCSEC/BLE key management is security-critical. Any change to `teslable:auth`, `tesla:key:*`, or session state must be reviewed against the VCSEC spec.

## Upstream (2026-06-20)

4 new commits on `main` since v0.4.1 (`49977a1`):

- HSM/TEE external integration: `Session` re-export + `ECDHPrivateKey` HSM/TEE requirements docs (`3c31a68`, `92b5dec`).
- Lint + gofmt cleanups in `pkg/protocol/external_impl_test.go` (`6609b57`, `e6e889b`).

No BLE distance changes (the relevant code is in a different subtree). Relevant only if we ever move key material to a hardware enclave — the `Session` / `ECDHPrivateKey` interfaces are the integration points. Full commit table: `docs/legacy/upstream-review-2026-06-20.md`.