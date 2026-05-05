#pragma once
// ── Minimal hand-coded protobuf encoder ──────────────────────────────────────
// No external protobuf library needed.  Only the wire types used by Tesla:
//   wire 0 = varint   (bool, int32, uint32, enum)
//   wire 2 = LEN      (bytes, string, embedded message)
//
// Usage:
//   uint8_t buf[256];
//   Proto pb(buf, sizeof(buf));
//   pb.fieldVarint(1, DOMAIN_VEHICLE_SECURITY);
//   pb.fieldBytes(2, myKey, 65);
//   pb.fieldMsg(3, innerPb);

#if BOARD_ENABLE_BLE

#include <stdint.h>
#include <stddef.h>
#include <string.h>

namespace Tesla
{

struct Proto
{
	uint8_t *buf;
	size_t cap;
	size_t len;

	Proto(uint8_t *b, size_t c) : buf(b), cap(c), len(0) {}

	bool ok() const
	{
		return len <= cap;
	}

	// ── primitives ────────────────────────────────────────────────────────────
	bool varint(uint64_t v)
	{
		do
		{
			if (len >= cap)
				return false;
			uint8_t byte = v & 0x7F;
			v >>= 7;
			if (v)
				byte |= 0x80;
			buf[len++] = byte;
		} while (v);
		return true;
	}

	bool rawBytes(const uint8_t *data, size_t n)
	{
		if (len + n > cap)
			return false;
		memcpy(buf + len, data, n);
		len += n;
		return true;
	}

	// ── typed fields ──────────────────────────────────────────────────────────
	bool tag(uint32_t field, uint8_t wire)
	{
		return varint(((uint64_t)field << 3) | wire);
	}

	// field + varint value (wire type 0)
	bool fieldVarint(uint32_t field, uint64_t v)
	{
		return tag(field, 0) && varint(v);
	}

	// field + length-delimited bytes (wire type 2)
	bool fieldBytes(uint32_t field, const uint8_t *data, size_t n)
	{
		return tag(field, 2) && varint(n) && rawBytes(data, n);
	}

	// field + nested encoded message (wire type 2)
	bool fieldMsg(uint32_t field, const Proto &inner)
	{
		return tag(field, 2) && varint(inner.len) && rawBytes(inner.buf, inner.len);
	}

	// field + c-string as bytes (wire type 2)
	bool fieldStr(uint32_t field, const char *s)
	{
		size_t n = strlen(s);
		return fieldBytes(field, (const uint8_t *)s, n);
	}

	// field + bool (wire type 0)
	bool fieldBool(uint32_t field, bool v)
	{
		return fieldVarint(field, v ? 1 : 0);
	}

	// empty nested message – just the tag + length=0 (wire type 2)
	bool fieldEmpty(uint32_t field)
	{
		return tag(field, 2) && varint(0);
	}
};

} // namespace Tesla

#endif // BOARD_ENABLE_BLE
