#include "fault_code_util.h"


void createReadableMessage(const char *faultCode, char *messageReadable, unsigned int messageReadableSize) {
    char buffer[40];

    for (unsigned int i = 0; i < sizeof(fault_code_messages) / sizeof(fault_code_message_t); i++) {
        if (!strcmp(fault_code_messages[i].fault_code, faultCode)) {
            sprintf(buffer, "[%s] %s", faultCode, fault_code_messages[i].message);
            strncpy(messageReadable, buffer, messageReadableSize);
            return;
        }
    }

    sprintf(buffer, "[%s] Unknown", faultCode);
    strncpy(messageReadable, buffer, messageReadableSize);
}

void splitMessage(const char *message, char *messageLine1, char *messageLine2,
    char *messageLine3, unsigned int messageLinesSize) {
    if (strlen(message) < messageLinesSize) {
        strncpy(messageLine1, message, messageLinesSize);
        strncpy(messageLine2, "", messageLinesSize);
        strncpy(messageLine3, "", messageLinesSize);
    } else if (strlen(message) < messageLinesSize*2) {
        strncpy(messageLine1, message, messageLinesSize-1);
        messageLine1[messageLinesSize-1] = '\0';
        strncpy(messageLine2, message+messageLinesSize-1, messageLinesSize-1);
        messageLine2[messageLinesSize-1] = '\0'; 
        strncpy(messageLine3, "", messageLinesSize);
    } else {
        strncpy(messageLine1, message, messageLinesSize-1);
        messageLine1[messageLinesSize-1] = '\0';
        strncpy(messageLine2, message+messageLinesSize-1, messageLinesSize-1);
        messageLine2[messageLinesSize-1] = '\0'; 
        strncpy(messageLine3, message+(messageLinesSize-1)*2, messageLinesSize);
        messageLine3[messageLinesSize-1] = '\0'; 
    }
}

void createReadableSplittedMessage(const char *faultCode, char *messageLine1, char *messageLine2, char *messageLine3, unsigned int messageLinesSize) {
    char fullMessage[messageLinesSize * 3];
    createReadableMessage(faultCode, fullMessage, sizeof(fullMessage));
    splitMessage(fullMessage, messageLine1, messageLine2, messageLine3, messageLinesSize);
}