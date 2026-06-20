---
title: wimaha-TeslaBleHttpProxy
description: A Go-based HTTP-to-BLE proxy that receives HTTP REST commands and forwards them over Bluetooth Low Energy to a Tesla veh
category: legacy
folder: legacy
tags: [legacy, community, external]
author: wimaha
repo: TeslaBleHttpProxy
---

# wimaha-TeslaBleHttpProxy

## Overview

A Go-based HTTP-to-BLE proxy that receives HTTP REST commands and forwards them over Bluetooth Low Energy to a Tesla vehicle. Designed primarily for integration with the evcc home energy management system, it provides a Fleet API–compatible interface for vehicle commands (wake, charge start/stop, set amps, lock/unlock, etc.) and vehicle data queries.

## Architecture

```mermaid
flowchart LR
    Evcc["evcc<br/>(home energy mgmt)"] -->|HTTP REST| Proxy["Go HTTP-to-BLE proxy"]
    Proxy -->|BLE (D-Bus)| Tesla["Tesla vehicle"]
    Proxy --> Fleet["Fleet API-compatible<br/>surface (wake, charge,<br/>set amps, lock, …)"]
    classDef path fill:#1e3a5f,stroke:#4a7fb5,color:#fff
    class Proxy,Fleet path
```

## Technical Details

- **Platform**: Linux (requires D-Bus for BLE); Docker or bare metal
- **Language**: Go
- **CAN Interface**: N/A (BLE-based, not CAN)
- **License**: Apache License 2.0

## Architecture

- `main.go` — Entry point. Initializes logging, config, BLE control, migrates legacy keys, sets up HTTP routes, and starts the HTTP server.
- `internal/api/routes/` — HTTP route definitions mirroring Tesla Fleet API endpoints.
- `internal/api/handlers/` — HTTP handler implementations (HTML pages, logs, Tesla commands, version).
- `internal/api/models/` — Request/response model types and state conversion utilities.
- `internal/ble/control/` — BLE connection management, key role handling (Owner vs Charging Manager), and command queue.
- `internal/tesla/` — Tesla vehicle protocol implementation.
- `internal/logging/` — Structured logging.
- `config/` — Configuration and environment variable handling.
- `html/` / `static/` — Embedded web dashboard for key generation, VIN setup, and role management.
- `docker-compose.yml` / `Dockerfile` — Docker deployment with privileged BLE access.
- `Makefile` — Build automation.

Key design: requests are queued and processed sequentially to ensure only one BLE connection to the vehicle at a time. Supports two key roles — Owner (full access) and Charging Manager (limited to charging commands, recommended for security).

## CAN Bus Integration

No direct CAN integration. This project communicates with the Tesla vehicle over Bluetooth Low Energy (BLE), not CAN bus. It provides an HTTP API that mimics the Tesla Fleet API for remote command execution.

## Relevance to Our Project

Low direct relevance since our project operates at the CAN bus level, not BLE. However, the Fleet API–compatible interface design and the security model (key roles, NFC card pairing) could inform a future remote-control layer on top of our CAN firmware.

- **Reusability**: Low
- **Key Takeaways**:
  - Fleet API endpoint structure is well-documented and could be replicated for our web interface's API design
  - Two-role key model (Owner vs Charging Manager) is a good security pattern
  - BLE command queuing (one connection at a time) is a useful concurrency pattern
  - Docker deployment with D-Bus passthrough for BLE is a reference for containerized vehicle tool deployment
