---
title: Documentation
description: Tesla CAN Modder documentation index for guides, reference, architecture, checklists, troubleshooting, and legacy research
category: index
folder: root
tags: [index, overview, navigation]
order: 0
---

# Tesla CAN Modder Documentation

This folder is the canonical documentation source for the repo and the client docs screen. The app now renders these raw markdown files directly, so there is no separate generated docs bundle to keep in sync.

## Documentation Map

```mermaid
flowchart TD
    Start([Start here]) --> Q1{What do you need?}
    Q1 -->|Set up the device| G["guides/<br/>getting-started, full-setup,<br/>hardware-setup, flasher, adapters"]
    Q1 -->|Send a command| R["reference/<br/>commands, can-ids,<br/>can-protocol, state-fields, signal-matrix"]
    Q1 -->|Understand the app| A["architecture/<br/>unified-client-guide,<br/>monitor-architecture, layout-system, feature-workflows"]
    Q1 -->|Cut a release| C["checklists/<br/>release-checklist,<br/>can-review-checklist, testing-plan"]
    Q1 -->|Debug a problem| T["troubleshooting/<br/>debug-guide"]
    Q1 -->|Research the ecosystem| L["legacy/<br/>80+ reference repos,<br/>COMPARISON, upstream-review"]
    G --> Build["Build → Flash → Drive"]
    R --> Build
    A --> Build
    classDef bucket fill:#1e3a5f,stroke:#4a7fb5,color:#fff
    class G,R,A,C,T,L bucket
```

## Start Here

- [Getting Started](guides/getting-started.md)
- [Full Setup Guide](guides/full-setup.md)
- [Command Reference](reference/commands.md)
- [Unified Client Guide](architecture/unified-client-guide.md)
- [Release Checklist](checklists/release-checklist.md)

## Route Map

| Route                                                   | Start Page                                                   | Use It For                                                              |
| ------------------------------------------------------- | ------------------------------------------------------------ | ----------------------------------------------------------------------- |
| [`guides/`](guides/getting-started.md)                  | [Getting Started](guides/getting-started.md)                 | setup, flashing, hardware wiring, adapters, and first install flow      |
| [`reference/`](reference/commands.md)                   | [Command Reference](reference/commands.md)                   | commands, CAN IDs, state fields, APIs, protocol notes, and signal data  |
| [`architecture/`](architecture/unified-client-guide.md) | [Unified Client Guide](architecture/unified-client-guide.md) | client structure, monitor workflows, layout rules, and design decisions |
| [`checklists/`](checklists/release-checklist.md)        | [Release Checklist](checklists/release-checklist.md)         | release gates, QA, regression checks, and validation plans              |
| [`troubleshooting/`](troubleshooting/debug-guide.md)    | [Debug Guide](troubleshooting/debug-guide.md)                | runtime debugging, issue isolation, and integration troubleshooting     |
| [`legacy/`](legacy/README.md)                           | [Legacy Index](legacy/README.md)                             | community project archive and comparison material                       |

## Quick Routes

- First install: [Getting Started](guides/getting-started.md), [Quickstart Checklist](guides/quickstart-checklist.md), [Hardware Setup](guides/hardware-setup.md), [Flasher Quickstart](guides/flasher-quickstart.md)
- Connect and validate: [USB and Bluetooth Adapters](guides/adapters.md), [Full Setup Guide](guides/full-setup.md), [Security Model](guides/security.md)
- Firmware and protocol work: [Command Reference](reference/commands.md), [State Fields Reference](reference/state-fields.md), [CAN Protocol](reference/can-protocol.md), [Signal Matrix](reference/signal-matrix.md)
- Client and UX work: [Unified Client Guide](architecture/unified-client-guide.md), [Monitor Architecture](architecture/monitor-architecture.md), [Feature Workflows](architecture/feature-workflows.md), [Layout System](architecture/layout-system.md)
- Release and QA: [Quickstart Checklist](guides/quickstart-checklist.md), [E2E Testing Plan](checklists/testing-plan.md), [Release Checklist](checklists/release-checklist.md)
- Research and comparisons: [Legacy Index](legacy/README.md), [Legacy Comparison](legacy/COMPARISON.md)
