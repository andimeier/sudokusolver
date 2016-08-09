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
#include "solve.h"
#include "util.h"
#include "grid.h"
#include "typedefs.h"
#include "fieldlist.h"
#include "numberlist.h"
#include "logfile.h"
#include "diagonal.h"

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

    candidates = (unsigned *) xmalloc(sizeof (unsigned) * maxNumber);
    uintdup(candidates, cands, 9);
    field->candidates = candidates;
    field->value = 0; // not solved yet

    field->candidatesLeft = 0;
    for (int i = 0; i < maxNumber; i++) {
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
    } else {
        if (value) {
            // should have a value, but is unsolved
            return 0;
        }
    }


    unsigned *candidates = (unsigned *) xmalloc(sizeof (unsigned) * maxNumber);
    for (int i = 0; i < maxNumber; i++) {
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
    for (int i = 0; i < maxNumber; i++) {
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

    unsigned *candidates = (unsigned *) xmalloc(sizeof (unsigned) * maxNumber);
    for (int i = 0; i < maxNumber; i++) {
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

void test_countDistinctCandidates(void) {
    FieldsVector fields[3];

    fields[0] = createField("E1", 0, "78");
    fields[1] = createField("E2", 0, "79");
    fields[2] = NULL;

    TEST_ASSERT_EQUAL(0, countDistinctCandidates(fields, 2));
    TEST_ASSERT_EQUAL(1, countDistinctCandidates(fields, 3));

    fields[0] = createField("F1", 0, "12378");
    fields[1] = createField("F2", 0, "79");
    fields[2] = NULL;

    TEST_ASSERT_EQUAL(0, countDistinctCandidates(fields, 2));
    TEST_ASSERT_EQUAL(0, countDistinctCandidates(fields, 3));
    TEST_ASSERT_EQUAL(0, countDistinctCandidates(fields, 4));
    TEST_ASSERT_EQUAL(0, countDistinctCandidates(fields, 5));
    TEST_ASSERT_EQUAL(1, countDistinctCandidates(fields, 6));
    TEST_ASSERT_EQUAL(1, countDistinctCandidates(fields, 7));

    fields[0] = createField("G1", 0, "12");
    fields[1] = createField("G2", 0, "12");
    fields[2] = NULL;

    TEST_ASSERT_EQUAL(0, countDistinctCandidates(fields, 1));
    TEST_ASSERT_EQUAL(1, countDistinctCandidates(fields, 2));
    TEST_ASSERT_EQUAL(1, countDistinctCandidates(fields, 3));
}

void test_findNakedPairsInContainer(void) {
    Container *container;
    int dimension;
    clock_t t;

    container = (Container *) xmalloc(sizeof (Container));
    container->name = strdup("row 3");
    container->fields = (Field **) xmalloc(sizeof (Field *) * maxNumber);

    for (int i = 0; i < maxNumber; i++) {
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

    FieldList *includedFields = createFieldList(dimension);
    FieldsVector *fieldsLeft = (FieldsVector *) xmalloc(sizeof (Field *) * (maxNumber + 1));

    t = clock();
    TEST_ASSERT_EQUAL(1, findNakedTuplesInContainer(container, dimension, includedFields, fieldsLeft));
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

    free(fieldsLeft);
    freeFieldList(includedFields);
}

void test_findNakedPairsInContainer2(void) {
    Field *field;
    Container *container;
    char name[20];
    int dimension;

    container = (Container *) xmalloc(sizeof (Container));
    container->name = strdup("box 1");
    container->fields = (Field **) xmalloc(sizeof (Field *) * maxNumber);

    for (int i = 0; i < maxNumber; i++) {
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
    FieldList *includedFields = createFieldList(dimension);
    FieldsVector *fieldsLeft = (FieldsVector *) xmalloc(sizeof (Field *) * (maxNumber + 1));

    TEST_ASSERT_EQUAL(1, findNakedTuplesInContainer(container, dimension, includedFields, fieldsLeft));

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

    free(fieldsLeft);
    freeFieldList(includedFields);
}

void test_findNakedPairsInContainer4(void) {
    Container *container;
    int dimension;

    container = (Container *) xmalloc(sizeof (Container));
    container->name = strdup("row 4");
    container->fields = (Field **) xmalloc(sizeof (Field *) * maxNumber);

    // no naked tuples
    for (int i = 0; i < maxNumber; i++) {
        char name[5];
        sprintf(name, "XX%u", i);
        container->fields[i] = createField(name, 0, "245678");
    }

    // allocate memory for strategy variables
    dimension = 2;
    FieldList *includedFields = createFieldList(dimension);
    FieldsVector *fieldsLeft = (FieldsVector *) xmalloc(sizeof (Field *) * (maxNumber + 1));

    TEST_ASSERT_EQUAL(0, findNakedTuplesInContainer(container, dimension, includedFields, fieldsLeft));

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

    free(fieldsLeft);
    freeFieldList(includedFields);
}

void test_findNakedPairsInContainer5(void) {
    Field *field;
    Container *container;
    char name[20];
    int dimension;

    container = (Container *) xmalloc(sizeof (Container));
    container->name = strdup("box 1");
    container->fields = (Field **) xmalloc(sizeof (Field *) * maxNumber);

    for (int i = 0; i < maxNumber; i++) {
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
    FieldList *includedFields = createFieldList(dimension);
    FieldsVector *fieldsLeft = (FieldsVector *) xmalloc(sizeof (Field *) * (maxNumber + 1));

    TEST_ASSERT_EQUAL(0, findNakedTuplesInContainer(container, dimension, includedFields, fieldsLeft));

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

    free(fieldsLeft);
    freeFieldList(includedFields);
}

void test_findNakedTriplesInContainer(void) {
    Container *container;
    int dimension;

    container = (Container *) xmalloc(sizeof (Container));
    container->name = strdup("row 5");
    container->fields = (Field **) xmalloc(sizeof (Field *) * maxNumber);

    for (int i = 0; i < maxNumber; i++) {
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
    FieldList *includedFields = createFieldList(dimension);
    FieldsVector *fieldsLeft = (FieldsVector *) xmalloc(sizeof (Field *) * (maxNumber + 1));

    // no pairs
    TEST_ASSERT_EQUAL(0, findNakedTuplesInContainer(container, 2, includedFields, fieldsLeft));

    // but a triple!
    TEST_ASSERT_EQUAL(1, findNakedTuplesInContainer(container, 3, includedFields, fieldsLeft));

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

    free(fieldsLeft);
    freeFieldList(includedFields);
}

void test_findNakedTriplesInContainer2(void) {
    Container *container;
    int dimension;

    container = (Container *) xmalloc(sizeof (Container));
    container->name = strdup("row C");
    container->fields = (Field **) xmalloc(sizeof (Field *) * maxNumber);

    for (int i = 0; i < maxNumber; i++) {
        char name[5];
        sprintf(name, "C%u", i + 1);

        switch (i) {
            case 0: container->fields[i] = createField(name, 5, NULL);
                break;
            case 1: container->fields[i] = createField(name, 6, NULL);
                break;
            case 2: container->fields[i] = createField(name, 0, "78");
                break;
            case 3: container->fields[i] = createField(name, 3, NULL);
                break;
            case 4: container->fields[i] = createField(name, 1, NULL);
                break;
            case 5: container->fields[i] = createField(name, 0, "27");
                break;
            case 6: container->fields[i] = createField(name, 0, "28");
                break;
            case 7: container->fields[i] = createField(name, 9, NULL);
                break;
            case 8: container->fields[i] = createField(name, 4, NULL);
                break;
                //            case 8: container->fields[i] = createField(name, 0, "245678"); break;
        }
    }

    // allocate memory for strategy variables
    dimension = 3;
    FieldList *includedFields = createFieldList(dimension);
    FieldsVector *fieldsLeft = (FieldsVector *) xmalloc(sizeof (Field *) * (maxNumber + 1));

    // no pairs
    TEST_ASSERT_EQUAL(0, findNakedTuplesInContainer(container, 2, includedFields, fieldsLeft));

    // although there is a naked triple, no candidates can be eliminated in
    // other fields, so the strategy function is supposed to return 0 (no change)
    TEST_ASSERT_EQUAL(0, findNakedTuplesInContainer(container, 3, includedFields, fieldsLeft));

    // check fields' values and candidates
    // all fields are unchanged (naked triple was detected, but it did not
    // change anything)
    TEST_ASSERT_EQUAL(1, compareField(container->fields[0], 5, NULL));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[1], 6, NULL));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[2], 0, "78"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[3], 3, NULL));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[4], 1, NULL));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[5], 0, "27"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[6], 0, "28"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[7], 9, NULL));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[8], 4, NULL));

    free(fieldsLeft);
    freeFieldList(includedFields);
}

void test_equalNumberOfFieldsAndCandidates(void) {
    FieldsVector *fieldsVector;
    unsigned *numbers;
    Field f;

    fieldsVector = (Field **) xmalloc(sizeof (Field *) * maxNumber);
    numbers = (unsigned *) xmalloc(sizeof (unsigned) * maxNumber);

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

void test_showCandidates() {
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

void test_findHiddenPairInContainer(void) {
    Container *container;
    NumberList *includedCandidates;
    unsigned *candidatesLeft;
    FieldsVector *fieldsWithCandidates;

    container = (Container *) xmalloc(sizeof (Container));
    container->name = strdup("row C");
    container->fields = (Field **) xmalloc(sizeof (Field *) * maxNumber);

    for (int i = 0; i < maxNumber; i++) {
        char name[5];
        sprintf(name, "C%u", i + 1);

        switch (i) {
            case 0: container->fields[i] = createField(name, 6, NULL);
                break;
            case 1: container->fields[i] = createField(name, 7, NULL);
                break;
            case 2: container->fields[i] = createField(name, 0, "1245");
                break;
            case 3: container->fields[i] = createField(name, 8, NULL);
                break;
            case 4: container->fields[i] = createField(name, 0, "123");
                break;
            case 5: container->fields[i] = createField(name, 0, "1345");
                break;
            case 6: container->fields[i] = createField(name, 0, "123");
                break;
            case 7: container->fields[i] = createField(name, 9, NULL);
                break;
            case 8: container->fields[i] = createField(name, 0, "123");
                break;
        }
    }

    // allocate memory for strategy variables
    includedCandidates = createNumberList(MAX_TUPLE_DIMENSION);
    candidatesLeft = (unsigned *) xmalloc(sizeof (unsigned) * (maxNumber + 1));
    fieldsWithCandidates = (FieldsVector *) xmalloc(sizeof (Field *) * (maxNumber + 1));

    // no pairs
    TEST_ASSERT_EQUAL(1, findHiddenTuplesInContainer(container, 2, includedCandidates, candidatesLeft, fieldsWithCandidates));

    // check fields' values and candidates
    // hidden triple was detected and all candidates except 4 and 5 should be
    // removed from these fields
    TEST_ASSERT_EQUAL(1, compareField(container->fields[0], 6, NULL));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[1], 7, NULL));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[2], 0, "45"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[3], 8, NULL));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[4], 0, "123"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[5], 0, "45"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[6], 0, "123"));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[7], 9, NULL));
    TEST_ASSERT_EQUAL(1, compareField(container->fields[8], 0, "123"));

    free(includedCandidates);
    free(candidatesLeft);
    freeFieldList(fieldsWithCandidates);
}

void test_determineDiagonalContainer() {
    
    TEST_ASSERT_EQUAL(1, determineDiagonalContainer(0, 8));
    TEST_ASSERT_EQUAL(1, determineDiagonalContainer(1, 7));
    TEST_ASSERT_EQUAL(1, determineDiagonalContainer(2, 6));
    TEST_ASSERT_EQUAL(1, determineDiagonalContainer(3, 5));
    TEST_ASSERT_EQUAL(1, determineDiagonalContainer(4, 4));
    TEST_ASSERT_EQUAL(1, determineDiagonalContainer(5, 3));
    TEST_ASSERT_EQUAL(1, determineDiagonalContainer(6, 2));
    TEST_ASSERT_EQUAL(1, determineDiagonalContainer(7, 1));
    TEST_ASSERT_EQUAL(1, determineDiagonalContainer(8, 0));

    TEST_ASSERT_EQUAL(-1, determineDiagonalContainer(8, 1));
    TEST_ASSERT_EQUAL(-1, determineDiagonalContainer(8, 2));
    TEST_ASSERT_EQUAL(-1, determineDiagonalContainer(0, 1));

    TEST_ASSERT_EQUAL(0, determineDiagonalContainer(0, 0));
    TEST_ASSERT_EQUAL(0, determineDiagonalContainer(1, 1));
    TEST_ASSERT_EQUAL(0, determineDiagonalContainer(2, 2));
    TEST_ASSERT_EQUAL(0, determineDiagonalContainer(3, 3));
    TEST_ASSERT_EQUAL(0, determineDiagonalContainer(4, 4));
    TEST_ASSERT_EQUAL(0, determineDiagonalContainer(5, 5));
    TEST_ASSERT_EQUAL(0, determineDiagonalContainer(6, 6));
    TEST_ASSERT_EQUAL(0, determineDiagonalContainer(7, 7));
    TEST_ASSERT_EQUAL(0, determineDiagonalContainer(8, 8));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_fieldCandidatesContainAllOf);
    RUN_TEST(test_fieldCandidatesAreSubsetOf);
    RUN_TEST(test_equalNumberOfFieldsAndCandidates);
    RUN_TEST(test_countDistinctCandidates);
    RUN_TEST(test_findNakedPairsInContainer);
    RUN_TEST(test_findNakedPairsInContainer2);
    RUN_TEST(test_findNakedPairsInContainer4);
    RUN_TEST(test_findNakedPairsInContainer5);
    RUN_TEST(test_findNakedTriplesInContainer);
    RUN_TEST(test_findNakedTriplesInContainer2);
    RUN_TEST(test_findHiddenPairInContainer);
    RUN_TEST(test_determineDiagonalContainer);
    RUN_TEST(test_showCandidates);
    //        RUN_TEST(test_setupGrid);
    return UNITY_END();
}
