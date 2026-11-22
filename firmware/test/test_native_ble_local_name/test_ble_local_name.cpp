/**
 * @file firmware/test/test_native_ble_local_name/test_ble_local_name.cpp
 * @brief Advertisement-filter test: VIN -> observed Tesla BLE Local Name
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 *
 * Observed Tesla BLE advertisement Local Name (community summary evidence, no
 * vehicle capture in this repo):
 *     "S" + uppercase(sha1(VIN) hex[:8]) + "C"
 *
 * The old firmware filter used the last 8 characters of the VIN, which cannot
 * match a real vehicle advert. This suite pins literal expected Local Names
 * computed OUTSIDE the firmware build (Python 3 hashlib, cross-checked with
 * Node.js crypto), then feeds the exact production derivation function an
 * independent test-only SHA-1 implementation. Expected values never come from
 * production code.
 *
 * Real-vehicle acceptance is UNVERIFIED — no advert capture exists here.
 */

#include <unity.h>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>

#include "vehicle/ble/local_name.h"

using Tesla::teslaLocalName;

// ---------------------------------------------------------------------------
// Independent test-only SHA-1 reference. Not shipped; firmware uses mbedTLS.
// ---------------------------------------------------------------------------

namespace
{
class ReferenceSha1
{
public:
	ReferenceSha1()
	    : h_{0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u}, blockLen_(0), bitLen_(0)
	{
	}

	void update(const uint8_t *data, size_t len)
	{
		bitLen_ += (uint64_t)len * 8u;
		while (len > 0)
		{
			size_t take = 64u - blockLen_;
			if (take > len)
				take = len;
			memcpy(block_ + blockLen_, data, take);
			blockLen_ += take;
			data += take;
			len -= take;
			if (blockLen_ == 64u)
			{
				process(block_);
				blockLen_ = 0;
			}
		}
	}

	void finish(uint8_t out[20])
	{
		const uint64_t bits = bitLen_;
		block_[blockLen_++] = 0x80u;
		if (blockLen_ > 56u)
		{
			memset(block_ + blockLen_, 0, 64u - blockLen_);
			process(block_);
			blockLen_ = 0;
		}
		memset(block_ + blockLen_, 0, 56u - blockLen_);
		for (unsigned i = 0; i < 8u; ++i)
			block_[56u + i] = (uint8_t)(bits >> (56u - 8u * i));
		process(block_);
		for (unsigned i = 0; i < 5u; ++i)
		{
			out[4u * i] = (uint8_t)(h_[i] >> 24u);
			out[4u * i + 1u] = (uint8_t)(h_[i] >> 16u);
			out[4u * i + 2u] = (uint8_t)(h_[i] >> 8u);
			out[4u * i + 3u] = (uint8_t)h_[i];
		}
	}

private:
	static uint32_t rotl(uint32_t v, unsigned n)
	{
		return (v << n) | (v >> (32u - n));
	}

	void process(const uint8_t *p)
	{
		uint32_t w[80];
		for (unsigned i = 0; i < 16u; ++i)
			w[i] = ((uint32_t)p[4u * i] << 24u) | ((uint32_t)p[4u * i + 1u] << 16u) |
			       ((uint32_t)p[4u * i + 2u] << 8u) | (uint32_t)p[4u * i + 3u];
		for (unsigned i = 16; i < 80; ++i)
			w[i] = rotl(w[i - 3u] ^ w[i - 8u] ^ w[i - 14u] ^ w[i - 16u], 1u);

		uint32_t a = h_[0], b = h_[1], c = h_[2], d = h_[3], e = h_[4];
		for (unsigned i = 0; i < 80; ++i)
		{
			uint32_t f, k;
			if (i < 20u)
			{
				f = (b & c) | (~b & d);
				k = 0x5A827999u;
			}
			else if (i < 40u)
			{
				f = b ^ c ^ d;
				k = 0x6ED9EBA1u;
			}
			else if (i < 60u)
			{
				f = (b & c) | (b & d) | (c & d);
				k = 0x8F1BBCDCu;
			}
			else
			{
				f = b ^ c ^ d;
				k = 0xCA62C1D6u;
			}
			const uint32_t tmp = rotl(a, 5u) + f + e + k + w[i];
			e = d;
			d = c;
			c = rotl(b, 30u);
			b = a;
			a = tmp;
		}
		h_[0] += a;
		h_[1] += b;
		h_[2] += c;
		h_[3] += d;
		h_[4] += e;
	}

	uint32_t h_[5];
	uint8_t block_[64];
	size_t blockLen_;
	uint64_t bitLen_;
};

bool referenceSha1(const uint8_t *data, size_t len, uint8_t digest[20])
{
	ReferenceSha1 sha;
	sha.update(data, len);
	sha.finish(digest);
	return true;
}

// ---------------------------------------------------------------------------
// Literal vectors — computed independently, NOT derived from production code.
// Source: Python 3 `hashlib.sha1(vin.encode()).hexdigest().upper()`,
// cross-checked with Node `crypto.createHash('sha1').update(vin).digest('hex')`.
// ---------------------------------------------------------------------------

struct Vector
{
	const char *vin;
	const char *sha1Hex;
	const char *localName;
};

const Vector kVectors[] = {
    {"5YJ3E1EA5KF000123", "4A0C1D370DE25321D9B962DD5133FDE4D48FC5CB", "S4A0C1D37C"},
    {"5YJYGDEE5MF000456", "0DB5F043C212B9A7257B37224E16440BC9AD46E8", "S0DB5F043C"},
};

} // namespace

void setUp()
{
}
void tearDown()
{
}

/** @brief Independent SHA-1 reference reproduces the pinned literal digests */
static void test_reference_sha1_matches_literal_vectors()
{
	for (const Vector &v : kVectors)
	{
		uint8_t digest[20] = {};
		TEST_ASSERT_TRUE(referenceSha1((const uint8_t *)v.vin, strlen(v.vin), digest));
		char hex[41] = {};
		static const char digits[] = "0123456789ABCDEF";
		for (unsigned i = 0; i < 20u; ++i)
		{
			hex[2u * i] = digits[(digest[i] >> 4u) & 0x0Fu];
			hex[2u * i + 1u] = digits[digest[i] & 0x0Fu];
		}
		TEST_ASSERT_EQUAL_STRING(v.sha1Hex, hex);
	}
}

/** @brief Production derivation yields the pinned literal Local Names */
static void test_local_name_matches_literal_vectors()
{
	for (const Vector &v : kVectors)
	{
		char name[11] = {};
		TEST_ASSERT_TRUE(teslaLocalName(v.vin, strlen(v.vin), name, &referenceSha1));
		TEST_ASSERT_EQUAL_STRING(v.localName, name);
		TEST_ASSERT_EQUAL_UINT(10u, (unsigned)strlen(name));
		TEST_ASSERT_EQUAL_CHAR('S', name[0]);
		TEST_ASSERT_EQUAL_CHAR('C', name[9]);
	}
}

/** @brief Filter is the hashed Local Name, never the old last-8-VIN suffix */
static void test_local_name_is_not_vin_suffix()
{
	for (const Vector &v : kVectors)
	{
		char name[11] = {};
		TEST_ASSERT_TRUE(teslaLocalName(v.vin, strlen(v.vin), name, &referenceSha1));
		const std::string vin(v.vin);
		const std::string oldSuffix = vin.substr(vin.size() - 8u);
		TEST_ASSERT_FALSE_MESSAGE(std::string(name) == oldSuffix, "filter regressed to last-8-VIN suffix");
		for (unsigned i = 1; i < 9u; ++i)
		{
			const char c = name[i];
			TEST_ASSERT_TRUE_MESSAGE(
			    (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'), "Local Name hex must be uppercase");
		}
	}
}

/** @brief Derivation fails closed on missing inputs */
static void test_local_name_rejects_missing_inputs()
{
	char name[11] = {};
	TEST_ASSERT_FALSE(teslaLocalName(nullptr, 17, name, &referenceSha1));
	TEST_ASSERT_FALSE(teslaLocalName("5YJ3E1EA5KF000123", 0, name, &referenceSha1));
	TEST_ASSERT_FALSE(teslaLocalName("5YJ3E1EA5KF000123", 17, name, nullptr));
}

int main()
{
	UNITY_BEGIN();
	RUN_TEST(test_reference_sha1_matches_literal_vectors);
	RUN_TEST(test_local_name_matches_literal_vectors);
	RUN_TEST(test_local_name_is_not_vin_suffix);
	RUN_TEST(test_local_name_rejects_missing_inputs);
	return UNITY_END();
}
