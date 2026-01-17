#ifndef DATA_MODEL_H
#define DATA_MODEL_H

#include <stdint.h>

/* Container for the latest decoded vehicle signals. 
 * This structure holds processed CAN data in physical units,
 * updated by the CAN parser and consumed by the dashboard layer.
 */

 typedef enum {
    MODE_NONE = 0,
    MODE_TEST = 1,
    MODE_SIMULATION = 2
} AppMode;

typedef enum {
    TEST_NOT_RUN = 0,
    TEST_PASS,
    TEST_WARNING,
    TEST_ERROR
} TestStatus;

typedef struct {
    char        name[32];
    char        input[64];
    char        output[64];
    TestStatus  status;
} TestResult;

#define MAX_TESTS 10

typedef struct {
    TestResult results[MAX_TESTS];
    int        count;
} TestDashboardData;

typedef struct
{
    /* Live Decoded Signals */
    float motor_rpm;          /* Motor speed in RPM */
    float vehicle_speed;      /* Vehicle speed */
    float battery_soc;        /* Battery state of charge (%) */
    float battery_voltage;    /* Battery voltage */
    float motor_temperature;  /* Motor temperature */
    /* Runtime Warnings */
    int rpm_warning;
    int speed_warning;
    int mode;
    TestDashboardData test_dashboard;
    
    struct {
        int correct_parsing;      // 1 = PASS
        int scaling_validation;   // 1 = PASS
        int range_validation;     // -1 = WARNING
        int invalid_message;      // 1 = PASS
    } tests;

} VehicleData;

/* Global vehicle data instance.
 * This structure is updated by the CAN parsing logic and
 * accessed by other modules for display or processing.
 */
extern VehicleData g_vehicle_data;

#endif /* DATA_MODEL_H */