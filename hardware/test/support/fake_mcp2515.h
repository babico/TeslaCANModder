#pragma once
// Fake MCP2515 for native tests — no SPI hardware needed.

#include <cstdint>
#include <cstring>
#include <vector>
#include <queue>

#define MCP2515_ERROR_OK    0
#define MCP2515_ERROR_FAIL  1
#define MCP2515_ERROR_NOMSG 2

// Match the real mcp2515 library's can_frame layout
struct can_frame {
  uint32_t can_id = 0;
  uint8_t  can_dlc = 0;
  uint8_t  data[8] = {};
};

// Stub SPI class (driver may reference SPI.begin)
struct SPIClass {
  void begin(int = -1, int = -1, int = -1, int = -1) {}
};
static SPIClass SPI;

class MCP2515 {
public:
  explicit MCP2515(uint8_t cs) : _cs(cs) {}

  uint8_t reset() { return _fail_reset ? MCP2515_ERROR_FAIL : MCP2515_ERROR_OK; }
  uint8_t setBitrate(long, int = 0) { return MCP2515_ERROR_OK; }
  uint8_t setNormalMode() { return MCP2515_ERROR_OK; }
  uint8_t setListenOnlyMode() { return MCP2515_ERROR_OK; }

  uint8_t readMessage(can_frame* f) {
    if (_rx.empty()) return MCP2515_ERROR_NOMSG;
    *f = _rx.front();
    _rx.pop();
    return MCP2515_ERROR_OK;
  }

  uint8_t sendMessage(const can_frame* f) {
    _tx_log.push_back(*f);
    return MCP2515_ERROR_OK;
  }

  void pushRx(uint32_t id, const uint8_t* data, uint8_t dlc) {
    can_frame f;
    f.can_id = id;
    f.can_dlc = dlc;
    memcpy(f.data, data, dlc > 8 ? 8 : dlc);
    _rx.push(f);
  }

  uint8_t csPin() const { return _cs; }

  bool _fail_reset = false;
  std::vector<can_frame> _tx_log;

private:
  uint8_t _cs;
  std::queue<can_frame> _rx;
};

// CAN clock constants the driver may reference
#define MCP_8MHZ  8
#define MCP_16MHZ 16
#define CAN_500KBPS 500
