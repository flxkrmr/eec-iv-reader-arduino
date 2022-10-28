#include <unity.h>
#include "fault_code_util.h"

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_createReadableMessage() {
    const char *messageCode = "111";
    char messageReadable[256];
    createReadableMessage(messageCode, messageReadable, sizeof(messageReadable));
    TEST_ASSERT_EQUAL_STRING(messageReadable, "[111] Something else");
}

int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_createReadableMessage);

    UNITY_END();
}