#include <unity.h>
#include "fault_code_util.h"

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_createReadableMessage_AllOk() {
    const char *messageCode = "111";
    char messageReadable[256];
    createReadableMessage(messageCode, messageReadable, sizeof(messageReadable));
    TEST_ASSERT_EQUAL_STRING(messageReadable, "[111] All systems work properly");
}

void test_createReadableMessage_Error() {
    const char *messageCode = "511";
    char messageReadable[256];
    createReadableMessage(messageCode, messageReadable, sizeof(messageReadable));
    TEST_ASSERT_EQUAL_STRING(messageReadable, "[511] ECM: ROM error");
}

void test_createReadableMessage_Unknown() {
    const char *messageCode = "abc";
    char messageReadable[256];
    createReadableMessage(messageCode, messageReadable, sizeof(messageReadable));
    TEST_ASSERT_EQUAL_STRING(messageReadable, "[abc] Unknown");
}

void test_splitMessage_singleLine() {
    const char *message = "abc";
    const unsigned short messageLineSize = 15;
    char messageLine1[messageLineSize];
    char messageLine2[messageLineSize];
    char messageLine3[messageLineSize];
    splitMessage(message, messageLine1, messageLine2, messageLine3, messageLineSize);
    TEST_ASSERT_EQUAL_STRING(messageLine1, "abc");
    TEST_ASSERT_EQUAL_STRING(messageLine1, "");
    TEST_ASSERT_EQUAL_STRING(messageLine1, "");
}

int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_createReadableMessage_AllOk);
    RUN_TEST(test_createReadableMessage_Error);
    RUN_TEST(test_createReadableMessage_Unknown);

    RUN_TEST(test_splitMessage_singleLine);

    UNITY_END();
}