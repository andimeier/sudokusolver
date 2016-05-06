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


    unsigned numb1[4] = {2, 4, 5, 0};
    uintdup(numbers, numb1, 4);

    TEST_ASSERT_EQUAL(1, fieldCandidatesSubsetOf(&field, numbers));

    unsigned numb2[3] = {2, 7, 0};
    uintdup(numbers, numb2, 3);

    TEST_ASSERT_EQUAL(0, fieldCandidatesSubsetOf(&field, numbers));

    unsigned cand2[9] = {0, 0, 0, 0, 5, 0, 0, 0, 0};
    uintdup(candidates, cand2, 9);

    unsigned numb3[2] = {5, 0};
    uintdup(numbers, numb3, 2);

    TEST_ASSERT_EQUAL(1, fieldCandidatesSubsetOf(&field, numbers));

    unsigned numb4[3] = {5, 7, 0};
    uintdup(numbers, numb4, 3);

    TEST_ASSERT_EQUAL(0, fieldCandidatesSubsetOf(&field, numbers));


    free(numbers);
    free(candidates);
}

void test_findNakedTuplesInContainer(void) {
    unsigned *candidates;
    Field *field;
    FieldsVector *container;

    container = (Field **) xmalloc(sizeof (Field *) * MAX_NUMBER);

    for (int i = 0; i < MAX_NUMBER; i++) {
        field = (Field *) xmalloc(sizeof (Field));
        container[i] = field;

        unsigned cand1[9] = {0, 2, 0, 4, 5, 6, 7, 8, 0};
        candidates = (unsigned *) xmalloc(sizeof (unsigned) * MAX_NUMBER);
        uintdup(candidates, cand1, 9);

        field->candidates = candidates;
    }

    // let 2 fields contain a naked tuple: 7, 8
    for (int i = 6; i <= 7; i++) {
        field = container[i];

        unsigned cand1[9] = {0, 0, 0, 0, 0, 0, 7, 8, 0};
        candidates = (unsigned *) xmalloc(sizeof (unsigned) * MAX_NUMBER);
        uintdup(candidates, cand1, 9);

        field->candidates = candidates;
    }

    TEST_ASSERT_EQUAL(1, findNakedTuplesInContainer(container, 2));

    free(candidates);
}

void test_equalNumberOfFieldsAndCandidates(void) {
    FieldsVector *fieldsVector;
    unsigned *numbers;
    Field f;

    fieldsVector = (Field **) xmalloc(sizeof (Field *) * MAX_NUMBER);
    numbers = (unsigned *) xmalloc(sizeof (unsigned) * MAX_NUMBER);
    
    fieldsVector[0] = &f;
    fieldsVector[1] = &f;
    fieldsVector[2] = NULL;
    
    numbers[0] = 2;
    numbers[1] = 1;
    numbers[2] = 0;
    
    TEST_ASSERT_EQUAL(1, equalNumberOfFieldsAndCandidates(fieldsVector, numbers));
}

int main(void) {
    UNITY_BEGIN();
    //RUN_TEST(test_Dummy);
    //RUN_TEST(test_Dummy2);
    RUN_TEST(test_fieldCandidatesSubsetOf);
    RUN_TEST(test_equalNumberOfFieldsAndCandidates);
    RUN_TEST(test_findNakedTuplesInContainer);
    return UNITY_END();
}
