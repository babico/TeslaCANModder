# CanFeather ทำงานยังไง — อธิบายทีละขั้นตอน

## ภาพรวม

CanFeather เป็น firmware สำหรับบอร์ด Adafruit Feather CAN ที่ทำหน้าที่เป็น **"คนกลาง" บน CAN Bus ของ Tesla** โดย:

1. **ดักฟัง** CAN frame ที่เกี่ยวกับ Autopilot
2. **ตรวจสอบ** ว่าผู้ใช้เปิด "Traffic Light and Stop Sign Control" หรือยัง
3. **แก้ไข bit เฉพาะจุด** ใน frame นั้น เพื่อเปิด FSD, ตั้ง speed profile, และกด nag
4. **ส่ง frame ที่แก้แล้ว** กลับเข้า CAN Bus

```
┌──────────┐    CAN Bus     ┌──────────────┐    CAN Bus     ┌──────────┐
│  Tesla    │ ──── frame ──→ │  CanFeather  │ ── modified ──→│  Tesla   │
│  ECU      │                │  (ดักแก้ไข)   │    frame       │  AP ECU  │
└──────────┘                 └──────────────┘                └──────────┘
```

---

## 1. การเริ่มต้นระบบ (`setup()`)

เมื่อบอร์ดเปิดเครื่อง จะทำ 3 อย่าง:

```cpp
void setup() {
  handler = std::make_unique<HW>();     // สร้าง handler ตามรุ่นรถ (LEGACY/HW3/HW4)
  delay(1500);
  Serial.begin(115200);                 // เปิด Serial สำหรับ debug

  mcp = std::make_unique<MCP2515>(CAN_CS);
  mcp->reset();
  mcp->setBitrate(CAN_500KBPS, MCP_16MHZ);  // Tesla CAN Bus วิ่งที่ 500 kbit/s
  mcp->setNormalMode();                       // เข้า Normal Mode (อ่าน+เขียน)
}
```

**ขั้นตอน:**
- เลือก handler ที่ตรงกับรุ่นรถ (`#define HW HW3` ที่ต้นไฟล์)
- ตั้ง MCP2515 ให้วิ่ง 500 kbit/s ตรงกับ CAN Bus ของ Tesla
- เข้า **Normal Mode** = สามารถทั้งอ่านและเขียน CAN frame ได้

---

## 2. Loop หลัก — อ่าน CAN Frame ตลอดเวลา

```cpp
__attribute__((optimize("O3"))) void loop() {
  can_frame frame;
  int r = mcp->readMessage(&frame);   // อ่าน CAN frame ที่เข้ามา
  if (r != MCP2515::ERROR_OK) {
    digitalWrite(LED_PIN, HIGH);       // ไม่มี frame → LED ติด
    return;
  }
  digitalWrite(LED_PIN, LOW);          // ได้ frame → LED ดับ
  handler->handelMessage(frame);       // ส่งต่อให้ handler จัดการ
}
```

- ใช้ `optimize("O3")` เพื่อให้ loop ทำงานเร็วที่สุด (CAN Bus ส่งข้อมูลเร็วมาก)
- ทุก frame ที่อ่านได้จะถูกส่งให้ handler ที่ตรงกับรุ่นรถ

---

## 3. โครงสร้าง Handler — Polymorphism ตามรุ่นรถ

```cpp
struct CarManagerBase {
  int speedProfile = 1;
  bool FSDEnabled = false;
  virtual void handelMessage(can_frame& frame);
};
```

มี 3 class ลูกสำหรับ 3 รุ่น:

| Class | รุ่นรถ | CAN ID ที่สนใจ |
|-------|--------|----------------|
| `LegacyHandler` | HW3 Retrofit (จอตั้ง) | `1006` |
| `HW3Handler` | HW3 (จอนอน) | `1016`, `1021` |
| `HW4Handler` | HW4 | `1016`, `1021` |

โปรแกรมเลือก handler ตอน compile:

```cpp
#define LEGACY LegacyHandler
#define HW3    HW3Handler
#define HW4    HW4Handler

#define HW HW3  // ← เปลี่ยนตรงนี้ตามรุ่นรถ

// ตอน setup()
handler = std::make_unique<HW>();  // จะกลายเป็น std::make_unique<HW3Handler>()
```

---

## 4. ฟังก์ชัน Utility สำคัญ

### 4.1 อ่าน Mux ID — ระบุ "ส่วน" ของ frame

```cpp
inline uint8_t readMuxID(const can_frame& frame) {
  return frame.data[0] & 0x07;  // 3 bit ล่างของ byte แรก
}
```

CAN frame เดียว (เช่น ID 1021) สามารถมีข้อมูลหลายประเภทใน payload เดียวกัน โดยใช้ **mux index** แยก:
- `index 0` → ข้อมูล FSD enable + speed offset
- `index 1` → ข้อมูล nag (hands-on-wheel)
- `index 2` → ข้อมูล speed profile

### 4.2 ตรวจว่าเปิด FSD ใน UI หรือยัง

```cpp
inline bool isFSDSelectedInUI(const can_frame& frame) {
  return (frame.data[4] >> 6) & 0x01;  // bit 6 ของ byte ที่ 4
}
```

ตรวจจาก CAN frame ว่าคนขับเปิด **"Traffic Light and Stop Sign Control"** ใน Autopilot settings หรือไม่ ถ้าเปิด = trigger ให้เปิด FSD

### 4.3 ตั้งค่า bit ใน frame

```cpp
inline void setBit(can_frame& frame, int bit, bool value) {
  int byteIndex = bit / 8;
  int bitIndex  = bit % 8;
  uint8_t mask  = static_cast<uint8_t>(1U << bitIndex);
  if (value) {
    frame.data[byteIndex] |= mask;     // set bit เป็น 1
  } else {
    frame.data[byteIndex] &= ~mask;    // clear bit เป็น 0
  }
}
```

ใช้แก้ไข bit ระดับ CAN frame เช่น `setBit(frame, 46, true)` = set bit ที่ 46 ให้เป็น 1

### 4.4 ตั้ง Speed Profile (V12/V13)

```cpp
inline void setSpeedProfileV12V13(can_frame& frame, int profile) {
  frame.data[6] &= ~0x06;           // เคลียร์ bit 1-2 ของ byte 6
  frame.data[6] |= (profile << 1);  // ใส่ค่า profile (0-2)
}
```

---

## 5. การทำงานของแต่ละ Handler

### 5.1 HW3Handler (ใช้บ่อยที่สุด)

```
CAN Frame เข้ามา
      │
      ├── CAN ID = 1016 → อ่าน Follow Distance จาก byte 5
      │     followDistance = (frame.data[5] & 0b11100000) >> 5
      │     แปลงเป็น speedProfile:
      │       1 → Hurry (2)
      │       2 → Normal (1)
      │       3 → Chill (0)
      │
      └── CAN ID = 1021 → ตรวจ mux index
            │
            ├── index=0 + FSD เปิดใน UI
            │     ✅ setBit(46, true)     → เปิด FSD enable bit
            │     ✅ setSpeedProfile()    → ตั้ง speed profile
            │     ✅ sendMessage()        → ส่ง frame กลับ CAN Bus
            │
            ├── index=1
            │     ✅ setBit(19, false)    → ลบ nag (hands-on-wheel)
            │     ✅ sendMessage()
            │
            └── index=2 + FSD เปิดใน UI
                  ✅ แก้ speed offset ใน byte 0-1
                  ✅ sendMessage()
```

**โค้ดจริง:**

```cpp
struct HW3Handler : public CarManagerBase {
  int speedOffset = 0;
  virtual void handelMessage(can_frame& frame) override {

    // ---- ขั้นที่ 1: อ่าน Follow Distance จาก CAN ID 1016 ----
    if (frame.can_id == 1016) {
      uint8_t followDistance = (frame.data[5] & 0b11100000) >> 5;
      switch (followDistance) {
        case 1: speedProfile = 2; break;  // Hurry
        case 2: speedProfile = 1; break;  // Normal
        case 3: speedProfile = 0; break;  // Chill
      }
      return;
    }

    // ---- ขั้นที่ 2: จัดการ CAN ID 1021 ----
    if (frame.can_id == 1021) {
      auto index = readMuxID(frame);
      auto FSDEnabled = isFSDSelectedInUI(frame);

      // mux 0: เปิด FSD + ตั้ง speed profile
      if (index == 0 && FSDEnabled) {
        setBit(frame, 46, true);                    // FSD enable bit
        setSpeedProfileV12V13(frame, speedProfile);  // speed profile
        mcp->sendMessage(&frame);                    // ส่งกลับ CAN Bus
      }

      // mux 1: ลบ nag (hands-on-wheel warning)
      if (index == 1) {
        setBit(frame, 19, false);   // clear nag bit
        mcp->sendMessage(&frame);
      }

      // mux 2: ตั้ง speed offset
      if (index == 2 && FSDEnabled) {
        frame.data[0] &= ~(0b11000000);
        frame.data[1] &= ~(0b00111111);
        frame.data[0] |= (speedOffset & 0x03) << 6;
        frame.data[1] |= (speedOffset >> 2);
        mcp->sendMessage(&frame);
      }
    }
  }
};
```

### 5.2 HW4Handler — เพิ่ม FSDV14 + 5 Speed Levels

ต่างจาก HW3 ตรงที่:

```cpp
// CAN ID 1016: Follow Distance → 5 ระดับ (แทนที่จะเป็น 3)
switch(fd) {
  case 1: speedProfile = 3; break;  // Max
  case 2: speedProfile = 2; break;  // Hurry
  case 3: speedProfile = 1; break;  // Normal
  case 4: speedProfile = 0; break;  // Chill
  case 5: speedProfile = 4; break;  // Sloth
}

// CAN ID 1021, index=0: set bit เพิ่มอีก 1 ตัว (FSDV14)
if (index == 0 && FSDEnabled) {
  setBit(frame, 46, true);   // FSD enable
  setBit(frame, 60, true);   // FSDV14 enable (ใหม่สำหรับ HW4)
  mcp->sendMessage(&frame);
}

// CAN ID 1021, index=1: set bit เพิ่มอีก 1 ตัว
if (index == 1) {
  setBit(frame, 19, false);  // clear nag
  setBit(frame, 47, true);   // bit เพิ่มเติมสำหรับ HW4
  mcp->sendMessage(&frame);
}

// CAN ID 1021, index=2: speed profile เก็บใน byte 7 แทน byte 6
if (index == 2) {
  frame.data[7] &= ~(0x07 << 4);              // เคลียร์ 3 bit
  frame.data[7] |= (speedProfile & 0x07) << 4; // ใส่ค่า 0-4
  mcp->sendMessage(&frame);
}
```

### 5.3 LegacyHandler — HW3 Retrofit (จอตั้ง)

ใช้ CAN ID `1006` อันเดียว แทนที่จะเป็น 2 ID:

```cpp
if (frame.can_id == 1006) {
  auto index = readMuxID(frame);

  // mux 0: อ่าน speed offset จาก frame → แปลงเป็น profile → เปิด FSD
  if (index == 0 && isFSDSelectedInUI(frame)) {
    auto off = (uint8_t)((frame.data[3] >> 1) & 0x3F) - 30;
    // off=0 → Chill, off=1 → Normal, off=2 → Hurry
    setBit(frame, 46, true);
    setSpeedProfileV12V13(frame, speedProfile);
    mcp->sendMessage(&frame);
  }

  // mux 1: ลบ nag
  if (index == 1) {
    setBit(frame, 19, false);
    mcp->sendMessage(&frame);
  }
}
```

---

## 6. CAN Frame Bit Map — สรุป bit ที่แก้

### CAN ID 1021 (HW3/HW4) / CAN ID 1006 (Legacy)

```
Byte:   [0]      [1]      [2]      [3]      [4]      [5]      [6]      [7]
Bit:  0......7 8.....15 16....23 24....31 32....39 40....47 48....55 56....63

Bit 46 (byte 5, bit 6) → FSD Enable          ← set true เมื่อเปิด FSD
Bit 19 (byte 2, bit 3) → Hands-on-Wheel Nag  ← set false เพื่อลบ nag
Bit 60 (byte 7, bit 4) → FSDV14 Enable       ← HW4 only
Bit 47 (byte 5, bit 7) → HW4 extra bit       ← HW4 only

Byte 6, bit 1-2        → Speed Profile (Legacy/HW3, V12/V13)
Byte 7, bit 4-6        → Speed Profile (HW4, 3 bits สำหรับ 5 ระดับ)
```

---

## 7. Flow สรุปทั้งหมด

```
บอร์ดเปิดเครื่อง
     │
     ▼
setup(): เลือก Handler → ตั้ง MCP2515 → 500kbps → Normal Mode
     │
     ▼
┌──────────────────────── loop() ────────────────────────┐
│                                                         │
│  อ่าน CAN frame จาก MCP2515                             │
│     │                                                   │
│     ├── ไม่มี frame → LED ติด → กลับไป loop             │
│     │                                                   │
│     └── ได้ frame → LED ดับ → ส่งให้ handler            │
│            │                                            │
│            ├── CAN ID 1016: อ่าน follow distance        │
│            │   → เก็บเป็น speedProfile                   │
│            │                                            │
│            └── CAN ID 1021:                             │
│                  ├── mux 0 + FSD on                     │
│                  │   → set FSD bit → set speed profile  │
│                  │   → ส่ง frame กลับ CAN Bus            │
│                  │                                      │
│                  ├── mux 1                              │
│                  │   → clear nag bit                    │
│                  │   → ส่ง frame กลับ CAN Bus            │
│                  │                                      │
│                  └── mux 2 + FSD on                     │
│                      → set speed offset/profile         │
│                      → ส่ง frame กลับ CAN Bus            │
│                                                         │
└─────────────────── วนซ้ำตลอดเวลา ──────────────────────┘
```

---

## 8. Pin ที่ใช้บนบอร์ด

```cpp
#define LED_PIN    PIN_LED            // GPIO13 — LED แสดงสถานะ
#define CAN_CS     PIN_CAN_CS         // GPIO19 — SPI Chip Select
#define CAN_INT    PIN_CAN_INTERRUPT  // GPIO22 — Interrupt (ไม่ได้ใช้, ใช้ polling)
#define CAN_STBY   PIN_CAN_STANDBY    // GPIO16 — CAN Transceiver standby
#define CAN_RESET  PIN_CAN_RESET      // GPIO18 — MCP2515 hardware reset
```

---

## 9. สรุปง่ายๆ

| สิ่งที่ทำ | วิธีการ |
|----------|--------|
| **เปิด FSD** | Set bit 46 = 1 ใน CAN ID 1021 (mux 0) |
| **เปิด FSDV14** | Set bit 60 = 1 (HW4 เท่านั้น) |
| **ลบ Nag** | Set bit 19 = 0 ใน CAN ID 1021 (mux 1) |
| **ตั้ง Speed** | แก้ byte 6 หรือ 7 ตาม HW version |
| **Trigger** | ผู้ใช้เปิด "Traffic Light and Stop Sign Control" ใน UI |
| **ความเร็ว CAN** | 500 kbit/s |
