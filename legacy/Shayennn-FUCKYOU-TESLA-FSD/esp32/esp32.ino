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

#include <cstring>
#include <driver/twai.h>
#include <esp_system.h>

#include "../shared/vehicle_logic.h"

#define LED_PIN    2
#define CAN_RX_PIN GPIO_NUM_27
#define CAN_TX_PIN GPIO_NUM_26

bool canInit() {
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t  t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t  f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) return false;
  if (twai_start() != ESP_OK) return false;
  return true;
}

int canRead(tesla_fsd::can_frame& frame) {
  twai_message_t msg;
  if (twai_receive(&msg, 0) != ESP_OK) return -1;
  frame.can_id  = msg.identifier;
  frame.can_dlc = msg.data_length_code;
  memcpy(frame.data, msg.data, 8);
  return 0;
}

bool canSend(const tesla_fsd::can_frame& frame) {
  twai_message_t msg = {};
  msg.identifier       = frame.can_id;
  msg.data_length_code = frame.can_dlc;
  memcpy(msg.data, frame.data, 8);
  return twai_transmit(&msg, 0) == ESP_OK;
}

namespace {

using ActiveHandler = tesla_fsd::HW3Handler;

constexpr uint8_t kTxFailureLimit = 8;

ActiveHandler handler;
tesla_fsd::FrameSink frameSink;
bool fastPathFailSafe = false;
uint8_t consecutiveTxFailures = 0;
uint32_t totalTxFailures = 0;
uint32_t totalTxSuccesses = 0;

bool sendFrameViaTwai(void*, const tesla_fsd::can_frame& frame) {
  return canSend(frame);
}

void enterFailSafe() {
  if (fastPathFailSafe) return;
  fastPathFailSafe = true;
  digitalWrite(LED_PIN, HIGH);
  Serial.printf("Fast path fail-safe after %lu TWAI TX failures\n",
                static_cast<unsigned long>(totalTxFailures));
}

}  // namespace

void setup() {
  delay(1500);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 1000) {}

  frameSink = {nullptr, &sendFrameViaTwai};

  Serial.printf("Reset reason: %d\n", static_cast<int>(esp_reset_reason()));

  if (!canInit()) {
    Serial.println("CAN init failed");
    while (true) { delay(1000); }
  }
  Serial.println("CAN ready @ 500k");
}


__attribute__((optimize("O3"))) void loop() {
  tesla_fsd::can_frame frame;
  if (canRead(frame) != 0) return;
  if (fastPathFailSafe) return;

  const tesla_fsd::HandleResult result = handler.handleMessage(frame, frameSink);
  if (!result.handled || !result.attemptedSend) return;

  if (result.sent) {
    consecutiveTxFailures = 0;
    ++totalTxSuccesses;
    return;
  }

  ++totalTxFailures;
  if (++consecutiveTxFailures >= kTxFailureLimit) {
    enterFailSafe();
  }
}
