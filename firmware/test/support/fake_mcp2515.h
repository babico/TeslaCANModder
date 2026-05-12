#pragma once

/**
 * @file firmware/test/support/fake_mcp2515.h
 * @brief Fake MCP2515 driver for native PlatformIO tests — no SPI hardware needed
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <cstdint>
#include <cstring>
#include <vector>
#include <queue>

#define MCP2515_ERROR_OK 0
#define MCP2515_ERROR_FAIL 1
#define MCP2515_ERROR_NOMSG 2

/**
 * @brief CAN frame structure matching the real mcp2515 library layout
 */
struct can_frame
{
	uint32_t can_id = 0;   // 11-bit or 29-bit CAN identifier
	uint8_t can_dlc = 0;   // Data length code (0–8)
	uint8_t data[8] = {};  // Payload bytes
};

/**
 * @brief Stub SPI class so driver code referencing SPI.begin() compiles
 */
struct SPIClass
{
	/**
	 * @brief No-op SPI initialization
	 */
	void begin(int = -1, int = -1, int = -1, int = -1) {}
};
static SPIClass SPI;

/**
 * @brief In-memory fake of the MCP2515 CAN controller for unit testing
 */
class MCP2515
{
  public:
	/**
	 * @brief Construct a fake MCP2515 bound to a chip-select pin
	 * @param cs Chip-select pin number (stored but unused in tests)
	 */
	explicit MCP2515(uint8_t cs) : _cs(cs) {}

	/**
	 * @brief Simulate MCP2515 reset; can be forced to fail via _fail_reset
	 * @return MCP2515_ERROR_OK on success, MCP2515_ERROR_FAIL if _fail_reset is set
	 */
	uint8_t reset()
	{
		return _fail_reset ? MCP2515_ERROR_FAIL : MCP2515_ERROR_OK;
	}

	/**
	 * @brief No-op bitrate configuration
	 * @return MCP2515_ERROR_OK always
	 */
	uint8_t setBitrate(long, int = 0)
	{
		return MCP2515_ERROR_OK;
	}

	/**
	 * @brief No-op normal mode transition
	 * @return MCP2515_ERROR_OK always
	 */
	uint8_t setNormalMode()
	{
		return MCP2515_ERROR_OK;
	}

	/**
	 * @brief No-op listen-only mode transition
	 * @return MCP2515_ERROR_OK always
	 */
	uint8_t setListenOnlyMode()
	{
		return MCP2515_ERROR_OK;
	}

	/**
	 * @brief Pop the next frame from the internal RX queue
	 * @param f Pointer to frame struct to populate
	 * @return MCP2515_ERROR_OK if a frame was available, MCP2515_ERROR_NOMSG otherwise
	 */
	uint8_t readMessage(can_frame *f)
	{
		if (_rx.empty())
			return MCP2515_ERROR_NOMSG;
		*f = _rx.front();
		_rx.pop();
		return MCP2515_ERROR_OK;
	}

	/**
	 * @brief Record a transmitted frame into _tx_log for test assertions
	 * @param f Pointer to the frame to send
	 * @return MCP2515_ERROR_OK always
	 */
	uint8_t sendMessage(const can_frame *f)
	{
		_tx_log.push_back(*f);
		return MCP2515_ERROR_OK;
	}

	/**
	 * @brief Inject a frame into the RX queue for the driver to read
	 * @param id CAN identifier
	 * @param data Pointer to payload bytes
	 * @param dlc Data length code (clamped to 8)
	 */
	void pushRx(uint32_t id, const uint8_t *data, uint8_t dlc)
	{
		can_frame f;
		f.can_id = id;
		f.can_dlc = dlc;
		memcpy(f.data, data, dlc > 8 ? 8 : dlc);
		_rx.push(f);
	}

	/**
	 * @brief Return the chip-select pin this instance was constructed with
	 * @return Chip-select pin number
	 */
	uint8_t csPin() const
	{
		return _cs;
	}

	bool _fail_reset = false;          // Set true to simulate reset failure
	std::vector<can_frame> _tx_log;    // Recorded TX frames for assertions

  private:
	uint8_t _cs;                       // Chip-select pin
	std::queue<can_frame> _rx;         // Injected RX frames awaiting read
};

// CAN oscillator frequency constants referenced by the driver
#define MCP_8MHZ 8
#define MCP_16MHZ 16
#define MCP_20MHZ 20
#define CAN_500KBPS 500
