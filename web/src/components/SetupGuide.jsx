import { useState } from 'react';
import {
  Bluetooth,
  Car,
  CheckCheck,
  CheckCircle2,
  Circle,
  Cpu,
  ListChecks,
  RotateCcw,
  ShieldAlert,
  ToggleLeft,
  ToggleRight,
  Usb,
  Wrench,
  Zap,
} from 'lucide-react';
import MermaidDiagram from './MermaidDiagram';
import { getBoardBrowserCapabilities } from '../lib/board/capabilities';

const ORDERED_KIT = {
  board: 'Arduino Uno R3 SMD CH340',
  can: 'MCP2515 + TJA1050 (8 MHz crystal)',
  bluetooth: 'HC-05',
  firmware: 'Single Uno firmware with runtime-selectable vehicle variant',
};

const FIRST_FLASH_BUILD_COMMAND = `cd hardware
pio run -e uno_usb`;
const FIRST_FLASH_HEX_PATH = 'hardware/.pio/build/uno_usb/firmware.hex';
const FIRST_FLASH_BT_BUILD_COMMAND = `cd hardware
pio run -e uno`;
const FIRST_FLASH_BT_HEX_PATH = 'hardware/.pio/build/uno/firmware.hex';

const PHASE_CARDS = [
  {
    id: 'bench',
    icon: Usb,
    eyebrow: 'Phase 1',
    title: 'Bench flash on USB',
    detail: 'PC USB powers the Uno first. Flash firmware, verify CH340 serial, and confirm the runtime vehicle variant before touching X179.',
  },
  {
    id: 'power',
    icon: Zap,
    eyebrow: 'Phase 2',
    title: 'Move power to X179',
    detail: 'X179 pin 1 and pin 20 feed the converter input, then the converter USB output becomes the Uno power source.',
  },
  {
    id: 'vehicle',
    icon: Car,
    eyebrow: 'Phase 3',
    title: 'Connect vehicle CAN last',
    detail: 'Only after USB and installed power are proven should X179 pin 13 and pin 14 be connected to the MCP2515 CAN pair.',
  },
];

const PREP_ITEMS = [
  {
    label: 'Required hardware',
    value: 'Uno R3 CH340, MCP2515/TJA1050 module, 9V-36V to 5V dual-USB 3A converter, jumper wires for short board links, USB cable',
  },
  {
    label: 'Optional hardware',
    value: 'HC-05, 3.3V regulator, 1k/2k divider on RX',
  },
  {
    label: 'PC/browser',
    value: 'Chrome or Edge with Web Serial support and a stable USB port',
  },
  {
    label: 'Before touching vehicle',
    value: 'Know X179 pins 1, 13, 14, and 20, remove CAN termination on the MCP2515, disconnect vehicle power',
  },
];

const SAFETY_RULES = [
  'Start on the bench with the Uno powered only from USB. Do not begin the first flash on the vehicle power branch.',
  'Installed power comes from X179 pin 1 (+12V) and pin 20 (GND) into the converter input, then from the converter USB output into the Uno USB port.',
  'Use USB for flashing and first bring-up, not as the final installed power source.',
  'Do not use loose jumper wires for X179 or converter input in the final install. Use proper screwed, crimped, or otherwise secured wiring there.',
  'Do not feed the ordered HC-05 from raw 5V. Treat it as a 3.3V device.',
  'Do not leave the MCP2515 termination resistor enabled on the live vehicle bus.',
  'Disconnect the vehicle low-voltage system before tapping X179 or any alternate CAN point.',
];

const GUIDE_MERMAID_CLASSES = String.raw`
  classDef hostTone fill:#10263e,stroke:#60a5fa,color:#eef0f6,stroke-width:2px;
  classDef boardTone fill:#1b2433,stroke:#94a3b8,color:#eef0f6,stroke-width:2px;
  classDef powerTone fill:#3b1c1c,stroke:#f87171,color:#eef0f6,stroke-width:2px;
  classDef vehicleTone fill:#173227,stroke:#34d399,color:#eef0f6,stroke-width:2px;
  classDef canTone fill:#2c1f4d,stroke:#c4b5fd,color:#eef0f6,stroke-width:2px;
  classDef bluetoothTone fill:#12352f,stroke:#34d399,color:#eef0f6,stroke-width:2px;
  classDef noteTone fill:#2b2112,stroke:#fbbf24,color:#eef0f6,stroke-width:2px;
  classDef neutralTone fill:#131720,stroke:#64748b,color:#eef0f6,stroke-width:2px;
`;

function buildGuideMermaid(direction, body) {
  return `flowchart ${direction}
${body}

${GUIDE_MERMAID_CLASSES}`.trim();
}

const HC05_MERMAID = buildGuideMermaid(
  'LR',
  String.raw`
  subgraph HOST["Browser / OS Pairing"]
    APP["TeslaCANModder Web Client"]
    COM["Paired Serial COM Port"]
    USB["USB / CH340"]
  end

  subgraph UNO["Arduino Uno (5V Logic)"]
    UTX["D5<br/>SoftwareSerial TX"]
    URX["D4<br/>SoftwareSerial RX"]
    U5["5V Rail"]
    UG["GND Rail"]
  end

  subgraph DIV["Arduino TX -> HC-05 RX Protection"]
    R1["R1 1kΩ"]
    NODE["Divider Node<br/>~3.3V"]
    R2["R2 2kΩ to GND"]
  end

  subgraph PWR["Bluetooth Power Conditioning"]
    REG["3.3V Regulator"]
    HV["HC-05 VCC"]
    HG["HC-05 GND"]
  end

  subgraph HC05["HC-05 Module"]
    HRX["RXD<br/>3.3V Logic Input"]
    HTX["TXD<br/>3.3V Logic Output"]
  end

  APP -->|Open board console| COM
  COM -.->|Bench setup or optional runtime link| USB
  UTX -->|5V UART TX| R1
  R1 --> NODE
  NODE -->|~3.3V UART RX| HRX
  NODE --> R2
  R2 -->|Reference ground| UG
  HTX -->|Direct UART TX| URX
  U5 -->|Feed regulator input| REG
  REG -->|3.3V rail| HV
  UG -->|Common ground| HG

  class APP,COM,USB hostTone;
  class UTX,URX,U5,UG boardTone;
  class R1,NODE,R2 canTone;
  class REG powerTone;
  class HV,HG,HRX,HTX bluetoothTone;
`,
);

const BENCH_POWER_MERMAID = buildGuideMermaid(
  'LR',
  String.raw`
  subgraph HOST["Bench Session"]
    PC["Laptop / Browser"]
    WEB["TeslaCANModder<br/>Flasher + Dashboard"]
    CABLE["USB-A to USB-B Cable"]
  end

  subgraph UNO["Arduino Uno R3 CH340"]
    UPORT["USB Port / CH340"]
    U5["5V Rail"]
    UG["GND Rail"]
  end

  subgraph MCP["MCP2515 + TJA1050"]
    MV["VCC"]
    MG["GND"]
    BUS["CAN-H / CAN-L"]
  end

  HOLD["Leave vehicle wiring disconnected in phase 1"]:::noteTone

  PC --> WEB --> CABLE
  CABLE -->|Power + serial| UPORT
  U5 -->|Bench 5V feed| MV
  UG -->|Bench ground| MG
  BUS -.->|No car connection yet| HOLD

  class PC,WEB,CABLE hostTone;
  class UPORT,U5,UG boardTone;
  class MV,MG neutralTone;
  class BUS canTone;
`,
);

function getPowerRouteMermaid(includeBluetooth) {
  return buildGuideMermaid(
    'LR',
    String.raw`
  subgraph X179["Vehicle Socket / X179"]
    X12["Pin 1<br/>+12V"]
    XG["Pin 20<br/>GND"]
  end

  subgraph CONV["9V-36V -> 5V Converter"]
    VINP["VIN+"]
    VING["VIN-"]
    USB5["USB 5V Output"]
  end

  subgraph UNO["Arduino Uno R3 CH340"]
    UUSB["USB Port / CH340"]
    U5["5V Rail"]
    UG["GND Rail"]
  end

  subgraph MCP["MCP2515"]
    MV["VCC"]
    MG["GND"]
  end
${includeBluetooth
  ? String.raw`

  subgraph BT["Optional Bluetooth Power"]
    REG["3.3V Regulator"]
    BV["HC-05 VCC"]
    BG["HC-05 GND"]
  end`
  : ''}

  WARN["Never feed X179 directly into the Uno 5V pin"]:::noteTone

  X12 -->|Installed power| VINP
  XG -->|Vehicle return| VING
  USB5 -->|USB-A to USB-B| UUSB
  U5 -->|5V module feed| MV
  UG -->|Shared ground| MG
  X12 -.-> WARN
${includeBluetooth
  ? String.raw`
  USB5 -->|Regulator input| REG
  REG -->|3.3V rail| BV
  UG -->|Shared ground| BG`
  : ''}

  class X12,XG vehicleTone;
  class VINP,VING,USB5 powerTone;
  class UUSB,U5,UG boardTone;
  class MV,MG neutralTone;
${includeBluetooth
  ? String.raw`  class REG powerTone;
  class BV,BG bluetoothTone;`
  : ''}
`,
  );
}

function getInstallFlowMermaid(includeBluetooth) {
  return buildGuideMermaid(
    'TB',
    String.raw`
  subgraph PHASE1["Phase 1: Bench Validation"]
    A["1. USB flash on the desk"]
    B["2. Confirm board status + runtime variant"]
    A --> B
  end

  subgraph PHASE2["Phase 2: Installed Power"]
    C["3. Build X179 -> converter input branch"]
    D["4. Move Uno power to converter USB output"]
    C --> D
  end

  subgraph PHASE3["Phase 3: Vehicle Bus"]
    E["5. Connect X179 CAN to MCP2515"]
    F["6. First live X179-powered check"]
    E --> F
  end
${includeBluetooth
  ? String.raw`

  subgraph PHASE4["Phase 4: Optional Wireless Link"]
    G["7. Pair HC-05 only after the wired install is stable"]
  end`
  : ''}

  B --> C
  D --> E
${includeBluetooth ? String.raw`  F --> G` : ''}

  class A,B hostTone;
  class C,D powerTone;
  class E,F vehicleTone;
${includeBluetooth ? String.raw`  class G bluetoothTone;` : ''}
`,
  );
}

const CAN_MERMAID = buildGuideMermaid(
  'LR',
  String.raw`
  subgraph UNO["Arduino Uno R3"]
    D10["D10 / CS"]
    D11["D11 / MOSI"]
    D12["D12 / MISO"]
    D13["D13 / SCK"]
    D2["D2 / INT"]
    U5["5V"]
    UG["GND"]
  end

  subgraph MCP["MCP2515 + TJA1050"]
    CS["CS"]
    MOSI["SI / MOSI"]
    MISO["SO / MISO"]
    SCK["SCK"]
    INT["INT"]
    MV["VCC"]
    MG["GND"]
    TERM["120Ω termination"]
  end

  WARN["Disable module termination before connecting to vehicle CAN"]:::noteTone

  D10 --> CS
  D11 --> MOSI
  D12 --> MISO
  D13 --> SCK
  D2 --> INT
  U5 --> MV
  UG --> MG
  TERM -.-> WARN

  class D10,D11,D12,D13,D2,U5,UG boardTone;
  class CS,MOSI,MISO,SCK,INT canTone;
  class MV,MG neutralTone;
  class TERM noteTone;
`,
);

const X179_MERMAID = buildGuideMermaid(
  'LR',
  String.raw`
  subgraph X179["X179 Connector"]
    X1["Pin 1<br/>+12V"]
    X20["Pin 20<br/>GND"]
    X13["Pin 13<br/>CAN-H"]
    X14["Pin 14<br/>CAN-L"]
  end

  subgraph POWER["Installed Power Branch"]
    VINP["Converter VIN+"]
    VING["Converter VIN-"]
  end

  subgraph BOARD["Board CAN Side"]
    CH["MCP2515 CAN-H"]
    CL["MCP2515 CAN-L"]
  end

  REF["Verify these four pins against the photos below before cutting or splicing"]:::noteTone

  X1 -->|Power feed| VINP
  X20 -->|Ground return| VING
  X13 -->|High line| CH
  X14 -->|Low line| CL
  X13 -.-> REF

  class X1,X20,X13,X14 vehicleTone;
  class VINP,VING powerTone;
  class CH,CL canTone;
`,
);

const HC05_REFERENCE_LINKS = [
  {
    label: 'Arduino Uno R3 official docs',
    href: 'https://docs.arduino.cc/hardware/uno-rev3',
    note: 'Board-level reference for Uno USB connectivity and digital I/O capability.',
  },
  {
    label: 'HC-05 datasheet',
    href: 'https://www.alldatasheet.com/datasheet-pdf/pdf/1492299/ETC/HC-05.html',
    note: 'Electrical basis for the guide: the HTML summary lists low-power operation and 1.8V to 3.6V I/O.',
  },
  {
    label: 'Mermaid official docs',
    href: 'https://mermaid.js.org/intro/',
    note: 'Layout pattern used for the rendered schema and the copyable source block in this guide.',
  },
];

const getQuickReferenceRows = (includeBluetooth) => {
  const rows = [
    {
      role: 'Bench flash',
      source: 'PC USB port',
      destination: 'Uno USB / CH340',
      note: 'First power and first serial link',
    },
    {
      role: 'Installed power +12V',
      source: 'X179 pin 1',
      destination: 'Converter VIN+',
      note: 'Vehicle feed into the converter input',
    },
    {
      role: 'Installed power GND',
      source: 'X179 pin 20',
      destination: 'Converter VIN-',
      note: 'Vehicle return for the installed power branch',
    },
    {
      role: 'Regulated 5V output',
      source: 'Converter USB output',
      destination: 'Uno USB port',
      note: 'Permanent board power after bench validation',
    },
    {
      role: 'Vehicle CAN-H',
      source: 'X179 pin 13',
      destination: 'MCP2515 CAN-H',
      note: 'Final vehicle high line',
    },
    {
      role: 'Vehicle CAN-L',
      source: 'X179 pin 14',
      destination: 'MCP2515 CAN-L',
      note: 'Final vehicle low line',
    },
    {
      role: 'MCP2515 interrupt',
      source: 'MCP2515 INT',
      destination: 'Uno D2',
      note: 'Required interrupt line',
    },
    {
      role: 'MCP2515 chip select',
      source: 'MCP2515 CS',
      destination: 'Uno D10',
      note: 'SPI chip select',
    },
  ];

  if (includeBluetooth) {
    rows.push(
      {
        role: 'Bluetooth RX path',
        source: 'Uno D5',
        destination: 'HC-05 RXD through 1k/2k divider',
        note: 'Protects the HC-05 RX input from 5V logic',
      },
      {
        role: 'Bluetooth TX path',
        source: 'HC-05 TXD',
        destination: 'Uno D4',
        note: 'Direct 3.3V-to-Uno serial return path',
      },
    );
  }

  return rows;
};

function QuickPhaseCards({ includeBluetooth }) {
  const cards = includeBluetooth
    ? [
        ...PHASE_CARDS,
        {
          id: 'bluetooth',
          icon: Bluetooth,
          eyebrow: 'Phase 4',
          title: 'Add HC-05 only after stability',
          detail: 'Pair the HC-05 after the USB and X179-powered wired paths are already stable. It is a convenience link, not the primary bring-up path.',
        },
      ]
    : PHASE_CARDS;

  return (
    <div className="sg-phase-grid">
      {cards.map((card) => (
        <div key={card.id} className="sg-phase-card">
          <div className="sg-phase-icon">
            <card.icon size={18} />
          </div>
          <div className="sg-phase-copy">
            <span className="sg-box-label">{card.eyebrow}</span>
            <strong>{card.title}</strong>
            <p>{card.detail}</p>
          </div>
        </div>
      ))}
    </div>
  );
}

function QuickReferenceTable({ title, description, rows }) {
  return (
    <div className="panel sg-summary-card">
      <div className="sg-summary-head">
        <ListChecks size={18} />
        <h2>{title}</h2>
      </div>
      <p className="sg-table-intro">{description}</p>
      <div className="sg-table-wrap">
        <table className="sg-table">
          <thead>
            <tr>
              <th>Connection</th>
              <th>From</th>
              <th>To</th>
              <th>Purpose</th>
            </tr>
          </thead>
          <tbody>
            {rows.map((row) => (
              <tr key={`${row.role}-${row.source}-${row.destination}`}>
                <td>{row.role}</td>
                <td>{row.source}</td>
                <td>{row.destination}</td>
                <td>{row.note}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
}

function SectionConnectionTable({ title, rows }) {
  return (
    <div className="sg-connection-table">
      <div className="sg-detail-head">
        <ListChecks size={16} />
        <strong>{title}</strong>
      </div>
      <div className="sg-connection-list">
        {rows.map((row) => (
          <div key={`${row.source}-${row.destination}-${row.note}`} className="sg-connection-row">
            <div className="sg-connection-endpoints">
              <strong>{row.source}</strong>
              <span>{row.destination}</span>
            </div>
            <p>{row.note}</p>
          </div>
        ))}
      </div>
    </div>
  );
}

function GuideMermaidPanel({ title, subtitle, code, minHeight = 420 }) {
  return (
    <div className="sg-schema-panel">
      <div className="sg-schema-head">
        <strong>{title}</strong>
        {subtitle ? <span>{subtitle}</span> : null}
      </div>
      <MermaidDiagram code={code} minHeight={minHeight} />
    </div>
  );
}

function SourceBlock({ title, code }) {
  const [copied, setCopied] = useState(false);

  const handleCopy = async () => {
    try {
      await navigator.clipboard.writeText(code);
      setCopied(true);
      window.setTimeout(() => setCopied(false), 1500);
    } catch {
      setCopied(false);
    }
  };

  return (
    <details className="sg-source-details">
      <summary className="sg-source-summary">
        <span>{title}</span>
        <span className="sg-source-summary-note">Show raw code</span>
      </summary>
      <div className="config-block">
        <div className="config-block-header">
          <span>{title}</span>
          <button type="button" className={`btn-copy ${copied ? 'copied' : ''}`} onClick={handleCopy}>
            {copied ? 'Copied' : 'Copy'}
          </button>
        </div>
        <pre><code>{code}</code></pre>
      </div>
    </details>
  );
}

function FirstFlashPanel() {
  return (
    <div className="sg-mermaid-stack">
      <div className="sg-detail-card">
        <div className="sg-detail-head">
          <Usb size={16} />
          <strong>Canonical first-flash source</strong>
        </div>
        <div className="sg-bullet-list">
          <div className="sg-bullet-item">
            <span className="sg-bullet-dot sg-bullet-dot-ok" />
            <span>The first firmware image should come from the `hardware` PlatformIO project, not from an unrelated HEX file.</span>
          </div>
          <div className="sg-bullet-item">
            <span className="sg-bullet-dot sg-bullet-dot-ok" />
            <span>If HC-05 is not physically installed, build `uno_usb` and flash that USB-only image first.</span>
          </div>
          <div className="sg-bullet-item">
            <span className="sg-bullet-dot sg-bullet-dot-ok" />
            <span>Use the Bluetooth-enabled `uno` image only when HC-05 is really wired, powered from 3.3V, and intentionally part of the install.</span>
          </div>
        </div>
      </div>
      <SourceBlock title="Recommended first flash build (no HC-05 installed)" code={FIRST_FLASH_BUILD_COMMAND} />
      <SourceBlock title="Recommended first flash HEX path" code={FIRST_FLASH_HEX_PATH} />
      <SourceBlock title="Optional Bluetooth-enabled build (HC-05 installed only)" code={FIRST_FLASH_BT_BUILD_COMMAND} />
      <SourceBlock title="Bluetooth-enabled HEX path" code={FIRST_FLASH_BT_HEX_PATH} />
    </div>
  );
}

function ReferenceLinks({ links }) {
  return (
    <div className="sg-reference-links">
      {links.map((link) => (
        <a
          key={link.href}
          className="sg-reference-link"
          href={link.href}
          target="_blank"
          rel="noreferrer"
        >
          <strong>{link.label}</strong>
          <span>{link.note}</span>
        </a>
      ))}
    </div>
  );
}

const BenchPowerDiagram = () => (
  <GuideMermaidPanel
    title="Bench Flash Power Diagram"
    subtitle="Phase 1 keeps the entire project on the desk: USB powers the Uno, the browser handles flashing and diagnostics, and the vehicle side stays disconnected."
    code={BENCH_POWER_MERMAID}
    minHeight={360}
  />
);

const PowerDiagram = ({ includeBluetooth }) => (
  <GuideMermaidPanel
    title="Installed Power Routing Diagram"
    subtitle="Phase 2 moves the already-proven board from PC USB power to X179-fed regulated power, then fans that rail out to the Uno and modules."
    code={getPowerRouteMermaid(includeBluetooth)}
    minHeight={420}
  />
);

const InstallFlowDiagram = ({ includeBluetooth }) => (
  <GuideMermaidPanel
    title="Install Order Diagram"
    subtitle="The process stays USB-first and vehicle-second. Mermaid is now the primary explanation layer for that install sequence too."
    code={getInstallFlowMermaid(includeBluetooth)}
    minHeight={380}
  />
);

const BluetoothDiagram = () => (
  <GuideMermaidPanel
    title="HC-05 Wiring and Browser Transport"
    subtitle="This is the optional secondary link after the wired USB and X179 install paths are already stable."
    code={HC05_MERMAID}
    minHeight={460}
  />
);

const CANDiagram = () => (
  <GuideMermaidPanel
    title="Uno to MCP2515 SPI Wiring"
    subtitle="This is the board-side wiring map before the module reaches the vehicle CAN pair."
    code={CAN_MERMAID}
    minHeight={360}
  />
);

const X179Diagram = () => (
  <GuideMermaidPanel
    title="X179 Power and CAN Pins"
    subtitle="These four pins are the installed power and CAN references used by this Uno build."
    code={X179_MERMAID}
    minHeight={340}
  />
);

const BluetoothSchemaPanel = () => (
  <div className="sg-mermaid-stack">
    <div className="sg-detail-grid">
      <div className="sg-detail-card">
        <div className="sg-detail-head">
          <Bluetooth size={16} />
          <strong>Why this repo differs from many online samples</strong>
        </div>
        <div className="sg-bullet-list">
          <div className="sg-bullet-item">
            <span className="sg-bullet-dot sg-bullet-dot-ok" />
            <span>Many public HC-05 examples use Uno hardware serial pins D0 and D1 or alternate SoftwareSerial pins like D2 and D3.</span>
          </div>
          <div className="sg-bullet-item">
            <span className="sg-bullet-dot sg-bullet-dot-ok" />
            <span>This project intentionally uses D4 as SoftwareSerial RX and D5 as SoftwareSerial TX because the firmware is written around that mapping.</span>
          </div>
          <div className="sg-bullet-item">
            <span className="sg-bullet-dot sg-bullet-dot-ok" />
            <span>The raw Mermaid code below is kept in the guide so you can reuse the same diagram in markdown docs, issues, or build notes.</span>
          </div>
        </div>
      </div>

      <div className="sg-detail-card sg-detail-card-warn">
        <div className="sg-detail-head">
          <ShieldAlert size={16} />
          <strong>Electrical Rules Behind the Diagram</strong>
        </div>
        <div className="sg-bullet-list">
          <div className="sg-bullet-item">
            <span className="sg-bullet-dot sg-bullet-dot-warn" />
            <span>The HC-05 RX path is protected with a 1k/2k divider because the HC-05 I/O domain is not a raw 5V logic input.</span>
          </div>
          <div className="sg-bullet-item">
            <span className="sg-bullet-dot sg-bullet-dot-warn" />
            <span>The HC-05 TX path can feed the Uno RX side directly because 3.3V is high enough for the Uno digital input threshold in normal UART use.</span>
          </div>
          <div className="sg-bullet-item">
            <span className="sg-bullet-dot sg-bullet-dot-warn" />
            <span>This project keeps the ordered HC-05 on a dedicated 3.3V rail even though some breakout-board tutorials on the internet show 5V VCC.</span>
          </div>
        </div>
      </div>
    </div>

    <SourceBlock title="Raw Mermaid Source" code={HC05_MERMAID} />
    <ReferenceLinks links={HC05_REFERENCE_LINKS} />
  </div>
);

const CanDiagram = () => (
  <GuideMermaidPanel
    title="Uno to MCP2515 Wiring Diagram"
    subtitle="The SPI and interrupt links are explicit component-to-component connections now, instead of a flat route list."
    code={CAN_MERMAID}
    minHeight={420}
  />
);

const VehicleDiagram = () => (
  <GuideMermaidPanel
    title="X179 Mandatory Pins Diagram"
    subtitle="The final installed harness is reduced to four required X179 pins, shown as component-to-component destinations."
    code={X179_MERMAID}
    minHeight={360}
  />
);

const ReferenceGallery = ({ images }) => (
  <div className="sg-reference-grid">
    {images.map((image) => (
      <figure key={image.src} className="sg-reference-card">
        <img src={image.src} alt={image.alt} className="sg-reference-image" />
        <figcaption>{image.caption}</figcaption>
      </figure>
    ))}
  </div>
);

const getWiringSteps = (includeBluetooth) => {
  const steps = [];

  // The guide mirrors the real install order: prove the board on USB first,
  // then migrate the same hardware to X179 power and vehicle CAN.
  steps.push({
    id: 'bench',
    title: 'Bench Flash + USB Power',
    icon: Usb,
    colorClass: 'color-blue',
    description: 'Do the first firmware flash and the first health check on the desk with the Uno powered only from USB. Leave X179 and the vehicle disconnected in this phase.',
    summary: 'USB is the first power source. Do not move to X179 until flashing, status replies, and variant selection all work on the bench.',
    highlights: [
      'USB powers Uno first',
      'Flash firmware on the desk',
      'Check dashboard status over USB',
      'Vehicle stays disconnected',
    ],
    checks: [
      'The OS exposes the CH340 serial port over USB.',
      'The web flasher finishes before any vehicle wiring starts.',
      'The dashboard returns healthy board status over USB.',
      'The runtime vehicle variant can be changed before moving to X179.',
    ],
    complete: [
      'The Uno is running the shared hardware firmware from USB power.',
      'The dashboard can read status, change variant, and start or stop the stream over USB.',
      'The vehicle harness is still untouched while the board is already known-good.',
    ],
    mistakes: [
      'Starting the first flash from the X179 power branch.',
      'Moving to vehicle wiring before the USB path is proven.',
      'Trying HC-05 before the USB serial link already works.',
    ],
    connections: [
      {
        source: 'Laptop USB port',
        destination: 'Uno USB / CH340',
        note: 'Provides power and the first serial path for flashing and diagnostics.',
      },
      {
        source: 'Uno 5V',
        destination: 'MCP2515 VCC',
        note: 'Bench power for the CAN board while the vehicle side stays disconnected.',
      },
      {
        source: 'Uno GND',
        destination: 'MCP2515 GND',
        note: 'Shared reference for bench bring-up.',
      },
    ],
    diagram: BenchPowerDiagram,
    extra: FirstFlashPanel,
    items: [
      { id: 'u1', text: 'Connect the Uno to the PC with USB and let the CH340 serial port appear in the OS first.' },
      { id: 'u2', text: 'Keep X179 disconnected during the first flash and first dashboard session.' },
      { id: 'u3', text: 'Build the firmware from `hardware` and flash `hardware/.pio/build/uno_usb/firmware.hex` while the Uno is powered only from USB unless the HC-05 is already installed.' },
      { id: 'u4', text: 'Open the dashboard over USB and confirm the board answers status requests before moving on.' },
      { id: 'u5', text: 'Set the runtime vehicle variant you want to test so the board behavior is known before the vehicle install.' },
    ],
  });

  steps.push({
    id: 'power',
    title: 'Move Power to X179 + Buck Converter',
    icon: Zap,
    colorClass: 'color-amber',
    description: 'After the bench flash passes, build the installed power path from X179. Pin 1 and pin 20 feed a buck converter, and that regulated 5V rail replaces USB power for the Uno and MCP2515.',
    summary: 'This is phase 2. Move the already-proven board from PC USB power to the X179-fed converter, then feed the Uno through the converter USB output.',
    highlights: [
      'X179 pin 1 -> buck input',
      'X179 pin 20 -> buck ground',
      'Converter USB output -> Uno USB port',
      'Uno 5V/GND -> MCP2515',
      includeBluetooth ? 'HC-05 on separate 3.3V regulator' : 'USB stays as setup/debug only',
    ],
    checks: [
      'The board already passed flash and dashboard checks over USB.',
      'The converter output is verified at 5V before touching the Uno power path.',
      'No module gets hot during the first powered idle check.',
      'Ground continuity exists between X179, converter, Uno, and MCP2515.',
      includeBluetooth ? 'The HC-05 stays on a true 3.3V rail.' : 'The CAN path is stable before adding optional Bluetooth.',
    ],
    complete: [
      'The Uno boots from the converter USB output instead of PC bench USB power.',
      'The MCP2515 still powers correctly from the Uno 5V rail.',
      'The common ground path is stable before vehicle CAN is connected.',
    ],
    mistakes: [
      'Skipping the bench flash and going straight to X179 power.',
      'Feeding X179 pin 1 directly into the Uno 5V rail without regulation.',
      'Trying to push loose jumper wires straight into the converter input for a permanent install.',
      'Leaving module grounds isolated from each other.',
      'Assuming the USB cable should be the permanent power source.',
      'Feeding the ordered HC-05 from raw 5V.',
    ],
    connections: [
      {
        source: 'X179 pin 1',
        destination: 'Converter VIN+',
        note: 'Vehicle feed into the converter input terminal.',
      },
      {
        source: 'X179 pin 20',
        destination: 'Converter VIN-',
        note: 'Vehicle ground return for the installed power path.',
      },
      {
        source: 'Converter USB output',
        destination: 'Uno USB port',
        note: 'Regulated 5V path used after the bench USB phase is complete.',
      },
      {
        source: 'Uno 5V / GND',
        destination: 'MCP2515 VCC / GND',
        note: 'Shared 5V rail and common ground for the CAN controller.',
      },
      ...(includeBluetooth
        ? [
            {
              source: '3.3V regulator',
              destination: 'HC-05 VCC',
              note: 'Dedicated Bluetooth supply with proper voltage headroom.',
            },
          ]
        : []),
    ],
    diagram: () => <PowerDiagram includeBluetooth={includeBluetooth} />,
    items: [
      { id: 'p1', text: 'Disconnect the board from bench USB power before moving the power rail to the converter output.' },
      { id: 'p2', text: 'Connect X179 pin 1 to the converter 12V input and X179 pin 20 to the converter ground input using secured wiring on the input terminal.' },
      ...(includeBluetooth
        ? [{ id: 'p3', text: 'Power the HC-05 from a stable 3.3V rail or regulator. Do not assume the ordered module accepts raw 5V input.' }]
        : []),
      { id: includeBluetooth ? 'p4' : 'p3', text: 'Verify the converter output is a stable 5V before connecting it to the Arduino power path.' },
      { id: includeBluetooth ? 'p5' : 'p4', text: 'Use a USB-A to USB-B cable from the converter output to the Uno USB port for permanent board power.' },
      { id: includeBluetooth ? 'p6' : 'p5', text: `Power the MCP2515 from Arduino 5V and connect Arduino GND to ${includeBluetooth ? 'the HC-05 GND plus ' : ''}the MCP2515 GND so every module shares one reference.` },
      ...(includeBluetooth
        ? [{ id: 'p7', text: 'The ordered Uno clone exposes a 3.3V pin rated for 50 mA, and the ordered HC-05 lists 50 mA draw. A dedicated 3.3V regulator gives safer margin than running at that limit.' }]
        : [{ id: 'p6', text: 'Reconnect USB later only for configuration or diagnostics. The installed power rail should now come from X179.' }])
    ]
  });

  if (includeBluetooth) {
    steps.push({
      id: 'bluetooth',
      title: 'HC-05 Bluetooth Module',
      icon: Bluetooth,
      colorClass: 'color-blue',
      description: 'Optional wireless serial bridge for the board after both the USB bench phase and the X179-powered install are already stable. Pair it in the operating system first, then open the paired serial port from the web client.',
      summary: 'Treat Bluetooth as the last step, not the first debug path.',
      highlights: [
        'HC-05 TX -> Uno D4',
        'Uno D5 -> HC-05 RX through divider',
        'Pair in OS before using the web client',
      ],
      checks: [
        'The OS creates a usable paired serial port.',
        'RX/TX are crossed correctly and D5 uses a divider.',
        'The board already responds over USB before switching to HC-05.',
      ],
      complete: [
        'The paired HC-05 serial port can connect from the web app.',
        'USB remains the documented recovery path even though runtime control now works wirelessly.',
      ],
      mistakes: [
        'Connecting HC-05 RX directly to a 5V Uno TX pin.',
        'Trying to use Web Bluetooth instead of the paired serial port.',
        'Debugging Bluetooth before verifying the USB path.',
      ],
      connections: [
        {
          source: 'HC-05 TXD',
          destination: 'Uno D4',
          note: 'SoftwareSerial RX input for the board firmware.',
        },
        {
          source: 'Uno D5',
          destination: 'HC-05 RXD through 1k/2k divider',
          note: 'Drops the Uno TX level to a safer logic level for the HC-05 RX pin.',
        },
        {
          source: '3.3V regulator',
          destination: 'HC-05 VCC',
          note: 'Keeps the Bluetooth module off the raw 5V rail.',
        },
      ],
      diagram: BluetoothDiagram,
      extra: BluetoothSchemaPanel,
      items: [
        { id: 'b1', text: 'Connect HC-05 VCC to the dedicated 3.3V rail from the power step.' },
        { id: 'b2', text: 'Connect HC-05 TX to Arduino D4. The firmware listens on SoftwareSerial RX on that pin.' },
        { id: 'b3', text: 'Connect Arduino D5 to HC-05 RX through a 1k/2k voltage divider so the 5V Uno TX line does not overdrive the Bluetooth module input.' },
        { id: 'b4', text: 'Pair the HC-05 in your OS first, then pick the paired COM port from the web app Web Serial connect dialog.' },
        { id: 'b5', text: 'The ordered HC-05 lists roughly 10 m open-air range, so keep it close during initial setup and testing.' }
      ]
    });
  }

  steps.push({
    id: 'can',
    title: 'MCP2515 CAN Controller',
    icon: Cpu,
    colorClass: 'color-purple',
    description: 'SPI communication between the Uno and the ordered MCP2515/TJA1050 module. Firmware defaults now assume its 8 MHz crystal at 500 kbps.',
    summary: 'Bench-wire and verify the CAN controller early, then keep those same stable SPI connections when you move the board from USB power to X179 power.',
    highlights: [
      'D10 CS',
      'D11 MOSI',
      'D12 MISO',
      'D13 SCK',
      'D2 INT',
    ],
    checks: [
      'Every SPI line lands on the expected pin.',
      'The interrupt pin reaches D2 and is not floating.',
      'Termination is only enabled for isolated bench testing.',
      'Any jumper wires used here are short, secured, and protected from vibration if they stay in the enclosure.',
    ],
    complete: [
      'The CAN controller is powered and pinned exactly to the Uno SPI and interrupt layout expected by firmware.',
      'The module termination state is known before the board touches vehicle CAN.',
    ],
    mistakes: [
      'Confusing SI and SO labels on clone modules.',
      'Leaving the CAN termination resistor enabled on the vehicle bus.',
      'Using long unstable jumper wires on the SPI lines.',
      'Using jumper wires as the actual X179 or converter input harness.',
    ],
    connections: [
      { source: 'Uno D10', destination: 'MCP2515 CS', note: 'SPI chip-select line.' },
      { source: 'Uno D11', destination: 'MCP2515 SI / MOSI', note: 'SPI controller-out line.' },
      { source: 'Uno D12', destination: 'MCP2515 SO / MISO', note: 'SPI controller-in line.' },
      { source: 'Uno D13', destination: 'MCP2515 SCK', note: 'SPI clock line.' },
      { source: 'Uno D2', destination: 'MCP2515 INT', note: 'Interrupt line used by the firmware.' },
      { source: 'Uno 5V / GND', destination: 'MCP2515 VCC / GND', note: 'Module power and common reference.' },
    ],
    diagram: CanDiagram,
    items: [
      { id: 'c1', text: 'Connect MCP2515 VCC to 5V.' },
      { id: 'c2', text: 'Connect MCP2515 GND to GND.' },
      { id: 'c3', text: 'Connect MCP2515 CS to Arduino D10.' },
      { id: 'c4', text: 'Connect MCP2515 SO (MISO) to Arduino D12.' },
      { id: 'c5', text: 'Connect MCP2515 SI (MOSI) to Arduino D11.' },
      { id: 'c6', text: 'Connect MCP2515 SCK to Arduino D13.' },
      { id: 'c7', text: 'Connect MCP2515 INT to Arduino D2.' },
      { id: 'c8', text: 'Keep the SPI wiring short and solid. Long jumper wires make MCP2515 bring-up unreliable on Uno-class boards.' },
      { id: 'c9', text: 'If you keep jumper wires inside the final box, secure them so vibration cannot back them out of the headers.' },
      { id: 'c10', text: 'The ordered CAN board includes a 120 ohm termination resistor. Leave it in place only for isolated bench testing and remove or disable it before connecting to the vehicle bus.' }
    ]
  });

  steps.push({
    id: 'vehicle',
    title: 'X179 Vehicle CAN Bus Connection',
    icon: Car,
    colorClass: 'color-green',
    description: 'Use the real X179 connector pinout for Model 3/Y where available. This section assumes the same X179 socket also provides the installed power branch after the USB bench phase is already complete.',
    summary: 'The final install uses four X179 pins: 1 and 20 for power, 13 and 14 for CAN. This happens after the USB flash and bench validation.',
    highlights: [
      'X179 pin 1 -> +12V',
      'X179 pin 13 -> CAN-H',
      'X179 pin 14 -> CAN-L',
      'X179 pin 20 -> GND',
      'Use the reference images, not wire-color guesses',
    ],
    diagram: VehicleDiagram,
    referenceImages: [
      {
        src: '/reference/Tesla_X179_Pinout.png',
        alt: 'Tesla X179 connector reference',
        caption: 'X179 connector reference image',
      },
      {
        src: '/reference/X179_Connector_Pinout_Colored.png',
        alt: 'Colored X179 connector pinout',
        caption: 'Colored X179 connector pinout',
      },
      {
        src: '/reference/IMG_4364.jpg',
        alt: 'Real X179 connector location photo',
        caption: 'Real X179 connector photo in the car',
      },
      {
        src: '/reference/IMG_8019.jpg',
        alt: 'Example harness and module layout photo',
        caption: 'Example harness and module layout for the adapter build',
      },
    ],
    alerts: [
      'Always disconnect the vehicle 12V/16V battery and high voltage loop before splicing.',
      'Remove or disable the MCP2515 module termination resistor before connecting to the vehicle CAN bus.',
      'Do not draw vehicle power directly into the Uno 5V pin. Regulate X179 pin 1 down to a clean 5V rail first.',
    ],
    checks: [
      'CAN-H and CAN-L are identified from the connector pinout, not assumptions.',
      'X179 pin 1 and pin 20 are confirmed before building the power branch.',
      'The board is already known-good on the desk before touching the vehicle.',
      'The CAN module is unpowered while final splices are made.',
    ],
    complete: [
      'The final installed harness has X179 power and CAN landing on the intended four pins only.',
      'The board is ready for the first X179-powered live traffic validation run.',
    ],
    mistakes: [
      'Using the wrong X179 pins for power and ground.',
      'Reversing CAN-H and CAN-L at the final connection.',
      'Using loose jumper wires as the final X179 harness.',
      'Assuming every car has the same access path and connector layout.',
      'Connecting to the vehicle before the board already works on USB.',
    ],
    connections: [
      { source: 'X179 pin 1', destination: 'Converter VIN+', note: 'Installed +12V feed for the converter input.' },
      { source: 'X179 pin 20', destination: 'Converter VIN-', note: 'Installed ground return for the power branch.' },
      { source: 'X179 pin 13', destination: 'MCP2515 CAN-H', note: 'Vehicle CAN high line.' },
      { source: 'X179 pin 14', destination: 'MCP2515 CAN-L', note: 'Vehicle CAN low line.' },
    ],
    items: [
      { id: 'v1', text: 'Locate the X179 connector under the front passenger-side area on supported Model 3/Y vehicles.' },
      { id: 'v2', text: 'Use X179 pin 1 as +12V feed to the converter input and pin 20 as ground if you want the board powered from the socket.' },
      { id: 'v3', text: 'Use X179 pin 13 as CAN-H and X179 pin 14 as CAN-L.' },
      { id: 'v4', text: 'Connect X179 pin 13 to board CAN-H and pin 14 to CAN-L after confirming the reference images.' },
      { id: 'v5', text: 'Do not rely on loose jumper wires for the final X179 splices. Use a secured harness or properly retained connections.' },
      { id: 'v6', text: 'If the vehicle does not have X179, confirm the correct alternate connector and pinout before wiring.' }
    ]
  });

  return steps;
};

const getBringUpSteps = (includeBluetooth) => [
  {
    title: 'Power the Uno from USB on the bench',
    detail: 'Keep the vehicle disconnected and use USB as both the power source and serial path for the first session.',
  },
  {
    title: 'Flash firmware and confirm the board over USB',
    detail: 'Use the web flasher, then open the Dashboard tab over USB and make sure the board is healthy before moving to the vehicle harness.',
  },
  {
    title: 'Select the right vehicle variant on the desk',
    detail: 'Use the runtime variant controls from the dashboard instead of reflashing for every behavior change.',
  },
  {
    title: 'Build the X179 12V branch and regulate it to 5V',
    detail: 'Route X179 pin 1 and pin 20 into the converter input terminal and verify the 5V USB output before that power path touches the Uno.',
  },
  {
    title: includeBluetooth ? 'Move board power from USB to X179, then add HC-05 later if wanted' : 'Move board power from USB to X179 after USB validation',
    detail: includeBluetooth
      ? 'Once the converter output is clean, move Uno power from PC USB to the X179-fed converter USB output, then pair the HC-05 only after the installed system is stable.'
      : 'Once USB validation is stable, move Uno power from PC USB to the X179-fed converter USB output and keep USB as the service path when needed.',
  },
  {
    title: 'Connect X179 CAN and do the first live X179-powered check last',
    detail: 'After the installed power branch and CAN taps are complete, watch live traffic from the dashboard and confirm the expected runtime variant.',
  },
];

const getTroubleshooting = (includeBluetooth) => {
  const tips = [
    {
      title: 'Board does not appear in the browser',
      detail: 'Check the CH340 driver, USB cable quality, and whether the operating system sees a COM port before opening the web client.',
    },
    {
      title: 'Board boots on USB but not from the converter',
      detail: 'Measure the converter USB output before reconnecting the Uno, recheck X179 pin 1 and pin 20 polarity, and confirm the converter input wiring is secured rather than held by loose jumper leads.',
    },
    {
      title: 'MCP2515 never shows healthy traffic',
      detail: 'Recheck D10, D11, D12, D13, and D2 wiring plus module power and the termination resistor state.',
    },
    {
      title: 'Random resets after moving to X179 power',
      detail: 'Treat that as a power-path issue first: verify converter stability, shared ground quality, and that the Uno is being fed through its USB port instead of an unregulated 5V pin injection.',
    },
    {
      title: 'Traffic exists but behavior is wrong',
      detail: 'Confirm the active runtime vehicle variant matches the target behavior in the dashboard.',
    },
  ];

  if (includeBluetooth) {
    tips.push({
      title: 'HC-05 pairs but the app cannot talk to it',
      detail: 'Make sure the OS created a serial COM port and use that from Web Serial. The app does not use Web Bluetooth.',
    });
  }

  return tips;
};

const getSectionCounts = (section, completedSteps) => ({
  done: section.items.filter((item) => completedSteps[item.id]).length,
  total: section.items.length,
});

const getNextPendingItem = (activeSteps, completedSteps) => {
  for (const section of activeSteps) {
    const nextItem = section.items.find((item) => !completedSteps[item.id]);
    if (nextItem) {
      return {
        sectionTitle: section.title,
        itemText: nextItem.text,
      };
    }
  }

  return null;
};

export default function SetupGuide({ board }) {
  const [completedSteps, setCompletedSteps] = useState({});
  const [includeBluetooth, setIncludeBluetooth] = useState(true);
  const [wizardStepIndex, setWizardStepIndex] = useState(0);
  const capabilities = board?.capabilities || getBoardBrowserCapabilities();

  const activeSteps = getWiringSteps(includeBluetooth);
  const bringUpSteps = getBringUpSteps(includeBluetooth);
  const troubleshooting = getTroubleshooting(includeBluetooth);
  const quickReferenceRows = getQuickReferenceRows(includeBluetooth);

  const toggleStep = (itemId) => {
    setCompletedSteps(prev => ({
      ...prev,
      [itemId]: !prev[itemId]
    }));
  };

  const resetChecklist = () => {
    setCompletedSteps({});
  };

  const activeItemIds = new Set(activeSteps.flatMap((section) => section.items.map((item) => item.id)));
  const totalSteps = activeSteps.reduce((acc, section) => acc + section.items.length, 0);
  const completedCount = Object.keys(completedSteps).filter((id) => completedSteps[id] && activeItemIds.has(id)).length;
  const remainingCount = Math.max(0, totalSteps - completedCount);
  const progress = totalSteps === 0 ? 0 : Math.round((completedCount / totalSteps) * 100);
  const nextPending = getNextPendingItem(activeSteps, completedSteps);
  const browserRows = [
    {
      role: 'Desktop Chrome / Edge',
      source: 'Web Serial available',
      destination: 'Full flash + control + monitor',
      note: 'Primary first-flash and service path over USB.',
    },
    {
      role: 'Android Chrome',
      source: 'Web Serial available',
      destination: 'Runtime control + guide',
      note: 'Target phone path for the paired HC-05 serial link after the wired install is stable.',
    },
    {
      role: 'Other mobile browsers',
      source: 'Guide only fallback',
      destination: 'Install workflow + compatibility help',
      note: 'Do not rely on first flash or serial runtime control there.',
    },
  ];

  const wizardSteps = [
    {
      id: 'supported-hardware',
      title: 'Confirm Supported Hardware',
      purpose: 'Start by confirming the exact hardware this repo supports so the later wiring and controls actually match your build.',
      diagram: <InstallFlowDiagram includeBluetooth={includeBluetooth} />,
      rows: quickReferenceRows.slice(0, includeBluetooth ? 10 : 8),
      avoid: [
        'Do not start if your board is not the Uno CH340 + MCP2515 8 MHz path this repo targets.',
        'Do not assume every feature from the legacy open-can-mod repo exists on this Uno build.',
      ],
      checks: [
        `Board: ${ORDERED_KIT.board}`,
        `CAN module: ${ORDERED_KIT.can}`,
        'Converter available for X179 installed power',
        includeBluetooth ? 'HC-05, regulator, and divider parts available if you want the optional wireless link' : 'HC-05 path intentionally skipped for now',
      ],
      next: 'Move to the bench flash while the board is still powered only from PC USB.',
    },
    {
      id: 'bench-flash',
      title: 'Bench Flash Over USB',
      purpose: 'Flash the shared Uno image before any vehicle wiring exists. USB is the first power source and the first serial path.',
      diagram: <BenchPowerDiagram />,
      rows: [
        quickReferenceRows[0],
        quickReferenceRows[6],
        quickReferenceRows[7],
      ],
      avoid: [
        'Do not flash for the first time from vehicle-installed power.',
        'Do not reflash per vehicle type. The firmware switches variants at runtime.',
      ],
      checks: [
        'Built firmware from hardware/.pio/build/uno_usb/firmware.hex, or hardware/.pio/build/uno/firmware.hex only if HC-05 is installed',
        'Board powers on from PC USB',
        'CH340 serial port appears in the OS',
      ],
      next: 'Open the dashboard over USB and validate the board before moving power to X179.',
    },
    {
      id: 'usb-validate',
      title: 'Validate the Board on USB',
      purpose: 'Use the dashboard while the board is still on the bench to verify the serial path, status replies, and runtime variant switching.',
      diagram: <FirstFlashPanel />,
      rows: browserRows,
      avoid: [
        'Do not close up the install before ping, status, stream, and variant switching all work over USB.',
      ],
      checks: [
        'Board boot message appears',
        'ping works',
        'status works',
        'stream:on and stream:off work',
        'variant:hw4 / hw3 / legacy switches without reflashing',
      ],
      next: 'Build the permanent installed power branch only after the bench USB path is proven.',
    },
    {
      id: 'installed-power',
      title: 'Build the Installed Power Path',
      purpose: 'Move from bench USB power to X179-fed regulated power, then keep the Uno powered through its USB port.',
      diagram: <PowerDiagram includeBluetooth={includeBluetooth} />,
      referenceImages: [
        {
          src: '/reference/Tesla_X179_Pinout.png',
          alt: 'Tesla X179 connector pinout reference for installed power wiring',
          caption: 'Use this X179 reference while locating pin 1 and pin 20 for the installed power branch.',
        },
      ],
      rows: [
        quickReferenceRows[1],
        quickReferenceRows[2],
        quickReferenceRows[3],
      ],
      avoid: [
        'Never feed X179 directly into the Uno 5V pin.',
        'Do not use loose jumper wires as the final X179 or converter-input harness.',
      ],
      checks: [
        'X179 pin 1 goes to converter VIN+',
        'X179 pin 20 goes to converter VIN-',
        'Converter output was measured before reconnecting the Uno',
        'Uno now boots from converter USB output',
      ],
      next: 'Once installed power is stable, connect CAN last.',
    },
    {
      id: 'connect-can',
      title: 'Connect Vehicle CAN',
      purpose: 'After power is stable, connect the live vehicle bus to the MCP2515 and confirm the module wiring matches the Uno pin map.',
      diagram: (
        <div className="sg-mermaid-stack">
          <CANDiagram />
          <X179Diagram />
        </div>
      ),
      referenceImages: [
        {
          src: '/reference/Tesla_X179_Pinout.png',
          alt: 'Tesla X179 connector pinout reference for CAN wiring',
          caption: 'Use this X179 reference while confirming pin 13 and pin 14 before landing CAN-H and CAN-L.',
        },
      ],
      rows: [
        quickReferenceRows[4],
        quickReferenceRows[5],
        quickReferenceRows[6],
        quickReferenceRows[7],
      ],
      avoid: [
        'Do not leave the MCP2515 termination resistor enabled on the live vehicle bus.',
        'Do not swap CAN-H and CAN-L.',
      ],
      checks: [
        'X179 pin 13 -> MCP2515 CAN-H',
        'X179 pin 14 -> MCP2515 CAN-L',
        'MCP2515 termination disabled',
        'SPI pins D10/D11/D12/D13 and INT on D2 are correct',
      ],
      next: 'Reconnect to the dashboard and confirm live frames appear.',
    },
    ...(includeBluetooth ? [{
      id: 'optional-hc05',
      title: 'Optional HC-05 Wireless Link',
      purpose: 'Add HC-05 only after the wired USB and X179 paths are already stable.',
      diagram: <BluetoothDiagram />,
      rows: quickReferenceRows.slice(-2),
      avoid: [
        'Do not power the ordered HC-05 like a raw 5V module.',
        'Do not connect Uno D5 directly to HC-05 RX without the divider.',
      ],
      checks: [
        'HC-05 TXD -> Uno D4',
        'Uno D5 -> 1k/2k divider -> HC-05 RXD',
        'HC-05 on a dedicated 3.3V regulator',
        'OS pairing creates a serial COM port',
      ],
      next: 'Use the paired serial COM port from Web Serial. The app does not use Web Bluetooth.',
    }] : []),
    {
      id: 'live-check',
      title: 'First Live Runtime Check',
      purpose: 'With installed power and CAN connected, confirm that the selected variant exposes the right runtime behavior.',
      diagram: <InstallFlowDiagram includeBluetooth={includeBluetooth} />,
      rows: [
        browserRows[0],
        browserRows[1],
      ],
      avoid: [
        'Do not assume every variant shows the same expert controls.',
      ],
      checks: [
        'Live frames appear',
        'Selected runtime variant matches the car target',
        'Expected controls appear for that variant',
        'Unsupported controls stay hidden',
      ],
      next: 'If something is wrong, use the troubleshooting step instead of rewiring blindly.',
    },
    {
      id: 'troubleshooting',
      title: 'Troubleshooting',
      purpose: 'Use symptoms first so you do not mix power, serial, and CAN issues together.',
      diagram: null,
      rows: troubleshooting.map((tip) => ({
        role: tip.title,
        source: 'Symptom',
        destination: 'Check',
        note: tip.detail,
      })),
      avoid: [
        'Do not change multiple wiring branches at once while debugging.',
      ],
      checks: [
        'USB issues: check CH340 driver, cable, and COM port first',
        'Installed power issues: measure the converter output first',
        'No CAN traffic: check termination and X179 CAN polarity',
      ],
      next: 'When the symptom is resolved, return to the previous wizard step and continue in order.',
    },
  ];

  const currentWizardStep = wizardSteps[wizardStepIndex];

  return (
    <div className="page-shell sg-page">
      <div className="panel sg-wizard-panel">
        <div className="sg-summary-head">
          <ListChecks size={18} />
          <h2>Simple Install Wizard</h2>
        </div>
        <p className="sg-table-intro">Follow these steps in order. The long-form guide is still available below as full reference mode, but this wizard is the intended install path.</p>

        <div className="sg-wizard-progress">
          {wizardSteps.map((step, index) => (
            <button
              key={step.id}
              type="button"
              className={`sg-wizard-step ${index === wizardStepIndex ? 'active' : ''} ${index < wizardStepIndex ? 'done' : ''}`}
              onClick={() => setWizardStepIndex(index)}
            >
              <span>{index + 1}</span>
              <strong>{step.title}</strong>
            </button>
          ))}
        </div>

        <div className="sg-wizard-card">
          <div className="sg-card-head">
            <div className="sg-card-icon color-blue">
              <ListChecks size={24} />
            </div>
            <div className="sg-card-title">
              <div className="sg-card-title-row">
                <h2>{currentWizardStep.title}</h2>
                <span className="sg-step-badge">Step {wizardStepIndex + 1} / {wizardSteps.length}</span>
              </div>
              <p>{currentWizardStep.purpose}</p>
            </div>
          </div>

          <div className="sg-card-body">
            {currentWizardStep.diagram}
            <SectionConnectionTable title="Exact Connections" rows={currentWizardStep.rows} />
            {currentWizardStep.referenceImages ? <ReferenceGallery images={currentWizardStep.referenceImages} /> : null}

            <div className="sg-detail-grid">
              <div className="sg-detail-card">
                <div className="sg-detail-head">
                  <CheckCheck size={16} />
                  <strong>Success Check</strong>
                </div>
                <div className="sg-bullet-list">
                  {currentWizardStep.checks.map((check) => (
                    <div key={check} className="sg-bullet-item">
                      <span className="sg-bullet-dot sg-bullet-dot-ok" />
                      <span>{check}</span>
                    </div>
                  ))}
                </div>
              </div>

              <div className="sg-detail-card sg-detail-card-warn">
                <div className="sg-detail-head">
                  <ShieldAlert size={16} />
                  <strong>Do Not Do This</strong>
                </div>
                <div className="sg-bullet-list">
                  {currentWizardStep.avoid.map((item) => (
                    <div key={item} className="sg-bullet-item">
                      <span className="sg-bullet-dot sg-bullet-dot-warn" />
                      <span>{item}</span>
                    </div>
                  ))}
                </div>
              </div>
            </div>

            <div className="sg-next-box">
              <span className="sg-box-label">Next action</span>
              <strong>{currentWizardStep.next}</strong>
              <p>{wizardStepIndex === wizardSteps.length - 1 ? 'Go back to the failed step after fixing the issue.' : 'Use Next to continue only after this step is already proven.'}</p>
            </div>

            <div className="sg-wizard-actions">
              <button
                type="button"
                className="sg-reset"
                disabled={wizardStepIndex === 0}
                onClick={() => setWizardStepIndex((current) => Math.max(0, current - 1))}
              >
                Previous
              </button>
              <button
                type="button"
                className="sg-toggle"
                disabled={wizardStepIndex === wizardSteps.length - 1}
                onClick={() => setWizardStepIndex((current) => Math.min(wizardSteps.length - 1, current + 1))}
              >
                Next
              </button>
            </div>
          </div>
        </div>
      </div>

      <details className="sg-reference-mode">
        <summary className="sg-source-summary">
          <span>Open Full Reference Guide</span>
          <span className="sg-source-summary-note">Long-form wiring, Mermaid source, photos, and full checklists</span>
        </summary>
        <div className="sg-reference-body">
      <div className="panel sg-header-box">
        <div className="sg-title">
          <h1>USB-First, X179-Second Install Guide</h1>
          <p>Start on the desk with the Uno powered from USB for the first flash, the first dashboard connection, and the first vehicle-variant check.</p>
          <p>Only after that bench phase passes should you move permanent power to X179 pin 1 and pin 20 through the converter input, feed the Uno from the converter USB output, and then connect X179 CAN on pins 13 and 14.</p>
          <div className="sg-alert" style={{ marginTop: '12px' }}>
            <ShieldAlert size={18} />
            <p>The ordered Uno clone uses a CH340 USB bridge, so Windows 7 and older macOS installs may need a driver while Linux and modern Windows usually work immediately. Use that USB link for firmware flashing and first validation before closing up the installation.</p>
          </div>
          <div className="sg-alert" style={{ marginTop: '12px' }}>
            <ShieldAlert size={18} />
            <p>If the HC-05 is not physically installed, leave the HC-05 path disabled in this guide and flash <code>uno_usb</code>. Do not install the Bluetooth-enabled firmware just because the optional wiring exists in the docs.</p>
          </div>

          <div className="sg-toolbar">
            <button
              type="button"
              onClick={() => setIncludeBluetooth(!includeBluetooth)}
              className="sg-toggle"
            >
              {includeBluetooth ? <ToggleRight size={20} color="var(--blue)" /> : <ToggleLeft size={20} color="var(--text-muted)" />}
              <span>Include Optional HC-05 Path</span>
            </button>

            <button
              type="button"
              onClick={resetChecklist}
              className="sg-reset"
            >
              <RotateCcw size={16} />
              <span>Reset Checklist</span>
            </button>
          </div>
        </div>

        <div className="sg-progress-stack">
          <div className="sg-progress-box">
            <div className="sg-progress-circle">
              <svg viewBox="0 0 64 64">
                <circle className="bg" cx="32" cy="32" r="28" strokeWidth="6" fill="transparent" />
                <circle
                  className="fg"
                  cx="32"
                  cy="32"
                  r="28"
                  strokeWidth="6"
                  fill="transparent"
                  strokeDasharray="175.9"
                  strokeDashoffset={175.9 - (175.9 * progress) / 100}
                />
              </svg>
              <span className="sg-progress-value">{progress}%</span>
            </div>
            <div className="sg-progress-text">
              <strong>Installation Progress</strong>
              <span>{completedCount} done, {remainingCount} remaining</span>
            </div>
          </div>

          <div className="sg-next-box">
            <span className="sg-box-label">Next recommended step</span>
            {nextPending ? (
              <>
                <strong>{nextPending.sectionTitle}</strong>
                <p>{nextPending.itemText}</p>
              </>
            ) : (
              <>
                <strong>Ready for the X179-powered live check</strong>
                <p>The bench flash, converter migration, and physical wiring checklist are complete. Validate the installed power path, connect the runtime link, and watch live CAN traffic.</p>
              </>
            )}
          </div>
        </div>

        <QuickPhaseCards includeBluetooth={includeBluetooth} />
      </div>

      <div className="panel sg-architecture-panel">
        <div className="sg-summary-head">
          <Zap size={18} />
          <h2>Two-Phase Install Plan</h2>
        </div>
        <div className="sg-architecture-grid">
          <div className="sg-architecture-diagrams">
            <BenchPowerDiagram />
            <PowerDiagram includeBluetooth={includeBluetooth} />
            <InstallFlowDiagram includeBluetooth={includeBluetooth} />
          </div>
          <div className="sg-architecture-copy">
            <div className="sg-key-list">
              <div className="sg-key-row">
                <span className="sg-key-label">Phase 1: bench power</span>
                <span className="sg-key-value">USB powers the Uno first. Use that phase to flash firmware, validate the CH340 serial path, and confirm the runtime vehicle variant.</span>
              </div>
              <div className="sg-key-row">
                <span className="sg-key-label">Phase 2: installed power</span>
                <span className="sg-key-value">X179 pin 1 provides the 12V feed, X179 pin 20 is ground, and both land on the converter input before the converter USB output replaces PC USB power at the Uno.</span>
              </div>
              <div className="sg-key-row">
                <span className="sg-key-label">Vehicle CAN path</span>
                <span className="sg-key-value">X179 pin 13 is CAN-H and pin 14 is CAN-L. Those lines go to the MCP2515 board after the module termination is disabled for vehicle use.</span>
              </div>
              <div className="sg-key-row">
                <span className="sg-key-label">USB role</span>
                <span className="sg-key-value">USB is the first power source and the preferred flash/debug link because it gives the cleanest board status path through CH340.</span>
              </div>
              <div className="sg-key-row">
                <span className="sg-key-label">Bluetooth role</span>
                <span className="sg-key-value">{includeBluetooth ? 'HC-05 stays optional and should only be added after the USB bench phase and the X179-powered install are already stable. Flash the Bluetooth-enabled firmware only when the module is physically installed.' : 'HC-05 is currently disabled in this view. Keep the wired install path and flash `uno_usb` while the module is absent.'}</span>
              </div>
            </div>
          </div>
        </div>
      </div>

      <div className="sg-overview-grid">
        <div className="panel sg-overview-card">
          <div className="sg-overview-icon"><Wrench size={18} /></div>
          <div>
            <span className="sg-box-label">Main board</span>
            <strong>{ORDERED_KIT.board}</strong>
            <p>Start with USB power on the bench. Permanent board power comes later from X179 through the converter, then into the Uno USB port.</p>
          </div>
        </div>
        <div className="panel sg-overview-card">
          <div className="sg-overview-icon"><Cpu size={18} /></div>
          <div>
            <span className="sg-box-label">CAN hardware</span>
            <strong>{ORDERED_KIT.can}</strong>
            <p>Expected firmware timing is 8 MHz MCP2515.</p>
          </div>
        </div>
        <div className="panel sg-overview-card">
          <div className="sg-overview-icon"><Usb size={18} /></div>
          <div>
            <span className="sg-box-label">Control path</span>
            <strong>{includeBluetooth ? 'USB first, HC-05 optional after install' : 'USB first, then X179 installed power'}</strong>
            <p>Use USB for the first flash and first diagnostics, then keep USB as the service link or move later to the paired HC-05 COM port. If HC-05 is absent, stay on the `uno_usb` firmware path.</p>
          </div>
        </div>
        <div className="panel sg-overview-card">
          <div className="sg-overview-icon"><ListChecks size={18} /></div>
          <div>
            <span className="sg-box-label">Firmware model</span>
            <strong>{ORDERED_KIT.firmware}</strong>
            <p>Change the runtime vehicle behavior from the web client instead of reflashing each time.</p>
          </div>
        </div>
      </div>

      <div className="sg-summary-grid">
        <QuickReferenceTable
          title="Quick Wiring Reference"
          description="Use this as the fast cross-check when you need one pin or one endpoint without reopening every section."
          rows={quickReferenceRows}
        />

        <QuickReferenceTable
          title="Phone and Browser Support"
          description={capabilities.compatibilitySummary}
          rows={browserRows}
        />

        <div className="panel sg-summary-card">
          <div className="sg-summary-head">
            <Wrench size={18} />
            <h2>What You Need</h2>
          </div>
          <div className="sg-key-list">
            {PREP_ITEMS.map((item) => (
              <div key={item.label} className="sg-key-row">
                <span className="sg-key-label">{item.label}</span>
                <span className="sg-key-value">{item.value}</span>
              </div>
            ))}
          </div>
        </div>

        <div className="panel sg-summary-card">
          <div className="sg-summary-head">
            <ShieldAlert size={18} />
            <h2>Non-Negotiable Safety Rules</h2>
          </div>
          <div className="sg-bullet-list">
            {SAFETY_RULES.map((rule) => (
              <div key={rule} className="sg-bullet-item">
                <span className="sg-bullet-dot" />
                <span>{rule}</span>
              </div>
            ))}
          </div>
        </div>
      </div>

      <div className="sg-grid">
        {activeSteps.map((section) => {
          const counts = getSectionCounts(section, completedSteps);

          return (
          <div key={section.id} className="panel sg-card">
            <div className="sg-card-head">
              <div className={`sg-card-icon ${section.colorClass}`}>
                <section.icon size={24} />
              </div>
              <div className="sg-card-title">
                <div className="sg-card-title-row">
                  <h2>{section.title}</h2>
                  <span className="sg-step-badge">{counts.done}/{counts.total} done</span>
                </div>
                <p>{section.description}</p>
              </div>
            </div>

            <div className="sg-card-body">
              <div className="sg-card-summary">{section.summary}</div>

              {section.diagram && (
                <section.diagram />
              )}

              {section.connections && (
                <SectionConnectionTable title="Connection Map" rows={section.connections} />
              )}

              {section.extra && (
                <section.extra />
              )}

              {section.highlights && (
                <div className="sg-chip-row">
                  {section.highlights.map((highlight) => (
                    <span key={highlight} className="sg-chip">{highlight}</span>
                  ))}
                </div>
              )}

              {section.referenceImages && (
                <ReferenceGallery images={section.referenceImages} />
              )}

              {section.alerts && section.alerts.map((alert, idx) => (
                <div key={idx} className="sg-alert">
                  <ShieldAlert size={18} />
                  <p>{alert}</p>
                </div>
              ))}

              <div className="checklist">
                {section.items.map((item) => {
                  const isChecked = completedSteps[item.id];
                  return (
                    <button
                      key={item.id}
                      type="button"
                      onClick={() => toggleStep(item.id)}
                      className={`sg-item ${isChecked ? 'done' : ''}`}
                    >
                      <div className="sg-item-icon">
                        {isChecked ? <CheckCircle2 size={20} /> : <Circle size={20} />}
                      </div>
                      <span className="sg-item-text">
                        {item.text}
                      </span>
                    </button>
                  );
                })}
              </div>

              {(section.checks || section.mistakes) && (
                <div className="sg-detail-grid">
                  {section.checks && (
                    <div className="sg-detail-card">
                      <div className="sg-detail-head">
                        <CheckCheck size={16} />
                        <strong>Verify Before Moving On</strong>
                      </div>
                      <div className="sg-bullet-list">
                        {section.checks.map((check) => (
                          <div key={check} className="sg-bullet-item">
                            <span className="sg-bullet-dot sg-bullet-dot-ok" />
                            <span>{check}</span>
                          </div>
                        ))}
                      </div>
                    </div>
                  )}

                  {section.mistakes && (
                    <div className="sg-detail-card sg-detail-card-warn">
                      <div className="sg-detail-head">
                        <ShieldAlert size={16} />
                        <strong>Common Mistakes</strong>
                      </div>
                      <div className="sg-bullet-list">
                        {section.mistakes.map((mistake) => (
                          <div key={mistake} className="sg-bullet-item">
                            <span className="sg-bullet-dot sg-bullet-dot-warn" />
                            <span>{mistake}</span>
                          </div>
                        ))}
                      </div>
                    </div>
                  )}
                </div>
              )}

              {section.complete && (
                <div className="sg-detail-card">
                  <div className="sg-detail-head">
                    <CheckCircle2 size={16} />
                    <strong>When This Step Is Complete</strong>
                  </div>
                  <div className="sg-bullet-list">
                    {section.complete.map((item) => (
                      <div key={item} className="sg-bullet-item">
                        <span className="sg-bullet-dot sg-bullet-dot-ok" />
                        <span>{item}</span>
                      </div>
                    ))}
                  </div>
                </div>
              )}
            </div>
          </div>
          );
        })}
      </div>

      <div className="sg-summary-grid">
        <div className="panel sg-summary-card">
          <div className="sg-summary-head">
            <Usb size={18} />
            <h2>Recommended Order</h2>
          </div>
          <div className="sg-sequence">
            {bringUpSteps.map((step, index) => (
              <div key={step.title} className="sg-sequence-item">
                <div className="sg-sequence-index">{index + 1}</div>
                <div>
                  <strong>{step.title}</strong>
                  <p>{step.detail}</p>
                </div>
              </div>
            ))}
          </div>
        </div>

        <div className="panel sg-summary-card">
          <div className="sg-summary-head">
            <Bluetooth size={18} />
            <h2>Quick Troubleshooting</h2>
          </div>
          <div className="sg-sequence">
            {troubleshooting.map((tip, index) => (
              <div key={tip.title} className="sg-sequence-item">
                <div className="sg-sequence-index sg-sequence-index-muted">{index + 1}</div>
                <div>
                  <strong>{tip.title}</strong>
                  <p>{tip.detail}</p>
                </div>
              </div>
            ))}
          </div>
        </div>
      </div>

      {progress === 100 && (
        <div className="panel sg-complete">
          <div className="sg-complete-icon">
            <CheckCircle2 size={32} />
          </div>
          <div className="sg-complete-text">
            <h3>Physical Wiring Complete</h3>
            <p>You are ready to power the install from X179, connect via USB{includeBluetooth ? ', then optionally switch to the paired HC-05 serial port,' : ''} and validate live behavior from the dashboard.</p>
          </div>
        </div>
      )}
        </div>
      </details>
    </div>
  );
}
