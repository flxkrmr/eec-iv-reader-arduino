#include "fault_code_util.h"


void createReadableMessage(const char *faultCode, char *messageReadable, unsigned int messageReadableSize) {
    fault_code_message_t fault_code_messages[] = {
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