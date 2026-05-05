import { readFile } from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";
import Ajv2020 from "ajv/dist/2020.js";

const rootDir = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");
const schemaDir = path.join(rootDir, "firmware", "lib", "io", "schemas");

const schemaPath = path.join(schemaDir, "io.schema.json");

const manifestFeatureKinds = new Set(["command", "query", "internal"]);
const manifestMessageKeys = new Set([
	"tag",
	"description",
	"schema",
	"alternativeSchema",
	"transports",
]);
const manifestFeatureKeys = new Set([
	"id",
	"title",
	"kind",
	"commands",
	"outputTags",
	"statePaths",
]);
const manifestTypePattern = /^[A-Za-z_][A-Za-z0-9_]*$/;
const manifestSchemaRefPattern = /^#\/\$defs\/[A-Za-z][A-Za-z0-9]*$/;
const featureIdPattern = /^[A-Za-z][A-Za-z0-9]*$/;
const knownTransports = new Set(["serial", "ble", "wifi"]);

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
	if (!Number.isInteger(manifest.schemaVersion) || manifest.schemaVersion < 1) {
		fail("io.schema.json schemaVersion must be an integer >= 1");
	}

	if (!Array.isArray(manifest.messages) || manifest.messages.length === 0) {
		fail("io.schema.json messages must be a non-empty array");
	}

	for (let index = 0; index < manifest.messages.length; index += 1) {
		const message = manifest.messages[index];
		const label = `io.schema.json messages[${index}]`;
		assertPlainObject(message, label);
		assertNoExtraKeys(message, manifestMessageKeys, label);
		assertNonEmptyString(message.tag, `${label}.tag`);
		if (!manifestTypePattern.test(message.tag)) {
			fail(`${label}.tag must match ${manifestTypePattern}`);
		}

		assertNonEmptyString(message.schema, `${label}.schema`);
		if (!manifestSchemaRefPattern.test(message.schema)) {
			fail(`${label}.schema must match ${manifestSchemaRefPattern}`);
		}

		if (message.alternativeSchema !== undefined) {
			assertNonEmptyString(message.alternativeSchema, `${label}.alternativeSchema`);
			if (!manifestSchemaRefPattern.test(message.alternativeSchema)) {
				fail(`${label}.alternativeSchema must match ${manifestSchemaRefPattern}`);
			}
		}

		assertNonEmptyString(message.description, `${label}.description`);
		assertUniqueStringArray(message.transports, `${label}.transports`, {
			allowedValues: knownTransports,
		});
	}

	if (!Array.isArray(manifest.features) || manifest.features.length === 0) {
		fail("io.schema.json features must be a non-empty array");
	}

	for (let index = 0; index < manifest.features.length; index += 1) {
		const feature = manifest.features[index];
		const label = `io.schema.json features[${index}]`;
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
			assertUniqueStringArray(feature.outputTags, `${label}.outputTags`, {
				allowEmpty: true,
			});
		}
		if (feature.statePaths !== undefined) {
			assertUniqueStringArray(feature.statePaths, `${label}.statePaths`, {
				allowEmpty: true,
			});
		}
	}
}

function _formatAjvErrors(errors) {
	if (!errors || errors.length === 0) {
		return "unknown validation error";
	}

	return errors.map((error) => `${error.instancePath || "/"} ${error.message}`.trim()).join("; ");
}

const ioSchema = await readJson(schemaPath);
const manifest = ioSchema;

const ajv = new Ajv2020({ allErrors: true, strict: false });
ajv.compile(ioSchema);
validateManifest(manifest);

const schemaDefs = new Set(Object.keys(ioSchema.$defs ?? {}));
const manifestMessageTags = new Set();
for (const message of manifest.messages) {
	if (manifestMessageTags.has(message.tag)) {
		fail(`io.schema.json contains duplicate message tag "${message.tag}"`);
	}

	manifestMessageTags.add(message.tag);

	if (!message.schema.startsWith("#/$defs/")) {
		fail(`message ${message.tag} has unsupported schema ${message.schema}`);
	}

	const defName = message.schema.slice("#/$defs/".length);
	if (!schemaDefs.has(defName)) {
		fail(`message ${message.tag} references missing schema definition ${message.schema}`);
	}

	if (message.alternativeSchema !== undefined) {
		const altDefName = message.alternativeSchema.slice("#/$defs/".length);
		if (!schemaDefs.has(altDefName)) {
			fail(
				`message ${message.tag} alternativeSchema references missing definition ${message.alternativeSchema}`,
			);
		}
	}
}

const featureIds = new Set();
for (const feature of manifest.features) {
	if (featureIds.has(feature.id)) {
		fail(`io.schema.json contains duplicate feature id "${feature.id}"`);
	}

	featureIds.add(feature.id);
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

for (const [messageTag, schemaRef] of requiredMappings) {
	const mapping = manifest.messages.find((message) => message.tag === messageTag);
	if (!mapping) {
		fail(`io.schema.json is missing required mapping for ${messageTag}`);
	}

	if (mapping.schema !== schemaRef) {
		fail(`message ${messageTag} should reference ${schemaRef}, got ${mapping.schema}`);
	}
}

console.warn("Validated unified io.schema.json contract.");
