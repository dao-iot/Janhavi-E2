#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>
#include <pthread.h>
#define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

#include "can_message.h"
#include "parser.h"
#include "web_server.h"
#include "tests.h"
#include "logger.h"

/* CAN MESSAGE UTILITIES */

/* Print a CAN message in human-readable form. */
void print_can_message(const CAN_Message *msg)
{
    printf("ID: 0x%03X | DLC: %d | Data: [", msg->id, msg->dlc);

    for (int i = 0; i < msg->dlc; i++) {
        printf("%02X", msg->data[i]);
        if (i < msg->dlc - 1) {
            printf(" ");
        }
    }

    printf("]\n");
}

/* CAN SIMULATOR STATE */

/* Used to simulate vehicle behavior. */
typedef struct
{
    float motor_rpm;
    float vehicle_speed;
    float battery_soc;
    float battery_voltage;
    float motor_temperature;

    int rpm_direction;
    int speed_increasing;
    
} CAN_Simulator;

/* Initialize simulator parameters. */
void simulator_init(CAN_Simulator *sim)
{
    sim->motor_rpm         = 0.0f;
    sim->vehicle_speed     = 0.0f;
    sim->battery_soc       = 100.0f;
    sim->battery_voltage   = 62.0f;
    sim->motor_temperature = 25.0f;
    sim->rpm_direction     = 1.0f;
    sim->speed_increasing  = 1.0f;
}

/* Simulated VEHICLE DYNAMICS */

void simulate_driving(CAN_Simulator *sim)
{
    /* Gradual RPM increase */
    if (sim->rpm_direction == 1) {
    sim->motor_rpm += 200;

    if (sim->motor_rpm >= 11000) {
        sim->rpm_direction = -1;
    }
} else {
    sim->motor_rpm -= 200;

    if (sim->motor_rpm <= 0) {
        sim->rpm_direction = 1;
    }
}

    /* Speed derived from RPM */
    /* Vehicle speed simulation */
if (sim->vehicle_speed < 130.0f && sim->speed_increasing) {
    sim->vehicle_speed += 2.5f;

    if (sim->vehicle_speed >= 130.0f) {
        sim->speed_increasing = 0;   // start reducing
    }
} 
else {
    sim->vehicle_speed -= 3.0f;

    if (sim->vehicle_speed <= 80.0f) {
        sim->speed_increasing = 1;   
    }
}


    /* Battery SOC drain */
    if (sim->battery_soc > 0.0f) {
        sim->battery_soc -= 0.01f;
    }

    /* Battery voltage estimation */
    sim->battery_voltage = 48.0f + (sim->battery_soc / 100.0f);

    /* Motor temperature rise with RPM */
    float target_temp = 25.0f + (sim->motor_rpm / 8000.0f) * 75.0f;
    if (sim->motor_temperature < target_temp) {
        sim->motor_temperature += 0.1f;
    }
}

/* CAN MESSAGE GENERATORS */

/* 0x101 – Motor RPM */
CAN_Message make_rpm_message(const CAN_Simulator *sim)
{
    CAN_Message msg = {0};
    uint16_t rpm = (uint16_t)sim->motor_rpm;

    msg.id        = 0x101;
    msg.dlc       = 2;
    msg.data[0]   = (rpm >> 8) & 0xFF;
    msg.data[1]   = rpm & 0xFF;
    msg.timestamp = time(NULL);

    return msg;
}

/* 0x102 – Vehicle speed (×10) */
CAN_Message make_speed_message(const CAN_Simulator *sim)
{
    CAN_Message msg = {0};
    uint16_t speed = (uint16_t)(sim->vehicle_speed * 10);

    msg.id        = 0x102;
    msg.dlc       = 2;
    msg.data[0]   = (speed >> 8) & 0xFF;
    msg.data[1]   = speed & 0xFF;
    msg.timestamp = time(NULL);

    return msg;
}

/* 0x103 – Battery SOC */
CAN_Message make_soc_message(const CAN_Simulator *sim)
{
    CAN_Message msg = {0};

    msg.id        = 0x103;
    msg.dlc       = 1;
    msg.data[0]   = (uint8_t)sim->battery_soc;
    msg.timestamp = time(NULL);

    return msg;
}

/* 0x104 – Battery voltage (×10) */
CAN_Message make_voltage_message(const CAN_Simulator *sim)
{
    CAN_Message msg = {0};
    uint16_t voltage = (uint16_t)(sim->battery_voltage * 10);

    msg.id        = 0x104;
    msg.dlc       = 2;
    msg.data[0]   = (voltage >> 8) & 0xFF;
    msg.data[1]   = voltage & 0xFF;
    msg.timestamp = time(NULL);

    return msg;
}

/* 0x105 – Motor temperature */
CAN_Message make_temp_message(const CAN_Simulator *sim)
{
    CAN_Message msg = {0};

    msg.id        = 0x105;
    msg.dlc       = 1;
    msg.data[0]   = (uint8_t)sim->motor_temperature;
    msg.timestamp = time(NULL);

    return msg;
}

/* SIMULATION LOOP */

void run_simulation(void)
{
    CAN_Simulator simulator;
    simulator_init(&simulator);

    while (1) {
        simulate_driving(&simulator);

        CAN_Message messages[] = {
            make_rpm_message(&simulator),
            make_speed_message(&simulator),
            make_soc_message(&simulator),
            make_voltage_message(&simulator),
            make_temp_message(&simulator)
        };

        for (int i = 0; i < 5; i++) {
            print_can_message(&messages[i]);
            parse_can_message(&messages[i]);
            SLEEP_MS(100);
        }
    }
}

/* WEB SERVER THREAD */

#ifdef _WIN32
DWORD WINAPI web_server_thread(LPVOID arg)
{
    (void)arg;
    start_web_server();
    return 0;
}
#else
void *web_server_thread(void *arg)
{
    (void)arg;
    start_web_server();
    return NULL;
}
#endif

/* MAIN APPLICATION */

typedef enum
{
    MODE_SIMULATION = 2,
    MODE_TEST       = 1
} AppMode;

int main(void)
{
    int choice = 0;
    logger_init();
    printf("Select Mode:\n");
    printf("  1. Run Parser Test Cases\n");
    printf("  2. Run CAN Simulation\n");
    printf("Enter your choice: ");

    if (scanf("%d", &choice) != 1) {
        printf("ERROR: Invalid input\n");
        return 1;
    }

    if (choice == MODE_TEST) {
        printf("\n--- Running TEST MODE ---\n");
        run_all_tests();
    }
    else if (choice == MODE_SIMULATION) {
        printf("\n--- Running SIMULATION MODE ---\n");

        #ifdef _WIN32
            CreateThread(NULL, 0, web_server_thread, NULL, 0, NULL);
        #else
            pthread_t server_tid;
            pthread_create(&server_tid, NULL, web_server_thread, NULL);
        #endif
            run_simulation();  
    }
    else {
        printf("ERROR: Unknown option selected\n");
    }

    return 0;
}
    