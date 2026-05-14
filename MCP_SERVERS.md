# MCP Servers

Model Context Protocol (MCP) servers extend OpenCode with additional tools for this project.

## Configured Servers

| Server                 | Package                                           | Purpose                                     | Status              |
| ---------------------- | ------------------------------------------------- | ------------------------------------------- | ------------------- |
| **github**             | `@github/github-mcp-server`                       | GitHub operations (issues, PRs, repos)      | Disabled by default |
| **playwright**         | `@playwright/mcp`                                 | Browser automation for testing web client   | Disabled by default |
| **memory**             | `@modelcontextprotocol/server-memory`             | Persistent memory across sessions           | Disabled by default |
| **sequentialthinking** | `@modelcontextprotocol/server-sequentialthinking` | Step-by-step reasoning for complex analysis | Disabled by default |
| **time**               | `@modelcontextprotocol/server-time`               | Time utilities and scheduling               | Disabled by default |
| **superpowers**        | `superpowers-mcp`                                 | General utility enhancements                | Disabled by default |

## Enabling Servers

All MCP servers are **disabled by default** to save context. Enable them in `opencode.json` or per-session:

### Enable in config

```json
{
    "mcp": {
        "github": {
            "enabled": true
        }
    }
}
```

### Enable per-agent

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

### Enable at runtime

```bash
opencode mcp enable github
```

## Environment Variables

| Server | Variable       | Required |
| ------ | -------------- | -------- |
| github | `GITHUB_TOKEN` | Yes      |

Set before starting OpenCode:

```bash
# Windows PowerShell
$env:GITHUB_TOKEN = "ghp_xxxxxxxxxxxx"

# macOS/Linux
export GITHUB_TOKEN="ghp_xxxxxxxxxxxx"
```

## Usage Examples

### GitHub MCP

```
Create a new issue for the missing DLC guard in firmware/lib/vehicle/can/feature/fsd.h
List open pull requests
Get details on issue #123
```

### Playwright MCP

```
Use playwright to open the client dashboard and verify the CAN frame viewer renders correctly
Test the flasher page in the browser
Screenshot the dashboard after connecting
```

### Memory MCP

```
Remember that the user's test board uses a 16MHz crystal on the chassis bus
What did we decide about the Vehicle CAN bus wiring?
Recall the last firmware build environment we used
```

### Sequential Thinking MCP

```
Use sequential thinking to analyze the CAN frame mutation flow from command to TX
Analyze the checksum calculation step by step
Trace the FSD injection path through the handler dispatch
```

### Time MCP

```
What time is it in UTC?
Schedule a reminder for the firmware build
Calculate the timestamp for the CAN frame log
```

### Superpowers MCP

```
Use superpowers to enhance the analysis
```

## Installation

MCP servers are auto-installed via `npx` when first used. No manual installation required.

Requirements:

- Node.js >= 18
- `npx` available in PATH

## Troubleshooting

```bash
# List all MCP servers and their status
opencode mcp list

# Debug a specific server
opencode mcp debug github

# View auth status for OAuth servers
opencode mcp auth list

# Remove stored credentials
opencode mcp logout github
```

### Common Issues

**Server not found:**

```bash
npx -y @github/github-mcp-server --help
# If this fails, check your internet connection and npm registry
```

**Token errors:**

```bash
# Verify token is set
opencode debug env
# Check if GITHUB_TOKEN is in the environment
```

**Context overflow:**

- MCP servers add tools to the context window
- If you hit token limits, disable unused servers
- Use `opencode mcp disable <server>` to free up context

## Per-Agent Control

Disable globally, enable only for specific agents:

```json
{
    "tools": {
        "github*": false,
        "playwright*": false
    },
    "agent": {
        "web-tester": {
            "tools": {
                "playwright*": true
            }
        },
        "issue-manager": {
            "tools": {
                "github*": true
            }
        }
    }
}
```

Glob patterns:

- `github*` matches all tools prefixed with `github_`
- `*` matches all MCP tools
- `?` matches exactly one character
