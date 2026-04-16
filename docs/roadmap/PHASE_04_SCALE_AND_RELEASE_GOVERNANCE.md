# Phase 04 - Scale And Release Governance

## Goal

Raise release confidence by adding integration checks, security gates, and quality governance.

## Planned Work

1. Add integration/E2E smoke suite for core workflows.
2. Add dependency/security scanning to CI.
3. Add firmware size/memory regression guardrails.
4. Add release checklist and quality gate policy docs.
5. Add canary verification path before stable tag release.

## Acceptance Criteria

- E2E smoke runs in CI on PRs and main.
- Security audit outputs are visible and enforced.
- Firmware resource regressions fail CI at defined thresholds.
- Release artifacts require checklist completion.

## Governance Checklist

- Protocol compatibility matrix updated.
- Firmware/web/mobile version alignment checked.
- Migration notes added for breaking changes.
- Rollback instructions verified.
