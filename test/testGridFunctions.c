/* 
 * File:   UnitTest.c
 * Author: alex
 *
 * Created on 26. April 2016, 20:27
 */

#include <stdlib.h>
#include <memory.h>
#include "unity.h"
#include "solve.h"
#include "util.h"
#include "grid.h"

unsigned * uintdup(unsigned *dest, unsigned const *src, size_t len) {
    memcpy(dest, src, len * sizeof (unsigned));
    return dest;
}

void test_Dummy(void) {
    TEST_ASSERT_EQUAL(2, 240);
}

void test_Dummy2(void) {
    TEST_ASSERT_EQUAL(12, 12);
    TEST_ASSERT_EQUAL(12, 12);
}

void test_fieldCandidatesSubsetOf(void) {
    unsigned *candidates;
    unsigned *numbers;
    Field field;

    candidates = (unsigned *) xmalloc(sizeof (unsigned) * 9);
    field.candidates = candidates;

    numbers = (unsigned *) xmalloc(sizeof (unsigned) * 9);

    
    unsigned cand1[9] = {0, 2, 0, 4, 5, 6, 0, 0, 0};
    uintdup(candidates, cand1, 9);


    unsigned numb1[4] = { 2, 4, 5, 0 };
    uintdup(numbers, numb1, 4);

    TEST_ASSERT_EQUAL(1, fieldCandidatesSubsetOf(&field, numbers));

    unsigned numb2[3] = { 2, 7, 0 };
    uintdup(numbers, numb2, 3);

    TEST_ASSERT_EQUAL(0, fieldCandidatesSubsetOf(&field, numbers));

    unsigned cand2[9] = {0, 0, 0, 0, 5, 0, 0, 0, 0};
    uintdup(candidates, cand2, 9);

    unsigned numb3[2] = { 5, 0 };
    uintdup(numbers, numb3, 2);

    TEST_ASSERT_EQUAL(1, fieldCandidatesSubsetOf(&field, numbers));

    unsigned numb4[3] = { 5, 7, 0 };
    uintdup(numbers, numb4, 3);

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
