#include <stdio.h>
#include <string.h>

#include "tests.h"
#include "parser.h"
#include "can_message.h"
#include "data_model.h"

static void add_test_result(const char *name, const char *input, const char *output, TestStatus status)
{
    TestDashboardData *td = &g_vehicle_data.test_dashboard;

    if (td->count >= MAX_TESTS)
        return;

    TestResult *r = &td->results[td->count++];

    snprintf(r->name, sizeof(r->name), "%s", name);
    snprintf(r->input, sizeof(r->input), "%s", input);
    snprintf(r->output, sizeof(r->output), "%s", output);
    r->status = status;
}

/* ------------------------------------------------------------
 * TEST 1: CORRECT PARSING (Motor RPM)
 * ------------------------------------------------------------ */
static void test_motor_rpm_parsing(void)
{
    CAN_Message msg = {
        .id  = 0x101,
        .dlc = 2,
        .data = {0x13, 0x88}   /* 5000 RPM */
    };

    parse_can_message(&msg);

    if ((int)g_vehicle_data.motor_rpm == 5000) {
        add_test_result(
            "Motor RPM Parsing",
            "ID=0x101 DLC=2 DATA=[13 88]",
            "Motor_RPM = 5000 rpm",
            TEST_PASS
        );
    } else {
        add_test_result(
            "Motor RPM Parsing",
            "ID=0x101 DLC=2 DATA=[13 88]",
            "Incorrect RPM decoding",
            TEST_ERROR
        );
    }
}

/* ------------------------------------------------------------
 * TEST 2: SCALING VALIDATION (Battery Voltage)
 * ------------------------------------------------------------ */
static void test_voltage_scaling(void)
{
    CAN_Message msg = {
        .id  = 0x104,
        .dlc = 2,
        .data = {0x02, 0x71}   /* 625 → 62.5 V */
    };

    parse_can_message(&msg);

    if ((int)(g_vehicle_data.battery_voltage * 10) == 625) {
        add_test_result(
            "Battery Voltage Scaling",
            "ID=0x104 DLC=2 DATA=[02 71]",
            "Voltage = 62.5 V",
            TEST_PASS
        );
    } else {
        add_test_result(
            "Battery Voltage Scaling",
            "ID=0x104 DLC=2 DATA=[02 71]",
            "Scaling error",
            TEST_ERROR
        );
    }
}

/* ------------------------------------------------------------
 * TEST 3: RANGE VALIDATION (Battery SOC)
 * ------------------------------------------------------------ */
static void test_soc_range_check(void)
{
    CAN_Message msg = {
        .id  = 0x103,
        .dlc = 1,
        .data = {0xFF}   /* 255% → invalid */
    };

    parse_can_message(&msg);

    if (g_vehicle_data.battery_soc > 100) {
        add_test_result(
            "Battery SOC Range Check",
            "ID=0x103 DLC=1 DATA=[FF]",
            "SOC out of range detected",
            TEST_WARNING
        );
    } else {
        add_test_result(
            "Battery SOC Range Check",
            "ID=0x103 DLC=1 DATA=[FF]",
            "Range check failed",
            TEST_ERROR
        );
    }
}

/* ------------------------------------------------------------
 * TEST 4A: UNKNOWN CAN ID
 * ------------------------------------------------------------ */
static void test_unknown_can_id(void)
{
    CAN_Message msg = {
        .id  = 0x999,
        .dlc = 2,
        .data = {0xAA, 0xBB}
    };

    VehicleData before = g_vehicle_data;

    parse_can_message(&msg);

    if (memcmp(&before, &g_vehicle_data, sizeof(VehicleData)) == 0) {
        add_test_result(
            "Unknown CAN ID Handling",
            "ID=0x999 DLC=2 DATA=[AA BB]",
            "Message ignored safely",
            TEST_PASS
        );
    } else {
        add_test_result(
            "Unknown CAN ID Handling",
            "ID=0x999 DLC=2 DATA=[AA BB]",
            "Unknown ID modified vehicle data",
            TEST_ERROR
        );
    }
}


/* ------------------------------------------------------------
 * TEST 4B: WRONG DLC
 * ------------------------------------------------------------ */
static void test_wrong_dlc(void)
{
    CAN_Message msg = {
        .id  = 0x101,   /* Motor RPM expects DLC = 2 */
        .dlc = 1,
        .data = {0x10}
    };

    VehicleData before = g_vehicle_data;

    parse_can_message(&msg);

    if (memcmp(&before, &g_vehicle_data, sizeof(VehicleData)) == 0) {
        add_test_result(
            "Wrong DLC Detection",
            "ID=0x101 DLC=1 DATA=[10]",
            "DLC mismatch correctly ignored",
            TEST_PASS
        );
    } else {
        add_test_result(
            "Wrong DLC Detection",
            "ID=0x101 DLC=1 DATA=[10]",
            "Data modified despite wrong DLC",
            TEST_ERROR
        );
    }
}


/* ------------------------------------------------------------
 * TEST RUNNER
 * ------------------------------------------------------------ */
void run_all_tests(void)
{
    /* Switch system to TEST MODE */
    g_vehicle_data.mode = 1;
    g_vehicle_data.test_dashboard.count = 0;

    test_motor_rpm_parsing();
    test_voltage_scaling();
    test_soc_range_check();
    test_unknown_can_id();
    test_wrong_dlc();

    printf("All tests executed.\n");
}
