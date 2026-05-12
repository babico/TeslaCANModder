/**
 * @file firmware/test/test_native_bluetooth/test_bluetooth.cpp
 * @brief Unit tests for Bluetooth serial adapter contract
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cstring>
#include <string>
#include <cstdint>

class __FlashStringHelper;

struct FakeBT
{
	bool started = false;
	std::string out;
	std::string inbuf;
	void begin(const char *)
	{
		started = true;
	}
	void print(const char *s)
	{
		out += s;
	}
	void print(const __FlashStringHelper *s)
	{
		out += reinterpret_cast<const char *>(s);
	}
	void print(long n)
	{
		char b[32];
		snprintf(b, sizeof(b), "%ld", n);
		out += b;
	}
	void print(uint8_t b, int base)
	{
		char buf[4];
		if (base == 16)
			snprintf(buf, sizeof(buf), "%X", b);
		else
			snprintf(buf, sizeof(buf), "%u", b);
		out += buf;
	}
	void println()
	{
		out += "\n";
	}
	int available()
	{
		return (int)inbuf.size();
	}
	int read()
	{
		if (inbuf.empty())
			return -1;
		char c = inbuf.front();
		inbuf.erase(0, 1);
		return c;
	}
};

static FakeBT btSerial;
static bool btReady = false;

void btInit()
{
	btSerial.begin("teslamod-test");
	btReady = true;
}
bool btIsReady()
{
	return btReady;
}
void btPrint(const char *s)
{
	if (btReady)
		btSerial.print(s);
}
void btPrintNum(long n)
{
	if (btReady)
		btSerial.print(n);
}
void btPrintHex(uint8_t b)
{
	if (!btReady)
		return;
	if (b < 0x10)
		btSerial.print("0");
	btSerial.print(b, 16);
}
void btPrintLn()
{
	if (btReady)
		btSerial.println();
}
int btAvailable()
{
	return btReady ? btSerial.available() : 0;
}
char btRead()
{
	return (char)btSerial.read();
}

/** @brief Reset BT ready flag and clear fake serial state before each test */
void setUp()
{
	btReady = false;
	btSerial = FakeBT();
}

/** @brief Cleanup after each test */
void tearDown() {}

/** @brief btIsReady returns false before btInit is called */
void test_bt_not_ready_before_init()
{
	TEST_ASSERT_FALSE(btIsReady());
}

/** @brief btInit sets ready flag and starts the underlying serial port */
void test_bt_init_sets_ready_and_starts_serial()
{
	btInit();
	TEST_ASSERT_TRUE(btIsReady());
	TEST_ASSERT_TRUE(btSerial.started);
}

/** @brief btPrint does not emit output when BT is not ready */
void test_btPrint_guarded_when_not_ready()
{
	btPrint("hello");
	TEST_ASSERT_EQUAL_STRING("", btSerial.out.c_str());
}

/** @brief btPrint emits the string when BT is ready */
void test_btPrint_emits_when_ready()
{
	btInit();
	btPrint("hello");
	TEST_ASSERT_EQUAL_STRING("hello", btSerial.out.c_str());
}

/** @brief btPrintHex zero-pads single-digit hex values */
void test_btPrintHex_zero_pads()
{
	btInit();
	btPrintHex(0x05);
	btPrintHex(0xAB);
	TEST_ASSERT_EQUAL_STRING("05AB", btSerial.out.c_str());
}

/** @brief btPrintLn emits a newline character */
void test_btPrintLn_emits_newline()
{
	btInit();
	btPrintLn();
	TEST_ASSERT_EQUAL_STRING("\n", btSerial.out.c_str());
}

/** @brief btAvailable returns zero when BT is not ready regardless of buffer content */
void test_btAvailable_zero_when_not_ready()
{
	btSerial.inbuf = "abc";
	TEST_ASSERT_EQUAL(0, btAvailable());
}

/** @brief btAvailable reflects the input buffer length when BT is ready */
void test_btAvailable_reflects_buffer_when_ready()
{
	btInit();
	btSerial.inbuf = "abc";
	TEST_ASSERT_EQUAL(3, btAvailable());
}

/** @brief btRead consumes characters from the input buffer in FIFO order */
void test_btRead_consumes_chars_in_order()
{
	btInit();
	btSerial.inbuf = "hi";
	TEST_ASSERT_EQUAL('h', btRead());
	TEST_ASSERT_EQUAL('i', btRead());
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_bt_not_ready_before_init);
	RUN_TEST(test_bt_init_sets_ready_and_starts_serial);
	RUN_TEST(test_btPrint_guarded_when_not_ready);
	RUN_TEST(test_btPrint_emits_when_ready);
	RUN_TEST(test_btPrintHex_zero_pads);
	RUN_TEST(test_btPrintLn_emits_newline);
	RUN_TEST(test_btAvailable_zero_when_not_ready);
	RUN_TEST(test_btAvailable_reflects_buffer_when_ready);
	RUN_TEST(test_btRead_consumes_chars_in_order);
	return UNITY_END();
}
