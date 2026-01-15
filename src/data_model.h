#ifndef DATA_MODEL_H
#define DATA_MODEL_H

#include <stdint.h>

/* Container for the latest decoded vehicle signals. 
 * This structure holds processed CAN data in physical units,
 * updated by the CAN parser and consumed by the dashboard layer.
 */

typedef struct
{
    float motor_rpm;          /* Motor speed in RPM */
    float vehicle_speed;      /* Vehicle speed */
    float battery_soc;        /* Battery state of charge (%) */
    float battery_voltage;    /* Battery voltage */
    float motor_temperature;  /* Motor temperature */

    int rpm_warning;

} VehicleData;

/* Global vehicle data instance.
 * This structure is updated by the CAN parsing logic and
 * accessed by other modules for display or processing.
 */
extern VehicleData g_vehicle_data;

#endif /* DATA_MODEL_H */