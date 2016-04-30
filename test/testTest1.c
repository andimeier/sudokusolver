/* 
 * File:   UnitTest.c
 * Author: alex
 *
 * Created on 26. April 2016, 20:27
 */

#include "unity.h"
#include "solve.h"

void test_Dummy(void) {
    TEST_ASSERT_EQUAL(2, 240);
}

void test_Dummy2(void) {
    TEST_ASSERT_EQUAL(12, 12);
    TEST_ASSERT_EQUAL(12, 12);
}


void test_fieldCandidatesSubsetOf(void) {
    unsigned *candidates = { 0, 2, 3, 4, 5, 6, 0, 0, 0 };
    Field field = {
        NULL, // unitPositions
        candidates,
        0, // initValue
        0, // candidatesLeft
        0, // value;
        "A2" // name
    };
    unsigned *numbers = { 2, 4, 5, 0 };
    
    TEST_ASSERT_EQUAL(fieldCandidatesSubsetOf(&field, numbers), 1);
}


int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_Dummy);
    RUN_TEST(test_Dummy2);
    RUN_TEST(test_fieldCandidatesSubsetOf);
    return UNITY_END();
}
