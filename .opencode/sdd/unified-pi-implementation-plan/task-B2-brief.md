# Task B2 — Wiring guide + listen-only bring-up

Where: second deployment task. Canonical hardware doc for driving the existing MCP2515 modules from a Pi.

## Requirement (verbatim from plan, expanded with the two cited guides)

Create `docs/guides/pi-can-bringup.md` (canonical doc; generated bundle regenerated in its own commit).

Steps:

1. Pin table: VCC 3.3 V to MCP2515, TJA1050 Vdd to 5 V via cut/jumper OR 3.3 V transceiver module; GND common; SCK GPIO11, MOSI GPIO10, MISO GPIO9, CS GPIO8 (CE0) / GPIO7 (CE1) / free GPIO for bus 3, INT GPIO25 (+ distinct INT for extra buses). Level shifters where any line is 5 V.
2. Device-tree: `dtparam=spi=on`, `dtoverlay=mcp2515-can0,oscillator=<module crystal>,interrupt=25`, `dtoverlay=spi-bcm2835` (+ `spi-dma` on old kernels); extra buses use distinct CS/INT. Verify `/sys/bus/spi/devices/spi0.0/net/can0` exists, then `ip link set can0 up type can bitrate 500000`.
3. Acceptance: `ip -details link show can0` reports 500 kbps, `candump can0` sees expected X179 frames with 120 Ω termination; record hardware provenance (Pi model, module crystal, transceiver variant, wiring photo).
4. Explicit "DO NOT ENABLE TX" section until Task B3 passes.
5. Cite both guides in the doc (read-only, no code copied); forum thread 403 unused.
    - https://38-3d.co.uk/blogs/blog/using-the-mcp2515-with-the-raspberry-pi
    - https://github.com/tolgakarakurt/CANBus-MCP2515-Raspi
6. Regenerate the in-app docs bundle in a SEPARATE commit; commit the guide first.

X179 mapping from existing `docs/guides/hardware-setup.md`: bus 0 pins 13/14, bus 1 pins 9/10, bus 2 pins 2/3.

## Global constraints

- No hardware here → every hardware claim UNVERIFIED; write it as a runbook to execute on real Pi, not as a claim of success.
- No CAN TX procedure beyond the explicit ban; no Sentry code copied.
- Stage ONLY files you change (~200 unrelated unstaged exist); verify staged lists before each commit.
- Feature branch `spike/raspberry-pi-portability`; never main, never force-push.
