#ifndef PARSER_H
#define PARSER_H
#include "can_message.h"

/* Parse and decode a received CAN message. */
void parse_can_message(const CAN_Message *msg);

#endif /* PARSER_H */