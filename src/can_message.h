//can_message.h

#ifndef CAN_MESSAGE_H
#define CAN_MESSAGE_H

#include <stdint.h>
#include <time.h>

/* Software representation of a CAN frame. */
typedef struct
{
    uint16_t id;        /* CAN identifier (standard 11-bit ID) */
    uint8_t  dlc;       /* Data Length Code (0–8 bytes) */
    uint8_t  data[8];   /* CAN payload */
    time_t   timestamp; /* Message generation or reception time */
} CAN_Message;

/* Prints a CAN message, Parameter passed is the msg Pointer to the CAN_Message to be printed. */
void print_can_message(const CAN_Message *msg);

#endif /* CAN_MESSAGE_H */