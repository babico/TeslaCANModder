---
title: mveplus/tesla-model3-resources
category: legacy
folder: legacy
tags: [legacy, tesla, model3, resources, reference]
---

# mveplus/tesla-model3-resources

| Field | Value |
| ----- | ----- |
| Submodule | `legacy/mveplus-tesla-model3-resources` |
| URL | https://github.com/mveplus/tesla-model3-resources |
| Author | mveplus |
| License | Unspecified; treat as reference-only |
| Language | Markdown / curated links |
| Primary focus | Curated list of Tesla Model 3 resources for owners and infrastructure engineers |

## Overview

A high-level index of official Tesla resources and community tools: data loggers (TeslaMate, TeslaLogger), Home Assistant integrations, Prometheus/Grafana exporters, Docker deployment patterns, Fleet API auth guides, and SDKs in Python/Go/Node.js. It is **not** a CAN database or signal reference; it does not contain DBC files, raw logs, or bit-level frame definitions.

## Relevant links inside the repo

| Link / tool | Why it matters |
| ----------- | -------------- |
| TeslaMate | Self-hosted data logger with MQTT bridge; relevant to our `mqtt:*` and `smt:*` features |
| Tesla Fleet Telemetry | Official streaming telemetry server; informs how telemetry leaves the vehicle |
| Tesla Prometheus Exporter | Metrics export pattern; could inspire our MQTT telemetry payload shape |
| Tesla Custom Component for Home Assistant | HA discovery conventions; align with `ha:*` commands |
| Tesla Vehicle Command | Already a separate submodule; this repo links to it |

## What it does **not** contain

- Raw Model 3 CAN captures
- DBC or JSON signal definitions
- Bit-position tables for specific CAN IDs
- Code samples for frame decoding

Because of this, its direct value to firmware CAN development is low. It is most useful as a pointer to ecosystem tooling and as a cross-check that we are covering the major third-party integration surfaces (MQTT, HA, Fleet API).

## Potential improvements for TeslaCANModder

1. **MQTT payload alignment**
   TeslaMate publishes a well-known MQTT topic tree (`teslamate/cars/$id/$field`). Compare our `mqtt:on` JSON snapshot schema against TeslaMate topics. Where the same signal exists, consider publishing a flattened MQTT topic in addition to JSON to ease HA ingestion.

2. **Home Assistant discovery**
   The linked HA integration registers entities via MQTT discovery. We could add `ha:discovery` as an alias/reimplementation of our existing `ha:discovery` that emits the standard HA discovery topics. Verify our current discovery payload matches HA 2024+ format.

3. **Fleet Telemetry bridge**
   `legacy/teslamotors-vehicle-command` already covers VCSEC/BLE; this repo points to Fleet Telemetry, which is the server-side counterpart. A future `fleet-telemetry:*` command family could stream selected CAN-like signals to a self-hosted telemetry server, complementing our current `mqtt:*` bridge.

4. **Documentation cross-links**
   Add links from our `docs/guides/getting-started.md` and `docs/guides/hardware-setup.md` to the authentication-flow diagrams in this repo for users setting up Fleet API or TeslaMate alongside the CAN mod.

## Safety / legal notes

- Curated links only. No code to review or port.
- Fleet API and TeslaMate both require accepting Tesla's Terms of Service. Keep any integration optional and clearly documented.
