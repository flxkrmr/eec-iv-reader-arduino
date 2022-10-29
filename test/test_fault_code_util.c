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
    TEST_ASSERT_EQUAL_STRING("[111] All systems work properly", messageReadable);
}

void test_createReadableMessage_Error() {
    const char *messageCode = "511";
    char messageReadable[256];
    createReadableMessage(messageCode, messageReadable, sizeof(messageReadable));
    TEST_ASSERT_EQUAL_STRING("[511] ECM: ROM error", messageReadable);
}

void test_createReadableMessage_Unknown() {
    const char *messageCode = "abc";
    char messageReadable[256];
    createReadableMessage(messageCode, messageReadable, sizeof(messageReadable));
    TEST_ASSERT_EQUAL_STRING("[abc] Unknown", messageReadable);
}

void test_splitMessage_emptyLine() {
    const char *message = "";
    const unsigned short messageLineSize = 15;
    char messageLine1[messageLineSize];
    char messageLine2[messageLineSize];
    char messageLine3[messageLineSize];

    splitMessage(message, messageLine1, messageLine2, messageLine3, messageLineSize);

    TEST_ASSERT_EQUAL_STRING(messageLine1, "");
    TEST_ASSERT_EQUAL_STRING(messageLine2, "");
    TEST_ASSERT_EQUAL_STRING(messageLine3, "");
}

void test_splitMessage_singleLine1() {
    const char *message = "abc";
    const unsigned short messageLineSize = 15;
    char messageLine1[messageLineSize];
    char messageLine2[messageLineSize];
    char messageLine3[messageLineSize];

    splitMessage(message, messageLine1, messageLine2, messageLine3, messageLineSize);

    TEST_ASSERT_EQUAL_STRING("abc", messageLine1);
    TEST_ASSERT_EQUAL_STRING("", messageLine2);
    TEST_ASSERT_EQUAL_STRING("", messageLine3);
}

void test_splitMessage_singleLine2() {
    const char *message = "abc";
    const unsigned short messageLineSize = 4;
    char messageLine1[messageLineSize];
    char messageLine2[messageLineSize];
    char messageLine3[messageLineSize];

    splitMessage(message, messageLine1, messageLine2, messageLine3, messageLineSize);

    TEST_ASSERT_EQUAL_STRING("abc", messageLine1);
    TEST_ASSERT_EQUAL_STRING("", messageLine2);
    TEST_ASSERT_EQUAL_STRING("", messageLine3);
}

void test_splitMessage_doubleLine1() {
    const char *message = "abcdefghij";
    const unsigned short messageLineSize = 6;
    char messageLine1[messageLineSize];
    char messageLine2[messageLineSize];
    char messageLine3[messageLineSize];

    splitMessage(message, messageLine1, messageLine2, messageLine3, messageLineSize);

    TEST_ASSERT_EQUAL_STRING("abcde", messageLine1);
    TEST_ASSERT_EQUAL_STRING("fghij", messageLine2);
    TEST_ASSERT_EQUAL_STRING("", messageLine3);
}

void test_splitMessage_doubleLine2() {
    const char *message = "abcdefgh";
    const unsigned short messageLineSize = 6;
    char messageLine1[messageLineSize];
    char messageLine2[messageLineSize];
    char messageLine3[messageLineSize];

    splitMessage(message, messageLine1, messageLine2, messageLine3, messageLineSize);

    TEST_ASSERT_EQUAL_STRING("abcde", messageLine1);
    TEST_ASSERT_EQUAL_STRING("fgh", messageLine2);
    TEST_ASSERT_EQUAL_STRING("", messageLine3);
}


void test_splitMessage_tripleLine1() {
    const char *message = "abcdefghijkl";
    const unsigned short messageLineSize = 6;
    char messageLine1[messageLineSize];
    char messageLine2[messageLineSize];
    char messageLine3[messageLineSize];

    splitMessage(message, messageLine1, messageLine2, messageLine3, messageLineSize);

    TEST_ASSERT_EQUAL_STRING("abcde", messageLine1);
    TEST_ASSERT_EQUAL_STRING("fghij", messageLine2);
    TEST_ASSERT_EQUAL_STRING("kl", messageLine3);
}

void test_splitMessage_tripleLine2() {
    const char *message = "abcdefghijklmno";
    const unsigned short messageLineSize = 6;
    char messageLine1[messageLineSize];
    char messageLine2[messageLineSize];
    char messageLine3[messageLineSize];

    splitMessage(message, messageLine1, messageLine2, messageLine3, messageLineSize);

    TEST_ASSERT_EQUAL_STRING("abcde", messageLine1);
    TEST_ASSERT_EQUAL_STRING("fghij", messageLine2);
    TEST_ASSERT_EQUAL_STRING("klmno", messageLine3);
}

void test_splitMessage_tripleLine3() {
    const char *message = "abcdefghijklmnopqrst";
    const unsigned short messageLineSize = 6;
    char messageLine1[messageLineSize];
    char messageLine2[messageLineSize];
    char messageLine3[messageLineSize];

    splitMessage(message, messageLine1, messageLine2, messageLine3, messageLineSize);

    TEST_ASSERT_EQUAL_STRING("abcde", messageLine1);
    TEST_ASSERT_EQUAL_STRING("fghij", messageLine2);
    TEST_ASSERT_EQUAL_STRING("klmno", messageLine3);
}

void test_createReadableSplittedMessage() {
    const char *faultCode = "111";
    const unsigned short messageLineSize = 10;
    char messageLine1[messageLineSize];
    char messageLine2[messageLineSize];
    char messageLine3[messageLineSize];

    createReadableSplittedMessage(faultCode, messageLine1, messageLine2, messageLine3, messageLineSize);

    TEST_ASSERT_EQUAL_STRING("[111] All", messageLine1);
    TEST_ASSERT_EQUAL_STRING(" systems ", messageLine2);
    TEST_ASSERT_EQUAL_STRING("work prop", messageLine3);
}

int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_createReadableMessage_AllOk);
    RUN_TEST(test_createReadableMessage_Error);
    RUN_TEST(test_createReadableMessage_Unknown);

    RUN_TEST(test_splitMessage_emptyLine);
    RUN_TEST(test_splitMessage_singleLine1);
    RUN_TEST(test_splitMessage_singleLine2);
    RUN_TEST(test_splitMessage_doubleLine1);
    RUN_TEST(test_splitMessage_doubleLine2);
    RUN_TEST(test_splitMessage_tripleLine1);
    RUN_TEST(test_splitMessage_tripleLine2);
    RUN_TEST(test_splitMessage_tripleLine3);

    RUN_TEST(test_createReadableSplittedMessage);

    UNITY_END();
}