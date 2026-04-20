import {
  DEFAULT_PRESET,
  LAB_PRESETS,
  applyPreset,
  buildApplyConnectionMessage,
  updateConnectionState,
  validateConnectionConfig,
  type ConnectionState,
  type Preset,
} from "../../src/state/monitorConnectionPresets";

describe("applyPreset", () => {
  it("applies vehicle AP preset", () => {
    const preset: Preset = {
      name: "Vehicle AP",
      connection: {
        baseUrl: "http://192.168.4.1",
        commandPath: "/api/command",
        statusPath: "/api/status",
      },
    };

    const result = applyPreset({ preset });

    expect(result.config).toEqual(preset.connection);
    expect(result.transportType).toBe("http");
    expect(result.confirmationMessage).toBe("Preset applied: Vehicle AP");
  });

  it("applies local bridge preset", () => {
    const preset: Preset = {
      name: "Local Bridge",
      connection: {
        baseUrl: "http://localhost:8080",
        commandPath: "/api/command",
        statusPath: "/api/status",
      },
    };

    const result = applyPreset({ preset });

    expect(result.config.baseUrl).toBe("http://localhost:8080");
    expect(result.confirmationMessage).toBe("Preset applied: Local Bridge");
  });

  it("preserves exact connection configuration", () => {
    const preset: Preset = {
      name: "Custom",
      connection: {
        baseUrl: "https://custom.example.com",
        commandPath: "/command",
        statusPath: "/status",
      },
    };

    const result = applyPreset({ preset });

    expect(result.config.baseUrl).toBe("https://custom.example.com");
    expect(result.config.commandPath).toBe("/command");
    expect(result.config.statusPath).toBe("/status");
  });
});

describe("updateConnectionState", () => {
  const currentState: ConnectionState = {
    baseUrl: "http://192.168.4.1",
    commandPath: "/api/command",
    statusPath: "/api/status",
  };

  it("updates baseUrl only", () => {
    const result = updateConnectionState({
      current: currentState,
      updates: { baseUrl: "http://localhost:8080" },
    });

    expect(result.baseUrl).toBe("http://localhost:8080");
    expect(result.commandPath).toBe("/api/command");
    expect(result.statusPath).toBe("/api/status");
  });

  it("updates commandPath only", () => {
    const result = updateConnectionState({
      current: currentState,
      updates: { commandPath: "/command/v2" },
    });

    expect(result.baseUrl).toBe("http://192.168.4.1");
    expect(result.commandPath).toBe("/command/v2");
    expect(result.statusPath).toBe("/api/status");
  });

  it("updates statusPath only", () => {
    const result = updateConnectionState({
      current: currentState,
      updates: { statusPath: "/status/v2" },
    });

    expect(result.baseUrl).toBe("http://192.168.4.1");
    expect(result.commandPath).toBe("/api/command");
    expect(result.statusPath).toBe("/status/v2");
  });

  it("updates multiple fields", () => {
    const result = updateConnectionState({
      current: currentState,
      updates: {
        baseUrl: "http://newhost",
        commandPath: "/new/command",
      },
    });

    expect(result.baseUrl).toBe("http://newhost");
    expect(result.commandPath).toBe("/new/command");
    expect(result.statusPath).toBe("/api/status");
  });

  it("preserves state when no updates provided", () => {
    const result = updateConnectionState({
      current: currentState,
      updates: {},
    });

    expect(result).toEqual(currentState);
  });

  it("handles undefined updates", () => {
    const result = updateConnectionState({
      current: currentState,
      updates: {
        baseUrl: undefined,
        commandPath: undefined,
        statusPath: undefined,
      },
    });

    expect(result).toEqual(currentState);
  });
});

describe("validateConnectionConfig", () => {
  it("accepts valid connection config", () => {
    const result = validateConnectionConfig({
      baseUrl: "http://192.168.4.1",
      commandPath: "/api/command",
      statusPath: "/api/status",
    });

    expect(result).toBe(true);
  });

  it("rejects empty baseUrl", () => {
    const result = validateConnectionConfig({
      baseUrl: "",
      commandPath: "/api/command",
      statusPath: "/api/status",
    });

    expect(result).toBe(false);
  });

  it("rejects whitespace-only baseUrl", () => {
    const result = validateConnectionConfig({
      baseUrl: "   ",
      commandPath: "/api/command",
      statusPath: "/api/status",
    });

    expect(result).toBe(false);
  });

  it("rejects empty commandPath", () => {
    const result = validateConnectionConfig({
      baseUrl: "http://localhost",
      commandPath: "",
      statusPath: "/api/status",
    });

    expect(result).toBe(false);
  });

  it("rejects empty statusPath", () => {
    const result = validateConnectionConfig({
      baseUrl: "http://localhost",
      commandPath: "/api/command",
      statusPath: "",
    });

    expect(result).toBe(false);
  });

  it("rejects when all fields are empty", () => {
    const result = validateConnectionConfig({
      baseUrl: "",
      commandPath: "",
      statusPath: "",
    });

    expect(result).toBe(false);
  });

  it("accepts https connections", () => {
    const result = validateConnectionConfig({
      baseUrl: "https://secure.example.com",
      commandPath: "/api/cmd",
      statusPath: "/api/stat",
    });

    expect(result).toBe(true);
  });

  it("accepts localhost connections", () => {
    const result = validateConnectionConfig({
      baseUrl: "http://localhost:8080",
      commandPath: "/api/command",
      statusPath: "/api/status",
    });

    expect(result).toBe(true);
  });
});

describe("buildApplyConnectionMessage", () => {
  it("builds message for vehicle AP connection", () => {
    const result = buildApplyConnectionMessage({
      baseUrl: "http://192.168.4.1",
      commandPath: "/api/command",
      statusPath: "/api/status",
    });

    expect(result).toBe("Applied HTTP connection: http://192.168.4.1/api/command");
  });

  it("builds message for localhost connection", () => {
    const result = buildApplyConnectionMessage({
      baseUrl: "http://localhost:8080",
      commandPath: "/command",
      statusPath: "/status",
    });

    expect(result).toContain("localhost:8080");
    expect(result).toContain("/command");
  });

  it("includes full base and command path", () => {
    const result = buildApplyConnectionMessage({
      baseUrl: "https://api.example.com",
      commandPath: "/v2/command",
      statusPath: "/v2/status",
    });

    expect(result).toContain("https://api.example.com");
    expect(result).toContain("/v2/command");
  });

  it("always indicates HTTP transport type", () => {
    const result = buildApplyConnectionMessage({
      baseUrl: "http://test",
      commandPath: "/cmd",
      statusPath: "/stat",
    });

    expect(result).toContain("HTTP");
  });
});

describe("DEFAULT_PRESET", () => {
  it("provides vehicle AP configuration", () => {
    expect(DEFAULT_PRESET.name).toBe("Vehicle AP");
    expect(DEFAULT_PRESET.connection.baseUrl).toBe("http://192.168.4.1");
    expect(DEFAULT_PRESET.connection.commandPath).toBe("/api/command");
    expect(DEFAULT_PRESET.connection.statusPath).toBe("/api/status");
  });

  it("has valid connection config", () => {
    const valid = validateConnectionConfig({
      baseUrl: DEFAULT_PRESET.connection.baseUrl,
      commandPath: DEFAULT_PRESET.connection.commandPath,
      statusPath: DEFAULT_PRESET.connection.statusPath,
    });

    expect(valid).toBe(true);
  });
});

describe("LAB_PRESETS", () => {
  it("includes multiple presets", () => {
    expect(LAB_PRESETS.length).toBeGreaterThanOrEqual(3);
  });

  it("includes default vehicle AP preset", () => {
    const vehicleAp = LAB_PRESETS.find((p) => p.name === "Vehicle AP");
    expect(vehicleAp).toBeDefined();
    expect(vehicleAp?.connection.baseUrl).toBe("http://192.168.4.1");
  });

  it("includes local bridge preset", () => {
    const localBridge = LAB_PRESETS.find((p) => p.name === "Local Bridge");
    expect(localBridge).toBeDefined();
    expect(localBridge?.connection.baseUrl).toBe("http://localhost:8080");
  });

  it("includes lab rig preset", () => {
    const labRig = LAB_PRESETS.find((p) => p.name === "Lab Rig");
    expect(labRig).toBeDefined();
    expect(labRig?.connection.baseUrl).toBe("http://192.168.10.20");
  });

  it("all presets have valid configs", () => {
    for (const preset of LAB_PRESETS) {
      const valid = validateConnectionConfig({
        baseUrl: preset.connection.baseUrl,
        commandPath: preset.connection.commandPath,
        statusPath: preset.connection.statusPath,
      });
      expect(valid).toBe(true);
    }
  });
});
