# Task C2 — Sentry integration smoke — report

Status: DONE. Commit `83ef83af8ddddd0bea6837515f33e27113bb8de5`,
`feat(pi): read-only Sentry USB drive-data connector`, pushed to
`origin/spike/raspberry-pi-portability`.

## Schema decision (asked/UNVERIFIED)

Brief: "If the real `drive-data.json` schema is unknown, ASK NOW — do not invent fields; use a
tolerant shape with UNVERIFIED labels."

No real `drive-data.json` schema or sample exists in this repo (grep found only plan/brief/report
references, no schema or fixture). The brief itself supplies the fallback rule: "Unknown schema
fields → make the parser tolerant and mark UNVERIFIED; never invent fields." Therefore I did not
block on a question and did not invent fields. The connector:

- loads and returns the parsed JSON untouched (`unknown`),
- joins only with caller-supplied key hints (`driveIdKey`, `clipDriveKey`); with no hints it returns
  no links,
- labels the path and all schema hints UNVERIFIED in code comments and the deploy README.

If the controller has the real schema, the only change needed is supplying those two keys at the
call site — no connector rewrite.

## TDD — RED then GREEN

RED (test written first; module absent):

```
npm test -w @teslacanmodder/pi -- sentry
FAIL test/sentry.test.ts
  ● Test suite failed to run
    test/sentry.test.ts:5:55 - error TS2307: Cannot find module '../src/sentry/connector.js'
Test Suites: 1 failed, 1 total
Tests:       0 total
```

GREEN (after implementing `connector.ts`):

```
PASS test/sentry.test.ts
  Sentry USB drive-data connector (read-only)
    √ loads a valid fixture and correlates clips to drives using caller-supplied keys
    √ degrades gracefully when drive-data.json is missing
    √ degrades gracefully on invalid JSON
    √ is tolerant of unknown shapes and invents no fields
    √ never writes to Sentry storage
Test Suites: 1 passed, 1 total
Tests:       5 passed, 5 total
```

One mid-TDD fix: the missing-file test initially got `status:"invalid"` because native `fs` errors
are from another realm under Jest VM modules, so `err instanceof Error` was false. Replaced with
duck typing on `{code, message}`; documented inline. No production semantics changed.

Final verification (run once before commit, as required):

```
npm test -w @teslacanmodder/pi
Test Suites: 8 passed, 8 total
Tests:       79 passed, 79 total

npm run lint:all
eslint .                       -> clean
prettier --check .             -> "All matched files use Prettier code style!"
markdownlint-cli2 (48 files)   -> 0 issues in 0 files
```

Post-commit husky hooks re-ran prettier/eslint --fix on staged files; sentry suite re-run after the
hooks: 5/5 pass. Working tree for the three touched paths is clean.

## Files

| File                         | Action | Notes                                                                 |
| ---------------------------- | ------ | --------------------------------------------------------------------- |
| `pi/src/sentry/connector.ts` | create | `readDriveData()`, `correlateClipsToDrives()`, types; read-only only. |
| `pi/test/sentry.test.ts`     | create | 5 tests; runtime-created JSON fixture (no committed Sentry sample).   |
| `pi/deploy/README.md`        | modify | 43 lines: read-only consumer, Studio/Drive separate apps, encryption. |

No other files staged or committed. ~200 pre-existing unrelated unstaged files left untouched.
No files under `legacy/` touched. No CAN TX, no hardware, no Sentry code copied.

## Self-review

- **Read-only enforced:** only `readFileSync`; a test snapshots content, mtime and size before/after
  both calls and asserts they are unchanged. No mkdir, no write stream, no Sentry config edits.
- **Graceful degradation:** `ENOENT` -> `status:"missing"`; parse/other errors -> `"invalid"`; both
  return `data:null, available:false, reason` and never throw. EACCES and other read failures are
  intentionally folded into `"invalid"` (reason carries the message).
- **No invented schema:** neutral fixture keys (`id`, `driveId`) are test-only hints, commented as
  such; the correlator hardcodes none. Unknown shapes, missing keys, non-record arrays and non-object
  roots all yield no links without throwing. Recursion is bounded (`maxDepth` default 4) with a
  `seen` set so cyclic/large JSON cannot hang the walk.
- **Licensing respected:** Sentry USB (PolyForm Noncommercial) is only consumed as data; Studio/Drive
  (MIT) documented as separate apps, not bundled or copied.
- **Docs:** path + schema + encryption warning all carry UNVERIFIED labels where not hardware-checked;
  existing deploy tests that read this README still pass (deploy.test.ts green).
- **Git hygiene:** explicit `git add <3 paths>`, staged list verified before commit; commit touched
  exactly those 3 files; pushed to feature branch with `-u`, no force-push, no main.

## Concerns / follow-ups

1. Real `drive-data.json` path and field names remain UNVERIFIED. Until confirmed on hardware,
   `correlateClipsToDrives()` returns no links in production — this is deliberate, not a bug. No
   caller in current shipping code yet; connector is a library for later `/tcm` panels.
2. 2026.20 "Encrypt Dashcam Recordings" warning is recorded from vendor docs; UNVERIFIED on the Pi.
3. Minor, unaddressed: a single record carrying both join keys could self-link. Benign for a
   tolerant correlation helper; add a `clip === drive` guard if the real schema makes that likely.
4. Report file intentionally not committed (matches task-B1 precedent).
