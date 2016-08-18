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
#include <assert.h>
#include "grid.h"
#include "util.h"
#include "logfile.h"

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

    assert(array != NULL);

    ptr = array;
    i = 0;
    while (*ptr++) i++;

    return i;
}

/**
 * append a Field reference to a NULL-terminated list of references to Fields.
 * If the list of field references already contains the field to be added,
 * nothing happens (the new field will not be added a second time).
 * 
 * @param fields already existing list of field references
 * @param newField the field reference to be added to the end of the list
 */
void appendField(Field **fields, Field *newField) {
    Field **ptr;
    int count;

    assert(fields != NULL);
    assert(newField != NULL);

    ptr = fields;

    // search end of current list
    count = 0;
    while (*ptr) {
        count++;
        sprintf(buffer, "inc counter to %d", count);
        logVerbose(buffer);

        // break if field is already in field list
        if (*ptr == newField) {
            return;
        }

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

    assert(*list != NULL);
    assert(field != NULL);

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

    assert(field != NULL);

    sprintf(buffer, "field to be checked in fieldCandidatesAreSubsetOf: %s", field->name);
    logVerbose(buffer);

    if (field->value) {
        // already solved => nothing to do with the candidates
        return 0;
    }

    for (int i = 0; i < maxNumber; i++) {
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
                return 0;
            }
        }
    }
    return 1;
}

/**
 * count the total number of distinct candidates of all given fields and checks
 * whether the total sum lower than or equal to the given limit.
 * 
 * @param fields list of fields of which the candidates should be counted
 * @param limit the maximum number of total distinct candidates
 */
int countDistinctCandidates(FieldsVector *fields, size_t limit) {
    size_t count;
    unsigned *candidatesSet;

    // TODO could already be pre-allocated by the strategy (performance optimisation))
    // ... a strategy-local buffer so to speak ...
    candidatesSet = (unsigned *) xmalloc(sizeof (unsigned) * maxNumber);

    // initialize all candidate counters with 0    
    for (int i = 0; i < maxNumber; i++) {
        candidatesSet[i] = 0;
    }

    count = 0;
    while (*fields) {
        for (int i = 0; i < maxNumber; i++) {
            unsigned candidate;

            candidate = (*fields)->candidates[i];

            if (candidate) {
                if (!candidatesSet[i]) {
                    if (count >= limit) {
                        // too many candidates (adding this candidates to the
                        // set would exceed the given limit)
                        return 0;
                    }

                    candidatesSet[i] = candidate;
                    count++;
                }
            }

        }
        fields++;
    }

    // no break until now => must be success
    return 1;
}

/**
 * determines in which fields of a given list of fields a given candidate can
 * be. 
 * 
 * @param fieldsWithCandidate fields containing a candidate. This is a 
 *   pre-allocated buffer to be used by this strategy. Performance issue, so 
 *   that not every iteration has to allocate buffer, but a "common" buffer is
 *   used
 * @param fields fields in which to search for the given candidate, must be
 *   maxNumber of entries
 * @param candidate the candidate to look for
 * @return a NULL-terminated list of field pointers holding all fields of the
 *   given field list for which the given candidate is possible
 */
FieldsVector *fieldsContainingCandidate(FieldsVector *fieldsWithCandidate, FieldsVector *fields, unsigned candidate) {
    FieldsVector *fieldsPtr;

    fieldsPtr = fieldsWithCandidate;
    while (*fields) {
        if ((*fields)->value == candidate) {
            // field is already solved with this candidate, so the candidate
            // is not "free" anymore and cannot occur in any fields of the 
            // container => look no further
            break;
        }

        if ((*fields)->candidates[candidate - 1]) {
            // add field to list of found fields
            *fieldsPtr++ = *fields;
        }

        fields++;
    }

    *fieldsPtr = NULL; // terminate list of found fields
    return fieldsWithCandidate;
}

/**
 * check if all given fields share the same container of the given type.
 * For a tuple of fields which share more than container of the given type, only
 * the first found container instance is returned. I cannot think of a case like
 * this. A single field can be member of two different container instances of
 * the same type (e.g. the center field is part of both diagonal containers),
 * but I don't see a case where *mulitple* fields share more than one common
 * containers. So this function doesn't take such a case into account and simply
 * returns the first found container.
 * 
 * @param fields NULL-terminated vector of field pointers specifying the fields
 *   which should be checked whether they *all* share the same container
 * @param containerSetIndex the index of the container set for which the 
 *   function should check whether all fields are in the same container of this
 *   container set
 * @return the common container or NULL if fields have no common container. If
 *   an empty field list is passed, the return value is NULL as well since no
 *   container can be determined at all.
 */
Container *getCommonContainer(FieldsVector *fields, size_t containerSetIndex) {
    Container *commonContainer;
    unsigned containerInstanceIndex;
    unsigned compareContainerInstanceIndex;
    FieldsVector *fieldsPtr;
    unsigned shared;
    Container *compareContainer;

    if (*fields == NULL) {
        // not even one field given => no common container
        return NULL;
    }

    /*
     * loop through all containers (of the given type) of the first field.
     * This is a loop because e.g. in the special case of a center field being
     * member of both diagonal containers, a field could be member of more than
     * one container of the same type.
     * So, for each of the containers (of same type) of the first field, check
     * if all other fields also *contain* this container in *their* potential
     * list of containers
     */
    for (commonContainer = NULL, containerInstanceIndex = 0;; containerInstanceIndex++) {
        // take the container #containerInstanceIndex of the first field as
        // a reference. Do all other fields share this container?
        commonContainer = (*fields)->containers[containerSetIndex][containerInstanceIndex];
        if (commonContainer == NULL) {
            // reached end of container list
            break;
        }

        // check the remaining fields if they share the same container
        for (fieldsPtr = fields + 1; *fieldsPtr != NULL; fieldsPtr++) {

            // loop through all container instances of this type of this field
            for (compareContainerInstanceIndex = 0;; compareContainerInstanceIndex++) {
                compareContainer = (*fieldsPtr)->containers[containerSetIndex][compareContainerInstanceIndex];
                if (compareContainer == NULL) {
                    // reached end of container list
                    shared = 0; // this field does not reference the commonContainer
                    break;
                }

                if (compareContainer == commonContainer) {
                    // found the same container in the comparison field
                    shared = 1;
                    break;
                }
            }

            if (!shared) {
                commonContainer = NULL;
                break;
            }
        }
        
        if (commonContainer != NULL) {
            // found a common container
            break;
        }
    }

    return commonContainer;
}
