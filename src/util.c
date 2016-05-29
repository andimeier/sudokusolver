/* 
 * File:   util.c
 * Author: aurez
 *
 * Grid- and Sudoku-agnostic utility functions
 * 
 * Created on 04. April 2016, 21:01
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "util.h"

/**
 * copies all numbers from one array to another.
 * Remember to free the allocated memory when you don't need it anymore
 * @param src
 * @param len
 * @return 
 */
int *unsigneddup(unsigned const *src, size_t len) {
    int *p = malloc(len * sizeof (unsigned));
    if (p == NULL) {
        exit(EXIT_FAILURE);
    }
    memcpy(p, src, len * sizeof (unsigned));
    return p;
}

void *xmalloc(size_t size) {
    void *value = malloc(size);
    if (value == NULL) {
        fprintf(stderr, "virtual memory exhausted\n");
        exit(EXIT_FAILURE);
    }

    return value;
}

/**
 * get the length of an array of unsigned ints by counting all non-NULL
 * items until the terminating 0.
 * 
 * @param array array of unsigned values
 * @return the length of the array = the first index of NULL
 */
unsigned ulength(unsigned *array) {
    unsigned i;
    unsigned *ptr;

    ptr = array;
    i = 0;
    while (*ptr++) i++;

    return i;
}

/**
 * append a Field reference to a NULL-terminated list of references to Fields.
 * 
 * @param fields already existing list of field references
 * @param newField the field reference to be added to the end of the list
 */
void appendField(Field **fields, Field *newField) {
    Field **ptr;
    
    ptr = fields;
    
    // search end of current list
    while (*ptr) {
        ptr++;
    }
    
    // extend list
    *ptr++ = newField;
    
    // terminate extended list
    *ptr = NULL;
}

/**
 * checks if the given field is in the specified list of fields
 * 
 * @param list list of Field references, terminated with NULL
 * @param field field to look for
 * @return 1 if the field is in the field list, 0 if it is not
 */
int containsField(Field **list, Field * field) {
    for (int i = 0; list[i] != NULL; i++) {
        if (field == list[i])
            return 1;
    }
    return 0;
}


/**
 * checks if the possible candidates for a field are a subset of the given 
 * numbers. If the field is already solved, returns 0.
 * 
 * @param field pointer to field for which the candidates should be checked
 * @param numbers vector of numbers, terminated with 0
 * @return 1 if the field's candidates are a (strict or non-strict) subset of
 *   the given numbers vector. 0 if they are not or if the field is already
 *   solved.
 */
int fieldCandidatesAreSubsetOf(Field *field, unsigned *numbers) {
    unsigned *numbersPtr;
    int found;

    printlog("[6jj]");
    if (field->value) {
        // already solved => nothing to do with the candidates
        return 0;
    }

    printlog("[6jj-1]");
    for (int i = 0; i < MAX_NUMBER; i++) {
        printlog("[6jj-ii]");
        if (field->candidates[i]) {

            // check if field candidate is in the numbers vector
            found = 0;
            numbersPtr = numbers;
            while (*numbersPtr) {
                if (*numbersPtr == field->candidates[i]) {
                    found = 1;
                    break;
                }
                numbersPtr++;
            }

            if (!found) {
                // found a field candidate which is not in the given list of
                // numbers
                printlog("[6jj-return0]");
                return 0;
            }
        }
    }
    printlog("[6jj-return1]");
    return 1;
}
