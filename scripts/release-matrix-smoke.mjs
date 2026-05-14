#!/usr/bin/env node

import fs from "node:fs";
import path from "node:path";
import { spawnSync } from "node:child_process";

const repoRoot = path.resolve(import.meta.dirname, "..");
const workflowPath = path.join(repoRoot, ".github", "workflows", "release.yml");
const firmwareDir = path.join(repoRoot, "firmware");
const smokeOutputDir = path.join(firmwareDir, "build", "release-matrix-smoke");

function writeStdout(message) {
  process.stdout.write(`${message}\n`);
}

function printUsage() {
  writeStdout(`Usage: node scripts/release-matrix-smoke.mjs [options]

Options:
  --env <name>       Limit to one or more firmware envs (repeatable, comma-separated)
  --suffix <value>   Limit to one or more profile suffixes (repeatable, comma-separated)
  --dry-run          Print the planned sweep without invoking PlatformIO
  --no-clean         Reuse existing PlatformIO build output instead of cleaning each entry
  --help             Show this help text
`);
}

function fail(message) {
  console.error(`[release-matrix-smoke] ERROR: ${message}`);
  process.exit(1);
}

function info(message) {
  writeStdout(`[release-matrix-smoke] ${message}`);
}

function parseCsvFlag(target, value) {
  value
    .split(",")
    .map((entry) => entry.trim())
    .filter(Boolean)
    .forEach((entry) => target.add(entry));
}

function parseArgs(argv) {
  const envFilter = new Set();
  const suffixFilter = new Set();
  let dryRun = false;
  let cleanEachBuild = true;

  for (let index = 0; index < argv.length; index += 1) {
    const arg = argv[index];
    if (arg === "--help") {
      printUsage();
      process.exit(0);
    }

    if (arg === "--dry-run") {
      dryRun = true;
      continue;
    }

    if (arg === "--no-clean") {
      cleanEachBuild = false;
      continue;
    }

    if (arg.startsWith("--env=")) {
      parseCsvFlag(envFilter, arg.slice("--env=".length));
      continue;
    }

    if (arg === "--env") {
      const value = argv[index + 1];
      if (!value) {
        fail("--env requires a value");
      }
      parseCsvFlag(envFilter, value);
      index += 1;
      continue;
    }

    if (arg.startsWith("--suffix=")) {
      parseCsvFlag(suffixFilter, arg.slice("--suffix=".length));
      continue;
    }

    if (arg === "--suffix") {
      const value = argv[index + 1];
      if (!value) {
        fail("--suffix requires a value");
      }
      parseCsvFlag(suffixFilter, value);
      index += 1;
      continue;
    }

    fail(`Unknown argument: ${arg}`);
  }

  return {
    envFilter,
    suffixFilter,
    dryRun,
    cleanEachBuild,
  };
}

function parseWorkflowMatrix() {
  if (!fs.existsSync(workflowPath)) {
    fail(`Release workflow not found at ${workflowPath}`);
  }

  const workflowText = fs.readFileSync(workflowPath, "utf8");
  const envMatch = workflowText.match(/^\s+env:\s*\[([^\]]+)\]/m);
  if (!envMatch) {
    fail("Could not find strategy.matrix.env in release workflow");
  }

  const envs = envMatch[1]
    .split(",")
    .map((entry) => entry.trim())
    .filter(Boolean);

  const profileBlockMatch = workflowText.match(/^\s+profile:\n([\s\S]*?)^\s+steps:\n/m);
  if (!profileBlockMatch) {
    fail("Could not find strategy.matrix.profile in release workflow");
  }

  const profiles = [];
  let current = null;
  for (const rawLine of profileBlockMatch[1].split(/\r?\n/)) {
    const line = rawLine.trim();
    if (!line) {
      continue;
    }

    if (line.startsWith("- chassis:")) {
      if (current) {
        fail("Encountered a new profile before the previous profile was complete");
      }
      current = {
        chassis: Number(line.slice("- chassis:".length).trim()),
      };
      continue;
    }

    if (!current) {
      continue;
    }

    if (line.startsWith("vehicle:")) {
      current.vehicle = Number(line.slice("vehicle:".length).trim());
      continue;
    }

    if (line.startsWith("body:")) {
      current.body = Number(line.slice("body:".length).trim());
      continue;
    }

    if (line.startsWith("suffix:")) {
      const suffixValue = line.slice("suffix:".length).trim();
      current.suffix = suffixValue === '""' ? "" : suffixValue;
      profiles.push(current);
      current = null;
    }
  }

  if (current) {
    fail("Release workflow profile block ended before the last profile was complete");
  }

  if (envs.length === 0 || profiles.length === 0) {
    fail("Release workflow matrix parsed but did not produce any envs or profiles");
  }

  return { envs, profiles };
}

function buildFlags(profile) {
  return [
    `-DBUS_CHASSIS_ACTIVE=${profile.chassis}`,
    `-DBUS_VEHICLE_ACTIVE=${profile.vehicle}`,
    `-DBUS_BODY_ACTIVE=${profile.body}`,
  ].join(" ");
}

function artifactName(envName, profile) {
  return `${envName}${profile.suffix}.bin`;
}

function getPlatformIoCommand() {
  if (process.platform === "win32") {
    return {
      command: "pwsh",
      prefixArgs: ["-NoProfile", "-File", ".\\pio.ps1"],
    };
  }

  return {
    command: "pio",
    prefixArgs: [],
  };
}

function runPlatformIo(commandConfig, args, extraEnv) {
  const result = spawnSync(commandConfig.command, [...commandConfig.prefixArgs, ...args], {
    cwd: firmwareDir,
    env: {
      ...process.env,
      ...extraEnv,
    },
    stdio: "inherit",
  });

  if (result.status !== 0) {
    fail(
      `PlatformIO command failed: ${commandConfig.command} ${[...commandConfig.prefixArgs, ...args].join(" ")}`,
    );
  }
}

function ensureArtifact(entry) {
  const sourcePath = path.join(firmwareDir, "build", "firmware", `${entry.env}.bin`);
  if (!fs.existsSync(sourcePath)) {
    fail(`Expected merged artifact missing: ${sourcePath}`);
  }

  const artifactBytes = fs.statSync(sourcePath).size;
  if (artifactBytes <= 0) {
    fail(`Merged artifact is empty: ${sourcePath}`);
  }

  const destinationPath = path.join(smokeOutputDir, artifactName(entry.env, entry.profile));
  fs.copyFileSync(sourcePath, destinationPath);

  return {
    destinationPath,
    artifactBytes,
  };
}

function main() {
  const options = parseArgs(process.argv.slice(2));
  const matrix = parseWorkflowMatrix();
  const entries = [];

  for (const envName of matrix.envs) {
    if (options.envFilter.size > 0 && !options.envFilter.has(envName)) {
      continue;
    }

    for (const profile of matrix.profiles) {
      if (options.suffixFilter.size > 0 && !options.suffixFilter.has(profile.suffix || "default")) {
        continue;
      }

      entries.push({
        env: envName,
        profile,
        flags: buildFlags(profile),
      });
    }
  }

  if (entries.length === 0) {
    fail("No release-matrix entries matched the requested filters");
  }

  info(
    `Parsed ${matrix.envs.length} envs and ${matrix.profiles.length} bus profiles from ${path.relative(repoRoot, workflowPath)}`,
  );
  info(`Planned smoke sweep entries: ${entries.length}`);

  for (const entry of entries) {
    info(`- ${entry.env}${entry.profile.suffix} :: ${entry.flags}`);
  }

  if (options.dryRun) {
    return;
  }

  fs.rmSync(smokeOutputDir, { recursive: true, force: true });
  fs.mkdirSync(smokeOutputDir, { recursive: true });

  const commandConfig = getPlatformIoCommand();
  const results = [];

  entries.forEach((entry, index) => {
    info(`(${index + 1}/${entries.length}) Building ${entry.env}${entry.profile.suffix}`);
    const commandEnv = {
      PLATFORMIO_BUILD_FLAGS: entry.flags,
    };

    if (options.cleanEachBuild) {
      runPlatformIo(commandConfig, ["run", "-t", "clean", "-e", entry.env], commandEnv);
    }

    runPlatformIo(commandConfig, ["run", "-e", entry.env], commandEnv);
    const artifact = ensureArtifact(entry);
    results.push({
      env: entry.env,
      suffix: entry.profile.suffix,
      chassis: entry.profile.chassis,
      vehicle: entry.profile.vehicle,
      body: entry.profile.body,
      flags: entry.flags,
      artifact: path.relative(firmwareDir, artifact.destinationPath).replace(/\\/g, "/"),
      artifactBytes: artifact.artifactBytes,
    });
  });

  const reportPath = path.join(smokeOutputDir, "report.json");
  fs.writeFileSync(
    reportPath,
    JSON.stringify(
      {
        generatedAt: new Date().toISOString(),
        workflow: path.relative(repoRoot, workflowPath).replace(/\\/g, "/"),
        entries: results,
      },
      null,
      2,
    ) + "\n",
    "utf8",
  );

  info(`Smoke sweep complete. Report: ${path.relative(repoRoot, reportPath).replace(/\\/g, "/")}`);
}

main();
