#ifndef FAULT_CODE_UTIL_H
#define FAULT_CODE_UTIL_H

#include <string.h>

void createReadableMessage(const char *messageCode, char *messageReadable, unsigned int messageReadableSize);

#endif