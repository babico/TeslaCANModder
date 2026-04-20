import { commands } from "./commands.js";

export type CommandBuilderName = keyof typeof commands;

export type FeatureSettingControlType =
  | "toggle"
  | "enum"
  | "number"
  | "text"
  | "action"
  | "query";

interface FeatureSettingBase {
  id: string;
  label: string;
  control: FeatureSettingControlType;
  commandNames: readonly CommandBuilderName[];
  note?: string;
}

export interface FeatureSettingToggleSpec extends FeatureSettingBase {
  control: "toggle";
}

export interface FeatureSettingEnumSpec extends FeatureSettingBase {
  control: "enum";
  options: readonly { value: string; label: string }[];
}

export interface FeatureSettingNumberSpec extends FeatureSettingBase {
  control: "number";
  min: number;
  max: number;
  step?: number;
  unit?: string;
}

export interface FeatureSettingTextSpec extends FeatureSettingBase {
  control: "text";
  minLength?: number;
  maxLength?: number;
}

export interface FeatureSettingActionSpec extends FeatureSettingBase {
  control: "action";
}

export interface FeatureSettingQuerySpec extends FeatureSettingBase {
  control: "query";
}

export type FeatureSettingSpec =
  | FeatureSettingToggleSpec
  | FeatureSettingEnumSpec
  | FeatureSettingNumberSpec
  | FeatureSettingTextSpec
  | FeatureSettingActionSpec
  | FeatureSettingQuerySpec;

export type FeatureSpecKind = "command" | "query" | "internal";

export interface FeatureSettingsSpec {
  featureId: string;
  title: string;
  kind: FeatureSpecKind;
  settings: readonly FeatureSettingSpec[];
  notes?: readonly string[];
}

export type FeatureId = FeatureSettingsSpec["featureId"];

const FEATURE_SETTINGS_BY_ID_INTERNAL = {
  "airRecirc": {
    "featureId": "airRecirc",
    "title": "Air Recirculation",
    "kind": "command",
    "settings": [
      {
        "id": "airRecircEnabled",
        "label": "Recirculation",
        "control": "toggle",
        "commandNames": [
          "airRecirc"
        ]
      }
    ]
  },
  "banShield": {
    "featureId": "banShield",
    "title": "Ban Shield",
    "kind": "command",
    "settings": [
      {
        "id": "banShieldEnabled",
        "label": "Enable Ban Shield",
        "control": "toggle",
        "commandNames": [
          "banShield"
        ]
      },
      {
        "id": "gtwShieldArm",
        "label": "GTW Shield Arm",
        "control": "action",
        "commandNames": [
          "gtwShieldArm"
        ]
      },
      {
        "id": "gtwShieldDisarm",
        "label": "GTW Shield Disarm",
        "control": "action",
        "commandNames": [
          "gtwShieldDisarm"
        ]
      },
      {
        "id": "gtwShieldReset",
        "label": "GTW Shield Reset",
        "control": "action",
        "commandNames": [
          "gtwShieldReset"
        ]
      }
    ]
  },
  "bmsTelemetry": {
    "featureId": "bmsTelemetry",
    "title": "BMS Telemetry",
    "kind": "query",
    "settings": [
      {
        "id": "bmsQuery",
        "label": "Read BMS Snapshot",
        "control": "query",
        "commandNames": [
          "bms"
        ]
      }
    ]
  },
  "canClock": {
    "featureId": "canClock",
    "title": "CAN Clock",
    "kind": "command",
    "settings": [
      {
        "id": "canClockMode",
        "label": "Clock Mode",
        "control": "enum",
        "commandNames": [
          "canClockAuto",
          "canClock"
        ],
        "options": [
          {
            "value": "auto",
            "label": "Auto"
          },
          {
            "value": "8",
            "label": "8 MHz"
          },
          {
            "value": "12",
            "label": "12 MHz"
          },
          {
            "value": "16",
            "label": "16 MHz"
          },
          {
            "value": "20",
            "label": "20 MHz"
          }
        ]
      }
    ],
    "notes": [
      "12 MHz is accepted as compatibility mode in firmware."
    ]
  },
  "rawCan": {
    "featureId": "rawCan",
    "title": "Raw CAN Stream",
    "kind": "command",
    "settings": [
      {
        "id": "rawCanEnabled",
        "label": "Listen Raw CAN",
        "control": "toggle",
        "commandNames": [
          "rawCan"
        ]
      }
    ]
  },
  "canSimulation": {
    "featureId": "canSimulation",
    "title": "CAN Simulation",
    "kind": "command",
    "settings": [
      {
        "id": "canSimStart",
        "label": "Start Simulator",
        "control": "action",
        "commandNames": [
          "canSimStart"
        ]
      },
      {
        "id": "canSimStop",
        "label": "Stop Simulator",
        "control": "action",
        "commandNames": [
          "canSimStop"
        ]
      }
    ]
  },
  "chargeControl": {
    "featureId": "chargeControl",
    "title": "Charge Control",
    "kind": "command",
    "settings": [
      {
        "id": "chargeStart",
        "label": "Start Charging",
        "control": "action",
        "commandNames": [
          "chargeStart"
        ]
      },
      {
        "id": "chargeStop",
        "label": "Stop Charging",
        "control": "action",
        "commandNames": [
          "chargeStop"
        ]
      },
      {
        "id": "chargePort",
        "label": "Toggle Charge Port",
        "control": "action",
        "commandNames": [
          "chargePort"
        ]
      }
    ]
  },
  "climate": {
    "featureId": "climate",
    "title": "Climate",
    "kind": "command",
    "settings": [
      {
        "id": "climateKeep",
        "label": "Keep Climate On",
        "control": "action",
        "commandNames": [
          "climateKeep"
        ]
      },
      {
        "id": "climateOff",
        "label": "Turn Climate Off",
        "control": "action",
        "commandNames": [
          "climateOff"
        ]
      }
    ]
  },
  "displayBrightness": {
    "featureId": "displayBrightness",
    "title": "Display Brightness",
    "kind": "command",
    "settings": [
      {
        "id": "mainDisplayLevel",
        "label": "Main Display Level",
        "control": "number",
        "commandNames": [
          "mainDisplay"
        ],
        "min": 0,
        "max": 127,
        "step": 1
      }
    ],
    "notes": [
      "Requires hasCtrl in firmware runtime."
    ]
  },
  "driveContext": {
    "featureId": "driveContext",
    "title": "Drive Context",
    "kind": "internal",
    "settings": [],
    "notes": [
      "No command handler; provides internal runtime context only."
    ]
  },
  "driveMode": {
    "featureId": "driveMode",
    "title": "Drive Mode Override",
    "kind": "command",
    "settings": [
      {
        "id": "driveMode",
        "label": "Drive Mode",
        "control": "enum",
        "commandNames": [
          "driveModeOff",
          "driveModeChill",
          "driveModeStandard",
          "driveModePerformance"
        ],
        "options": [
          {
            "value": "off",
            "label": "Off"
          },
          {
            "value": "chill",
            "label": "Chill"
          },
          {
            "value": "standard",
            "label": "Standard"
          },
          {
            "value": "performance",
            "label": "Performance"
          }
        ]
      }
    ]
  },
  "fsd": {
    "featureId": "fsd",
    "title": "FSD",
    "kind": "command",
    "settings": [
      {
        "id": "fsdEnabled",
        "label": "FSD Injection",
        "control": "toggle",
        "commandNames": [
          "fsd"
        ]
      },
      {
        "id": "fsdForceEnabled",
        "label": "Force FSD",
        "control": "toggle",
        "commandNames": [
          "fsdForce"
        ]
      },
      {
        "id": "enhancedAutopilot",
        "label": "Enhanced Autopilot",
        "control": "toggle",
        "commandNames": [
          "eap"
        ]
      },
      {
        "id": "emergencyVehicleDetection",
        "label": "Emergency Vehicle Detection",
        "control": "toggle",
        "commandNames": [
          "evd"
        ]
      }
    ]
  },
  "fwCompat": {
    "featureId": "fwCompat",
    "title": "Firmware Compatibility",
    "kind": "query",
    "settings": [
      {
        "id": "fwCompatQuery",
        "label": "Read FW Compatibility",
        "control": "query",
        "commandNames": [
          "fwCompat"
        ]
      }
    ]
  },
  "isaChime": {
    "featureId": "isaChime",
    "title": "ISA Chime",
    "kind": "command",
    "settings": [
      {
        "id": "isaChimeSuppress",
        "label": "Suppress ISA Chime",
        "control": "toggle",
        "commandNames": [
          "isaChime"
        ]
      }
    ],
    "notes": [
      "Feature is HW4-only in firmware."
    ]
  },
  "lights": {
    "featureId": "lights",
    "title": "Lighting",
    "kind": "command",
    "settings": [
      {
        "id": "lightAction",
        "label": "Light Action",
        "control": "enum",
        "commandNames": [
          "lightFogFront",
          "lightFogRear",
          "lightHighbeamAuto",
          "lightAmbient",
          "lightHome",
          "lightDomeOff",
          "lightDomeOn",
          "lightDomeAuto"
        ],
        "options": [
          {
            "value": "fogFront",
            "label": "Fog Front"
          },
          {
            "value": "fogRear",
            "label": "Fog Rear"
          },
          {
            "value": "highbeamAuto",
            "label": "Highbeam Auto"
          },
          {
            "value": "ambient",
            "label": "Ambient"
          },
          {
            "value": "home",
            "label": "Coming Home"
          },
          {
            "value": "domeOff",
            "label": "Dome Off"
          },
          {
            "value": "domeOn",
            "label": "Dome On"
          },
          {
            "value": "domeAuto",
            "label": "Dome Auto"
          }
        ]
      }
    ]
  },
  "lockControl": {
    "featureId": "lockControl",
    "title": "Lock and Horn",
    "kind": "command",
    "settings": [
      {
        "id": "lockAction",
        "label": "Lock Action",
        "control": "enum",
        "commandNames": [
          "lock",
          "unlock",
          "lockChild",
          "horn"
        ],
        "options": [
          {
            "value": "lock",
            "label": "Lock"
          },
          {
            "value": "unlock",
            "label": "Unlock"
          },
          {
            "value": "child",
            "label": "Child Lock"
          },
          {
            "value": "horn",
            "label": "Horn"
          }
        ]
      }
    ]
  },
  "mirrorControl": {
    "featureId": "mirrorControl",
    "title": "Mirrors",
    "kind": "command",
    "settings": [
      {
        "id": "mirrorAction",
        "label": "Mirror Action",
        "control": "enum",
        "commandNames": [
          "mirrorFold",
          "mirrorUnfold",
          "mirrorHeat",
          "mirrorAutofold",
          "mirrorDip"
        ],
        "options": [
          {
            "value": "fold",
            "label": "Fold"
          },
          {
            "value": "unfold",
            "label": "Unfold"
          },
          {
            "value": "heat",
            "label": "Heat"
          },
          {
            "value": "autofoldPulse",
            "label": "Auto-fold Pulse"
          },
          {
            "value": "dip",
            "label": "Dip"
          }
        ]
      },
      {
        "id": "mirrorAutofoldPersist",
        "label": "Auto-fold on Lock",
        "control": "toggle",
        "commandNames": [
          "mirrorAutoFold"
        ]
      }
    ]
  },
  "mqttBridge": {
    "featureId": "mqttBridge",
    "title": "MQTT Bridge",
    "kind": "command",
    "settings": [
      {
        "id": "mqttEnabled",
        "label": "Enable MQTT",
        "control": "toggle",
        "commandNames": [
          "mqtt"
        ]
      },
      {
        "id": "mqttBroker",
        "label": "Broker Host",
        "control": "text",
        "commandNames": [
          "mqttBroker"
        ],
        "minLength": 1,
        "maxLength": 63
      },
      {
        "id": "mqttPort",
        "label": "Broker Port",
        "control": "number",
        "commandNames": [
          "mqttPort"
        ],
        "min": 1,
        "max": 65535
      },
      {
        "id": "mqttInterval",
        "label": "Publish Interval",
        "control": "number",
        "commandNames": [
          "mqttInterval"
        ],
        "min": 100,
        "max": 60000,
        "step": 100,
        "unit": "ms"
      }
    ]
  },
  "nagControl": {
    "featureId": "nagControl",
    "title": "Nag Control",
    "kind": "command",
    "settings": [
      {
        "id": "nagSuppress",
        "label": "Suppress Nag",
        "control": "toggle",
        "commandNames": [
          "nag"
        ]
      },
      {
        "id": "nagKillerEnabled",
        "label": "Enable Nag Killer",
        "control": "toggle",
        "commandNames": [
          "nagKiller"
        ]
      },
      {
        "id": "nagKillerMode",
        "label": "Nag Killer Mode",
        "control": "enum",
        "commandNames": [
          "nagKillerMode"
        ],
        "options": [
          {
            "value": "legacy",
            "label": "Legacy"
          },
          {
            "value": "safe",
            "label": "Safe"
          },
          {
            "value": "natural",
            "label": "Natural (Random Jitter)"
          }
        ]
      }
    ]
  },
  "speedOffset": {
    "featureId": "speedOffset",
    "title": "Speed Offset",
    "kind": "command",
    "settings": [
      {
        "id": "offset",
        "label": "Offset",
        "control": "number",
        "commandNames": [
          "offset"
        ],
        "min": 0,
        "max": 100,
        "step": 1
      },
      {
        "id": "offsetAuto",
        "label": "Offset Auto",
        "control": "action",
        "commandNames": [
          "offsetAuto"
        ]
      },
      {
        "id": "offsetOff",
        "label": "Offset Off",
        "control": "action",
        "commandNames": [
          "offsetOff"
        ]
      }
    ],
    "notes": [
      "Firmware constrains max offset by hardware variant at runtime."
    ]
  },
  "pedalMode": {
    "featureId": "pedalMode",
    "title": "Pedal Mode",
    "kind": "command",
    "settings": [
      {
        "id": "pedalMode",
        "label": "Pedal Response",
        "control": "enum",
        "commandNames": [
          "pedalStandard",
          "pedalChill",
          "pedalSport"
        ],
        "options": [
          {
            "value": "standard",
            "label": "Standard"
          },
          {
            "value": "chill",
            "label": "Chill"
          },
          {
            "value": "sport",
            "label": "Sport"
          }
        ]
      }
    ]
  },
  "powerState": {
    "featureId": "powerState",
    "title": "Power State",
    "kind": "command",
    "settings": [
      {
        "id": "powerStateAction",
        "label": "Power Action",
        "control": "enum",
        "commandNames": [
          "powerAccOn",
          "powerAccOff",
          "powerReady",
          "powerOff"
        ],
        "options": [
          {
            "value": "accOn",
            "label": "ACC On"
          },
          {
            "value": "accOff",
            "label": "ACC Off"
          },
          {
            "value": "ready",
            "label": "Ready"
          },
          {
            "value": "off",
            "label": "Power Off"
          }
        ]
      }
    ]
  },
  "powertrainTelemetry": {
    "featureId": "powertrainTelemetry",
    "title": "Powertrain Telemetry",
    "kind": "query",
    "settings": [
      {
        "id": "powertrainQuery",
        "label": "Read Powertrain Snapshot",
        "control": "query",
        "commandNames": [
          "powertrain"
        ]
      }
    ]
  },
  "precondition": {
    "featureId": "precondition",
    "title": "Precondition",
    "kind": "command",
    "settings": [
      {
        "id": "preconditionEnabled",
        "label": "Enable Preconditioning",
        "control": "toggle",
        "commandNames": [
          "precondition"
        ]
      }
    ]
  },
  "speedProfile": {
    "featureId": "speedProfile",
    "title": "Speed Profile",
    "kind": "command",
    "settings": [
      {
        "id": "profile",
        "label": "Profile Level",
        "control": "number",
        "commandNames": [
          "profile"
        ],
        "min": 0,
        "max": 4,
        "step": 1
      },
      {
        "id": "profileOverride",
        "label": "Override Mode",
        "control": "enum",
        "commandNames": [
          "profileAuto",
          "profileLock",
          "profileUnlock"
        ],
        "options": [
          {
            "value": "auto",
            "label": "Auto"
          },
          {
            "value": "lock",
            "label": "Lock"
          },
          {
            "value": "unlock",
            "label": "Unlock"
          }
        ]
      }
    ],
    "notes": [
      "Firmware also accepts profile aliases via sp:."
    ]
  },
  "regenMode": {
    "featureId": "regenMode",
    "title": "Regeneration Mode",
    "kind": "command",
    "settings": [
      {
        "id": "regenMode",
        "label": "Regeneration",
        "control": "enum",
        "commandNames": [
          "regenOff",
          "regenLow",
          "regenStd",
          "regenMax"
        ],
        "options": [
          {
            "value": "off",
            "label": "Off"
          },
          {
            "value": "low",
            "label": "Low"
          },
          {
            "value": "std",
            "label": "Standard"
          },
          {
            "value": "max",
            "label": "Max"
          }
        ]
      }
    ],
    "notes": [
      "Firmware accepts regen:standard alias in addition to regen:std."
    ]
  },
  "regionCodec": {
    "featureId": "regionCodec",
    "title": "Region Codec",
    "kind": "internal",
    "settings": [],
    "notes": [
      "No direct command handler in this header; mapping helper only."
    ]
  },
  "regionSpoof": {
    "featureId": "regionSpoof",
    "title": "Region Spoof",
    "kind": "command",
    "settings": [
      {
        "id": "spoofedRegion",
        "label": "Spoofed Region",
        "control": "enum",
        "commandNames": [
          "regionSpoof"
        ],
        "options": [
          { "value": "off", "label": "Off (use real region)" },
          { "value": "na", "label": "North America" },
          { "value": "eu", "label": "Europe" },
          { "value": "cn", "label": "China" },
          { "value": "apac", "label": "Asia Pacific" },
          { "value": "me", "label": "Middle East" }
        ]
      }
    ],
    "notes": [
      "Spoof region detection to bypass region-locked features. Use with caution."
    ]
  },
  "alc": {
    "featureId": "alc",
    "title": "Auto Lane Change Auto-Confirm",
    "kind": "command",
    "settings": [
      {
        "id": "alcAutoConfirm",
        "label": "Auto-confirm lane changes",
        "control": "toggle",
        "commandNames": [
          "alc"
        ]
      }
    ],
    "notes": [
      "Automatically confirms turn-signal-initiated lane changes by injecting steering wheel torque."
    ]
  },
  "seatHeating": {
    "featureId": "seatHeating",
    "title": "Seat Heating",
    "kind": "command",
    "settings": [
      {
        "id": "seatFrontLeft",
        "label": "Front Left Seat",
        "control": "number",
        "commandNames": [
          "seatFL"
        ],
        "min": 0,
        "max": 3,
        "step": 1
      },
      {
        "id": "seatFrontRight",
        "label": "Front Right Seat",
        "control": "number",
        "commandNames": [
          "seatFR"
        ],
        "min": 0,
        "max": 3,
        "step": 1
      },
      {
        "id": "seatRearLeft",
        "label": "Rear Left Seat",
        "control": "number",
        "commandNames": [
          "seatRL"
        ],
        "min": 0,
        "max": 3,
        "step": 1
      },
      {
        "id": "seatRearRight",
        "label": "Rear Right Seat",
        "control": "number",
        "commandNames": [
          "seatRR"
        ],
        "min": 0,
        "max": 3,
        "step": 1
      },
      {
        "id": "seatRearCenter",
        "label": "Rear Center Seat",
        "control": "number",
        "commandNames": [
          "seatRC"
        ],
        "min": 0,
        "max": 3,
        "step": 1
      }
    ]
  },
  "seatbeltEmulation": {
    "featureId": "seatbeltEmulation",
    "title": "Rear Seatbelt Emulation",
    "kind": "command",
    "settings": [
      {
        "id": "seatbeltEmulation",
        "label": "Seatbelt Emulation",
        "control": "toggle",
        "commandNames": [
          "seatbelt"
        ]
      }
    ]
  },
  "sentry": {
    "featureId": "sentry",
    "title": "Sentry Mode",
    "kind": "command",
    "settings": [
      {
        "id": "sentryMode",
        "label": "Sentry Mode",
        "control": "toggle",
        "commandNames": [
          "sentryOn",
          "sentryOff"
        ],
        "note": "Use on/off actions in UI wiring."
      }
    ]
  },
  "singleShotTx": {
    "featureId": "singleShotTx",
    "title": "Single-shot TX",
    "kind": "command",
    "settings": [
      {
        "id": "singleShotTx",
        "label": "Single-shot Transmission",
        "control": "toggle",
        "commandNames": [
          "singleShot"
        ]
      }
    ]
  },
  "stopMode": {
    "featureId": "stopMode",
    "title": "Stop Mode",
    "kind": "command",
    "settings": [
      {
        "id": "stopMode",
        "label": "Stop Behavior",
        "control": "enum",
        "commandNames": [
          "stopCreep",
          "stopRoll",
          "stopHold"
        ],
        "options": [
          {
            "value": "creep",
            "label": "Creep"
          },
          {
            "value": "roll",
            "label": "Roll"
          },
          {
            "value": "hold",
            "label": "Hold"
          }
        ]
      }
    ]
  },
  "stream": {
    "featureId": "stream",
    "title": "Stream",
    "kind": "command",
    "settings": [
      {
        "id": "streamEnabled",
        "label": "Enable Stream",
        "control": "toggle",
        "commandNames": [
          "stream"
        ]
      }
    ]
  },
  "summon": {
    "featureId": "summon",
    "title": "Summon",
    "kind": "command",
    "settings": [
      {
        "id": "summonInject",
        "label": "Enable Summon Injection",
        "control": "toggle",
        "commandNames": [
          "summonInject"
        ]
      },
      {
        "id": "summonDirection",
        "label": "Summon Direction",
        "control": "enum",
        "commandNames": [
          "summon",
          "summonForward",
          "summonReverse",
          "summonStop"
        ],
        "options": [
          {
            "value": "start",
            "label": "Start (current direction)"
          },
          {
            "value": "forward",
            "label": "Forward"
          },
          {
            "value": "reverse",
            "label": "Reverse"
          },
          {
            "value": "stop",
            "label": "Stop"
          }
        ]
      }
    ],
    "notes": [
      "Firmware gate requires summonInject and hasCtrl for active movement."
    ]
  },
  "tlsscRestore": {
    "featureId": "tlsscRestore",
    "title": "TLSSC Restore",
    "kind": "command",
    "settings": [
      {
        "id": "tlsscRestore",
        "label": "TLSSC Restore",
        "control": "toggle",
        "commandNames": [
          "tlssc"
        ]
      }
    ]
  },
  "tpmsTelemetry": {
    "featureId": "tpmsTelemetry",
    "title": "TPMS",
    "kind": "query",
    "settings": [
      {
        "id": "tpmsQuery",
        "label": "Read TPMS Snapshot",
        "control": "query",
        "commandNames": [
          "tpms"
        ]
      }
    ]
  },
  "trackMode": {
    "featureId": "trackMode",
    "title": "Track Mode",
    "kind": "command",
    "settings": [
      {
        "id": "trackModeEnabled",
        "label": "Track Mode",
        "control": "toggle",
        "commandNames": [
          "trackMode"
        ]
      }
    ],
    "notes": [
      "Legacy variant rejects track mode commands in firmware."
    ]
  },
  "trunkFrunk": {
    "featureId": "trunkFrunk",
    "title": "Trunk and Frunk",
    "kind": "command",
    "settings": [
      {
        "id": "trunkAction",
        "label": "Compartment Action",
        "control": "enum",
        "commandNames": [
          "frunkOpen",
          "frunkClose",
          "trunkOpen",
          "trunkClose",
          "glovebox"
        ],
        "options": [
          {
            "value": "frunkOpen",
            "label": "Open Frunk"
          },
          {
            "value": "frunkClose",
            "label": "Close Frunk"
          },
          {
            "value": "trunkOpen",
            "label": "Open Trunk"
          },
          {
            "value": "trunkClose",
            "label": "Close Trunk"
          },
          {
            "value": "glovebox",
            "label": "Open Glovebox"
          }
        ]
      }
    ]
  },
  "turnSignal": {
    "featureId": "turnSignal",
    "title": "Turn Signals",
    "kind": "command",
    "settings": [
      {
        "id": "turnSignalAction",
        "label": "Signal Action",
        "control": "enum",
        "commandNames": [
          "turnLeft3",
          "turnRight3",
          "turnHazard",
          "turnOff"
        ],
        "options": [
          {
            "value": "left3",
            "label": "Left 3-blink"
          },
          {
            "value": "right3",
            "label": "Right 3-blink"
          },
          {
            "value": "hazard",
            "label": "Hazard"
          },
          {
            "value": "off",
            "label": "Off"
          }
        ]
      }
    ]
  },
  "variant": {
    "featureId": "variant",
    "title": "Variant",
    "kind": "command",
    "settings": [
      {
        "id": "variant",
        "label": "Firmware Variant",
        "control": "enum",
        "commandNames": [
          "variant"
        ],
        "options": [
          {
            "value": "auto",
            "label": "Auto"
          },
          {
            "value": "hw3",
            "label": "HW3"
          },
          {
            "value": "hw4",
            "label": "HW4"
          },
          {
            "value": "legacy",
            "label": "Legacy"
          }
        ]
      }
    ]
  },
  "vehicleConfig": {
    "featureId": "vehicleConfig",
    "title": "Vehicle Config",
    "kind": "query",
    "settings": [
      {
        "id": "vehicleConfigQuery",
        "label": "Read Vehicle Config",
        "control": "query",
        "commandNames": [
          "vehicle"
        ]
      }
    ]
  },
  "windowVent": {
    "featureId": "windowVent",
    "title": "Window Vent",
    "kind": "command",
    "settings": [
      {
        "id": "windowVentPreset",
        "label": "Vent Preset",
        "control": "enum",
        "commandNames": [
          "windowVentOpen",
          "windowVentClose",
          "ventOpen",
          "ventClose"
        ],
        "options": [
          {
            "value": "open",
            "label": "Open"
          },
          {
            "value": "close",
            "label": "Close"
          }
        ]
      },
      {
        "id": "windowVentPosition",
        "label": "Vent Position",
        "control": "number",
        "commandNames": [
          "windowVent"
        ],
        "min": 0,
        "max": 100,
        "step": 1,
        "unit": "%"
      }
    ],
    "notes": [
      "Legacy variant rejects window vent commands in firmware."
    ]
  },
  "wiper": {
    "featureId": "wiper",
    "title": "Wiper",
    "kind": "command",
    "settings": [
      {
        "id": "wiperSpeed",
        "label": "Wiper Speed",
        "control": "enum",
        "commandNames": [
          "wiperOff",
          "wiper1",
          "wiper2",
          "wiper3"
        ],
        "options": [
          {
            "value": "off",
            "label": "Off"
          },
          {
            "value": "1",
            "label": "Speed 1"
          },
          {
            "value": "2",
            "label": "Speed 2"
          },
          {
            "value": "3",
            "label": "Speed 3"
          }
        ]
      },
      {
        "id": "wiperPersist",
        "label": "Persist Wiper Speed",
        "control": "toggle",
        "commandNames": [
          "wiperPersist"
        ]
      }
    ]
  }
} as const satisfies Record<string, FeatureSettingsSpec>;

export const ALL_FEATURE_SETTINGS_SPECS: readonly FeatureSettingsSpec[] = Object.freeze(
  Object.values(FEATURE_SETTINGS_BY_ID_INTERNAL),
);

export const FEATURE_IDS: readonly FeatureId[] = Object.freeze(
  ALL_FEATURE_SETTINGS_SPECS.map((spec) => spec.featureId),
) as readonly FeatureId[];

export const FEATURE_SETTINGS_BY_ID: Readonly<Record<string, FeatureSettingsSpec>> = Object.freeze(
  FEATURE_SETTINGS_BY_ID_INTERNAL,
);

export function getFeatureSettingsSpecById(featureId: FeatureId): FeatureSettingsSpec | undefined {
  return FEATURE_SETTINGS_BY_ID[featureId];
}
