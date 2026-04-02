#define ARDUINO_ARCH_AVR 1
#define BOARD_ENABLE_BT 1

#include <unity.h>
#include <stdint.h>
#include <stdio.h>
#include <string>

#include "board/commands.h"
#include "board/config.h"
#include "board/state.h"

static unsigned long fakeMillisValue = 0;

unsigned long millis()
{
    return fakeMillisValue;
}

class FakeTransport
{
public:
    std::string output;

    bool readUsbChar(char &character)
    {
        (void)character;
        return false;
    }

    bool readBluetoothChar(char &character)
    {
        (void)character;
        return false;
    }

    void print(const char *text)
    {
        output += text;
    }

    void printNumber(long number)
    {
        char buffer[32] = {};
        snprintf(buffer, sizeof(buffer), "%ld", number);
        output += buffer;
    }

    void printHexByte(uint8_t value)
    {
        char buffer[3] = {};
        snprintf(buffer, sizeof(buffer), "%02X", value);
        output += buffer;
    }

    void println()
    {
        output += "\n";
    }
};

static void assertContains(const std::string &text, const char *needle)
{
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(text.c_str(), needle), needle);
}

void setUp()
{
    fakeMillisValue = 0;
}

void tearDown()
{
}

void test_bt_enabled_config_strings_match_install_contract()
{
    TEST_ASSERT_TRUE(board::kBluetoothEnabled);
    TEST_ASSERT_EQUAL_STRING("HC-05", BOARD_BT_NAME);
    assertContains(board::kArduinoReadyMessage, "USB + HC-05 enabled");
}

void test_status_includes_bluetooth_capability_metadata_when_enabled()
{
    BoardCommandRouter router;
    BoardState state;
    FakeTransport transport;

    router.sendStatus(transport, state, 900);

    assertContains(transport.output, "\"cap\":\"usb+bluetooth\"");
    assertContains(transport.output, "\"bt\":1");
    assertContains(transport.output, "\"ready\":\"bench-ready\"");
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_bt_enabled_config_strings_match_install_contract);
    RUN_TEST(test_status_includes_bluetooth_capability_metadata_when_enabled);

    return UNITY_END();
}
