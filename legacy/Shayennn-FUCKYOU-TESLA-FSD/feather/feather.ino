/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <cstring>

#include <Adafruit_NeoPixel.h>
#include <CANSAME5x.h>

#include "vehicle_logic.h"

using tesla_fsd::can_frame;

#define NAV_STOPS_ENABLED false  // Toggle before upload: true or false.

CANSAME5x CAN;
Adafruit_NeoPixel pixel(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

const uint32_t COLOR_RED = Adafruit_NeoPixel::Color(255, 0, 0);
const uint32_t COLOR_GREEN = Adafruit_NeoPixel::Color(0, 255, 0);
const uint32_t COLOR_BLUE = Adafruit_NeoPixel::Color(0, 0, 255);
const uint32_t COLOR_YELLOW = Adafruit_NeoPixel::Color(255, 255, 0);
const uint32_t COLOR_PURPLE = Adafruit_NeoPixel::Color(128, 0, 255);

inline void setNeoColor(uint32_t color) {
  pixel.setPixelColor(0, color);
  pixel.show();
}

#define LEGACY tesla_fsd::LegacyHandler
#define HW3 tesla_fsd::HW3Handler
#define HW4 tesla_fsd::HW4Handler  // HW4 since version 2026.2.3 uses FSDV14.

#define HW HW3  // Change to LEGACY, HW3, or HW4.

#define ENABLE_PRINT

#define LED_PIN PIN_LED

inline uint8_t sanitizeDlc(int dlc) {
  if (dlc < 0) return 0;
  if (dlc > 8) return 8;
  return static_cast<uint8_t>(dlc);
}

int canRead(can_frame& frame) {
  int packetSize = CAN.parsePacket();
  if (packetSize <= 0 || CAN.packetRtr()) return -1;

  // Keep the original DLC even when the library reports a shorter payload.
  frame.can_id = static_cast<uint32_t>(CAN.packetId());
  frame.can_dlc = sanitizeDlc(CAN.packetDlc() > 0 ? CAN.packetDlc() : packetSize);
  memset(frame.data, 0, sizeof(frame.data));

  const int bytesToRead = std::min(packetSize, static_cast<int>(sizeof(frame.data)));
  for (int i = 0; i < bytesToRead; i++) {
    frame.data[i] = CAN.read();
  }

  while (CAN.available()) {
    CAN.read();
  }

  return 0;
}

bool canSend(const can_frame& frame) {
  const uint8_t dlc = sanitizeDlc(frame.can_dlc);
  const bool began = frame.can_id > 0x7FF ? CAN.beginExtendedPacket(frame.can_id)
                                          : CAN.beginPacket(frame.can_id);
  if (!began) {
#ifdef ENABLE_PRINT
    Serial.printf("CAN TX begin failed for id=0x%03lX\n", static_cast<unsigned long>(frame.can_id));
#endif
    return false;
  }

  if (CAN.write(frame.data, dlc) != dlc) {
#ifdef ENABLE_PRINT
    Serial.printf("CAN TX write failed for id=0x%03lX len=%u\n",
                  static_cast<unsigned long>(frame.can_id), dlc);
#endif
    return false;
  }

  if (!CAN.endPacket()) {
#ifdef ENABLE_PRINT
    Serial.printf("CAN TX end failed for id=0x%03lX\n", static_cast<unsigned long>(frame.can_id));
#endif
    return false;
  }

  return true;
}

namespace {

using ActiveHandler = HW;

constexpr uint8_t kTxFailureLimit = 8;

ActiveHandler handler;
tesla_fsd::FrameSink frameSink;
uint8_t consecutiveTxFailures = 0;
bool failSafe = false;

bool sendFrameViaCan(void*, const can_frame& frame) {
  return canSend(frame);
}

#ifdef ENABLE_PRINT
void printHandlerState(const tesla_fsd::LegacyHandler& active, const can_frame& frame) {
  if (frame.can_id == 1006 && tesla_fsd::readMuxID(frame) == 0) {
    Serial.printf("Legacy profile=%d\n", active.speedProfile);
  }
}

void printHandlerState(const tesla_fsd::HW3Handler& active, const can_frame& frame) {
  if (frame.can_id == tesla_fsd::UI_DRIVER_ASSIST_CONTROL_ID ||
      (frame.can_id == tesla_fsd::UI_AUTOPILOT_CONTROL_ID && tesla_fsd::readMuxID(frame) == 0)) {
    Serial.printf("HW3 profile=%d offset=%d\n", active.speedProfile, active.speedOffset);
  }
}

void printHandlerState(const tesla_fsd::HW4Handler& active, const can_frame& frame) {
  if (frame.can_id == tesla_fsd::UI_DRIVER_ASSIST_CONTROL_ID ||
      (frame.can_id == tesla_fsd::UI_AUTOPILOT_CONTROL_ID && tesla_fsd::readMuxID(frame) == 2)) {
    Serial.printf("HW4 profile=%d\n", active.speedProfile);
  }
}
#else
template <typename Handler>
void printHandlerState(const Handler&, const can_frame&) {}
#endif

void enterFailSafe() {
  if (failSafe) return;
  failSafe = true;
  setNeoColor(COLOR_RED);
#ifdef ENABLE_PRINT
  Serial.println("CAN fast-path fail-safe after repeated TX failures");
#endif
}

}  // namespace


void setup() {
  pinMode(PIN_NEOPIXEL_POWER, OUTPUT);
  digitalWrite(PIN_NEOPIXEL_POWER, HIGH);
  pixel.begin();
  pixel.setBrightness(30);
  setNeoColor(COLOR_BLUE);

  delay(1500);

  handler.navStopsEnabled = NAV_STOPS_ENABLED;
  if (!NAV_STOPS_ENABLED) {
    setNeoColor(COLOR_PURPLE);
    delay(500);
    setNeoColor(COLOR_BLUE);
  }

  frameSink = {nullptr, &sendFrameViaCan};

#ifdef ENABLE_PRINT
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 1000) {}
#endif

  pinMode(PIN_CAN_STANDBY, OUTPUT);
  digitalWrite(PIN_CAN_STANDBY, false);
  pinMode(PIN_CAN_BOOSTEN, OUTPUT);
  digitalWrite(PIN_CAN_BOOSTEN, true);

  if (!CAN.begin(500E3)) {
#ifdef ENABLE_PRINT
    Serial.println("CAN init failed");
#endif
    setNeoColor(COLOR_RED);
    while (true) { delay(1000); }
  }
#ifdef ENABLE_PRINT
  Serial.println("CAN ready @ 500k");
  Serial.printf("navStopsEnabled=%d\n", handler.navStopsEnabled ? 1 : 0);
#endif
}


__attribute__((optimize("O3"))) void loop() {
  can_frame frame;
  if (canRead(frame) != 0) {
    setNeoColor(COLOR_BLUE);
    return;
  }

  if (failSafe) return;

  const tesla_fsd::HandleResult result = handler.handleMessage(frame, frameSink);
  if (!result.handled || !result.attemptedSend) {
    setNeoColor(COLOR_YELLOW);
    return;
  }

  if (!result.sent) {
    if (++consecutiveTxFailures >= kTxFailureLimit) {
      enterFailSafe();
    } else {
      setNeoColor(COLOR_RED);
    }
    return;
  }

  consecutiveTxFailures = 0;
  setNeoColor(COLOR_GREEN);
  printHandlerState(handler, frame);
}
