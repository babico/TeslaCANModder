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

#include <SPI.h>
#include <mcp2515.h>


#define LEGACY LegacyHandler
#define HW3 HW3Handler
#define HW4 HW4Handler //HW4 since Version 2026.2.3 uses FSDV14, before that compile for HW3, even for HW4 vehicles.


#define HW HW3  //for what car to compile

bool enablePrint = true;


#define LED_PIN     9      // 改用 D9，避免和 SPI SCK(D13) 冲突
// #define LED_PIN   13    // 原板载 LED，但 D13 同时是 SPI SCK，可能干扰通信
#define CAN_CS      10     // MCP2515 模块的 CS 引脚，接 D10
// 以下三个引脚在 Nano 上不存在板卡预定义，如不需要可删掉或手动指定
// #define CAN_INT_PIN  2   // 如需中断模式，接 D2
// #define CAN_STBY      -1 // 外部模块一般不需要
// #define CAN_RESET     -1 // 外部模块一般不需要

MCP2515 mcp(CAN_CS);

struct CarManagerBase {
  int speedProfile = 1;
  bool FSDEnabled = false;
  virtual void handelMessage(can_frame& frame);
};

inline uint8_t readMuxID(const can_frame& frame) {
  return frame.data[0] & 0x07;
}

inline bool isFSDSelectedInUI(const can_frame& frame) {
  return (frame.data[4] >> 6) & 0x01;
}

inline void setSpeedProfileV12V13(can_frame& frame, int profile) {
  frame.data[6] &= ~0x06;
  frame.data[6] |= (profile << 1);
}

inline void setBit(can_frame& frame, int bit, bool value) {
  // Determine which byte and which bit within that byte
  int byteIndex = bit / 8;
  int bitIndex = bit % 8;
  // Set the desired bit
  uint8_t mask = static_cast<uint8_t>(1U << bitIndex);
  if (value) {
    frame.data[byteIndex] |= mask;
  } else {
    frame.data[byteIndex] &= static_cast<uint8_t>(~mask);
  }
}


struct LegacyHandler : public CarManagerBase {
  virtual void handelMessage(can_frame& frame) override {
    if (frame.can_id == 1006) {
      auto index = readMuxID(frame);
      if (index == 0) {
        auto off = (uint8_t)((frame.data[3] >> 1) & 0x3F) - 30;
        switch (off) {
          case 2: speedProfile = 2; break;
          case 1: speedProfile = 1; break;
          case 0: speedProfile = 0; break;
          default: break;
        }
        setBit(frame, 46, true);
        setSpeedProfileV12V13(frame, speedProfile);
        debugSend(frame);
      }
      if (index == 1) {
        setBit(frame, 19, false);
        debugSend(frame);
      }
      if (index == 0 && enablePrint) {
        Serial.print("LegacyHandler: Profile: ");
        Serial.println(speedProfile);
      }
    }
  }
};

struct HW3Handler : public CarManagerBase {
  int speedOffset = 0;
  virtual void handelMessage(can_frame& frame) override {
    if (frame.can_id == 1016) {
      uint8_t followDistance = (frame.data[5] & 0b11100000) >> 5;
      switch (followDistance) {
        case 1:
          speedProfile = 2;
          break;
        case 2:
          speedProfile = 1;
          break;
        case 3:
          speedProfile = 0;
          break;
        default:
          break;
      }
      return;
    }
    if (frame.can_id == 1021) {
      auto index = readMuxID(frame);
      if (index == 0) {
        speedOffset = constrain(((uint8_t)((frame.data[3] >> 1) & 0x3F) - 30) * 5, 0, 100);
        auto off = (uint8_t)((frame.data[3] >> 1) & 0x3F) - 30;
        switch (off) {
          case 2: speedProfile = 2; break;
          case 1: speedProfile = 1; break;
          case 0: speedProfile = 0; break;
          default: break;
        }
        setBit(frame, 46, true);
        setSpeedProfileV12V13(frame, speedProfile);
        debugSend(frame);
      }
      if (index == 1) {
        setBit(frame, 19, false);
        debugSend(frame);
      }
      if (index == 2) {
        frame.data[0] &= ~(0b11000000);
        frame.data[1] &= ~(0b00111111);
        frame.data[0] |= (speedOffset & 0x03) << 6;
        frame.data[1] |= (speedOffset >> 2);
        debugSend(frame);
      }
      if (index == 0 && enablePrint) {
        Serial.print("HW3Handler: Profile: ");
        Serial.print(speedProfile);
        Serial.print(", Offset: ");
        Serial.println(speedOffset);
      }
    }
  }
};

struct HW4Handler : public CarManagerBase {
  virtual void handelMessage(can_frame& frame) override {
    if (frame.can_id == 1016) {
      auto fd = (frame.data[5] & 0b11100000) >> 5;
      switch(fd){
        case 1: speedProfile = 3; break;
        case 2: speedProfile = 2; break;
        case 3: speedProfile = 1; break;
        case 4: speedProfile = 0; break;
        case 5: speedProfile = 4; break;
      }
    }
    if (frame.can_id == 1021) {
      auto index = readMuxID(frame);
      if (index == 0) {
        setBit(frame, 46, true);
        setBit(frame, 60, true);
        debugSend(frame);
      }
      if (index == 1) {
        setBit(frame, 19, false);
        setBit(frame, 47, true);
        debugSend(frame);
      }
      if(index == 2){
        frame.data[7] &= ~(0x07 << 4);
        frame.data[7] |= (speedProfile & 0x07) << 4;
        debugSend(frame);
      }
      if (index == 0 && enablePrint) {
        Serial.print("HW4Handler: profile: ");
        Serial.println(speedProfile);
      }
    }
  }
};


CarManagerBase* handler;


void setup() {
  static HW hwHandler;
  handler = &hwHandler;
  pinMode(LED_PIN, OUTPUT);
  delay(1500);
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 1000) {}

  mcp.reset();
  MCP2515::ERROR e = mcp.setBitrate(CAN_500KBPS, MCP_8MHZ);
  if (e != MCP2515::ERROR_OK) Serial.println("setBitrate failed");
  mcp.setNormalMode();
  Serial.println("MCP25625 ready @ 500k 1");
}


void printFrame(const char* label, const can_frame& frame) {
  // 用于调试是否收到CAN信号
  // Serial.print(label);
  // Serial.print(" ID:");
  // Serial.print(frame.can_id);
  // Serial.print(" DLC:");
  // Serial.print(frame.can_dlc);
  // Serial.print(" Data:");
  // for (int i = 0; i < frame.can_dlc; i++) {
  //   if (frame.data[i] < 0x10) Serial.print('0');
  //   Serial.print(frame.data[i], HEX);
  //   Serial.print(' ');
  // }
  // Serial.println();
}

void debugSend(can_frame& frame) {
  if (enablePrint) printFrame("TX >>", frame);
  mcp.sendMessage(&frame);
}

void loop() {
  can_frame frame;
  int r = mcp.readMessage(&frame);
  if (r != MCP2515::ERROR_OK) {
    digitalWrite(LED_PIN, HIGH);
    return;
  }
  digitalWrite(LED_PIN, LOW);
  if (enablePrint) printFrame("RX <<", frame);
  handler->handelMessage(frame);
}
