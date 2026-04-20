# Serial JSON Schemas

This folder documents JSON line outputs emitted by serial backends in:

- `hardware/lib/io/serial/common.h`
- `hardware/lib/io/serial/esp32.h`
- `hardware/lib/io/serial/uno.h`

## Files

- `serial-output.schema.json`: Unified schema for all known serial output message types.

## Notes

- Some large status payloads (`boot`, `status`, `status_*`) intentionally allow extra keys to remain backward-compatible while firmware fields evolve.
- Numeric booleans are represented as `0|1` because firmware prints booleans as integers.
