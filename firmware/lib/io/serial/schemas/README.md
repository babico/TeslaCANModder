# Serial JSON Schemas

This folder documents JSON line outputs emitted by serial backends in:

- `firmware/lib/io/serial/common.h`
- `firmware/lib/io/serial/board.h`

## Files

- `serial-output.schema.json`: The single unified contract file. Contains the JSON Schema for all message types plus the feature manifest (`schemaVersion`, `messageSections`, `messages`, `features`) as top-level properties.

## Notes

- Numeric booleans are represented as `0|1` because firmware prints booleans as integers.
- Run `npm run validate:serial-contract` from the repo root after editing the schema.
- The feature manifest data lives directly in `serial-output.schema.json` — there is no separate manifest file.

## Contract Surface

- Full snapshots use `boot` and `status`.
- Split status queries use `status:meta`, `status:state`, `status:features`, `status:can`, and `status:compact`.
- `platform`, `vehicle`, `fwcompat`, `powertrain`, `tpms`, and `bms` each map to dedicated schema definitions.

The `messages` and `features` arrays embedded in the schema link those message tags and query commands to their `$defs` entries.
