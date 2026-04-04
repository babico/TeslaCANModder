# CAN Control Review Checklist

Use this checklist whenever a change touches TeslaCANModder code that can alter CAN frame behavior.

Primary review scope:

- `hardware/lib/can/*`
- `hardware/lib/drivers/*`
- `hardware/lib/handlers/*`
- `hardware/lib/packages/fsd/*`
- `hardware/lib/board/bridge.h`

This checklist is meant for review and regression discipline. It is not a generic style guide.

## 1. Frame mutation safety

- Every direct `frame.data[...]` read has a matching DLC guard before access.
- Every direct `frame.data[...]` write has a matching DLC guard before mutation.
- Shared helpers still reject invalid bit indexes and writes outside the frame DLC.
- Bit writes only touch the intended field and do not leak into adjacent bits.
- Existing checksum logic is recalculated when a changed byte requires it.

## 2. Bit and field ownership

- The exact bit numbers changed by the patch are documented in the PR or commit notes.
- Changed bits were cross-checked against the active TeslaCANModder behavior, not only an old legacy sketch.
- If legacy references disagree, the reviewer notes which source is canonical for the active repo.
- Mux-specific behavior is preserved: a mux-only mutation must not leak into other mux paths.
- RX and TX ownership is still clear: the code only transmits frames on intended paths.

## 3. Variant behavior review

- `legacy`, `hw3`, and `hw4` were all considered, even if only one variant changed.
- Unsupported controls still remain unsupported on the wrong variant.
- `hw3` speed offset changes still preserve profile and nag behavior.
- `hw4` ISA speed-chime changes still preserve mux and checksum behavior.
- Legacy behavior was not changed accidentally by reusing HW3/HW4 helper logic.

## 4. Stream and board protocol review

- Any change to transmitted frames was checked against `hardware/lib/board/bridge.h`.
- If frame message shape changed, the web protocol normalizer was reviewed at the same time.
- `boot` and `status` still describe capability changes accurately.
- Stream output still preserves `dir`, `id`, `dlc`, `d`, and any required metadata fields.

## 5. Required regression tests

At least one of these must be added or updated when CAN-control behavior changes:

- exact bit-set / bit-clear assertion for the changed field
- short-frame regression test proving no send happens when DLC is too small
- variant-specific regression test proving unsupported variants do not drift
- stream/message-shape assertion when board output changes

Recommended existing suites:

- `hardware/test/test_native_helpers`
- `hardware/test/test_native_hw3`
- `hardware/test/test_native_hw4`
- `hardware/test/test_native_legacy`
- `hardware/test/test_native_bridge`

## 6. Manual reviewer questions

- Does this patch change which CAN IDs are intercepted or transmitted?
- Does this patch change any bit position, checksum, mux rule, or profile mapping?
- Does this patch add any path that can transmit when the feature is disabled?
- Does this patch change the browser-visible meaning of streamed frames or status?
- If this behavior came from a legacy sketch, is the old sketch actually the correct reference for the active repo?

## 7. High-risk change types

Treat these as review-sensitive even if the diff looks small:

- helper changes in `hardware/lib/can/helpers.h`
- mux handling changes in FSD packages
- checksum changes
- handler filter ID changes
- stream message shape changes
- command/router changes that affect CAN mutation state

## 8. Minimum acceptance bar

A CAN-control change is not complete until:

- the intended bit-level behavior is asserted in tests
- short-frame behavior is asserted where relevant
- the active variant behavior is still explicit
- the reviewer can explain exactly which CAN IDs and bits changed
