# Tesla CAN Explorer

Tesla CAN Explorer is an open-source research portal for browsing decoded Tesla CAN frames and signal value maps.

This research is open source on [GitHub](https://github.com/mikegapinski/tesla-can-explorer) and sponsored by [Tesla Android](https://teslaandroid.com/). Want Android apps in your Tesla? Visit [teslaandroid.com](https://teslaandroid.com/).

Independent project disclaimer: this project is unaffiliated with and not endorsed by Tesla, Inc.

## Dataset Scope

- Vehicle: `Model 3`
- Firmware: `2026.2`
- Sources: `libQtCarCANData.so`, `libQtCarVAPI.so`
- Variant `MCU2 (Intel)`: `./data/can_frames_decoded_all_values_mcu2.json`
- Variant `MCU3 (AMD)`: `./data/can_frames_decoded_all_values_mcu3.json`

Related VAPI artifacts:

- `./data/vapi_can_digest_mcu2.json`
- `./data/vapi_eth_signal_aliases_mcu2.csv`
- `./data/vapi_can_digest_mcu3.json`
- `./data/vapi_eth_signal_aliases_mcu3.csv`

## Run Locally

```bash
python3 -m http.server 8080
```

Open:

- `http://localhost:8080/` (default: MCU2)
- `http://localhost:8080/?source=mcu2`
- `http://localhost:8080/?source=mcu3`

Optional override:

- `http://localhost:8080/?data=./data/can_frames_decoded_all_values_mcu3.json`

## Features

- Search across frame names, addresses, signals, enum maps, enum labels, and VAPI aliases.
- Independent left/right panel scrolling for easier browsing on large datasets.
- Filter by bus/module and sort by address, name, signal count, enum count, or VAPI alias count.
- Expand per-signal decoded values directly in the frame detail view.

## Credits

- Copyright © 2026 Michał Gapiński ([gapinski.eu](https://gapinski.eu))
- Tesla Android ([teslaandroid.com](https://teslaandroid.com/))
- X: [@mikegapinski](https://x.com/mikegapinski), [@teslaandroid](https://x.com/teslaandroid)

## License

This project is licensed under `0BSD` (see `LICENSE`).
