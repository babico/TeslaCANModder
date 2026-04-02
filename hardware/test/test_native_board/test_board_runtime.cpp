#include <unity.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "board/commands.h"
#include "board/state.h"
#include "board/variant.h"
#include "handlers/variants/index.h"

static unsigned long fakeMillisValue = 0;

unsigned long millis()
{
    return fakeMillisValue;
}

class FakeTransport
{
public:
    std::string usbInput;
    std::string bluetoothInput;
    std::string output;

    bool readUsbChar(char &character)
    {
        if (usbCursor >= usbInput.size())
        {
            return false;
        }

        character = usbInput[usbCursor++];
        return true;
    }

    bool readBluetoothChar(char &character)
    {
        if (bluetoothCursor >= bluetoothInput.size())
        {
            return false;
        }

        character = bluetoothInput[bluetoothCursor++];
        return true;
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

        for (size_t i = 0; i < output.size(); ++i)
        {
            const char ch = output[i];
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

private:
    size_t usbCursor = 0;
    size_t bluetoothCursor = 0;
};

class StubHandler : public Handler
{
public:
    int speed = 1;
    bool fsd = false;

    int &speedProfile() override
    {
        return speed;
    }

    bool &fsdEnabled() override
    {
        return fsd;
    }

    void handleMessage(Frame & /*frame*/, Driver & /*driver*/) override
    {
    }

    const uint32_t *filterIds() const override
    {
        return nullptr;
    }

    uint8_t filterIdCount() const override
    {
        return 0;
    }
};

static void assertLineContains(const std::string &line, const char *needle)
{
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(line.c_str(), needle), needle);
}

void setUp()
{
    fakeMillisValue = 0;
}

void tearDown()
{
}

void test_board_state_defaults()
{
    BoardState state;
    TEST_ASSERT_EQUAL_INT(1, state.speedProfile());
    TEST_ASSERT_FALSE(state.fsdEnabled());
    TEST_ASSERT_EQUAL_STRING("hw4", state.variantName());
    TEST_ASSERT_FALSE(state.consumeVariantChange());
}

void test_board_state_apply_and_sync_roundtrip()
{
    BoardState state;
    StubHandler handler;

    state.setSpeedProfile(4);
    state.setFsdEnabled(true);
    state.applyTo(handler);

    TEST_ASSERT_EQUAL_INT(4, handler.speedProfile());
    TEST_ASSERT_TRUE(handler.fsdEnabled());

    handler.speedProfile() = 2;
    handler.fsdEnabled() = false;
    state.syncFrom(handler);

    TEST_ASSERT_EQUAL_INT(2, state.speedProfile());
    TEST_ASSERT_FALSE(state.fsdEnabled());
}

void test_variant_parse_and_create_handler()
{
    board::Variant variant = board::Variant::Legacy;

    TEST_ASSERT_TRUE(board::parseVariantName("hw4", variant));
    TEST_ASSERT_EQUAL_STRING("hw4", board::variantName(variant));
    TEST_ASSERT_TRUE(dynamic_cast<Hw4 *>(board::createHandler(variant)) != nullptr);

    TEST_ASSERT_TRUE(board::parseVariantName("hw3", variant));
    TEST_ASSERT_TRUE(dynamic_cast<Hw3 *>(board::createHandler(variant)) != nullptr);

    TEST_ASSERT_TRUE(board::parseVariantName("legacy", variant));
    TEST_ASSERT_TRUE(dynamic_cast<Legacy *>(board::createHandler(variant)) != nullptr);
    TEST_ASSERT_FALSE(board::parseVariantName("bad", variant));
}

void test_board_state_variant_dirty_tracking()
{
    BoardState state;

    TEST_ASSERT_TRUE(state.setVariantByName("hw3"));
    TEST_ASSERT_TRUE(state.consumeVariantChange());
    TEST_ASSERT_FALSE(state.consumeVariantChange());

    TEST_ASSERT_TRUE(state.setVariantByName("hw3"));
    TEST_ASSERT_FALSE(state.consumeVariantChange());

    TEST_ASSERT_FALSE(state.setVariantByName("nope"));
    TEST_ASSERT_EQUAL_STRING("hw3", state.variantName());
}

void test_command_ping_returns_pong()
{
    BoardCommandRouter router;
    BoardState state;
    FakeTransport transport;

    router.runCommand(transport, state, "ping", 42);

    const std::vector<std::string> lines = transport.lines();
    TEST_ASSERT_EQUAL_UINT32(1, static_cast<uint32_t>(lines.size()));
    TEST_ASSERT_EQUAL_STRING("{\"t\":\"pong\",\"v\":1}", lines[0].c_str());
}

void test_command_stream_on_and_off_toggle_router_state()
{
    BoardCommandRouter router;
    BoardState state;
    FakeTransport transport;

    router.runCommand(transport, state, "stream:on", 100);
    TEST_ASSERT_TRUE(router.isFrameStreamingEnabled());

    router.runCommand(transport, state, "stream:off", 120);
    TEST_ASSERT_FALSE(router.isFrameStreamingEnabled());

    const std::vector<std::string> lines = transport.lines();
    TEST_ASSERT_EQUAL_UINT32(2, static_cast<uint32_t>(lines.size()));
    TEST_ASSERT_EQUAL_STRING("{\"t\":\"ack\",\"cmd\":\"stream:on\"}", lines[0].c_str());
    TEST_ASSERT_EQUAL_STRING("{\"t\":\"ack\",\"cmd\":\"stream:off\"}", lines[1].c_str());
}

void test_command_fsd_and_profile_update_state_and_emit_status()
{
    BoardCommandRouter router;
    BoardState state;
    FakeTransport transport;

    router.runCommand(transport, state, "fsd:on", 500);
    router.runCommand(transport, state, "profile:4", 501);
    router.runCommand(transport, state, "sp:2", 502);
    router.runCommand(transport, state, "fsd:toggle", 503);

    TEST_ASSERT_EQUAL_INT(2, state.speedProfile());
    TEST_ASSERT_FALSE(state.fsdEnabled());

    const std::vector<std::string> lines = transport.lines();
    TEST_ASSERT_EQUAL_UINT32(8, static_cast<uint32_t>(lines.size()));
    assertLineContains(lines[1], "\"t\":\"status\"");
    assertLineContains(lines[1], "\"cap\":\"usb\"");
    assertLineContains(lines[1], "\"ready\":\"bench-ready\"");
    assertLineContains(lines[1], "\"bt\":0");
    assertLineContains(lines[1], "\"fsd\":1");
    assertLineContains(lines[3], "\"sp\":4");
    assertLineContains(lines[5], "\"sp\":2");
    assertLineContains(lines[7], "\"fsd\":0");
}

void test_command_variant_and_invalid_commands()
{
    BoardCommandRouter router;
    BoardState state;
    FakeTransport transport;

    router.runCommand(transport, state, "variant:legacy", 700);
    router.runCommand(transport, state, "variant:nope", 701);
    router.runCommand(transport, state, "profile:8", 702);
    router.runCommand(transport, state, "unknown", 703);

    TEST_ASSERT_EQUAL_STRING("legacy", state.variantName());

    const std::vector<std::string> lines = transport.lines();
    TEST_ASSERT_EQUAL_UINT32(5, static_cast<uint32_t>(lines.size()));
    TEST_ASSERT_EQUAL_STRING("{\"t\":\"ack\",\"cmd\":\"variant:legacy\"}", lines[0].c_str());
    assertLineContains(lines[1], "\"variant\":\"legacy\"");
    TEST_ASSERT_EQUAL_STRING("{\"t\":\"error\",\"msg\":\"Invalid variant\"}", lines[2].c_str());
    TEST_ASSERT_EQUAL_STRING("{\"t\":\"error\",\"msg\":\"Invalid speed profile\"}", lines[3].c_str());
    TEST_ASSERT_EQUAL_STRING("{\"t\":\"error\",\"msg\":\"Unknown command\"}", lines[4].c_str());
}

void test_update_reads_usb_and_bluetooth_buffers_and_periodic_status()
{
    BoardCommandRouter router;
    BoardState state;
    FakeTransport transport;

    transport.usbInput = "ping\r\nstream:on\n";
    transport.bluetoothInput = "fsd:on\n";

    fakeMillisValue = 500;
    router.update(transport, state);

    TEST_ASSERT_TRUE(router.isFrameStreamingEnabled());
    TEST_ASSERT_TRUE(state.fsdEnabled());

    const std::vector<std::string> lines = transport.lines();
    TEST_ASSERT_EQUAL_UINT32(5, static_cast<uint32_t>(lines.size()));
    assertLineContains(lines[0], "\"t\":\"status\"");
    assertLineContains(lines[0], "\"cap\":\"usb\"");
    TEST_ASSERT_EQUAL_STRING("{\"t\":\"pong\",\"v\":1}", lines[1].c_str());
    TEST_ASSERT_EQUAL_STRING("{\"t\":\"ack\",\"cmd\":\"stream:on\"}", lines[2].c_str());
    TEST_ASSERT_EQUAL_STRING("{\"t\":\"ack\",\"cmd\":\"fsd:on\"}", lines[3].c_str());
    assertLineContains(lines[4], "\"fsd\":1");
}

void test_invalid_partial_bluetooth_command_does_not_corrupt_usb_command()
{
    BoardCommandRouter router;
    BoardState state;
    FakeTransport transport;

    transport.bluetoothInput = "variant:bad";
    transport.usbInput = "status\n";

    fakeMillisValue = 500;
    router.update(transport, state);

    const std::vector<std::string> firstPass = transport.lines();
    TEST_ASSERT_EQUAL_UINT32(2, static_cast<uint32_t>(firstPass.size()));
    assertLineContains(firstPass[0], "\"t\":\"status\"");
    assertLineContains(firstPass[1], "\"variant\":\"hw4\"");

    transport.bluetoothInput += "\n";
    fakeMillisValue = 700;
    router.update(transport, state);

    const std::vector<std::string> secondPass = transport.lines();
    TEST_ASSERT_EQUAL_STRING("{\"t\":\"error\",\"msg\":\"Invalid variant\"}", secondPass[2].c_str());
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_board_state_defaults);
    RUN_TEST(test_board_state_apply_and_sync_roundtrip);
    RUN_TEST(test_variant_parse_and_create_handler);
    RUN_TEST(test_board_state_variant_dirty_tracking);
    RUN_TEST(test_command_ping_returns_pong);
    RUN_TEST(test_command_stream_on_and_off_toggle_router_state);
    RUN_TEST(test_command_fsd_and_profile_update_state_and_emit_status);
    RUN_TEST(test_command_variant_and_invalid_commands);
    RUN_TEST(test_update_reads_usb_and_bluetooth_buffers_and_periodic_status);
    RUN_TEST(test_invalid_partial_bluetooth_command_does_not_corrupt_usb_command);

    return UNITY_END();
}
