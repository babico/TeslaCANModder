#include <unity.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "board/bridge.h"

class FakeBridgeTransport
{
public:
    std::string output;

    void begin()
    {
    }

    bool bluetoothEnabled() const
    {
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

    std::vector<std::string> lines() const
    {
        std::vector<std::string> result;
        std::string current;

        for (char ch : output)
        {
            if (ch == '\n')
            {
                result.push_back(current);
                current.clear();
                continue;
            }

            current += ch;
        }

        if (!current.empty())
        {
            result.push_back(current);
        }

        return result;
    }
};

class FakeCommandTransport : public FakeBridgeTransport
{
public:
    bool readUsbChar(char &)
    {
        return false;
    }

    bool readBluetoothChar(char &)
    {
        return false;
    }
};

static void assertContains(const std::string &line, const char *needle)
{
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(line.c_str(), needle), needle);
}

void setUp()
{
}

void tearDown()
{
}

void test_bridge_boot_reports_stream_defaults()
{
    BoardBridge bridge;
    BoardState state;
    FakeBridgeTransport transport;

    bridge.writeBoot(transport, state);

    const std::vector<std::string> lines = transport.lines();
    TEST_ASSERT_EQUAL_UINT32(1, static_cast<uint32_t>(lines.size()));
    assertContains(lines[0], "\"t\":\"boot\"");
    assertContains(lines[0], "\"stream\":{\"on\":0,\"emitted\":0}");
}

void test_bridge_frame_serialization_includes_seq_ms_and_ext()
{
    BoardBridge bridge;
    BoardState state;
    FakeCommandTransport commandTransport;
    FakeBridgeTransport frameTransport;

    bridge.routerForTest().runCommand(commandTransport, state, "stream:on", 50);

    Frame frame = {};
    frame.id = 0x80000000 | 0x123;
    frame.dlc = 3;
    frame.data[0] = 0xAA;
    frame.data[1] = 0xBB;
    frame.data[2] = 0xCC;

    bridge.writeFrame(frameTransport, frame, "tx", 1234);

    const std::vector<std::string> lines = frameTransport.lines();
    TEST_ASSERT_EQUAL_UINT32(1, static_cast<uint32_t>(lines.size()));
    assertContains(lines[0], "\"t\":\"frame\"");
    assertContains(lines[0], "\"dir\":\"tx\"");
    assertContains(lines[0], "\"seq\":1");
    assertContains(lines[0], "\"ms\":1234");
    assertContains(lines[0], "\"ext\":1");
    assertContains(lines[0], "\"dlc\":3");
    assertContains(lines[0], "\"d\":\"AABBCC\"");
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_bridge_boot_reports_stream_defaults);
    RUN_TEST(test_bridge_frame_serialization_includes_seq_ms_and_ext);

    return UNITY_END();
}
