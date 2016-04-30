/* 
 * File:   UnitTest.c
 * Author: alex
 *
 * Created on 26. April 2016, 20:27
 */

#include <stdlib.h>
#include "unity.h"
#include "solve.h"
#include "util.h"

void test_Dummy(void) {
    TEST_ASSERT_EQUAL(2, 240);
}

void test_Dummy2(void) {
    TEST_ASSERT_EQUAL(12, 12);
    TEST_ASSERT_EQUAL(12, 12);
}

void test_fieldCandidatesSubsetOf(void) {
    unsigned *candidates;
    Field field;
    unsigned *numbers;

    candidates = (unsigned *) xmalloc(sizeof (unsigned) * 9);
    for (unsigned i = 0; i < 9; i++) {
        candidates[i] = 0;
    }
    candidates[1] = 2;
    candidates[3] = 4;
    candidates[4] = 5;
    candidates[5] = 6;

    field.candidates = candidates;

    numbers = (unsigned *) xmalloc(sizeof (unsigned) * 9);
    numbers[0] = 2;
    numbers[1] = 4;
    numbers[2] = 5;
    numbers[3] = 0;

    TEST_ASSERT_EQUAL(1, fieldCandidatesSubsetOf(&field, numbers));

    numbers[0] = 2;
    numbers[1] = 7;
    numbers[2] = 0;

    TEST_ASSERT_EQUAL(0, fieldCandidatesSubsetOf(&field, numbers));

    for (unsigned i = 0; i < 9; i++) {
        candidates[i] = 0;
    }
    candidates[4] = 5;

    numbers[0] = 5;
    numbers[1] = 0;
    numbers[2] = 0;

    TEST_ASSERT_EQUAL(1, fieldCandidatesSubsetOf(&field, numbers));

    numbers[0] = 5;
    numbers[1] = 7;
    numbers[2] = 0;

    TEST_ASSERT_EQUAL(0, fieldCandidatesSubsetOf(&field, numbers));



    free(numbers);
    free(candidates);
}

int main(void) {
    UNITY_BEGIN();
    //RUN_TEST(test_Dummy);
    //RUN_TEST(test_Dummy2);
    RUN_TEST(test_fieldCandidatesSubsetOf);
    return UNITY_END();
}
