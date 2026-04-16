# Phase 02 - Protocol And Runtime Reliability

## Goal

Increase runtime safety and deterministic behavior across protocol, web transport, and mobile transport.

## Planned Work

1. Add strict input guards in `packages/protocol/src/commands.ts`.
2. Extend parser result types to distinguish malformed input from ignorable noise.
3. Add user-visible send/ack timeout handling in web and mobile board state hooks.
4. Improve mobile BLE reconnect policy and surface failures to UI.
5. Add command cooldown/debounce for high-risk vehicle commands.

## Detailed Task Breakdown

- Protocol:
  - Validate range for profile, offset, seat, display, regen.
  - Add typed error or result wrappers where applicable.
- Web:
  - Add transport error channel surfaced in UI components.
  - Add ack timeout message for command flow.
- Mobile:
  - Add reconnect attempt strategy and bounded retry window.
  - Add permission diagnostics messaging for BLE availability.

## Acceptance Criteria

- Out-of-range command values do not get emitted.
- Parse failures are observable and test-covered.
- User gets explicit error state for timeout/disconnect paths.
- BLE disconnect recovery works without app restart.

## Testing

- Unit tests for command guardrails.
- Parser malformed/noisy input tests.
- Hook tests for timeout and disconnect transitions.
- BLE transport reconnect tests with mocks.
