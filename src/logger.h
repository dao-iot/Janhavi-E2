#ifndef LOGGER_H
#define LOGGER_H

#include "can_message.h"

/* Initialize log file */
void logger_init(void);

/* Log one decoded CAN message */
void log_can_message(const CAN_Message *msg,
                     const char *signal_name,
                     float physical_value,
                     const char *unit,
                     int warning_flag);

/* Close log file */
void logger_close(void);

#endif
