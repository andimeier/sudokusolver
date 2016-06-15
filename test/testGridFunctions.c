/* 
 * File:   UnitTest.c
 * Author: alex
 *
 * Created on 26. April 2016, 20:27
 */

#include <stdlib.h>
#include <memory.h>
#include <time.h>
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
 * compares whether a field has the given value or candidates.
 * If value is given, the candidates are not checked.
 * If no value is given, the candidates are compared.
 * 
 * @param field
 * @param candidatesString C string of candidates, e.g. "146"
 * @return 1 if the field's value resp. its candidates are the same, 0 if not
 */
int compareField(Field *field, unsigned value, char *candidatesString) {
    if (field->value) {
        return field->value == value;
    }

    unsigned *candidates = (unsigned *) xmalloc(sizeof (unsigned) * MAX_NUMBER);
    for (int i = 0; i < MAX_NUMBER; i++) {
        // initialize with zeros
        candidates[i] = 0;
    }
    // set candidates to be checked
    while (*candidatesString) {
        unsigned candidate = (unsigned) (*candidatesString - '0');
        if (candidate > 0) {
            candidates[candidate - 1] = candidate;
        }
        candidatesString++;
    }

    // compare candidates
    for (int i = 0; i < MAX_NUMBER; i++) {
        if (field->candidates[i] != candidates[i]) {
            return 0;
        }
    }

    // no early exit => candidates are the same
    return 1;
}

/**
 * creates a Field object with the given field name and the given candidates
 * 
 * @param name name of the field, e.g. "A2"
 * @param value if not 0, the field is solved and the candidates will be ignored
 * @param candidatesString C string of candidates, e.g. "146" or "020400089" 
 *   ('0' can be used as a placeholder, will be ignored)
 * @return the created field
 */
Field *createField(char *name, unsigned value, char *candidatesString) {
    Field *field;

    field = (Field *) xmalloc(sizeof (Field));
    strcpy(field->name, name);

    unsigned *candidates = (unsigned *) xmalloc(sizeof (unsigned) * MAX_NUMBER);
    for (int i = 0; i < MAX_NUMBER; i++) {
        // initialize with zeros
        candidates[i] = 0;
    }

    if (value) {
        field->value = value;
        field->candidatesLeft = 0;
    } else {

        // compare candidates
        while (*candidatesString) {
            unsigned candidate = (unsigned) (*candidatesString - '0');
            if (candidate > 0) {
                candidates[candidate - 1] = candidate;
            }
            candidatesString++;
        }

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
    clock_t t;

    container = (Container *) xmalloc(sizeof (Container));
    container->name = strdup("row 3");
    container->fields = (Field **) xmalloc(sizeof (Field *) * MAX_NUMBER);

    for (int i = 0; i < MAX_NUMBER; i++) {
        char name[5];
        sprintf(name, "XX%u", i);
        container->fields[i] = createField(name, 0, "245678");
    }

    // let 2 fields contain a naked tuple: 7, 8
    for (int i = 6; i <= 7; i++) {
        char name[5];
        sprintf(name, "NA%u", i);
        container->fields[i] = createField(name, 0, "78");
    }

    // allocate memory for strategy variables
    dimension = 2;
    unsigned *numbers = (unsigned *) xmalloc(sizeof (unsigned) * (dimension + 1));
    FieldsVector *foundFields = (FieldsVector *) xmalloc(sizeof (FieldsVector) * (MAX_NUMBER + 1));

    t = clock();
    TEST_ASSERT_EQUAL(1, findNakedTuplesInContainer(container, dimension, numbers, foundFields));
    t = clock() - t;

    // check fields' values and candidates
    TEST_ASSERT_EQUAL(1, compareField(container->fields[0], 0, "2456"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[1], 0, "2456"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[2], 0, "2456"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[3], 0, "2456"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[4], 0, "2456"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[5], 0, "2456"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[6], 0, "78"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[7], 0, "78"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[8], 0, "2456"));

    double elapsedTime = ((double) t) / CLOCKS_PER_SEC; // in seconds
    sprintf(buffer, "Elapsed time: %5.2f seconds", elapsedTime);
    logVerbose(buffer);

    free(foundFields);
    free(numbers);
}

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
            field = createField(name, 0, "258");
        }
        if (i == 5) {
            field = createField(name, 0, "25");
        }
        if (i == 6) {
            field = createField(name, 0, "38");
        }
        if (i == 8) {
            field = createField(name, 0, "38");
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
    FieldsVector *foundFields = (FieldsVector *) xmalloc(sizeof (FieldsVector) * (MAX_NUMBER + 1));

    TEST_ASSERT_EQUAL(1, findNakedTuplesInContainer(container, dimension, numbers, foundFields));

    // check fields' values and candidates
    TEST_ASSERT_EQUAL(1, compareField(container->fields[0], 4, NULL));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[1], 0, "25"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[2], 9, NULL));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[3], 6, NULL));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[4], 1, NULL));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[5], 0, "25"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[6], 0, "38"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[7], 7, NULL));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[8], 0, "38"));

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
        char name[5];
        sprintf(name, "XX%u", i);
        container->fields[i] = createField(name, 0, "245678");
    }

    // allocate memory for strategy variables
    dimension = 2;
    unsigned *numbers = (unsigned *) xmalloc(sizeof (unsigned) * (dimension + 1));
    FieldsVector *foundFields = (FieldsVector *) xmalloc(sizeof (FieldsVector) * (MAX_NUMBER + 1));

    TEST_ASSERT_EQUAL(0, findNakedTuplesInContainer(container, dimension, numbers, foundFields));

    // check fields' values and candidates
    TEST_ASSERT_EQUAL(1, compareField(container->fields[0], 0, "245678"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[1], 0, "245678"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[2], 0, "245678"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[3], 0, "245678"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[4], 0, "245678"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[5], 0, "245678"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[6], 0, "245678"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[7], 0, "245678"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[8], 0, "245678"));

    free(foundFields);
    free(numbers);
}

void test_findNakedPairsInContainer5(void) {
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

        // fields
        if (i == 0) field = createField(name, 0, "4789");
        if (i == 1) field = createField(name, 0, "289");
        if (i == 2) field = createField(name, 0, "789");
        if (i == 3) field = createField(name, 0, "3478");
        if (i == 4) field = createField(name, 0, "238");
        if (i == 5) field = createField(name, 1, NULL);
        if (i == 6) field = createField(name, 5, NULL);
        if (i == 7) field = createField(name, 6, NULL);
        if (i == 8) field = createField(name, 0, "78");

        container->fields[i] = field;
    }

    // allocate memory for strategy variables
    dimension = 3;
    unsigned *numbers = (unsigned *) xmalloc(sizeof (unsigned) * (dimension + 1));
    FieldsVector *foundFields = (FieldsVector *) xmalloc(sizeof (FieldsVector) * (MAX_NUMBER + 1));

    TEST_ASSERT_EQUAL(0, findNakedTuplesInContainer(container, 3, numbers, foundFields));

    // check fields' values and candidates, should be unchanged
    TEST_ASSERT_EQUAL(1, compareField(container->fields[0], 0, "4789"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[1], 0, "289"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[2], 0, "789"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[3], 0, "3478"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[4], 0, "238"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[5], 1, NULL));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[6], 5, NULL));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[7], 6, NULL));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[8], 0, "78"));

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
        char name[5];
        sprintf(name, "XX%u", i);
        container->fields[i] = createField(name, 0, "245678");
    }

    // let 3 fields contain a naked triple: 6, 7, 8
    for (int i = 2; i <= 4; i++) {
        char name[5];
        sprintf(name, "NA%u", i);
        container->fields[i] = createField(name, 0, "678");
    }

    // allocate memory for strategy variables
    dimension = 3;
    unsigned *numbers = (unsigned *) xmalloc(sizeof (unsigned) * (dimension + 1));
    FieldsVector *foundFields = (FieldsVector *) xmalloc(sizeof (FieldsVector) * (MAX_NUMBER + 1));

    // no pairs
    TEST_ASSERT_EQUAL(0, findNakedTuplesInContainer(container, 2, numbers, foundFields));

    // but a triple!
    TEST_ASSERT_EQUAL(1, findNakedTuplesInContainer(container, 3, numbers, foundFields));

    // check fields' values and candidates
    TEST_ASSERT_EQUAL(1, compareField(container->fields[0], 0, "245"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[1], 0, "245"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[2], 0, "678"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[3], 0, "678"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[4], 0, "678"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[5], 0, "245"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[6], 0, "245"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[7], 0, "245"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[8], 0, "245"));

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

test_showCandidates() {
    //    Field *field;
    //    unsigned *candidates;
    //
    //    unsigned cand1[9] = {0, 2, 0, 4, 5, 6, 0, 0, 0};
    //    uintdup(candidates, cand1, 9);
    //    
    //    field = createField(strdup("field A", 0, );
    //
    //    showCandidates(field);
    //
    //    // FIXME how to test stdout output?
    //    
    //    TEST_ASSERT_EQUAL(0, strcmp(allContainers[18 + 0].name, "box 1"));
    //    TEST_ASSERT_EQUAL(0, strcmp(allContainers[18 + 1].name, "box 2"));
    //    TEST_ASSERT_EQUAL(0, strcmp(allContainers[18 + 8].name, "box 9"));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_fieldCandidatesContainAllOf);
    RUN_TEST(test_fieldCandidatesAreSubsetOf);
    RUN_TEST(test_equalNumberOfFieldsAndCandidates);
    RUN_TEST(test_findNakedPairsInContainer);
    RUN_TEST(test_findNakedPairsInContainer2);
    RUN_TEST(test_findNakedPairsInContainer4);
    RUN_TEST(test_findNakedPairsInContainer5);
    RUN_TEST(test_findNakedTriplesInContainer);
    RUN_TEST(test_showCandidates);
    //        RUN_TEST(test_setupGrid);
    return UNITY_END();
}
