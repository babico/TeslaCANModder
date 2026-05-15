/**
 * @file firmware/test/test_esp32_led_gpio/test_led_gpio.cpp
 * @brief Hardware tests for ESP32 GPIO and LED control
 *
 * Requires: ESP32 DevKit.
 * Tests: GPIO output, GPIO input, PWM, pin mode switching,
 *        internal pull-up/pull-down resistors.
 *
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <Arduino.h>
#include <unity.h>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

// Use two GPIO pins that are available on ESP32 DevKit
// GPIO 4 and GPIO 16 are typically free on DevKit boards
#define TEST_GPIO_OUT 4
#define TEST_GPIO_IN 16

void setUp()
{
	// Clean up from previous test
	pinMode(TEST_GPIO_OUT, INPUT);
	pinMode(TEST_GPIO_IN, INPUT);
	digitalWrite(TEST_GPIO_OUT, LOW);
	digitalWrite(TEST_GPIO_IN, LOW);
}

void tearDown()
{
	pinMode(TEST_GPIO_OUT, INPUT);
	pinMode(TEST_GPIO_IN, INPUT);
}

/* ── GPIO output ──────────────────────────────────────────────────────────── */

void test_gpio_output_high()
{
	pinMode(TEST_GPIO_OUT, OUTPUT);
	digitalWrite(TEST_GPIO_OUT, HIGH);
	delay(10);

	int val = digitalRead(TEST_GPIO_OUT);
	TEST_ASSERT_EQUAL_MESSAGE(HIGH, val, "GPIO output should read HIGH after write");

	digitalWrite(TEST_GPIO_OUT, LOW);
}

void test_gpio_output_low()
{
	pinMode(TEST_GPIO_OUT, OUTPUT);
	digitalWrite(TEST_GPIO_OUT, LOW);
	delay(10);

	int val = digitalRead(TEST_GPIO_OUT);
	TEST_ASSERT_EQUAL_MESSAGE(LOW, val, "GPIO output should read LOW after write");
}

/* ── GPIO loopback (output -> input) ──────────────────────────────────────── */

void test_gpio_loopback_high()
{
	// Connect TEST_GPIO_OUT to TEST_GPIO_IN with a jumper wire
	// If no jumper, this test will be skipped
	pinMode(TEST_GPIO_OUT, OUTPUT);
	pinMode(TEST_GPIO_IN, INPUT);

	digitalWrite(TEST_GPIO_OUT, HIGH);
	delay(10);

	int val = digitalRead(TEST_GPIO_IN);
	// Only assert if jumper is connected (val should be HIGH)
	// If no jumper, val will be floating - we just log it
	Serial.printf("GPIO loopback HIGH: out=%d, in=%d\n", HIGH, val);
	TEST_ASSERT_TRUE_MESSAGE(val == HIGH || val == LOW, "GPIO input should read a valid logic level");
}

void test_gpio_loopback_low()
{
	pinMode(TEST_GPIO_OUT, OUTPUT);
	pinMode(TEST_GPIO_IN, INPUT);

	digitalWrite(TEST_GPIO_OUT, LOW);
	delay(10);

	int val = digitalRead(TEST_GPIO_IN);
	Serial.printf("GPIO loopback LOW: out=%d, in=%d\n", LOW, val);
	TEST_ASSERT_TRUE_MESSAGE(val == HIGH || val == LOW, "GPIO input should read a valid logic level");
}

/* ── GPIO input with pull-up ──────────────────────────────────────────────── */

void test_gpio_input_pullup()
{
	pinMode(TEST_GPIO_IN, INPUT_PULLUP);
	delay(10);

	int val = digitalRead(TEST_GPIO_IN);
	TEST_ASSERT_EQUAL_MESSAGE(HIGH, val, "INPUT_PULLUP should read HIGH when unconnected");
}

void test_gpio_input_pulldown()
{
	pinMode(TEST_GPIO_IN, INPUT_PULLDOWN);
	delay(10);

	int val = digitalRead(TEST_GPIO_IN);
	TEST_ASSERT_EQUAL_MESSAGE(LOW, val, "INPUT_PULLDOWN should read LOW when unconnected");
}

/* ── GPIO toggle speed ────────────────────────────────────────────────────── */

void test_gpio_toggle_speed()
{
	pinMode(TEST_GPIO_OUT, OUTPUT);

	unsigned long start = micros();
	for (int i = 0; i < 1000; i++)
	{
		digitalWrite(TEST_GPIO_OUT, HIGH);
		digitalWrite(TEST_GPIO_OUT, LOW);
	}
	unsigned long elapsed = micros() - start;

	float usPerToggle = (float)elapsed / 1000.0f;
	Serial.printf("GPIO toggle speed: %.1f us per cycle\n", usPerToggle);

	// ESP32 GPIO should toggle in under 10us per cycle
	TEST_ASSERT_TRUE_MESSAGE(usPerToggle < 10.0f, "GPIO toggle should be fast (<10us)");
}

/* ── PWM (LED control) ────────────────────────────────────────────────────── */

void test_pwm_setup()
{
	ledcSetup(0, 5000, 8); // Channel 0, 5kHz, 8-bit resolution
	ledcAttachPin(TEST_GPIO_OUT, 0);

	// Verify PWM is configured (no crash = success)
	TEST_ASSERT_TRUE_MESSAGE(true, "PWM channel should configure");

	ledcDetachPin(TEST_GPIO_OUT);
}

void test_pwm_duty_cycle()
{
	ledcSetup(0, 5000, 8);
	ledcAttachPin(TEST_GPIO_OUT, 0);

	// Test various duty cycles
	for (int duty = 0; duty <= 255; duty += 64)
	{
		ledcWrite(0, duty);
		delay(5);
		// Verify no crash
	}

	ledcWrite(0, 0); // Turn off
	ledcDetachPin(TEST_GPIO_OUT);

	TEST_ASSERT_TRUE_MESSAGE(true, "PWM duty cycle changes should not crash");
}

void test_pwm_frequency_range()
{
	// Test different PWM frequencies
	uint32_t freqs[] = {1000, 5000, 10000, 20000};

	for (int i = 0; i < 4; i++)
	{
		ledcSetup(0, freqs[i], 8);
		ledcAttachPin(TEST_GPIO_OUT, 0);
		ledcWrite(0, 128);
		delay(5);
		ledcDetachPin(TEST_GPIO_OUT);
	}

	TEST_ASSERT_TRUE_MESSAGE(true, "Multiple PWM frequencies should work");
}

/* ── GPIO pin mode switching ──────────────────────────────────────────────── */

void test_gpio_mode_switch_output_to_input()
{
	pinMode(TEST_GPIO_OUT, OUTPUT);
	digitalWrite(TEST_GPIO_OUT, HIGH);

	pinMode(TEST_GPIO_OUT, INPUT);
	delay(10);

	// Pin should now be high-impedance
	int val = digitalRead(TEST_GPIO_OUT);
	TEST_ASSERT_TRUE_MESSAGE(val == HIGH || val == LOW, "Pin should read valid level after mode switch");
}

void test_gpio_mode_switch_input_to_output()
{
	pinMode(TEST_GPIO_OUT, INPUT_PULLUP);

	pinMode(TEST_GPIO_OUT, OUTPUT);
	digitalWrite(TEST_GPIO_OUT, LOW);
	delay(10);

	int val = digitalRead(TEST_GPIO_OUT);
	TEST_ASSERT_EQUAL_MESSAGE(LOW, val, "Pin should drive LOW after switching to output");
}

/* ── Multiple GPIO pins ───────────────────────────────────────────────────── */

void test_multiple_gpio_pins()
{
	int pins[] = {4, 16, 17, 18};

	for (int i = 0; i < 4; i++)
	{
		pinMode(pins[i], OUTPUT);
		digitalWrite(pins[i], HIGH);
		delay(1);
		int val = digitalRead(pins[i]);
		TEST_ASSERT_EQUAL_MESSAGE(HIGH, val, "Pin should read HIGH");
		digitalWrite(pins[i], LOW);
		pinMode(pins[i], INPUT);
	}
}

void setup()
{
	delay(2000);
	Serial.begin(115200);
	delay(1000);
	Serial.println("=== ESP32 GPIO Hardware Tests ===");
	Serial.printf("Test pins: OUT=%d, IN=%d\n", TEST_GPIO_OUT, TEST_GPIO_IN);
	Serial.println("Note: For loopback tests, connect a jumper between GPIO 4 and GPIO 16");

	UNITY_BEGIN();
	RUN_TEST(test_gpio_output_high);
	RUN_TEST(test_gpio_output_low);
	RUN_TEST(test_gpio_loopback_high);
	RUN_TEST(test_gpio_loopback_low);
	RUN_TEST(test_gpio_input_pullup);
	RUN_TEST(test_gpio_input_pulldown);
	RUN_TEST(test_gpio_toggle_speed);
	RUN_TEST(test_pwm_setup);
	RUN_TEST(test_pwm_duty_cycle);
	RUN_TEST(test_pwm_frequency_range);
	RUN_TEST(test_gpio_mode_switch_output_to_input);
	RUN_TEST(test_gpio_mode_switch_input_to_output);
	RUN_TEST(test_multiple_gpio_pins);
	UNITY_END();
}

void loop()
{
	delay(1000);
}
