/* 
 * File:   UnitTest.c
 * Author: alex
 *
 * Created on 26. April 2016, 20:27
 */

#include "..\..\unity\src\unity.h"
#include "..\src\solve.h"

void test_Dummy(void) {
    TEST_ASSERT_EQUAL(2, 240);
}

void test_Dummy2(void) {
    TEST_ASSERT_EQUAL(12, 12);
    TEST_ASSERT_EQUAL(12, 12);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_Dummy);
    RUN_TEST(test_Dummy2);
    return UNITY_END();
}