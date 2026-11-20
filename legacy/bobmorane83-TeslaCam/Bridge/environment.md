# Environment Configuration - ESP32 CAN Project

**Project Type:** Embedded CAN Communication System  
**Date Created:** 22 février 2026 | **Updated:** Mars 2026  
**Platform:** LilyGo T-2CAN v1.0 (ESP32-S3-WROOM-1U)

---

## Hardware Configuration

### Device Specifications
- **MCU:** ESP32-S3-WROOM-1U MCN16R8 (Dual-core LX7, 240MHz each)
- **RAM:** 512 KB SRAM + 8 MB OPI PSRAM (integrated)
- **Flash:** 16 MB QIO
- **Connectivity:** WiFi 802.11 b/g/n, BLE 5.0
- **USB:** USB-C JTAG/serial debug (CDC on boot)

### Connected Peripherals
- **CAN Interface:** MCP2515 via SPI (integrated on T-2CAN PCB)
  - **Communication Speed:** 500 kbps
  - **Crystal:** 16 MHz (onboard)
  - **SPI Bus:** Standard
  - **No INT pin** — SPI polling via `checkReceive()`
  - **RST Pin:** GPIO 9 (hardware reset at boot)

### Pin Allocation (LilyGo T-2CAN)
```
SPI Interface (MCP2515 — routed on PCB):
- MOSI: GPIO 11
- MISO: GPIO 13
- SCK:  GPIO 12
- CS:   GPIO 10
- RST:  GPIO 9
- INT:  Not connected (SPI polling)

CAN Bus (onboard screw terminal):
- CAN_H / CAN_L

Other:
- Serial/Debug: USB-C JTAG/CDC @ 115200 baud
- TWAI: GPIO 7 (TX), GPIO 6 (RX) — ChassisBus (built-in CAN controller)
```

---

## Development Environment

### IDE & Build System
- **Primary IDE:** VSCode + PlatformIO
- **Framework:** Arduino (Arduino-ESP32 v3.x)
- **Compiler:** xtensa-esp32-elf-gcc
- **Python:** Python 3.8+ (required for PlatformIO)

### Required Dependencies
```ini
[env:t2can]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
board_build.mcu = esp32s3
board_build.f_cpu = 240000000L
board_build.flash_mode = qio
board_upload.flash_size = 16MB
board_build.partitions = default_16MB.csv
board_build.arduino.memory_type = qio_opi

monitor_speed = 115200
upload_speed = 921600

build_flags = 
    -D DEBUG_LEVEL=2
    -D BOARD_HAS_PSRAM
    -D ARDUINO_USB_MODE=1
    -D ARDUINO_USB_CDC_ON_BOOT=1
    -O2

lib_deps =
    coryjfowler/MCP_CAN @ ^1.5.0
```

### Library Dependencies
**Essential Libraries:**
- **MCP_CAN:** CAN communication via MCP2515
  - Install via PlatformIO: `hideakitai/MCP_CAN`
  - Alternatives: "MCP2515-Arduino-Driver", "CAN-BUS Shield"

**Optional Libraries (add as needed):**
- **ArduinoJSON:** For configuration data
- **SPIFFS/LittleFS:** File system storage
- **EEPROM:** Non-volatile memory

---

## Memory Management Strategy

### Strategy: Balanced (Speed + Memory Efficiency)

**RAM Allocation:**
- Global variables: Keep < 50 KB
- Stack per function: Estimate and avoid deep recursion
- Dynamic allocation: Use malloc/free sparingly; prefer static buffers

**Best Practices:**
```cpp
// ✓ GOOD - Static allocation (predictable)
uint8_t buffer[256];

// ✗ AVOID - Repeated dynamic allocation
uint8_t* data = (uint8_t*)malloc(256);  // memory fragmentation risk

// ✓ GOOD - Object pool for fixed resources
struct CANMessage {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
};
CANMessage msgPool[16];  // Pre-allocated
```

**PSRAM Usage (if available):**
- Enable only if needed: `CONFIG_SPIRAM_MODE`
- Use for large buffers (image processing, audio)
- Slower than SRAM (~5x slower)

---

## Real-Time Constraints

### Timing Critical (Hard Real-Time)
- **CAN Message Processing:** < 5 ms
- **Interrupt Service Routines (ISR):** < 100 μs
- **Main Loop Cycle:** Target 10-50 ms

**Timing Guarantees:**
```cpp
// Use hardware timers for critical timing
hw_timer_t * timer = timerBegin(0, 80, true);  // 1 MHz
timerAlarmWrite(timer, 5000, true);  // 5 ms interrupt (no WiFi interference)
timerAlarmEnable(timer);
```

### WiFi Impact
- WiFi operations are NOT real-time safe
- Keep WiFi transmissions in non-critical paths
- Consider WiFi off during CAN message bursts

---

## Bare Metal Architecture

**No OS/RTOS Used:** Direct hardware control

### Program Structure
```
setup()
├─ Initialize GPIO/SPI
├─ Initialize CAN (MCP2515)
├─ Configure Serial debug
└─ Setup interrupts

loop()
├─ Check CAN messages (polled or ISR-driven)
├─ Process CAN data
├─ Optional WiFi operations
└─ Yield to watchdog
```

### Interrupt Handling
```cpp
// ISR best practice - keep short!
volatile bool canMessageReady = false;

void IRAM_ATTR canInterruptHandler() {
    canMessageReady = true;
    // Minimal processing, defer heavy work to loop()
}

attachInterrupt(digitalPinToInterrupt(INT_PIN), canInterruptHandler, FALLING);
```

---

## Data Persistence

### Available Storage Options

**1. NVS (Non-Volatile Storage) - Recommended**
- Hardware-backed flash memory
- Key-value store, survives reboots
- Use for: Configuration, calibration, device ID
```cpp
#include <nvs_flash.h>
nvs_flash_init();
```

**2. SPIFFS (SPI Flash File System)**
- File-based storage
- Limited wear leveling
- Use for: Log files, JSON configs

**3. EEPROM (Emulated)**
- 512 bytes to 4 KB
- Use for: Small configuration parameters only

**Recommended Strategy:**
- **NVS** for: Device settings, WiFi credentials, CAN filters
- **SPIFFS** for: Error logs, data logging
- **Avoid:** Frequent writes to same location (flash wear)

---

## Logging & Debugging

### Logging Level: Adaptive (DEBUG when needed, INFO in production)

**Logging Macros:**
```cpp
#define DEBUG_LEVEL 2  // 0=NONE, 1=ERROR, 2=INFO, 3=DEBUG

#define LOG_ERROR(msg) if(DEBUG_LEVEL >= 1) Serial.println("[ERR] " + String(msg))
#define LOG_INFO(msg)  if(DEBUG_LEVEL >= 2) Serial.println("[INF] " + String(msg))
#define LOG_DEBUG(msg) if(DEBUG_LEVEL >= 3) Serial.println("[DBG] " + String(msg))
```

**Serial Configuration:**
- Baud Rate: 115200
- Encoding: UTF-8
- Buffer Size: 256 bytes (default)

**Debug Output Format:**
```
[HH:MM:SS.mmm] [LVL] Module: Message
Example: [14:32:55.423] [INF] CAN: Message RX ID=0x123
```

---

## Coding Standards & Best Practices

### Embedded-Specific Guidelines

**1. Avoid Blocking Code**
```cpp
// ✗ AVOID - Blocks entire system
delay(1000);
while(Serial.available() == 0);

// ✓ GOOD - Non-blocking
static unsigned long lastCheck = 0;
if (millis() - lastCheck > 1000) {
    lastCheck = millis();
    // Execute task
}
```

**2. Stack Safety**
```cpp
// ✗ AVOID - Large stack allocations
void badFunction() {
    uint8_t bigBuffer[4096];  // Stack overflow risk
}

// ✓ GOOD - Static or heap allocation with bounds checks
static uint8_t bigBuffer[4096];
```

**3. ISR Safety**
```cpp
// Use IRAM for critical ISRs
void IRAM_ATTR fastISR() {
    // Must be under 100 μs
    // No Serial, no malloc, no String
}
```

**4. Memory Efficiency**
- Use `const` for fixed data
- Store strings in PROGMEM: `const char msg[] PROGMEM = "Hello";`
- Minimize global variables
- Use bitwise operations for flags

**5. Power Consumption**
```cpp
// Enable power saving modes when idle
esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);

// Reduce CPU frequency if allowed by timing
setCpuFrequencyMhz(80);  // Default 240 MHz
```

---

## CAN Communication Configuration

### MCP2515 Setup
```cpp
#include <MCP_CAN.h>
#include <SPI.h>

// SPI pins (LilyGo T-2CAN)
#define MCP_CS_PIN    10
#define MCP_SCK_PIN   12
#define MCP_MOSI_PIN  11
#define MCP_MISO_PIN  13
#define MCP_RST_PIN   9

MCP_CAN CAN(MCP_CS_PIN);

void setupCAN() {
    // Hardware reset MCP2515
    pinMode(MCP_RST_PIN, OUTPUT);
    digitalWrite(MCP_RST_PIN, HIGH);
    delay(100);
    digitalWrite(MCP_RST_PIN, LOW);
    delay(100);
    digitalWrite(MCP_RST_PIN, HIGH);
    delay(100);

    SPI.begin(MCP_SCK_PIN, MCP_MISO_PIN, MCP_MOSI_PIN, MCP_CS_PIN);

    if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK) {
        CAN.setMode(MCP_LISTENONLY);  // Listen-only for Tesla
    }
}
```

### CAN Message Structure
```cpp
struct CANMessage {
    uint32_t id;      // CAN ID (11-bit or 29-bit)
    uint8_t dlc;      // Data Length (0-8)
    uint8_t data[8];  // Payload
    uint32_t timestamp;  // For timestamping
};
```

---

## WiFi Integration (Secondary)

### WiFi Profile
- Mode: Station (STA)
- Security: WPA2/WPA3
- Power: Medium (WiFi off during CAN critical periods)
- Reconnect Strategy: Exponential backoff

**Avoid:** Mixing WiFi transmissions with hard real-time CAN processing

---

## Watchdog Configuration

**Hardware Watchdog (IWDT):**
```cpp
// Initialize watchdog timer
esp_task_wdt_init(watchdogTimeout, true);
esp_task_wdt_add(NULL);

// Feed watchdog in loop
esp_task_wdt_reset();
```

**Recommended Timeout:** 5-10 seconds (tuneable based on loop speed)

---

## Performance Monitoring

### Key Metrics to Track
- **CAN Message Loss:** Count failed RX/TX
- **Loop Cycle Time:** Measure via timestamp
- **Free Heap Memory:** Monitor for leaks
- **Watchdog Resets:** Sign of blocking code
- **Processing Latency:** CAN message to response time

---

## Build & Upload

### PlatformIO Commands
```bash
# Build
platformio run -e esp32dev

# Upload
platformio run -e esp32dev -t upload

# Monitor serial output
platformio device monitor -e esp32dev

# Clean
platformio run -e esp32dev -t clean

# Full build + upload + monitor
platformio run -e esp32dev -t upload --monitor
```

---

## Version Control & Documentation

- Maintain git history (discard binaries)
- Document all GPIO assignments
- Keep README with setup instructions
- Version firmware with semantic versioning (v1.2.3)

---

## Checklist for New Modules

When adding new functionality:

- [ ] Declare all pins used
- [ ] Estimate RAM/Flash requirements
- [ ] Check SPI/I2C address conflicts
- [ ] Measure ISR execution time
- [ ] Verify non-blocking implementation
- [ ] Add library to `platformio.ini`
- [ ] Test memory stability (24+ hours idle)
- [ ] Validate real-time constraints
- [ ] Document power consumption
- [ ] Update this environment file

---

## Resources & References

- **MCP2515 Datasheet:** https://ww1.microchip.com/downloads/en/DeviceDoc/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf
- **ESP32 Datasheet:** https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
- **Arduino-ESP32 Core:** https://github.com/espressif/arduino-esp32
- **PlatformIO Docs:** https://docs.platformio.org/

---

**Last Updated:** 22 février 2026
