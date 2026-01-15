//tests.c

#include <stdio.h>

#include "tests.h"
#include "parser.h"
#include "can_message.h"

/* ------------------------------------------------------------
 * Test Case 1: Correct Parsing
 * ------------------------------------------------------------ */

/**
 * @brief Verify correct parsing of Motor RPM.
 */
static void test_correct_parsing(void)
{
    printf("\n[Test 1] Correct Parsing (Motor RPM)\n");

    CAN_Message msg = {
        .id   = 0x101,
        .dlc  = 2,
        .data = {0x13, 0x88}   /* 0x1388 = 5000 RPM */
    };

    print_can_message(&msg);
    parse_can_message(&msg);
}

/* ------------------------------------------------------------
 * Test Case 2: Scaling Validation
 * ------------------------------------------------------------ */

/**
 * @brief Verify scaling logic for battery voltage.
 */
static void test_scaling(void)
{
    printf("\n[Test 2] Scaling Validation (Battery Voltage)\n");

    CAN_Message msg = {
        .id   = 0x104,
        .dlc  = 2,
        .data = {0x02, 0x71}   /* 0x0271 = 625 → 62.5 V */
    };

    print_can_message(&msg);
    parse_can_message(&msg);
}

/* ------------------------------------------------------------
 * Test Case 3: Range Validation
 * ------------------------------------------------------------ */

/**
 * @brief Verify range checking for Battery SOC.
 */
static void test_range_validation(void)
{
    printf("\n[Test 3] Range Validation (Battery SOC)\n");

    CAN_Message msg = {
        .id   = 0x103,
        .dlc  = 1,
        .data = {0xFF}         /* 255% → invalid */
    };

    print_can_message(&msg);
    parse_can_message(&msg);
}

/* ------------------------------------------------------------
 * Test Case 4: Invalid Messages
 * ------------------------------------------------------------ */

/**
 * @brief Validate error handling for malformed CAN frames.
 */
static void test_invalid_messages(void)
{
    printf("\n[Test 4A] Unknown CAN ID\n");

    CAN_Message unknown_id = {
        .id   = 0x999,
        .dlc  = 2,
        .data = {0x00, 0x00}
    };
    print_can_message(&unknown_id);
    parse_can_message(&unknown_id);

    printf("\n[Test 4B] Incorrect DLC\n");

    CAN_Message wrong_dlc = {
        .id   = 0x101,
        .dlc  = 1,             /* Should be 2 */
        .data = {0x10}
    };
    print_can_message(&wrong_dlc);
    parse_can_message(&wrong_dlc);

    printf("\n[Test 4C] Malformed Data\n");

    CAN_Message malformed = {
        .id   = 0x104,
        .dlc  = 2,
        .data = {0xFF}         /* Missing second byte */
    };
    print_can_message(&malformed);
    parse_can_message(&malformed);
}

/* ------------------------------------------------------------
 * Test Runner
 * ------------------------------------------------------------ */

void run_all_tests(void)
{
    printf("\n--- CAN PARSER TEST MODE ---\n");

    test_correct_parsing();
    test_scaling();
    test_range_validation();
    test_invalid_messages();

    printf("\n--- TESTS COMPLETE ---\n");
}
