//data_model.c

#include "data_model.h"

/**
 * @brief Global vehicle data instance.
 *
 * Holds the latest decoded CAN signal values.
 * This structure is updated by the parser module
 * and consumed by the application, UI, or logging layers.
 */
VehicleData g_vehicle_data = {
    .motor_rpm         = 0.0f,
    .vehicle_speed     = 0.0f,
    .battery_soc       = 0.0f,
    .battery_voltage   = 0.0f,
    .motor_temperature = 0.0f,

};
