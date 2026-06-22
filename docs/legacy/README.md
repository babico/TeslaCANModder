# Legacy Repository Index

Community Tesla projects archived as git submodules under `legacy/`. Reference
material for feature mining, CAN ID validation, protocol reverse-engineering,
and architecture comparison. Submodules are read-only — never copy legacy code
into shipping code.

Per-repo analysis lives beside this file. The cross-repo planning lives in
`.kiro/codebase-improvement-plan.md`.

## Tier 1 — Primary sources actively informing the firmware

High-value repos directly driving current or planned features.

| Submodule | Author | License | Hardware | Key contribution |
| --------- | ------ | ------- | -------- | ---------------- |
| [ev-open-can-tools](ev-open-can-tools-ev-open-can-tools.md) | ev-open-can-tools | GPL-3.0 | ESP32, RP2040, M4, M5Stack | Upstream reference; AP Injection Gate architecture (v2.5.x), HW3/HW4 handler structure |
| [zdenekbouresh/ev-open-can-tools fork](zdenekbouresh-ev-open-can-tools.md) | zdenekbouresh | GPL-3.0 | Same as upstream | `feat/das-aware-nag-suppression` branch: DAS-gated organic torque, grip excursion pattern |
| [linuchoicoegwangsu-soft-nag-bypass](linuchoicoegwangsu-soft-nag-bypass.md) | linuchoicoegwangsu | — (local) | — | Complete C++ snippet + state-machine spec for organic nag (direct porting source for `NAG_KILLER_ORGANIC`) |
| [hypery11-flipper-tesla-fsd](hypery11-flipper-tesla-fsd.md) | hypery11 | GPL-3.0 | Flipper Zero, ESP32 | 0x7FF Ban Shield, TLSSC Restore (0x331), ban detection, MCP2515 8 MHz |
| [nicolozak-nag-killer](nicolozak-nag-killer.md) | nicolozak | GPL-3.0 | ESP32 | Counter+1 echo technique, Mode B/C fallbacks, checksum formula `+0x73` |
| [slxslx-tesla-open-can-mod-slx-repo](slxslx-tesla-open-can-mod-slx-repo.md) | slxslx | GPL-3.0 | 8+ boards | Multi-board config, web dashboard, MCP2515 8 MHz crystal reference |
| [Shayennn-FUCKYOU-TESLA-FSD](Shayennn-FUCKYOU-TESLA-FSD.md) | Shayennn | GPL-3.0/MIT | Feather M4, ESP32, ESP32-IDF | Shared `vehicle_logic.h`, CI + sanitizer tests, ESP-IDF v6.0 |
| [tesla-local-control/tesla_ble_mqtt_docker](tesla-local-control-tesla-ble-mqtt-docker.md) | tesla-local-control | Apache-2.0 | Linux/Docker | MQTT ⇄ BLE bidirectional architecture, HA auto-discovery |
| [yoziru-esphome-tesla-ble](yoziru-esphome-tesla-ble.md) | yoziru | AGPL-3.0 | ESP32, M5Stack | Tesla vehicle-command BLE protobuf dispatch (climate/lock/trunk/charging) |
| [teslamotors/vehicle-command](teslamotors-vehicle-command.md) | Tesla Motors | Apache-2.0 | Go/Linux | Official VCSEC/BLE vehicle-command SDK; BLE key distance reference |

## Tier 2 — FSD / CAN mod firmware

Alternative implementations useful for cross-validating CAN IDs, bit positions,
and checksum formulas. All targeting FSD enable, nag suppression, or speed
profile features.

| Submodule | Author | Hardware | Focus |
| --------- | ------ | -------- | ----- |
| [1-v-1-tesla-fsd-can-mod](1-v-1-tesla-fsd-can-mod.md) | 1-v-1 | ESP32 | FSD enable |
| [1-v-1-tesla-open-can-mod](1-v-1-tesla-open-can-mod.md) | 1-v-1 | ESP32 | Open CAN mod |
| [alzza-tesla-open-can-mod](alzza-tesla-open-can-mod.md) | alzza | ESP32 | Open CAN mod fork |
| [alzza-tesla-can-moniter](alzza-tesla-can-moniter.md) | alzza | ESP32 | CAN monitor fork |
| [binfen1-tesla-fsd-can-mod](binfen1-tesla-fsd-can-mod.md) | binfen1 | ESP32 | FSD enable |
| [dongho74s-tesla-open-can-mod](dongho74s-tesla-open-can-mod.md) | dongho74s | ESP32 | Open CAN mod fork |
| [enstw-tesla-can-mod-guide](enstw-tesla-can-mod-guide.md) | enstw | — | Install guide |
| [EzeLLM-fsd-spoofing](EzeLLM-fsd-spoofing.md) | EzeLLM | Nano, ESP32, RP2040 | Wiring guides, Canada workaround, $15 Nano build |
| [herrfrei-tesla-fsd-canbus-esp32](herrfrei-tesla-fsd-canbus-esp32.md) | herrfrei | ESP32 | FSD CAN bus |
| [honeer-Tesla-ESP-CAN](honeer-Tesla-ESP-CAN.md) | honeer | ESP32 | Tesla ESP CAN |
| [iubns-tesla-fsd-can-mod](iubns-tesla-fsd-can-mod.md) | iubns | ESP32 | FSD enable |
| [J0811-flipper-tesla-fsd](J0811-flipper-tesla-fsd.md) | J0811 | Flipper Zero | FSD via Flipper |
| [JelloEa-tesla-fsd-controller](JelloEa-tesla-fsd-controller.md) | JelloEa | ESP32 | FSD controller |
| [JelloEa-Tesla-Open-CAN-Mod](JelloEa-Tesla-Open-CAN-Mod.md) | JelloEa | RP2040, ESP32 | Multi-platform FSD mod |
| [juamiso-tesla-fsd-can-enabler](juamiso-tesla-fsd-can-enabler.md) | juamiso | ESP32 | FSD enabler |
| [jvanakker-tesla-fsd-can-mod](jvanakker-tesla-fsd-can-mod.md) | jvanakker | ESP32 | FSD enable |
| [linesoft2-tesla-fsd-can-mod-fork](linesoft2-tesla-fsd-can-mod-fork.md) | linesoft2 | ESP32 | FSD fork |
| [MrStarTraveller-tesla-fsd-can-mod](MrStarTraveller-tesla-fsd-can-mod.md) | MrStarTraveller | ESP32 | FSD enable |
| [tesla-fsd-can-mod-main](tesla-fsd-can-mod-main.md) | — | ESP32 | FSD reference |
| [tesla-fsd-can-mod-2-main](tesla-fsd-can-mod-2-main.md) | — | ESP32 | FSD v2 |
| [tesla-fsd.netlify.app](tesla-fsd.netlify.app.md) | — | Web | FSD docs site |
| [tesla-open-can-mod-main](tesla-open-can-mod-main.md) | — | ESP32 | Open CAN reference |
| [tuncasoftbildik-tesla-can-mod](tuncasoftbildik-tesla-can-mod.md) | tuncasoftbildik | ESP32 | CAN mod |
| [ylovex75-tesla-open-can-mod-release](ylovex75-tesla-open-can-mod-release.md) | ylovex75 | ESP32 | Open CAN release |

## Tier 3 — Plugin / rule packs

Signal reference only — we compile features into the binary, not via runtime plugins.

| Submodule | Author | Focus |
| --------- | ------ | ----- |
| [ev-open-can-tools plugins](ev-open-can-tools-ev-open-can-tools-plugins.md) | ev-open-can-tools | JSON plugin schema, HW3/HW4 signal targets |

## Tier 4 — CAN analysis, DBC, logging tools

Signal databases, DBC files, and analysis tools. Useful as cross-references
for our `ids.h` and signal documentation.

| Submodule | Author | Focus |
| --------- | ------ | ----- |
| [Adminius-ESP32-ScanMyTesla](Adminius-ESP32-ScanMyTesla.md) | Adminius | Tesla CAN scanner |
| [amzoo-TMS_2016_LOGS](amzoo-TMS_2016_LOGS.md) | amzoo | Model S CAN logs |
| [Arkay92-TeslaCANInterpreter](Arkay92-TeslaCANInterpreter.md) | Arkay92 | CAN interpreter |
| [automotive-stuff-Tesla_canbus](automotive-stuff-Tesla_canbus.md) | automotive-stuff | CAN bus docs |
| [BluedDot-IT-TeslaCANalyzer-Controller](BluedDot-IT-TeslaCANalyzer-Controller.md) | BluedDot-IT | CAN analyzer |
| [bruvv-tesla-can-explorer](bruvv-tesla-can-explorer.md) | bruvv | CAN explorer |
| [canhackers-jupiter](canhackers-jupiter.md) | canhackers | CAN hacking tool |
| [ekr-candash](ekr-candash.md) | ekr | CAN dashboard |
| [hanswolff-TeslaCanBusInspector](hanswolff-TeslaCanBusInspector.md) | hanswolff | CAN inspector |
| [joshwardell-model3dbc](joshwardell-model3dbc.md) | joshwardell | Model 3 DBC file |
| [jsamuel1-tesla_canlogjs](jsamuel1-tesla_canlogjs.md) | jsamuel1 | CAN log JS |
| [krconv-tesla_can_decoding](krconv-tesla_can_decoding.md) | krconv | CAN decoding |
| [LeeGaHyeon-tesla_CAN_traffic_decode](LeeGaHyeon-tesla_CAN_traffic_decode.md) | LeeGaHyeon | CAN traffic decode |
| [MatthewDriver-TeslaCAN](MatthewDriver-TeslaCAN.md) | MatthewDriver | Tesla CAN |
| [mcirish-TeslaModelS_RefreshCAN.dbc](mcirish-TeslaModelS_RefreshCAN.dbc.md) | mcirish | Refresh Model S DBC |
| [mhpetiwala-TeslaCAN](mhpetiwala-TeslaCAN.md) | mhpetiwala | Tesla CAN |
| [mikegapinski-tesla-can-explorer](mikegapinski-tesla-can-explorer.md) | mikegapinski | CAN explorer |
| [RuairidhScott-Brown-TeslaCAN](RuairidhScott-Brown-TeslaCAN.md) | RuairidhScott-Brown | Tesla CAN |
| [sahilcc7-tesla_can](sahilcc7-tesla_can.md) | sahilcc7 | Tesla CAN |
| [SergeyStaroletov-Tesla-CAN-packets-generator](SergeyStaroletov-Tesla-CAN-packets-generator.md) | SergeyStaroletov | CAN packet gen |
| [SlipknotTN-Tesla_CanBus_Reader](SlipknotTN-Tesla_CanBus_Reader.md) | SlipknotTN | CAN reader |
| [stylylsty-TelemetryX](stylylsty-TelemetryX.md) | stylylsty | Telemetry |
| [talas9-tesla_can_signals](talas9-tesla_can_signals.md) | talas9 | Multi-model CAN signals (JSON) |
| [tumik-S3XY-candump](tumik-S3XY-candump.md) | tumik | CAN dump |
| [uhi22-tesla-crc](uhi22-tesla-crc.md) | uhi22 | Tesla CRC algorithms |
| [mveplus-tesla-model3-resources](mveplus-tesla-model3-resources.md) | mveplus | Curated ecosystem links: TeslaMate, Fleet Telemetry, HA, SDKs |

## Tier 5 — BMS / battery

| Submodule | Author | Focus |
| --------- | ------ | ----- |
| [jamiejones85-ESP32TeslaShuntCan](jamiejones85-ESP32TeslaShuntCan.md) | jamiejones85 | Shunt CAN |
| [jomytec-My_TeslaBMS](jomytec-My_TeslaBMS.md) | jomytec | Tesla BMS |
| [oliwiah-Tesla_Battery_Range_Calc_React](oliwiah-Tesla_Battery_Range_Calc_React.md) | oliwiah | Range calculator |

## Tier 6 — Dashboards and display apps

| Submodule | Author | Focus |
| --------- | ------ | ----- |
| [bbrightwell-forklift-dash](bbrightwell-forklift-dash.md) | bbrightwell | CAN dashboard |
| [cbusillo-TeslaPiCAN](cbusillo-TeslaPiCAN.md) | cbusillo | Pi CAN dashboard |
| [erikhedb-WE-EV-CAN-Dashboard_POC](erikhedb-WE-EV-CAN-Dashboard_POC.md) | erikhedb | EV dashboard POC |
| [ibmthinkpad-model3mon](ibmthinkpad-model3mon.md) | ibmthinkpad | Model 3 monitor |
| [JonnoFTW-rpi-can-logger](JonnoFTW-rpi-can-logger.md) | JonnoFTW | RPi CAN logger |
| [kangbumhee-TLA-SpeedAlert](kangbumhee-TLA-SpeedAlert.md) | kangbumhee | Speed alert |
| [mgerczuk-TeslaCANPi](mgerczuk-TeslaCANPi.md) | mgerczuk | Pi CAN |
| [nicholasyangyang-ESP32-dash-direct](nicholasyangyang-ESP32-dash-direct.md) | nicholasyangyang | ESP32 dash |
| [nicholasyangyang-my-tt](nicholasyangyang-my-tt.md) | nicholasyangyang | Tesla tools |
| [rossklonowski-CANserver](rossklonowski-CANserver.md) | rossklonowski | CAN server |

## Tier 7 — Tesla API / BLE / third-party

| Submodule | Author | Focus |
| --------- | ------ | ----- |
| [Akisoft41-TeslapLX](Akisoft41-TeslapLX.md) | Akisoft41 | Tesla pLX |
| [bobmorane83-TeslaCam](bobmorane83-TeslaCam.md) | bobmorane83 | Tesla Cam |
| [codethaumaturge-911-tesla-gauges](codethaumaturge-911-tesla-gauges.md) | codethaumaturge | Custom gauges |
| [ColinM-sys-tesla-can-boost](ColinM-sys-tesla-can-boost.md) | ColinM-sys | CAN boost |
| [DemiVis-charge-port-opener](DemiVis-charge-port-opener.md) | DemiVis | Charge port |
| [denysvitali-tesla-sentry-viewer](denysvitali-tesla-sentry-viewer.md) | denysvitali | Sentry viewer |
| [evoffer-can-decoder-firmware](evoffer-can-decoder-firmware.md) | evoffer | CAN decoder FW |
| [gregjhogan-tesla-pre-ap-epas-patch](gregjhogan-tesla-pre-ap-epas-patch.md) | gregjhogan | Pre-AP EPAS patch |
| [jberstler-tesla-warmer](jberstler-tesla-warmer.md) | jberstler | Tesla warmer |
| [jiezaichan-teslaAuthFlutter](jiezaichan-teslaAuthFlutter.md) | jiezaichan | Auth Flutter app |
| [monster-xxx-tesla-can-controller](monster-xxx-tesla-can-controller.md) | monster-xxx | CAN controller |
| [rafal83-Car-Light-Sync](rafal83-Car-Light-Sync.md) | rafal83 | Light sync |
| [riderx-autosteerplus](riderx-autosteerplus.md) | riderx | Autosteer+ |
| [rjyo-homebridge-tesla-remote](rjyo-homebridge-tesla-remote.md) | rjyo | Homebridge plugin |
| [rrrovalle-tesla-car-app](rrrovalle-tesla-car-app.md) | rrrovalle | Tesla car app |
| [sydneyg007-Tesla-Model-3-EPAS-emulator](sydneyg007-Tesla-Model-3-EPAS-emulator.md) | sydneyg007 | EPAS emulator |
| [sydneyg007-Tesla-Model-3-Front-DI-emulator](sydneyg007-Tesla-Model-3-Front-DI-emulator.md) | sydneyg007 | Front DI emulator |
| [tesberry-tesberry](tesberry-tesberry.md) | tesberry | Raspberry Pi Tesla |
| [wimaha-TeslaBleHttpProxy](wimaha-TeslaBleHttpProxy.md) | wimaha | BLE HTTP proxy |
| [bogosj-tesla](bogosj-tesla.md) | bogosj | Unofficial Tesla Owner API + OAuth token helper (Go) |

## Tier 8 — Unrelated / historical

| Submodule | Author | Focus |
| --------- | ------ | ----- |
| [devon-smith-six-pack-capacitor-tesla-coil](devon-smith-six-pack-capacitor-tesla-coil.md) | devon-smith | Tesla coil hardware (not a Tesla EV project) |

## See also

- `COMPARISON.md` — cross-repo feature matrix
- `.kiro/codebase-improvement-plan.md` — unified implementation plan derived from these repos
