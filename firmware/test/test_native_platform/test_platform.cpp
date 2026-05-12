/** @file firmware/test/test_native_platform/test_platform.cpp
 *  @brief Unit tests for platform abstraction layer
 *  @author Tesla CAN Mod Contributors
 *  @license GPL-3.0
 */

#include <unity.h>
#include <cstring>

class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper *>(s))

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "vehicle/can/ids.h"
#include "core/platform.h"

void setUp() {}
void tearDown() {}


void test_model_name_strings()
{
	TEST_ASSERT_EQUAL_STRING("Model S", teslaModelName(MODEL_S));
	TEST_ASSERT_EQUAL_STRING("Model X", teslaModelName(MODEL_X));
	TEST_ASSERT_EQUAL_STRING("Model 3", teslaModelName(MODEL_3));
	TEST_ASSERT_EQUAL_STRING("Model Y", teslaModelName(MODEL_Y));
	TEST_ASSERT_EQUAL_STRING("Cybertruck", teslaModelName(MODEL_CYBERTRUCK));
	TEST_ASSERT_EQUAL_STRING("Unknown", teslaModelName(MODEL_UNKNOWN));
}

void test_parse_model_valid()
{
	TeslaModel m;
	TEST_ASSERT_TRUE(parseTeslaModel("s", m));
	TEST_ASSERT_EQUAL(MODEL_S, m);
	TEST_ASSERT_TRUE(parseTeslaModel("model3", m));
	TEST_ASSERT_EQUAL(MODEL_3, m);
	TEST_ASSERT_TRUE(parseTeslaModel("ct", m));
	TEST_ASSERT_EQUAL(MODEL_CYBERTRUCK, m);
}

void test_parse_model_invalid()
{
	TeslaModel m;
	TEST_ASSERT_FALSE(parseTeslaModel("unknown", m));
	TEST_ASSERT_FALSE(parseTeslaModel("roadster", m));
}


void test_hw_gen_names()
{
	TEST_ASSERT_EQUAL_STRING("legacy", hwGenerationName(HW_LEGACY));
	TEST_ASSERT_EQUAL_STRING("hw3", hwGenerationName(HW_3));
	TEST_ASSERT_EQUAL_STRING("hw4", hwGenerationName(HW_4));
	TEST_ASSERT_EQUAL_STRING("unknown", hwGenerationName(HW_UNKNOWN));
}

void test_variant_to_hwgen_mapping()
{
	TEST_ASSERT_EQUAL(HW_4, variantToHWGen(HW4));
	TEST_ASSERT_EQUAL(HW_3, variantToHWGen(HW3));
	TEST_ASSERT_EQUAL(HW_LEGACY, variantToHWGen(LEGACY));
}

void test_hwgen_to_variant_mapping()
{
	TEST_ASSERT_EQUAL(HW4, hwGenToVariant(HW_4));
	TEST_ASSERT_EQUAL(HW3, hwGenToVariant(HW_3));
	TEST_ASSERT_EQUAL(LEGACY, hwGenToVariant(HW_LEGACY));
}


void test_sw_version_valid()
{
	TeslaSoftwareVersion v = {2026, 14, 1, 0};
	TEST_ASSERT_TRUE(v.valid());
}

void test_sw_version_invalid()
{
	TeslaSoftwareVersion v = {0, 0, 0, 0};
	TEST_ASSERT_FALSE(v.valid());
	TeslaSoftwareVersion v2 = {2018, 0, 0, 0};
	TEST_ASSERT_FALSE(v2.valid());
}

void test_sw_version_compare()
{
	TeslaSoftwareVersion a = {2026, 14, 1, 0};
	TeslaSoftwareVersion b = {2026, 2, 9, 7};
	TEST_ASSERT_TRUE(a >= b);
	TEST_ASSERT_TRUE(b < a);

	TeslaSoftwareVersion c = {2025, 45, 9, 0};
	TEST_ASSERT_TRUE(a >= c);
	TEST_ASSERT_TRUE(c < a);
}

void test_sw_version_equal()
{
	TeslaSoftwareVersion a = {2026, 2, 9, 7};
	TeslaSoftwareVersion b = {2026, 2, 9, 7};
	TEST_ASSERT_EQUAL(0, a.compare(b));
	TEST_ASSERT_TRUE(a >= b);
	TEST_ASSERT_FALSE(a < b);
}


void test_fsd_proto_hw3_always_v12()
{
	TeslaSoftwareVersion sw = {2026, 14, 1, 0};
	TEST_ASSERT_EQUAL(FSD_PROTO_V12, detectFsdProtocol(sw, HW_3));
}

void test_fsd_proto_legacy_always_v12()
{
	TeslaSoftwareVersion sw = {2026, 2, 9, 7};
	TEST_ASSERT_EQUAL(FSD_PROTO_V12, detectFsdProtocol(sw, HW_LEGACY));
}

void test_fsd_proto_hw4_2026_2_9_is_v14()
{
	TeslaSoftwareVersion sw = {2026, 2, 9, 0};
	TEST_ASSERT_EQUAL(FSD_PROTO_V14, detectFsdProtocol(sw, HW_4));
}

void test_fsd_proto_hw4_2026_8_is_v13()
{
	TeslaSoftwareVersion sw = {2026, 8, 6, 0};
	TEST_ASSERT_EQUAL(FSD_PROTO_V13, detectFsdProtocol(sw, HW_4));
}

void test_fsd_proto_hw4_2026_14_is_v14()
{
	TeslaSoftwareVersion sw = {2026, 14, 1, 0};
	TEST_ASSERT_EQUAL(FSD_PROTO_V14, detectFsdProtocol(sw, HW_4));
}

void test_fsd_proto_hw4_2025_45_is_v14()
{
	TeslaSoftwareVersion sw = {2025, 45, 5, 0};
	TEST_ASSERT_EQUAL(FSD_PROTO_V14, detectFsdProtocol(sw, HW_4));
}

void test_fsd_proto_invalid_sw_returns_unknown()
{
	TeslaSoftwareVersion sw = {0, 0, 0, 0};
	TEST_ASSERT_EQUAL(FSD_PROTO_UNKNOWN, detectFsdProtocol(sw, HW_4));
}


void test_platform_resolve_full()
{
	VehiclePlatform p;
	TeslaSoftwareVersion sw = {2026, 2, 9, 7};
	p.resolve(MODEL_Y, HW_4, sw);
	TEST_ASSERT_TRUE(p.resolved);
	TEST_ASSERT_EQUAL(MODEL_Y, p.model);
	TEST_ASSERT_EQUAL(HW_4, p.hwGen);
	TEST_ASSERT_EQUAL(FSD_PROTO_V14, p.fsdProto);
}

void test_platform_resolve_unknown_model()
{
	VehiclePlatform p;
	TeslaSoftwareVersion sw = {2026, 2, 9, 0};
	p.resolve(MODEL_UNKNOWN, HW_4, sw);
	TEST_ASSERT_FALSE(p.resolved);
}

void test_platform_resolve_from_state()
{
	State s;
	s.vehicleModel = MODEL_3;
	s.variant = HW4;
	s.hwAutoDetected = true;
	s.detectedHW = 3;
	s.fwYear = 2026;
	s.fwRelease = 14;
	s.fwMinor = 1;

	VehiclePlatform p;
	p.resolveFromState(s);
	TEST_ASSERT_TRUE(p.resolved);
	TEST_ASSERT_EQUAL(MODEL_3, p.model);
	TEST_ASSERT_EQUAL(HW_4, p.hwGen);
	TEST_ASSERT_EQUAL(2026, p.software.year);
	TEST_ASSERT_EQUAL(14, p.software.week);
	TEST_ASSERT_EQUAL(FSD_PROTO_V14, p.fsdProto);
}

void test_platform_resolve_from_state_variant_fallback()
{
	State s;
	s.vehicleModel = MODEL_S;
	s.variant = HW3;
	s.hwAutoDetected = false;
	s.fwYear = 2025;
	s.fwRelease = 38;
	s.fwMinor = 5;

	VehiclePlatform p;
	p.resolveFromState(s);
	TEST_ASSERT_TRUE(p.resolved);
	TEST_ASSERT_EQUAL(MODEL_S, p.model);
	TEST_ASSERT_EQUAL(HW_3, p.hwGen);
	TEST_ASSERT_EQUAL(FSD_PROTO_V12, p.fsdProto);
}


void test_sync_platform_to_state()
{
	State s;
	VehiclePlatform p;
	TeslaSoftwareVersion sw = {2026, 14, 1, 0};
	p.resolve(MODEL_CYBERTRUCK, HW_4, sw);
	syncPlatformToState(p, s);

	TEST_ASSERT_EQUAL(MODEL_CYBERTRUCK, s.platformModel);
	TEST_ASSERT_EQUAL(HW_4, s.platformHwGen);
	TEST_ASSERT_EQUAL(2026, s.platformSwYear);
	TEST_ASSERT_EQUAL(14, s.platformSwWeek);
	TEST_ASSERT_EQUAL(1, s.platformSwRelease);
	TEST_ASSERT_EQUAL(FSD_PROTO_V14, s.platformFsdProto);
	TEST_ASSERT_TRUE(s.platformResolved);
}


void test_sw_compat_ok_hw4_2026_2_9()
{
	VehiclePlatform p;
	TeslaSoftwareVersion sw = {2026, 2, 9, 7};
	p.resolve(MODEL_3, HW_4, sw);
	TEST_ASSERT_EQUAL(SW_COMPAT_OK, checkSoftwareCompat(p));
}

void test_sw_compat_warn_hw4_2026_8_6()
{
	VehiclePlatform p;
	TeslaSoftwareVersion sw = {2026, 8, 6, 0};
	p.resolve(MODEL_Y, HW_4, sw);
	TEST_ASSERT_EQUAL(SW_COMPAT_WARN, checkSoftwareCompat(p));
}

void test_sw_compat_warn_hw4_old_software()
{
	VehiclePlatform p;
	TeslaSoftwareVersion sw = {2023, 20, 5, 0};
	p.resolve(MODEL_3, HW_4, sw);
	TEST_ASSERT_EQUAL(SW_COMPAT_WARN, checkSoftwareCompat(p));
}

void test_sw_compat_ok_hw3_2025()
{
	VehiclePlatform p;
	TeslaSoftwareVersion sw = {2025, 45, 9, 0};
	p.resolve(MODEL_3, HW_3, sw);
	TEST_ASSERT_EQUAL(SW_COMPAT_OK, checkSoftwareCompat(p));
}

void test_sw_compat_unknown_no_sw()
{
	VehiclePlatform p;
	TeslaSoftwareVersion sw = {0, 0, 0, 0};
	p.resolve(MODEL_3, HW_4, sw);
	TEST_ASSERT_EQUAL(SW_COMPAT_UNKNOWN, checkSoftwareCompat(p));
}


void test_capabilities_cybertruck()
{
	PlatformCapabilities cap = getPlatformCapabilities(MODEL_CYBERTRUCK, HW_4);
	TEST_ASSERT_TRUE(cap.supportsFsd);
	TEST_ASSERT_TRUE(cap.supportsTrackMode);
	TEST_ASSERT_TRUE(cap.supportsDualMotor);
}

void test_capabilities_legacy_limited()
{
	PlatformCapabilities cap = getPlatformCapabilities(MODEL_3, HW_LEGACY);
	TEST_ASSERT_TRUE(cap.supportsFsd);
	TEST_ASSERT_FALSE(cap.supportsSummon);
	TEST_ASSERT_FALSE(cap.supportsBanShield);
}


int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_model_name_strings);
	RUN_TEST(test_parse_model_valid);
	RUN_TEST(test_parse_model_invalid);

	RUN_TEST(test_hw_gen_names);
	RUN_TEST(test_variant_to_hwgen_mapping);
	RUN_TEST(test_hwgen_to_variant_mapping);

	RUN_TEST(test_sw_version_valid);
	RUN_TEST(test_sw_version_invalid);
	RUN_TEST(test_sw_version_compare);
	RUN_TEST(test_sw_version_equal);

	RUN_TEST(test_fsd_proto_hw3_always_v12);
	RUN_TEST(test_fsd_proto_legacy_always_v12);
	RUN_TEST(test_fsd_proto_hw4_2026_2_9_is_v14);
	RUN_TEST(test_fsd_proto_hw4_2026_8_is_v13);
	RUN_TEST(test_fsd_proto_hw4_2026_14_is_v14);
	RUN_TEST(test_fsd_proto_hw4_2025_45_is_v14);
	RUN_TEST(test_fsd_proto_invalid_sw_returns_unknown);

	RUN_TEST(test_platform_resolve_full);
	RUN_TEST(test_platform_resolve_unknown_model);
	RUN_TEST(test_platform_resolve_from_state);
	RUN_TEST(test_platform_resolve_from_state_variant_fallback);

	RUN_TEST(test_sync_platform_to_state);

	RUN_TEST(test_sw_compat_ok_hw4_2026_2_9);
	RUN_TEST(test_sw_compat_warn_hw4_2026_8_6);
	RUN_TEST(test_sw_compat_warn_hw4_old_software);
	RUN_TEST(test_sw_compat_ok_hw3_2025);
	RUN_TEST(test_sw_compat_unknown_no_sw);

	RUN_TEST(test_capabilities_cybertruck);
	RUN_TEST(test_capabilities_legacy_limited);

	return UNITY_END();
}

