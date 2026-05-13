# OpenCode Project Configuration

This directory contains OpenCode-specific configuration for the TeslaCANModder project.

## Structure

```
.opencode/
├── skills/                    # Agent skills (reusable instruction sets)
│   ├── firmware-development/  # PlatformIO ESP32 firmware
│   ├── can-protocol-safety/   # CAN frame mutation safety
│   ├── client-development/    # Expo React Native client
│   ├── protocol-package/      # Shared TypeScript protocol
│   ├── tools-development/     # Debug CLI and bridge
│   ├── testing-validation/    # Test matrix and CI
│   └── release-qa/            # Release quality gates
└── specs/                     # Specification documents (if needed)
```

## Skills

Each skill is a directory containing a `SKILL.md` file with YAML frontmatter:

```markdown
---
name: skill-name
description: What this skill does
---

## What I do

...

## When to use me

...
```

Skills are auto-discovered by OpenCode and loaded on-demand via the `skill` tool.

## Usage

1. OpenCode walks up from your cwd to find `.opencode/skills/`
2. Available skills appear in the `skill` tool description
3. The agent loads a skill by calling `skill({ name: "firmware-development" })`
4. Skill instructions are injected into the agent context

## Global Skills

You can also place skills in `~/.config/opencode/skills/` for cross-project reuse.

## See Also

- [OpenCode Skills Docs](https://opencode.ai/docs/skills/)
- [Project Setup Guide](../docs/guides/opencode-setup.md)
- [Root Config](../opencode.json)
