/* 
 * File:   UnitTest.c
 * Author: alex
 *
 * Created on 26. April 2016, 20:27
 */

#include <stdlib.h>
#include <memory.h>
#include "unity.h"
#include "global.h"
#include "solve.h"
#include "util.h"
#include "grid.h"
#include "typedefs.h"

// prototypes
unsigned * uintdup(unsigned *dest, unsigned const *src, size_t len);
void setCandidates(Field *field, unsigned *cands);

/**
 * sets the candidates for a field
 * @param field
 * @param cands
 */
void setCandidates(Field *field, unsigned *cands) {
    unsigned *candidates;

    candidates = (unsigned *) xmalloc(sizeof (unsigned) * MAX_NUMBER);
    uintdup(candidates, cands, 9);
    field->candidates = candidates;
    field->value = 0; // not solved yet

    field->candidatesLeft = 0;
    for (int i = 0; i < MAX_NUMBER; i++) {
        if (candidates[i]) {
            field->candidatesLeft++;
        }
    }
}

unsigned * uintdup(unsigned *dest, unsigned const *src, size_t len) {
    memcpy(dest, src, len * sizeof (unsigned));
    return dest;
}

#ifdef BUGGY_TRY_TO_REVIVE_TESTS

void test_Dummy(void) {
    TEST_ASSERT_EQUAL(2, 240);
}

void test_Dummy2(void) {
    TEST_ASSERT_EQUAL(12, 12);
    TEST_ASSERT_EQUAL(12, 12);
}

void test_fieldCandidatesContainAllOf(void) {
    unsigned *candidates;
    unsigned *numbers;
    Field field;

    candidates = (unsigned *) xmalloc(sizeof (unsigned) * 9);
    field.candidates = candidates;
    field.value = 0;

    numbers = (unsigned *) xmalloc(sizeof (unsigned) * 9);


    unsigned cand1[9] = {0, 2, 0, 4, 5, 6, 0, 0, 0};
    uintdup(candidates, cand1, 9);


    unsigned numb1[4] = {2, 4, 5, 0};
    uintdup(numbers, numb1, 4);

    TEST_ASSERT_EQUAL(1, fieldCandidatesContainAllOf(&field, numbers));

    unsigned numb2[3] = {2, 7, 0};
    uintdup(numbers, numb2, 3);

    TEST_ASSERT_EQUAL(0, fieldCandidatesContainAllOf(&field, numbers));

    unsigned cand2[9] = {0, 0, 0, 0, 5, 0, 0, 0, 0};
    uintdup(candidates, cand2, 9);

    unsigned numb3[2] = {5, 0};
    uintdup(numbers, numb3, 2);

    TEST_ASSERT_EQUAL(1, fieldCandidatesContainAllOf(&field, numbers));

    unsigned numb4[3] = {5, 7, 0};
    uintdup(numbers, numb4, 3);

    TEST_ASSERT_EQUAL(0, fieldCandidatesContainAllOf(&field, numbers));

    // for solved cells always return false
    unsigned numb5[2] = {5, 0};
    uintdup(numbers, numb5, 2);
    setValue(&field, 1);

    TEST_ASSERT_EQUAL(0, fieldCandidatesContainAllOf(&field, numbers));

    free(numbers);
    free(candidates);
}

void test_fieldCandidatesAreSubsetOf(void) {
    unsigned *candidates;
    unsigned *numbers;
    Field field;

    candidates = (unsigned *) xmalloc(sizeof (unsigned) * 9);
    field.candidates = candidates;
    field.value = 0;

    numbers = (unsigned *) xmalloc(sizeof (unsigned) * 9);


    unsigned cand1[9] = {0, 2, 0, 4, 0, 0, 0, 0, 0};
    uintdup(candidates, cand1, 9);


    unsigned numb1[4] = {2, 4, 5, 0};
    uintdup(numbers, numb1, 4);

    TEST_ASSERT_EQUAL(1, fieldCandidatesAreSubsetOf(&field, numbers));

    unsigned numb2[3] = {2, 0};
    uintdup(numbers, numb2, 2);

    printf("numbers: %u-%u-%u\n", numbers[0], numbers[1], numbers[2]);
    TEST_ASSERT_EQUAL(0, fieldCandidatesAreSubsetOf(&field, numbers));

    unsigned cand2[9] = {0, 0, 0, 0, 5, 0, 0, 0, 0};
    uintdup(candidates, cand2, 9);

    unsigned numb3[2] = {5, 0};
    uintdup(numbers, numb3, 2);

    TEST_ASSERT_EQUAL(1, fieldCandidatesAreSubsetOf(&field, numbers));

    unsigned numb4[3] = {5, 7, 0};
    uintdup(numbers, numb4, 3);

    TEST_ASSERT_EQUAL(1, fieldCandidatesAreSubsetOf(&field, numbers));

    // for solved cells always return false
    unsigned numb5[2] = {5, 0};
    uintdup(numbers, numb5, 2);
    setValue(&field, 1);

    TEST_ASSERT_EQUAL(0, fieldCandidatesAreSubsetOf(&field, numbers));

    free(numbers);
    free(candidates);
}

void test_findNakedTuplesInContainer(void) {
    Field *field;
    FieldsVector *container;

    container = (Field **) xmalloc(sizeof (Field *) * MAX_NUMBER);

    for (int i = 0; i < MAX_NUMBER; i++) {
        field = (Field *) xmalloc(sizeof (Field));
        field->name = strdup("XX");

        container[i] = field;

        unsigned cand1[9] = {0, 2, 0, 4, 5, 6, 7, 8, 0};
        setCandidates(field, cand1);
    }

    // let 2 fields contain a naked tuple: 7, 8
    for (int i = 6; i <= 7; i++) {
        field = container[i];
        field->name = strdup("NA");

        unsigned cand1[9] = {0, 0, 0, 0, 0, 0, 7, 8, 0};
        setCandidates(field, cand1);
    }

    TEST_ASSERT_EQUAL(1, findNakedTuplesInContainer(container, 2));
}

#endif

void test_findNakedTuplesInContainer2(void) {
    Field *field;
    Container *container;
    char buffer[20];

    container = (Container *) xmalloc(sizeof (Container));
    container->name = strdup("box 1");
    container->fields = (Field **) xmalloc(sizeof (Field *) * MAX_NUMBER);
    
    for (int i = 0; i < MAX_NUMBER; i++) {
        field = (Field *) xmalloc(sizeof (Field));
        sprintf(buffer, "%c%u", (char) (i % 3), i / 3);
        strcpy(field->name, buffer);

        // unsolved fields
        if (i == 1) {
            // unsolved fields
            unsigned cand1[9] = {0, 2, 0, 0, 5, 0, 0, 8, 0};
            setCandidates(field, cand1);
        }
        if (i == 5) {
            // unsolved fields
            unsigned cand2[9] = {0, 2, 0, 0, 5, 0, 0, 0, 0};
            setCandidates(field, cand2);
        }
        if (i == 6) {
            // unsolved fields
            unsigned cand3[9] = {0, 0, 3, 0, 0, 0, 0, 8, 0};
            setCandidates(field, cand3);
        }
        if (i == 8) {
            // unsolved fields
            unsigned cand4[9] = {0, 0, 3, 0, 0, 0, 0, 8, 0};
            setCandidates(field, cand4);
        }

        // already solved fields
        if (i == 0) {
            field->value = 4;
            field->candidatesLeft = 0;
        }
        if (i == 2) {
            field->value = 9;
            field->candidatesLeft = 0;
        }
        if (i == 3) {
            field->value = 6;
            field->candidatesLeft = 0;
        }
        if (i == 4) {
            field->value = 1;
            field->candidatesLeft = 0;
        }
        if (i == 7) {
            field->value = 7;
            field->candidatesLeft = 0;
        }

        container->fields[i] = field;

        unsigned cand1[9] = {0, 2, 0, 4, 5, 6, 7, 8, 0};
        setCandidates(field, cand1);
    }

    TEST_ASSERT_EQUAL(1, findNakedTuplesInContainer(container, 2));
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

void test_setupGrid(void) {

    setupGrid();

    // test containers
    TEST_ASSERT_EQUAL(0, strcmp(allContainers[0].name, "row A"));
    TEST_ASSERT_EQUAL(0, strcmp(allContainers[1].name, "row B"));
    TEST_ASSERT_EQUAL(0, strcmp(allContainers[8].name, "row I"));

    TEST_ASSERT_EQUAL(0, strcmp(allContainers[9 + 0].name, "column 1"));
    TEST_ASSERT_EQUAL(0, strcmp(allContainers[9 + 1].name, "column 2"));
    TEST_ASSERT_EQUAL(0, strcmp(allContainers[9 + 8].name, "column 9"));

    TEST_ASSERT_EQUAL(0, strcmp(allContainers[18 + 0].name, "box 1"));
    TEST_ASSERT_EQUAL(0, strcmp(allContainers[18 + 1].name, "box 2"));
    TEST_ASSERT_EQUAL(0, strcmp(allContainers[18 + 8].name, "box 9"));
}

int main(void) {
    UNITY_BEGIN();
    //RUN_TEST(test_Dummy);
    //RUN_TEST(test_Dummy2);
//    RUN_TEST(test_fieldCandidatesContainAllOf);
//    RUN_TEST(test_fieldCandidatesAreSubsetOf);
    RUN_TEST(test_equalNumberOfFieldsAndCandidates);
    //    RUN_TEST(test_findNakedTuplesInContainer);
    RUN_TEST(test_findNakedTuplesInContainer2);
//    RUN_TEST(test_setupGrid);
    return UNITY_END();
}
