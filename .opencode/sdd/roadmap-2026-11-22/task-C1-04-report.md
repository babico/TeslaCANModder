# Task C1-04 Report — BLE scan filter

Status: COMPLETE. Real-vehicle acceptance: **UNVERIFIED** (no vehicle advert capture exists in this repo).

## SHA-1 availability (asked in brief "Before You Begin")

SHA-1 is **not trivially available in the native test environment**: `[env:native]` sets
`BOARD_ENABLE_BLE=0`, links no mbedTLS, and still sets `lib_ldf_mode = off`. mbedTLS headers/
symbols exist only in the ESP32 SDK (`framework-arduinoespressif32/.../mbedtls/sha1.h`,
`MBEDTLS_SHA1_C` defined, `mbedtls_sha1_ret` declared).

Smallest safe option chosen (no hand-rolled crypto in shipping code):

1. Device derives the digest with one line of `mbedtls_sha1_ret` in `dispatch.h`.
2. The VIN -> Local Name formatting lives in a new dependency-free header with an
   injected `Sha1Fn`, so the **exact production derivation** runs natively with an
   independent test-only SHA-1 reference. No mbedTLS in the native test target.
3. Expected values are literal, precomputed outside the firmware build.

## Change

| File                                                                     | Change                                                                                                                                                                                                                                                                                                                                                       |
| ------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `firmware/lib/vehicle/ble/local_name.h` (new)                            | `teslaLocalName(vin, len, out, sha1)` -> `S` + uppercase(sha1(VIN)[0..3] as 8 hex chars) + `C`, NUL-terminated (11-byte out). Defensive null/empty checks.                                                                                                                                                                                                   |
| `firmware/lib/vehicle/ble/handler/dispatch.h`                            | `connectToVehicle` no longer builds a last-8-VIN suffix; hashes the stored VIN via `mbedtls_sha1_ret`, derives the Local Name, passes it as scan filter. Hashes fail closed. VIN threshold changed `>= 8` to `>= 5` to match `tesla:vin` storage validation (5-17); short/missing VIN still yields `nullptr` filter = accept any Tesla (previous semantics). |
| `firmware/lib/io/ble/esp32/ble_client.h`                                 | Param/comment rename `vinSuffix` -> `localName`; matching logic (`endsWith`) unchanged.                                                                                                                                                                                                                                                                      |
| `firmware/test/test_native_ble_local_name/test_ble_local_name.cpp` (new) | 4 Unity tests; independent test-only SHA-1 + two literal vectors.                                                                                                                                                                                                                                                                                            |

C1-01..03 BLE wire tags, add-key request, and role handling were not touched. No `legacy/` code copied.

## Literal vectors (independent sources)

Computed with Python 3 `hashlib.sha1(vin.encode()).hexdigest().upper()`, cross-checked with
Node `crypto.createHash('sha1').update(vin).digest('hex')`. Neither tool uses firmware code.

| VIN                 | SHA-1                                      | Expected Local Name |
| ------------------- | ------------------------------------------ | ------------------- |
| `5YJ3E1EA5KF000123` | `4A0C1D370DE25321D9B962DD5133FDE4D48FC5CB` | `S4A0C1D37C`        |
| `5YJYGDEE5MF000456` | `0DB5F043C212B9A7257B37224E16440BC9AD46E8` | `S0DB5F043C`        |

Tests: literal digests vs independent reference; production derivation vs literal Local Names;
length/prefix/suffix/uppercase pins; regression that the filter is not the old last-8-VIN suffix;
fail-closed on null/empty inputs.

## TDD evidence

- RED: focused run before production header existed ->
  `fatal error: vehicle/ble/local_name.h: No such file or directory`, test ERRORED.
- (Intermediate RED during implementation: formatter initially consumed 8 digest bytes = 16 hex
  chars and overflowed the 11-byte buffer; native crash made it visible. Fixed to 4 digest bytes.)
- GREEN: focused run 4/4 PASSED.

## Verification

- `pio.ps1 test -e native -f test_native_ble_local_name`: 4/4 PASSED.
- `pio.ps1 test -e native`: **1053/1053 PASSED** (includes new suite).
- `pio.ps1 run -e esp32_ble_chassis_8mhz`: **SUCCESS** — verifies `mbedtls_sha1_ret` and
  changed `ble_client.h` compile/link on device (RAM 22.1%, Flash 56.2%).
- `clang-format` not installed in this environment; C++ style matched manually to
  `.clang-format` (tabs width 4, Allman braces, 120 cols). No JS/TS/MD changed.

## Concerns / limitations

- **Real-vehicle acceptance UNVERIFIED.** The formula and VIN input are community summary
  evidence only; no advert capture exists here, so `endsWith(localName)` against a real Tesla
  advert has not been confirmed on hardware.
- NimBLE scan path in `ble_client.h` cannot be exercised natively; its behavior change is a
  parameter rename/comment only, so matching logic is unchanged.
- Only one VIN-hash implementation is exercised on device (mbedTLS); the test uses an
  independent reference for the formatting path. mbedTLS itself is a 1-line call and was
  compile-verified, not run on hardware.
- VIN threshold change from 8 to 5 is a deliberate small behavior change (see Change table).
