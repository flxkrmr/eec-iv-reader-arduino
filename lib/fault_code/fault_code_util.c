#include "fault_code_util.h"

void createReadableMessage(const char *messageCode, char *messageReadable, unsigned int messageReadableSize) {
  sprintf(messageReadable, "[%0d] %s", messageCode, "Something else");
}