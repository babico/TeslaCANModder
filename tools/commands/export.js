/**
 * export command — convert a Tesla CAN decoder JSON dataset to DBC, CSV, or
 * compact summary JSON.
 *
 * Usage (via tcm-debug):
 *   node tools/debug.js export --input <file.json> --format dbc|csv|json [--output <out>]
 *
 * Or run standalone:
 *   node tools/commands/export.js --input mcu2.json --format dbc
 */

import { accessSync, constants, readFileSync, statSync, writeFileSync, existsSync } from 'node:fs';
import path from 'node:path';
import { toDbcString, toCsvString, toSummaryJson } from '../lib/canExport.js';
import { parseArgs } from '../lib/args.js';

const FORMATS = ['dbc', 'csv', 'json'];
const MAX_INPUT_BYTES = 25 * 1024 * 1024;

export async function runExport(opts, out) {
  const { input, format = 'json', output, overwrite = false } = opts;

  if (!input) {
    out.fail('--input <file> is required');
    return 1;
  }

  if (!FORMATS.includes(format)) {
    out.fail(`Unknown format "${format}". Choose: ${FORMATS.join(', ')}`);
    return 1;
  }

  const inputPath = path.resolve(String(input));

  let inputStat;
  try {
    inputStat = statSync(inputPath);
    if (!inputStat.isFile()) {
      out.fail(`Input must be a file: ${inputPath}`);
      return 1;
    }
    if (inputStat.size > MAX_INPUT_BYTES) {
      out.fail(`Input file too large (${inputStat.size} bytes). Limit: ${MAX_INPUT_BYTES} bytes`);
      return 1;
    }
    accessSync(inputPath, constants.R_OK);
  } catch (err) {
    out.fail(`Input file not readable: ${err.message}`);
    return 1;
  }

  let dataset;
  try {
    const raw = readFileSync(inputPath, 'utf8');
    dataset = JSON.parse(raw);
  } catch (err) {
    out.fail(`Failed to parse input JSON: ${err.message}`);
    return 1;
  }

  if (!dataset || !Array.isArray(dataset.frames)) {
    out.fail('Invalid dataset: expected a top-level "frames" array');
    return 1;
  }

  const src = dataset.dataset_source ?? {};
  out.section(`Exporting CAN dataset → ${format.toUpperCase()}`);
  out.info(`vehicle=${src.vehicle ?? '?'}  firmware=${src.firmware ?? '?'}  mcu=${src.mcu ?? '?'}  soc=${src.soc ?? '?'}`);
  out.info(`frames=${(dataset.frames ?? []).length}`);

  let result;
  if (format === 'dbc') {
    result = toDbcString(dataset);
  } else if (format === 'csv') {
    result = toCsvString(dataset);
  } else {
    result = JSON.stringify(toSummaryJson(dataset), null, 2);
  }

  if (output) {
    const outputPath = path.resolve(String(output));
    const outputDir = path.dirname(outputPath);

    try {
      const dirStat = statSync(outputDir);
      if (!dirStat.isDirectory()) {
        out.fail(`Output directory is not a directory: ${outputDir}`);
        return 1;
      }
      accessSync(outputDir, constants.W_OK);
    } catch (err) {
      out.fail(`Output directory not writable: ${err.message}`);
      return 1;
    }

    if (!overwrite && existsSync(outputPath)) {
      out.fail(`Output already exists: ${outputPath} (use --overwrite to replace)`);
      return 1;
    }

    try {
      writeFileSync(outputPath, result, { encoding: 'utf8', flag: overwrite ? 'w' : 'wx' });
    } catch (err) {
      out.fail(`Failed to write output file: ${err.message}`);
      return 1;
    }

    out.pass(`Written to ${outputPath}`);
  } else {
    process.stdout.write(result + '\n');
  }

  return 0;
}

// ── Standalone entry point ────────────────────────────────────────────────────
if (process.argv[1] && process.argv[1].endsWith('export.js')) {
  const args = parseArgs(process.argv.slice(2));

  // Minimal output shim so we don't need full createOutput
  const out = {
    section: (t) => console.error(`\n${t}`),
    info:    (m) => console.error(`  ${m}`),
    pass:    (m) => console.error(`  ✓ ${m}`),
    fail:    (m) => { console.error(`  ✗ ${m}`); },
  };

  const opts = {
    input:  args.input  ?? args.i ?? null,
    format: args.format ?? args.f ?? 'json',
    output: args.output ?? args.o ?? null,
    overwrite: Boolean(args.overwrite),
  };

  runExport(opts, out).then((code) => process.exit(code ?? 0));
}
