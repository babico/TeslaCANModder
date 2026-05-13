# OpenCode GitHub Integration

This repository uses [OpenCode](https://opencode.ai) for AI-powered GitHub automation.

## Workflows

| Workflow         | File                  | Trigger                          | Purpose                                          |
| ---------------- | --------------------- | -------------------------------- | ------------------------------------------------ |
| **Interactive**  | `opencode.yml`        | `/oc` or `/opencode` in comments | Fix issues, implement features, answer questions |
| **PR Review**    | `opencode-review.yml` | PR opened/updated                | Automated code review with CAN safety focus      |
| **Issue Triage** | `opencode-triage.yml` | New issue opened                 | Categorize and route issues                      |
| **Weekly Audit** | `opencode-audit.yml`  | Mondays 6am UTC                  | Security, code health, CAN safety audit          |

## Usage

### In Issues

```
/oc explain this issue
/oc fix this
/oc how do I enable the Vehicle CAN bus?
```

### In PR Comments

```
/opencode review the CAN frame mutation logic
/oc check if DLC guards are present
```

### On Code Lines (Files tab)

```
/oc add error handling here
```

## Setup Requirements

1. **Install the OpenCode GitHub App**: [github.com/apps/opencode-agent](https://github.com/apps/opencode-agent)
2. **Add API key secret**: `ANTHROPIC_API_KEY` in repository secrets
3. **Optional**: Add `GITHUB_TOKEN` for operations as the action runner

## Safety Notes

- OpenCode runs inside GitHub Actions runners (secure)
- The PR reviewer agent is read-only (`write: false`, `edit: false`)
- The interactive agent can create branches and PRs when triggered by `/oc fix`
- All changes require human review before merging

## Agents

| Agent             | Purpose                   | Tools       |
| ----------------- | ------------------------- | ----------- |
| `code`            | General coding tasks      | Full access |
| `plan`            | Planning and architecture | Full access |
| `github-reviewer` | PR review only            | Read-only   |

## Custom Prompts

Each workflow includes project-specific prompts that instruct OpenCode about:

- TeslaCANModder project structure (firmware/client/protocol/tools)
- CAN safety requirements (DLC guards, checksums, variant behavior)
- Coding standards (.clang-format, eslint.config.mjs)
- Required checklists (can-review, release, testing)
