# Task C1-02 report — BLE add-key rebuild

Status: **DONE_WITH_CONCERNS**

Branch: `chore/legacy-tesla-drug-fork-submodules`
Commit: `0767463 fix(ble): rebuild add-key with nested WhitelistOperation path`
Pushed: `c2f6eda..0767463 HEAD -> chore/legacy-tesla-drug-fork-submodules`

## Implemented

Rebuilt `buildAddKeyRequest` in `firmware/lib/vehicle/ble/msg.h` to emit the
SDK nested path instead of a flat `UnsignedMessage.field 2`:

```
UnsignedMessage.WhitelistOperation (f16)
  -> WhitelistOperation.addKeyToWhitelistAndAddPermissions (f5)
       -> PermissionChange.key (f1) = PublicKey.PublicKeyRaw (f1, 65 raw bytes)
          PermissionChange.keyRole (f4)
          PermissionChange.metadataForKey (f6) = KeyMetadata.keyFormFactor (f1)
```

Wire grows from 98 -> 105 bytes for the pinned inputs. Outer RoutableMessage
tags (6/10/51, C1-01) untouched. `Proto` helpers (`fieldMsg`, varint tags)
handle the 2-byte tag for field 16 without new helpers.

In the same commit, flipped `test_add_key_exact_wire` from its CHARACTERIZATION
baseline (98 bytes, flat field 2) to the recorded SDK vector (105 bytes) and
updated the companion `test_add_key_tags` pin, which also asserted the old flat
structure. No other CHARACTERIZATION tests were changed (C1-15..C1-18 remain).

## TDD RED / GREEN evidence

Commands run from `firmware/` with `.pio.ps1`.

RED (tests updated first, production unchanged):

```
.\pio.ps1 test -e native -f test_native_ble_proto
...
test\test_native_ble_proto\test_ble_proto.cpp:240: test_add_key_tags: Expected 80 Was 73   [FAILED]
test\test_native_ble_proto\test_ble_proto.cpp:357: test_add_key_exact_wire: Expected 105 Was 98 [FAILED]
...
============ 19 test cases: 2 failed, 16 succeeded in 00:00:01.937 ============
```

GREEN (after production fix):

```
.\pio.ps1 test -e native -f test_native_ble_proto
...
test\test_native_ble_proto\test_add_key_tags           [PASSED]
test\test_native_ble_proto\test_add_key_exact_wire     [PASSED]
...
================= 18 test cases: 18 succeeded in 00:00:01.324 =================
```

Full native suite (once, before commit):

```
.\pio.ps1 test -e native
...
native         test_native_ble_proto          PASSED    00:00:00.801
=============== 1046 test cases: 1046 succeeded in 00:01:17.987 ===============
```

## Files changed

- `firmware/lib/vehicle/ble/msg.h` — production nested-path rebuild.
- `firmware/test/test_native_ble_proto/test_ble_proto.cpp` — SDK vector pin +
  shared `SDK_KEY_FORM_FACTOR_NFC_CARD = 1` test input constant.

Staging: `git add -- <two paths>` only; `git diff --cached --name-only` showed
exactly those two files. The 191 pre-existing unstaged files were not touched.

## Self-review

- Only `buildAddKeyRequest` changed in production; no role constants, no other
  wire builders, no `RoutableMessage` tags (C1-01) modified.
- Local stack buffers sized against worst case: publicKey 67 B / 80, keyMeta
  2 B / 8, permission 75 B / 96, whitelist 77 B / 112, UnsignedMessage 80 B / 128. Every `Proto::ok()` still checked; failure path unchanged.
- Exact-wire pin asserts both total length (105) and every byte, derived
  independently with literal arrays (no production encoder in the assertion).
- `test_add_key_tags` regression checks (old top-level fields 2/5/14 absent)
  still valid: field 5 now exists only nested inside the UnsignedMessage.
- No code copied from `legacy/`; field numbers read from read-only protos only.

## Concerns

1. **UNVERIFIED — bench capture.** Acceptance also asks for a bench capture of
   an accepted add-key. No vehicle/BLE hardware here; this is UNVERIFIED, not a
   blocker. The in-repo exact-byte test is the only proof produced.
2. **C1-03 dependency — value constants.** This task pins `keyRole = 4`
   (existing `role`) and SDK `keyFormFactor = 1` in the test. Production still
   defines `ROLE_OWNER=4`, `ROLE_CHARGING_MANAGER=5` (key.h) and
   `KEY_FORM_FACTOR_NFC_CARD=3` (msg.h), and `dispatch.h:204` passes that 3.
   Therefore real provisioning still encodes form factor 3 until C1-03 lands;
   C1-03 changing `ROLE_OWNER` to 2 will also require re-pinning the role byte
   in `test_add_key_exact_wire` / `test_add_key_tags`. Constants were left
   untouched per brief. The test uses a named SDK-value constant rather than
   the production constant specifically so C1-03's scope stays clean.
3. **Recorded SDK vector places `metadataForKey` (f6) inside the f5 submessage.**
   The read-only `vcsec.proto` defines `metadataForKey` as field 6 of
   `WhitelistOperation` (sibling of the `addKey...` oneof), not a field of
   `PermissionChange`. The C1-14 recorded vector nests it inside f5; this task
   implements that exact byte sequence as instructed. Worth confirming against
   a bench capture or the SDK encoder before treating this sub-field placement
   as final. Byte lengths are identical either way (77/75/67), so only the
   placement of the 4-byte `0x32 0x02 0x08 0x01` run differs.
4. RED output showed `Program received signal SIGINT (Interrupt)` / suite
   ERRORED alongside the two genuine assertion failures; GREEN runs are clean.
   Appears to be the Unity runner's behavior on failure, not a code defect.
