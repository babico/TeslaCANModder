import { readFile } from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";
import Ajv2020 from "ajv/dist/2020.js";

const rootDir = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");
const schemaDir = path.join(rootDir, "firmware", "lib", "io", "serial", "schemas");

const serialSchemaPath = path.join(schemaDir, "serial-output.schema.json");

const manifestSectionNames = new Set([
	"meta",
	"connectivity",
	"state",
	"driverAssist",
	"vehicle",
	"platform",
	"firmware",
	"battery",
	"safety",
	"can",
	"features",
	"stream",
	"canHealth",
]);

const manifestFeatureKinds = new Set(["command", "query", "internal"]);
const manifestMessageSectionKeys = new Set(["boot", "status"]);
const manifestMessageKeys = new Set(["type", "command", "schemaRef", "sections", "notes"]);
const manifestFeatureKeys = new Set(["id", "title", "kind", "commands", "outputTags", "statePaths", "notes"]);
const manifestTypePattern = /^[A-Za-z_][A-Za-z0-9_]*$/;
const manifestSchemaRefPattern = /^#\/\$defs\/[A-Za-z][A-Za-z0-9]*$/;
const featureIdPattern = /^[A-Za-z][A-Za-z0-9]*$/;

const readJson = async (filePath) => JSON.parse(await readFile(filePath, "utf8"));

function fail(message) {
	throw new Error(message);
}

function isPlainObject(value) {
	return typeof value === "object" && value !== null && !Array.isArray(value);
}

function assertPlainObject(value, label) {
	if (!isPlainObject(value)) {
		fail(`${label} must be an object`);
	}
}

function assertNoExtraKeys(object, allowedKeys, label) {
	const extraKeys = Object.keys(object).filter((key) => !allowedKeys.has(key));
	if (extraKeys.length > 0) {
		fail(`${label} contains unsupported keys: ${extraKeys.join(", ")}`);
	}
}

function assertNonEmptyString(value, label) {
	if (typeof value !== "string" || value.length === 0) {
		fail(`${label} must be a non-empty string`);
	}
}

function assertOptionalString(value, label) {
	if (value !== undefined && typeof value !== "string") {
		fail(`${label} must be a string when provided`);
	}
}

function assertUniqueStringArray(values, label, { allowEmpty = false, allowedValues } = {}) {
	if (!Array.isArray(values)) {
		fail(`${label} must be an array`);
	}

	if (!allowEmpty && values.length === 0) {
		fail(`${label} must not be empty`);
	}

	const seen = new Set();
	for (let index = 0; index < values.length; index += 1) {
		const value = values[index];
		if (typeof value !== "string" || value.length === 0) {
			fail(`${label}[${index}] must be a non-empty string`);
		}

		if (allowedValues && !allowedValues.has(value)) {
			fail(`${label}[${index}] references unknown value "${value}"`);
		}

		if (seen.has(value)) {
			fail(`${label} contains duplicate value "${value}"`);
		}

		seen.add(value);
	}
}

function validateManifest(manifest) {
	if (!Number.isInteger(manifest.schemaVersion) || manifest.schemaVersion < 2) {
		fail("serial-output.schema.json schemaVersion must be an integer >= 2");
	}

	assertPlainObject(manifest.messageSections, "serial-output.schema.json messageSections");
	assertNoExtraKeys(manifest.messageSections, manifestMessageSectionKeys, "serial-output.schema.json messageSections");
	assertUniqueStringArray(manifest.messageSections.boot, "serial-output.schema.json messageSections.boot", {
		allowedValues: manifestSectionNames,
	});
	assertUniqueStringArray(manifest.messageSections.status, "serial-output.schema.json messageSections.status", {
		allowedValues: manifestSectionNames,
	});

	if (!Array.isArray(manifest.messages) || manifest.messages.length === 0) {
		fail("serial-output.schema.json messages must be a non-empty array");
	}

	for (let index = 0; index < manifest.messages.length; index += 1) {
		const message = manifest.messages[index];
		const label = `serial-output.schema.json messages[${index}]`;
		assertPlainObject(message, label);
		assertNoExtraKeys(message, manifestMessageKeys, label);
		assertNonEmptyString(message.type, `${label}.type`);
		if (!manifestTypePattern.test(message.type)) {
			fail(`${label}.type must match ${manifestTypePattern}`);
		}

		if (message.command !== null) {
			assertNonEmptyString(message.command, `${label}.command`);
		}

		assertNonEmptyString(message.schemaRef, `${label}.schemaRef`);
		if (!manifestSchemaRefPattern.test(message.schemaRef)) {
			fail(`${label}.schemaRef must match ${manifestSchemaRefPattern}`);
		}

		assertUniqueStringArray(message.sections, `${label}.sections`, {
			allowEmpty: true,
			allowedValues: manifestSectionNames,
		});
		assertOptionalString(message.notes, `${label}.notes`);
	}

	if (!Array.isArray(manifest.features) || manifest.features.length === 0) {
		fail("serial-output.schema.json features must be a non-empty array");
	}

	for (let index = 0; index < manifest.features.length; index += 1) {
		const feature = manifest.features[index];
		const label = `serial-output.schema.json features[${index}]`;
		assertPlainObject(feature, label);
		assertNoExtraKeys(feature, manifestFeatureKeys, label);
		assertNonEmptyString(feature.id, `${label}.id`);
		if (!featureIdPattern.test(feature.id)) {
			fail(`${label}.id must match ${featureIdPattern}`);
		}

		assertNonEmptyString(feature.title, `${label}.title`);
		if (!manifestFeatureKinds.has(feature.kind)) {
			fail(`${label}.kind must be one of ${Array.from(manifestFeatureKinds).join(", ")}`);
		}

		if (feature.commands !== undefined) {
			assertUniqueStringArray(feature.commands, `${label}.commands`, { allowEmpty: true });
		}
		if (feature.outputTags !== undefined) {
			assertUniqueStringArray(feature.outputTags, `${label}.outputTags`, { allowEmpty: true });
		}
		if (feature.statePaths !== undefined) {
			assertUniqueStringArray(feature.statePaths, `${label}.statePaths`, { allowEmpty: true });
		}
		assertOptionalString(feature.notes, `${label}.notes`);
	}
}

function _formatAjvErrors(errors) {
	if (!errors || errors.length === 0) {
		return "unknown validation error";
	}

	return errors
		.map((error) => `${error.instancePath || "/"} ${error.message}`.trim())
		.join("; ");
}

const serialSchema = await readJson(serialSchemaPath);
const manifest = serialSchema;

const ajv = new Ajv2020({ allErrors: true, strict: false });
ajv.compile(serialSchema);
validateManifest(manifest);

const schemaDefs = new Set(Object.keys(serialSchema.$defs ?? {}));
const manifestMessageTypes = new Set();
for (const message of manifest.messages) {
	if (manifestMessageTypes.has(message.type)) {
fail(`serial-output.schema.json contains duplicate message type "${message.type}"`);
	}

	manifestMessageTypes.add(message.type);

	if (!message.schemaRef.startsWith("#/$defs/")) {
		fail(`message ${message.type} has unsupported schemaRef ${message.schemaRef}`);
	}

	const defName = message.schemaRef.slice("#/$defs/".length);
	if (!schemaDefs.has(defName)) {
		fail(`message ${message.type} references missing schema definition ${message.schemaRef}`);
	}
}

const featureIds = new Set();
for (const feature of manifest.features) {
	if (featureIds.has(feature.id)) {
		fail(`serial-output.schema.json contains duplicate feature id "${feature.id}"`);
	}

	featureIds.add(feature.id);
}

const sectionUniverse = new Set([
	...manifest.messageSections.boot,
	...manifest.messageSections.status,
	"stream",
	"canHealth",
]);

for (const message of manifest.messages) {
	for (const section of message.sections) {
		if (!sectionUniverse.has(section)) {
			fail(`message ${message.type} references unknown section "${section}"`);
		}
	}
}

const requiredMappings = new Map([
	["status", "#/$defs/Status"],
	["status_meta", "#/$defs/StatusMeta"],
	["status_features", "#/$defs/StatusFeatures"],
	["status_can", "#/$defs/StatusCan"],
	["status_state", "#/$defs/StatusState"],
	["status_compact", "#/$defs/StatusCompact"],
	["statusLive", "#/$defs/StatusLive"],
	["platform", "#/$defs/Platform"],
	["vehicle", "#/$defs/Vehicle"],
	["fwcompat", "#/$defs/FwCompat"],
	["powertrain", "#/$defs/Powertrain"],
	["tpms", "#/$defs/Tpms"],
	["bms", "#/$defs/Bms"],
]);

for (const [messageType, schemaRef] of requiredMappings) {
	const mapping = manifest.messages.find((message) => message.type === messageType);
	if (!mapping) {
		fail(`serial-output.schema.json is missing required mapping for ${messageType}`);
	}

	if (mapping.schemaRef !== schemaRef) {
		fail(`message ${messageType} should reference ${schemaRef}, got ${mapping.schemaRef}`);
	}
}

console.warn("Validated unified serial-output.schema.json contract.");
