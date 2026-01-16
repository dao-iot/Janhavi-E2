#include "logger.h"
#include <stdio.h>
#include <time.h>

static FILE *log_file = NULL;

void logger_init(void)
{
    log_file = fopen("can_log.txt", "w");
    if (log_file) {
        fprintf(log_file,
                "TIMESTAMP | CAN_ID | DLC | DATA | SIGNAL | VALUE | STATUS\n");
        fflush(log_file);
    }
}

static void format_timestamp(char *buf, size_t len)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buf, len, "%Y-%m-%d %H:%M:%S", t);
}

void log_can_message(const CAN_Message *msg,
                     const char *signal_name,
                     float physical_value,
                     const char *unit,
                     int warning_flag)
{
    if (!log_file)
        return;

    char timestamp[32];
    format_timestamp(timestamp, sizeof(timestamp));

    fprintf(log_file,
            "%s | 0x%03X | %d | ",
            timestamp, msg->id, msg->dlc);

    for (int i = 0; i < msg->dlc; i++) {
        fprintf(log_file, "%02X ", msg->data[i]);
    }

    fprintf(log_file,
            "| %s | %.2f %s | %s\n",
            signal_name,
            physical_value,
            unit,
            warning_flag ? "WARNING" : "OK");

    fflush(log_file);
}

void logger_close(void)
{
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}
