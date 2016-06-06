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
#include "log.h"

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

/**
 * creates a Field object with the given field name and the given candidates
 * 
 * @param name name of the field, e.g. "A2"
 * @param value if not 0, the field is solved and the candidates will be ignored
 * @param candidates
 * @return the created field
 */
Field *createField(char *name, unsigned value, unsigned *candidates) {
    Field *field;

    field = (Field *) xmalloc(sizeof (Field));
    strcpy(field->name, name);

    if (value) {
        field->value = value;
        field->candidatesLeft = 0;
    } else {
        setCandidates(field, candidates);
    }

    return field;
}

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

    sprintf(buffer, "numbers: %u-%u-%u", numbers[0], numbers[1], numbers[2]);
    logVerbose(buffer);
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

void test_findNakedPairsInContainer(void) {
    Container *container;
    int dimension;

    container = (Container *) xmalloc(sizeof (Container));
    container->name = strdup("row 3");
    container->fields = (Field **) xmalloc(sizeof (Field *) * MAX_NUMBER);

    for (int i = 0; i < MAX_NUMBER; i++) {
        unsigned cand1[9] = {0, 2, 0, 4, 5, 6, 7, 8, 0};
        char name[5];
        sprintf(name, "XX%u", i);
        container->fields[i] = createField(name, 0, cand1);
    }

    // let 2 fields contain a naked tuple: 7, 8
    for (int i = 6; i <= 7; i++) {
        unsigned cand1[9] = {0, 0, 0, 0, 0, 0, 7, 8, 0};
        char name[5];
        sprintf(name, "NA%u", i);
        container->fields[i] = createField(name, 0, cand1);
    }

    // allocate memory for strategy variables
    dimension = 2;
    unsigned *numbers = (unsigned *) xmalloc(sizeof (unsigned) * (dimension + 1));
    FieldsVector *foundFields = (FieldsVector *) xmalloc(sizeof (FieldsVector) * (dimension + 1));

    TEST_ASSERT_EQUAL(1, findNakedTuplesInContainer(container, dimension));

    free(foundFields);
    free(numbers);
}

#ifdef BUGGY_TRY_TO_REVIVE_TESTS

#endif

void test_findNakedPairsInContainer2(void) {
    Field *field;
    Container *container;
    char name[20];
    int dimension;

    container = (Container *) xmalloc(sizeof (Container));
    container->name = strdup("box 1");
    container->fields = (Field **) xmalloc(sizeof (Field *) * MAX_NUMBER);

    for (int i = 0; i < MAX_NUMBER; i++) {
        field = (Field *) xmalloc(sizeof (Field));
        sprintf(name, "%c%u", (char) (i % 3), i / 3);

        // unsolved fields
        if (i == 1) {
            unsigned cand1[9] = {0, 2, 0, 0, 5, 0, 0, 8, 0};
            field = createField(name, 0, cand1);
        }
        if (i == 5) {
            unsigned cand2[9] = {0, 2, 0, 0, 5, 0, 0, 0, 0};
            field = createField(name, 0, cand2);
        }
        if (i == 6) {
            unsigned cand3[9] = {0, 0, 3, 0, 0, 0, 0, 8, 0};
            field = createField(name, 0, cand3);
        }
        if (i == 8) {
            unsigned cand4[9] = {0, 0, 3, 0, 0, 0, 0, 8, 0};
            field = createField(name, 0, cand4);
        }

        // already solved fields
        if (i == 0) {
            field = createField(name, 4, NULL);
        }
        if (i == 2) {
            field = createField(name, 9, NULL);
        }
        if (i == 3) {
            field = createField(name, 6, NULL);
        }
        if (i == 4) {
            field = createField(name, 1, NULL);
        }
        if (i == 7) {
            field = createField(name, 7, NULL);
        }

        container->fields[i] = field;
    }

    // allocate memory for strategy variables
    dimension = 2;
    unsigned *numbers = (unsigned *) xmalloc(sizeof (unsigned) * (dimension + 1));
    FieldsVector *foundFields = (FieldsVector *) xmalloc(sizeof (FieldsVector) * (dimension + 1));

    TEST_ASSERT_EQUAL(1, findNakedTuplesInContainer(container, dimension));

    free(foundFields);
    free(numbers);
}

void test_findNakedPairsInContainer4(void) {
    Container *container;
    int dimension;

    container = (Container *) xmalloc(sizeof (Container));
    container->name = strdup("row 4");
    container->fields = (Field **) xmalloc(sizeof (Field *) * MAX_NUMBER);

    // no naked tuples
    for (int i = 0; i < MAX_NUMBER; i++) {
        unsigned cand1[9] = {0, 2, 0, 4, 5, 6, 7, 8, 0};
        char name[5];
        sprintf(name, "XX%u", i);
        container->fields[i] = createField(name, 0, cand1);
    }

    // allocate memory for strategy variables
    dimension = 2;
    unsigned *numbers = (unsigned *) xmalloc(sizeof (unsigned) * (dimension + 1));
    FieldsVector *foundFields = (FieldsVector *) xmalloc(sizeof (FieldsVector) * (dimension + 1));

    TEST_ASSERT_EQUAL(0, findNakedTuplesInContainer(container, dimension));

    free(foundFields);
    free(numbers);
}

void test_findNakedTriplesInContainer(void) {
    Container *container;
    int dimension;

    container = (Container *) xmalloc(sizeof (Container));
    container->name = strdup("row 5");
    container->fields = (Field **) xmalloc(sizeof (Field *) * MAX_NUMBER);

    for (int i = 0; i < MAX_NUMBER; i++) {
        unsigned cand1[9] = {0, 2, 0, 4, 5, 6, 7, 8, 0};
        char name[5];
        sprintf(name, "XX%u", i);
        container->fields[i] = createField(name, 0, cand1);
    }

    // let 2 fields contain a naked tuple: 7, 8
    for (int i = 2; i <= 4; i++) {
        unsigned cand1[9] = {0, 0, 0, 0, 0, 6, 7, 8, 0};
        char name[5];
        sprintf(name, "NA%u", i);
        container->fields[i] = createField(name, 0, cand1);
    }

    // allocate memory for strategy variables
    dimension = 3;
    unsigned *numbers = (unsigned *) xmalloc(sizeof (unsigned) * (dimension + 1));
    FieldsVector *foundFields = (FieldsVector *) xmalloc(sizeof (FieldsVector) * (dimension + 1));

    // no pairs
    TEST_ASSERT_EQUAL(0, findNakedTuplesInContainer(container, 2));

    // but a triple!
    TEST_ASSERT_EQUAL(1, findNakedTuplesInContainer(container, 3));

    free(foundFields);
    free(numbers);
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
    RUN_TEST(test_fieldCandidatesContainAllOf);
    RUN_TEST(test_fieldCandidatesAreSubsetOf);
    RUN_TEST(test_equalNumberOfFieldsAndCandidates);
    RUN_TEST(test_findNakedPairsInContainer);
    RUN_TEST(test_findNakedPairsInContainer2);
    RUN_TEST(test_findNakedPairsInContainer4);
    RUN_TEST(test_findNakedTriplesInContainer);
    //        RUN_TEST(test_setupGrid);
    return UNITY_END();
}
