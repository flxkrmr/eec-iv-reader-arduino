#ifndef FAULT_CODE_UTIL_H
#define FAULT_CODE_UTIL_H

#include <string.h>
#include <stdio.h>

typedef struct fault_code_message_s
{
    const char *fault_code;
    const char *message;
} fault_code_message_t;

void createReadableMessage(const char *faultCode, char *messageReadable, unsigned int messageReadableSize);
void splitMessage(const char *message, char *messageLine1, char *messageLine2,
    char *messageLine3, unsigned int messageLinesSize);
void createReadableSplittedMessage(const char *faultCode, char *messageLine1, char *messageLine2,
    char *messageLine3, unsigned int messageLinesSize);

#endif