import {
  ALL_FEATURE_SETTINGS_SPECS,
  FEATURE_IDS,
  FEATURE_SETTINGS_BY_ID,
  commands,
  getFeatureSettingsSpecById,
} from "../../src/index.js";

describe("feature settings specs", () => {
  it("indexes every feature by stable feature id", () => {
    expect(FEATURE_IDS).toHaveLength(ALL_FEATURE_SETTINGS_SPECS.length);

    const seen = new Set(FEATURE_IDS);
    expect(seen.size).toBe(FEATURE_IDS.length);

    for (const spec of ALL_FEATURE_SETTINGS_SPECS) {
      expect(FEATURE_SETTINGS_BY_ID[spec.featureId]).toBe(spec);
      expect(getFeatureSettingsSpecById(spec.featureId)).toBe(spec);
    }
  });

  it("references valid command builders in each setting", () => {
    const commandNames = new Set(Object.keys(commands));

    for (const spec of ALL_FEATURE_SETTINGS_SPECS) {
      for (const setting of spec.settings) {
        for (const commandName of setting.commandNames) {
          expect(commandNames.has(commandName)).toBe(true);
        }
      }
    }
  });

  it("marks internal features without user settings", () => {
    const internal = ALL_FEATURE_SETTINGS_SPECS.filter((spec) => spec.kind === "internal");
    expect(internal.map((spec) => spec.featureId).sort()).toEqual([
      "driveContext",
      "regionCodec",
    ]);
    for (const spec of internal) {
      expect(spec.settings).toHaveLength(0);
    }
  });
});
