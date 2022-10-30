#ifndef FAULT_CODE_UTIL_H
#define FAULT_CODE_UTIL_H

#include <string.h>
#include <stdio.h>

typedef struct fault_code_message_s
{
    const char *fault_code;
    const char *message;
} fault_code_message_t;

static fault_code_message_t fault_code_messages[] = {
    (fault_code_message_t) {"111", "All systems work properly"},
    (fault_code_message_t) {"112", "IAT-sensor: voltage low"},
    (fault_code_message_t) {"113", "IAT-sensor: voltage high"},
    (fault_code_message_t) {"114", "IAT-sensor: range"},
    (fault_code_message_t) {"116", "ECT-sensor: range"},
    (fault_code_message_t) {"117", "ECT-sensor: voltage low"},
    (fault_code_message_t) {"118", "ECT-sensor: voltage high"},
    (fault_code_message_t) {"511", "ECM: ROM error"},
    (fault_code_message_t) {"512", "ECM: KAM error"},
    (fault_code_message_t) {"513", "ECM: internal reference voltage"},
};

void createReadableMessage(const char *faultCode, char *messageReadable, unsigned int messageReadableSize);
void splitMessage(const char *message, char *messageLine1, char *messageLine2,
    char *messageLine3, unsigned int messageLinesSize);
void createReadableSplittedMessage(const char *faultCode, char *messageLine1, char *messageLine2,
    char *messageLine3, unsigned int messageLinesSize);

#endif