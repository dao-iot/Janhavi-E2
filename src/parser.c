#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "parser.h"
#include "data_model.h"
#include "can_message.h"
#include "logger.h"

/* DBC-LIKE SIGNAL DEFINITION */

/* Describes a CAN signal using DBC-style. */
typedef struct
{
    /* Message-level metadata (BO_) */
    uint16_t    can_id;
    const char *message_name;
    uint8_t     dlc;

    /* Signal-level metadata (SG_) */
    const char *signal_name;
    uint8_t     start_bit;
    uint8_t     bit_length;

    /* Conversion parameters */
    float scale;
    float offset;

    /* Validation limits */
    float min;
    float max;

    /* Display unit */
    const char *unit;

} CAN_SignalDef;

/* SIGNAL TABLE (MINI DBC) */

static CAN_SignalDef signal_table[] =
{
    { 0x101, "MotorRPM",       2, "Motor_RPM",       0, 16, 1.0f, 0.0f,   0.0f, 10000.0f, "rpm"  },
    { 0x102, "VehicleSpeed",   2, "Vehicle_Speed",    0, 16, 0.1f, 0.0f,   0.0f,   120.0f, "km/h" },
    { 0x103, "BatterySOC",     1, "Battery_SOC",      0,  8, 1.0f, 0.0f,   0.0f,   100.0f, "%"    },
    { 0x104, "BatteryVoltage", 2, "Battery_Voltage",  0, 16, 0.1f, 0.0f,   0.0f,   100.0f, "V"    },
    { 0x105, "MotorTemp",      1, "Motor_Temperature",0,  8, 1.0f, 0.0f,   0.0f,   150.0f, "C"    }
};

#define SIGNAL_COUNT (sizeof(signal_table) / sizeof(signal_table[0]))

/* RAW VALUE EXTRACTION */

static uint32_t extract_raw_value(const CAN_Message *msg,
                                  const CAN_SignalDef *sig)
{
    uint32_t raw = 0;
    int start_byte = sig->start_bit / 8;
    int byte_len   = sig->bit_length / 8;

    for (int i = 0; i < byte_len; i++) {
        raw = (raw << 8) | msg->data[start_byte + i];
    }

    return raw;
}

/* PARSER ENTRY POINT */

void parse_can_message(const CAN_Message *msg)
{
    for (int i = 0; i < SIGNAL_COUNT; i++) {

        const CAN_SignalDef *signal = &signal_table[i];

        if (signal->can_id != msg->id)
            continue;

        /* Validate DLC */
        if (msg->dlc != signal->dlc) {
            printf("ERROR: DLC mismatch for %s (expected %d, got %d)\n",
                   signal->message_name, signal->dlc, msg->dlc);
            return;
        }

        /* Decode */
        uint32_t raw = extract_raw_value(msg, signal);
        float physical = raw * signal->scale + signal->offset;

        /* Range validation */
        int out_of_range = (physical < signal->min || physical > signal->max);

        if (out_of_range) {
            printf("WARNING: %s out of range (%.2f %s)\n",
                   signal->signal_name, physical, signal->unit);
        }

        printf("Decoded | %s = %.2f %s\n",
               signal->signal_name, physical, signal->unit);
        
        log_can_message(msg,
                signal->signal_name,
                physical,
                signal->unit,
                out_of_range);


        /* Update shared vehicle data */
        if (strcmp(signal->signal_name, "Motor_RPM") == 0) {
            g_vehicle_data.motor_rpm  = physical;
            g_vehicle_data.rpm_warning = out_of_range;
        }
        else if (strcmp(signal->signal_name, "Vehicle_Speed") == 0) {
            g_vehicle_data.vehicle_speed = physical;
            g_vehicle_data.speed_warning = out_of_range;
        }
        else if (strcmp(signal->signal_name, "Battery_SOC") == 0) {
            g_vehicle_data.battery_soc = physical;
        }
        else if (strcmp(signal->signal_name, "Battery_Voltage") == 0) {
            g_vehicle_data.battery_voltage = physical;
        }
        else if (strcmp(signal->signal_name, "Motor_Temperature") == 0) {
            g_vehicle_data.motor_temperature = physical;
        }

        return;
    }

    /* Unknown CAN ID */
    printf("INFO: Unknown CAN ID 0x%03X ignored\n", msg->id);
}
