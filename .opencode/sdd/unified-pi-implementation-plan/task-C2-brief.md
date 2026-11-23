# Task C2 — Sentry integration smoke

Where: last implementation task. Read-only integration with Sentry USB data + launch docs for Studio/Drive.

## Requirement (verbatim from plan)

Files:

- Create: `pi/src/sentry/connector.ts`, `pi/test/sentry.test.ts`
- Modify: `pi/deploy/README.md`

Steps:

1. Read `drive-data.json` (Sentry USB) for clip↔drive correlation; no Sentry code copied, no writes to its storage.
2. Test with a fixture; assert missing file degrades gracefully.
3. Document launching Sentry Studio / Sentry Drive as separate apps; record that 2026.20 dashcam encryption must be off for Sentry USB.
4. Commit.

## Binding context

- Sentry USB: PolyForm Noncommercial 1.0 — read-only file consumption, no code copy, no fork. Studio/Drive are MIT and used as separate apps.
- Sentry USB warns 2026.20 AMD cars need "Encrypt Dashcam Recordings" OFF.
- Unknown schema fields → make the parser tolerant and mark UNVERIFIED; never invent fields.
- No CAN TX; no hardware.
- Stage ONLY files you change (~200 unrelated unstaged exist); verify staged list before commit.
- Feature branch `spike/raspberry-pi-portability`; never main, never force-push.
