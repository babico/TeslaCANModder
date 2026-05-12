#pragma once

/**
 * @file firmware/lib/vehicle/ble/proto.h
 * @brief Minimal hand-coded protobuf encoder for Tesla BLE communication
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#if BOARD_ENABLE_BLE

#include <stdint.h>
#include <stddef.h>
#include <string.h>

namespace Tesla
{

/**
 * @brief Lightweight protobuf encoder that writes fields into a caller-supplied buffer.
 *
 * Supports only the wire types used by the Tesla vehicle protocol:
 *   - Wire 0 (varint): bool, int32, uint32, enum
 *   - Wire 2 (LEN): bytes, string, embedded message
 *
 * No external protobuf library is required.
 */
struct Proto
{
	uint8_t *buf;  // Output buffer pointer
	size_t cap;    // Buffer capacity in bytes
	size_t len;    // Current write position (bytes written so far)

	Proto(uint8_t *b, size_t c) : buf(b), cap(c), len(0) {}

	/**
	 * @brief Check whether all writes so far fit within the buffer capacity.
	 * @return True if no overflow has occurred.
	 */
	bool ok() const
	{
		return len <= cap;
	}

	/**
	 * @brief Encode a raw varint (variable-length integer) into the buffer.
	 * @param v Value to encode using LEB128 encoding.
	 * @return True if the value was written without exceeding capacity.
	 */
	bool varint(uint64_t v)
	{
		do
		{
			if (len >= cap)
				return false;
			uint8_t byte = v & 0x7F;
			v >>= 7;
			if (v)
				byte |= 0x80; // Set continuation bit
			buf[len++] = byte;
		} while (v);
		return true;
	}

	/**
	 * @brief Copy raw bytes into the buffer at the current write position.
	 * @param data Pointer to source bytes.
	 * @param n Number of bytes to copy.
	 * @return True if the bytes were written without exceeding capacity.
	 */
	bool rawBytes(const uint8_t *data, size_t n)
	{
		if (len + n > cap)
			return false;
		memcpy(buf + len, data, n);
		len += n;
		return true;
	}

	/**
	 * @brief Write a protobuf field tag (field number + wire type).
	 * @param field Protobuf field number.
	 * @param wire Wire type (0 = varint, 2 = length-delimited).
	 * @return True if the tag was written successfully.
	 */
	bool tag(uint32_t field, uint8_t wire)
	{
		return varint(((uint64_t)field << 3) | wire);
	}

	/**
	 * @brief Write a varint field (wire type 0).
	 * @param field Protobuf field number.
	 * @param v Value to encode.
	 * @return True on success.
	 */
	bool fieldVarint(uint32_t field, uint64_t v)
	{
		return tag(field, 0) && varint(v);
	}

	/**
	 * @brief Write a length-delimited bytes field (wire type 2).
	 * @param field Protobuf field number.
	 * @param data Pointer to the byte payload.
	 * @param n Length of the payload in bytes.
	 * @return True on success.
	 */
	bool fieldBytes(uint32_t field, const uint8_t *data, size_t n)
	{
		return tag(field, 2) && varint(n) && rawBytes(data, n);
	}

	/**
	 * @brief Write a nested message field (wire type 2) from an already-encoded Proto.
	 * @param field Protobuf field number.
	 * @param inner Previously encoded Proto whose buffer contents become the payload.
	 * @return True on success.
	 */
	bool fieldMsg(uint32_t field, const Proto &inner)
	{
		return tag(field, 2) && varint(inner.len) && rawBytes(inner.buf, inner.len);
	}

	/**
	 * @brief Write a string field (wire type 2) from a null-terminated C string.
	 * @param field Protobuf field number.
	 * @param s Null-terminated string to encode.
	 * @return True on success.
	 */
	bool fieldStr(uint32_t field, const char *s)
	{
		size_t n = strlen(s);
		return fieldBytes(field, (const uint8_t *)s, n);
	}

	/**
	 * @brief Write a boolean field (wire type 0, encoded as varint 0 or 1).
	 * @param field Protobuf field number.
	 * @param v Boolean value.
	 * @return True on success.
	 */
	bool fieldBool(uint32_t field, bool v)
	{
		return fieldVarint(field, v ? 1 : 0);
	}

	/**
	 * @brief Write an empty nested message (tag + length 0, wire type 2).
	 * @param field Protobuf field number.
	 * @return True on success.
	 */
	bool fieldEmpty(uint32_t field)
	{
		return tag(field, 2) && varint(0);
	}
};

} // namespace Tesla

#endif // BOARD_ENABLE_BLE
