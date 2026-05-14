---
title: OpenCode Setup
title_tr: OpenCode Kurulumu
description: Complete guide to OpenCode AI agent setup, MCP servers, skills, and GitHub integration for TeslaCANModder
category: guides
folder: guides
tags: [opencode, ai, agents, mcp, skills, github]
order: 20
icon: 🤖
---

# OpenCode Setup Guide

This project uses [OpenCode](https://opencode.ai) for AI-powered development assistance. This guide covers local setup, MCP servers, agent skills, GitHub integration, and usage.

---

## Table of Contents

1. [Quick Start](#quick-start)
2. [Configuration](#configuration)
3. [MCP Servers](#mcp-servers)
4. [Agent Skills](#agent-skills)
5. [Custom Commands](#custom-commands)
6. [GitHub Integration](#github-integration)
7. [Agents](#agents)
8. [LSP & Formatters](#lsp--formatters)
9. [Troubleshooting](#troubleshooting)

---

## Quick Start

### 1. Install OpenCode

Download from [opencode.ai](https://opencode.ai) or install via package manager:

```bash
# macOS (Homebrew)
brew install opencode

# Windows (Winget)
winget install Anomaly.OpenCode

# Or download directly from https://opencode.ai
```

### 2. Get API Key

This project is configured to use **OpenCode Go** ($5 first month, then $10/month):

1. Sign in at [opencode.ai/auth](https://opencode.ai/auth)
2. Subscribe to **OpenCode Go**
3. Copy your API key

### 3. Connect

```bash
# In the TUI, run:
/connect
# Select "OpenCode Go" and paste your API key
```

Or set the environment variable:

```bash
# Windows (PowerShell)
$env:OPENCODE_GO_API_KEY = "your-key-here"

# macOS/Linux
export OPENCODE_GO_API_KEY="your-key-here"
```

### 4. Navigate to Project

```bash
cd path/to/TeslaCANModder
opencode
```

OpenCode will auto-load `opencode.json` and discover skills in `.opencode/skills/`.

---

## Configuration

The project config lives in [`opencode.json`](../../opencode.json) at the repository root.

### Key Settings

| Setting       | Value                      | Description                              |
| ------------- | -------------------------- | ---------------------------------------- |
| `model`       | `opencode-go/kimi-k2.6`    | Main coding model                        |
| `small_model` | `opencode-go/qwen3.5-plus` | Fast tasks (titles, etc.)                |
| `shell`       | `pwsh`                     | Default shell on Windows                 |
| `share`       | `manual`                   | Conversations shared only when requested |

### Instructions

OpenCode reads these files for project context:

- `AGENTS.md` - Project overview and rules
- `CONTRIBUTING.md` - Contribution guidelines
- `docs/reference/commands.md` - Command reference
- `docs/reference/can-protocol.md` - CAN protocol reference

### Provider Setup

```json
{
    "provider": {
        "opencode-go": {
            "options": {
                "apiKey": "{env:OPENCODE_GO_API_KEY}"
            }
        }
    }
}
```

Uses environment variable substitution. The `{env:VAR}` syntax reads from your shell environment.

---

## MCP Servers

[MCP (Model Context Protocol)](https://modelcontextprotocol.io) servers extend OpenCode with additional tools.

### Available Servers

| Server                 | Package                                           | Purpose                   | When to Enable                        |
| ---------------------- | ------------------------------------------------- | ------------------------- | ------------------------------------- |
| **github**             | `@github/github-mcp-server`                       | GitHub issues, PRs, repos | When managing GitHub issues/PRs       |
| **playwright**         | `@playwright/mcp`                                 | Browser automation        | When testing web client UI            |
| **memory**             | `@modelcontextprotocol/server-memory`             | Persistent memory         | When you want context across sessions |
| **sequentialthinking** | `@modelcontextprotocol/server-sequentialthinking` | Step-by-step reasoning    | Complex CAN analysis, debugging       |
| **time**               | `@modelcontextprotocol/server-time`               | Time utilities            | Scheduling, timestamps                |
| **superpowers**        | `superpowers-mcp`                                 | General utilities         | Enhanced capabilities                 |

All servers are **disabled by default** to save context tokens.

### Enable a Server

**In TUI:**

```
/opencode mcp enable github
```

**In config (`opencode.json`):**

```json
{
    "mcp": {
        "github": {
            "enabled": true
        }
    }
}
```

**Per-agent:**

```json
{
    "tools": {
        "github*": false
    },
    "agent": {
        "my-agent": {
            "tools": {
                "github*": true
            }
        }
    }
}
```

### Environment Variables

| Server | Variable       | Required |
| ------ | -------------- | -------- |
| github | `GITHUB_TOKEN` | Yes      |

Set before starting OpenCode:

```bash
$env:GITHUB_TOKEN = "ghp_xxxxxxxxxxxx"
```

### Usage Examples

**GitHub MCP:**

```
Create an issue for the missing DLC guard in fsd.h
List open pull requests
```

**Playwright MCP:**

```
Open the client dashboard and screenshot the CAN frame viewer
Test the flasher page in the browser
```

**Memory MCP:**

```
Remember my test board uses 16MHz crystal
What did we decide about the Vehicle CAN bus wiring?
```

**Sequential Thinking MCP:**

```
Use sequential thinking to trace the FSD command from UI to CAN TX
Analyze the checksum calculation flow step by step
```

### Troubleshooting MCP

```bash
# List all MCP servers and status
opencode mcp list

# Debug a specific server
opencode mcp debug github

# View auth status
opencode mcp auth list

# Remove stored credentials
opencode mcp logout github
```

---

## Agent Skills

Skills are reusable instruction sets stored in `.opencode/skills/<name>/SKILL.md`. OpenCode auto-discovers them and loads them on-demand.

### Available Skills

| Skill                  | Description                                               | When to Load                    |
| ---------------------- | --------------------------------------------------------- | ------------------------------- |
| `firmware-development` | PlatformIO ESP32, build envs, CAN buses, feature patterns | Working in `firmware/`          |
| `can-protocol-safety`  | DLC guards, checksums, variant behavior, bus routing      | Changing CAN frame logic        |
| `client-development`   | Expo React Native, state mgmt, hardware connections       | Working in `client/`            |
| `protocol-package`     | TypeScript ESM, commands, decoder data                    | Working in `packages/protocol/` |
| `tools-development`    | Debug CLI, serial bridge, validation scripts              | Working in `tools/`             |
| `testing-validation`   | Test matrix, CI checks, validation workflow               | Running tests or QA             |
| `release-qa`           | Release checklist, version alignment, rollback            | Preparing releases              |

### Using Skills

OpenCode lists available skills in the `skill` tool. The agent can load them automatically or you can prompt it:

```
Use the firmware-development skill to help me add a new CAN feature handler
```

Or let the agent detect and load the right skill based on what you're working on.

### Skill Permissions

All skills are allowed globally. You can restrict per-agent in `opencode.json`:

```json
{
    "agent": {
        "github-reviewer": {
            "permission": {
                "skill": {
                    "can-protocol-safety": "allow",
                    "firmware-development": "allow",
                    "release-qa": "deny"
                }
            }
        }
    }
}
```

---

## Custom Commands

Slash commands are defined in `opencode.json` under the `command` key.

### Available Commands

| Command           | Description                                                                 |
| ----------------- | --------------------------------------------------------------------------- |
| `/test`           | Run full test suite (`npm run test:all`)                                    |
| `/lint`           | Run lint and format checks (`npm run lint:all`)                             |
| `/build-firmware` | Build full firmware (`pio run -e esp32_wifi_ble_chassis_vehicle_body_8mhz`) |
| `/test-firmware`  | Run firmware native tests (`pio test -e native`)                            |

### Usage

In the TUI:

```
/test
```

Or create your own:

```json
{
    "command": {
        "flash": {
            "template": "Flash the firmware to COM4 using `cd firmware && .\\pio.ps1 run -e esp32_chassis_8mhz -t upload`. Report success or errors.",
            "description": "Flash firmware to device"
        }
    }
}
```

---

## GitHub Integration

OpenCode integrates with GitHub via the [OpenCode GitHub App](https://github.com/apps/opencode-agent) and GitHub Actions workflows.

### Workflows

| Workflow              | Trigger                          | Purpose                           |
| --------------------- | -------------------------------- | --------------------------------- |
| `opencode.yml`        | `/oc` or `/opencode` in comments | Fix issues, implement features    |
| `opencode-review.yml` | PR opened/updated                | Automated code review             |
| `opencode-triage.yml` | New issue opened                 | Categorize and route issues       |
| `opencode-audit.yml`  | Mondays 6am UTC                  | Weekly security/code health audit |

### Setup

1. **Install the app**: [github.com/apps/opencode-agent](https://github.com/apps/opencode-agent)
2. **Add secret**: `OPENCODE_GO_API_KEY` in **Settings → Secrets → Actions**
3. (Optional) Add `GITHUB_TOKEN` for enhanced GitHub operations

### Usage in GitHub

**Issues:**

```
/oc explain this issue
/oc fix this
```

**PR Comments:**

```
/opencode review the CAN mutation logic
/oc check if DLC guards are present
```

**Code Lines (Files tab):**

```
/oc add error handling here
```

### GitHub Model

The GitHub Action uses `opencode-go/kimi-k2.6` from your OpenCode Go subscription. This is cheaper than using Anthropic directly for automated reviews.

---

## Agents

### Built-in Agents

| Agent   | Purpose                   | Tools       |
| ------- | ------------------------- | ----------- |
| `code`  | General coding tasks      | Full access |
| `plan`  | Planning and architecture | Full access |
| `build` | Build and compilation     | Full access |

### Custom Agents

#### `github-reviewer`

Read-only PR reviewer focused on CAN safety:

```json
{
    "agent": {
        "github-reviewer": {
            "description": "Automated PR reviewer for TeslaCANModder",
            "model": "opencode-go/kimi-k2.6",
            "prompt": "You are a TeslaCANModder code reviewer. Focus on: CAN frame safety...",
            "tools": {
                "write": false,
                "edit": false
            }
        }
    }
}
```

### Creating Custom Agents

Add markdown files to `.opencode/agents/`:

```markdown
---
name: can-expert
description: CAN protocol expert for TeslaCANModder
model: opencode-go/kimi-k2.6
---

## What I do

Specialize in Tesla CAN bus protocol analysis, frame decoding, and mutation logic.

## When to use me

- Analyzing CAN frame structures
- Debugging bus routing issues
- Reviewing checksum calculations
```

### Agent Permissions

Control tool access per-agent:

```json
{
    "agent": {
        "safe-mode": {
            "tools": {
                "bash": false,
                "write": false
            }
        }
    }
}
```

---

## LSP & Formatters

### LSP Servers

| Server     | Files                | Purpose                          |
| ---------- | -------------------- | -------------------------------- |
| TypeScript | `.ts`, `.tsx`        | Type checking, IntelliSense      |
| ESLint     | `.ts`, `.tsx`, `.js` | Lint diagnostics                 |
| clangd     | `.cpp`, `.h`, `.c`   | C++ code navigation, diagnostics |
| Python     | `.py`                | Python support                   |
| YAML       | `.yml`, `.yaml`      | YAML validation                  |
| Bash       | `.sh`, `.ps1`        | Shell script support             |

### Formatters

| Formatter    | Files              | Command                |
| ------------ | ------------------ | ---------------------- |
| Prettier     | JS/TS/JSON/MD/YAML | `npx prettier --write` |
| clang-format | C/C++              | `clang-format -i`      |

Format on save is supported via the TUI. Run `opencode` and use the formatter keybind.

---

## Troubleshooting

### OpenCode doesn't load project config

```bash
# Verify config is valid
opencode debug config

# Check if skills are discovered
opencode debug skills
```

### MCP server fails to start

```bash
# Check Node.js version (>= 18 required)
node --version

# Debug the server
opencode mcp debug <server-name>

# Check if npx works
npx -y @github/github-mcp-server --help
```

### Skills not showing up

- Verify `SKILL.md` is in all caps
- Check frontmatter has `name` and `description`
- Ensure skill name matches directory name exactly
- Run `opencode debug skills` to see discovered skills

### Model not responding

```bash
# Check provider connection
opencode debug provider

# Verify API key is set
echo $env:OPENCODE_GO_API_KEY  # PowerShell
echo $OPENCODE_GO_API_KEY      # bash
```

### Context too long

If you hit token limits:

- Disable unused MCP servers
- Use `compaction` settings (already configured with `reserved: 15000`)
- Break tasks into smaller chunks

---

## Resources

- [OpenCode Docs](https://opencode.ai/docs)
- [OpenCode Go Models](https://opencode.ai/docs/go/)
- [Agent Skills](https://opencode.ai/docs/skills/)
- [MCP Servers](https://opencode.ai/docs/mcp-servers/)
- [GitHub Integration](https://opencode.ai/docs/github/)
- [Config Schema](https://opencode.ai/config.json)
- [Project Config (`opencode.json`)](../../opencode.json)
- [MCP Reference (`MCP_SERVERS.md`)](../../MCP_SERVERS.md)
