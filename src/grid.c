/**
 * File:   grid.c
 * Author: aurez
 *
 * functions for creating the Sudoku grid, releasing (freeing) it. And some
 * utility functions.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "grid.h"
#include "log.h"
#include "util.h"
#include "container.h"
#include "gametype.h"

// function prototypes
void initFields();
void initContainers();
void initGrid();
void freeFields();
void freeContainers();
void freeGrid();


//UnitDefs unitDefs;
Field *fields; // the fields of the game board
Container *allContainers; // all containers of the game board
ContainerSet *containerSets; // all container types (e.g. [row, column, box])
size_t numberOfContainerSets;
size_t numberOfContainers;

void setupGrid() {

    initFields();
    initContainers();
    initGrid();
}

void releaseGrid() {
    freeContainers();
    freeGrid();
    freeFields();
}

/**
 * init empty fields, just the memory for the field and candidates, but
 * with no data, except the field name. This will be done in initGrid().
 */
void initFields() {
    Field *field;
    unsigned *candidates;

    fields = (Field *) xmalloc(sizeof (Field) * NUMBER_OF_FIELDS);

    for (int f = 0; f < NUMBER_OF_FIELDS; f++) {
        field = fields + f;

        field->x = f % MAX_NUMBER;
        field->y = f / MAX_NUMBER;

        // allocate candidates
        candidates = (unsigned *) xmalloc(sizeof (unsigned) * MAX_NUMBER);
        field->candidates = candidates;

        // use the ROWS and COLS coordinates as the "name" of the field
        // reserve space for coordinates up to "Z26" (a theoretical limit of
        // a 26-number-Sudoku)
        sprintf(field->name, "%c%u", (char) (field->y + (int) 'A'), field->x + 1);
    }
    printlog("Finished initializing fields");
}

/**
 * free fields memory
 */
void freeFields() {

    for (int f = 0; f < NUMBER_OF_FIELDS; f++) {
        free(fields[f].candidates);
    }

    free(fields);
}

/**
 * init the container types and containers), but with no link to containing 
 * fields. This will be done later in initGrid().
 */
void initContainers() {
    ContainerSet *containerSetPtr;
    unsigned *containerTypes;

    // assuming a standard Sudoku, 
    // we have 3 types of containers (row, column, box)
    containerTypes = getContainerTypes(GAME_STANDARD_SUDOKU);
    numberOfContainerSets = ulength(containerTypes);

    assert(numberOfContainerSets > 0);

    setupContainerSets();

    containerSets = (ContainerSet *) xmalloc(sizeof (ContainerSet) * (numberOfContainerSets));
    containerSetPtr = containerSets;

    numberOfContainers = 0;
    while (*containerTypes) {
        // set container set (holding no containers yet)
        setContainerSet(containerSetPtr, *containerTypes);
        numberOfContainers += containerSetPtr->numberOfContainers;

        containerSetPtr++;
        containerTypes++;
    }

    assert(numberOfContainers > 0);

    // init and populate "all containers" vector
    allContainers = (Container *) xmalloc(sizeof (Container) * (numberOfContainers));
    Container *containersPtr = allContainers;

    /*
     * go through all container sets and generate all child containers for each
     * container set in turn
     */
    for (unsigned set = 0; set < numberOfContainerSets; set++) {
        ContainerSet *containerSet = &(containerSets[set]);

        // generate the corresponding child containers
        for (unsigned containerIndex = 0; containerIndex < containerSet->numberOfContainers; containerIndex++) {
            containersPtr->name = containerSet->getContainerName(containerIndex);
            containersPtr->type = containerSet->type;
            containersPtr->fields = (Field **) xmalloc(sizeof (Field *) * MAX_NUMBER);

            // link to container set
            containerSet->containers[containerIndex] = containersPtr;

            // FIXME debugging code
            sprintf(buffer, "name of container: %s", containersPtr->name);
            logVerbose(buffer);

            containersPtr++;
        }
    }
}

/**
 * free units memory
 */
void freeContainers() {
    //Container *containerPtr;

    //    for (int i = 0; i < unitDefs.count; i++) {
    //        free(unitDefs.containerTypes[i].name);
    //
    //        containerPtr = &(unitDefs.containerTypes[i].containers);
    //        while (*containerPtr) {
    //            free(containerPtr->name);
    //            free(containerPtr->fields);
    //        }
    //        free(unitDefs.containerTypes[i].containers);
    //    }
    //    free(unitDefs.containerTypes);

    free(allContainers);

}

void initGrid() {
    int x, y;
    Field *field;
    ContainerSet *containerSet;
    Container **containers;
    Container **fieldContainer;

    // init all fields (set all candidates, link to container, etc.)
    for (int f = 0; f < NUMBER_OF_FIELDS; f++) {
        field = fields + f;

        x = field->x;
        y = field->y;

        // allow all candidates for the field
        for (int n = 0; n < MAX_NUMBER; n++) {
            field->candidates[n] = n + 1;
        }

        field->candidatesLeft = MAX_NUMBER;
        field->value = 0;
        field->initialValue = 0;

        int *containerIndexes = (int *) xmalloc(sizeof (int) * numberOfContainerSets);
        int * indexPtr = containerIndexes;

        containers = (Container **) xmalloc(sizeof (Container *) * numberOfContainerSets);
        fieldContainer = containers;

        /*
         * for each field, determine its position in the respective containers,
         * register the link between field and container in the container sets,
         * as well as in the field (back reference to container)
         */
        for (unsigned set = 0; set < numberOfContainerSets; set++) {
            containerSet = containerSets + set;

            // determine container index of field 
            *indexPtr = (containerSet->getContainerIndex)(x, y);

            // add reference to container containing this field
            if (*indexPtr != -1) {
                *fieldContainer = containerSet->containers[*indexPtr];
            } else {
                // no container of this type contains this field
                *fieldContainer = NULL;
            }

            // at the same time, add the field to the container which contains
            // it
            appendField(containerSet->containers[*indexPtr]->fields, field);

            indexPtr++;
            containerSet++;
            fieldContainer++;
        }

        field->containerIndexes = containerIndexes; // FIXME property field.containerIndexes needed at all?
        field->containers = containers;
    }
}

/**
 * frees memory allocated for the grid fields
 */
void freeGrid() {
    for (int f = 0; f < NUMBER_OF_FIELDS; f++) {
        free(fields[f].containerIndexes);
    }
}

/**
 * sets the value of a field and eliminates this number from all candidates
 * of neighboring fields
 * 
 * @param field pointer to the Field structure
 * @param value the number to be set as result field value
 */
void setValue(Field *field, unsigned value) {
    Container *container;
    Field *otherField;
    
    assert(value <= MAX_NUMBER);

    field->value = value;

    // check if the number does not occur in any neighbors in any containers
    for (int containerIndex = 0; containerIndex < numberOfContainerSets; containerIndex++) {
        container = field->containers[containerIndex];
        if (container) {
            for (int pos = 0; pos < MAX_NUMBER; pos++) {
                otherField = container->fields[pos];
                if (otherField != field) {
                    /* 
                     * check that the number to which "our" field should be set
                     * is not an already solved number in the "other" field (in 
                     * the same container, each number must be unique)
                     */
                    assert (otherField->value != value);
                }
            }
        }
    }

    // remove all candidates from this field
    unsigned *candidates = field->candidates;
    for (unsigned n = 1; n <= MAX_NUMBER; n++) {
        candidates[n - 1] = (n == value) ? value : 0;
    }

    forbidNumberInNeighbors(field, value);
}

/**
 * forbids a number in all neighbor fields of the given field. This is used
 * e.g. after setting the value of a field to eliminate this number from all
 * neighbors. The "neighbors" are determined in all containers containing
 * the specified field.
 * 
 * @param field
 * @param n the number to be forbidden in neighboring fields
 */
void forbidNumberInNeighbors(Field *field, unsigned n) {
    Container *container;
    unsigned numbers[2];

    assert(n <= MAX_NUMBER);

    sprintf(buffer, "Forbid number %u in neighbors of field %s ...", n, field->name);
    logVerbose(buffer);

    // go through all positions (numbers) of the container and 
    // forbid this number in all other fields of the container

    // build tuple to search for (just because forbidNumbersInOtherFields
    // wants a vector, not a single number)
    numbers[0] = n;
    numbers[1] = 0; // terminate vector

    // preserve candidate in "our" field only
    Field * preserve[2];
    preserve[0] = field;
    preserve[1] = NULL;

    // forbid number in all other "neighboring fields"
    for (unsigned containerType = 0; containerType < numberOfContainerSets; containerType++) {
        container = field->containers[containerType];

        forbidNumbersInOtherFields(container, numbers, preserve);
    }
}

/**
 * eliminates candidates in all fields of a container - except in the specified
 * fields. This it typically needed after identifying a naked pair, where the
 * candidates of the naked pair can be removed from all other fields in the 
 * same container.
 *  
 * @param container container in which candidates should be eliminated
 * @param n ... tuple of candidates to be removed from "other fields". This
 *   is a vector of numbers, terminated with 0, e.g. an array of 
 *   [ 2, 5, 6, 0 ] means that the candidates 2, 5 and 6 shall be removed from 
 *   all "other fields" (fields in the container other than those in parameter 
 *   dontTouch)
 * @param dontTouch ... NULL terminated list of Field pointers. These fields
 *   will not be touched. In all other fields in the container, the given 
 *   numbers will be removed as candidates
 * @return progress flag: 1 if something has changed, 0 if nothing has changed
 */
int forbidNumbersInOtherFields(Container *container, unsigned *n, Field **dontTouch) {
    int progress;
    Field *field;

    //    showAllCandidates();

    progress = 0; // nothing has changed yet

    // walk through entire container
    for (int pos = 0; pos < MAX_NUMBER; pos++) {
        field = container->fields[pos];

        // don't touch the 'dontTouch' fields
        if (!containsField(dontTouch, field)) {
            // forbid the tuple numbers
            for (int i = 0; i < MAX_NUMBER; i++) {
                if (n[i]) {
                    // was a candidate until now => remove candidate now
                    if (!field->value && field->candidates[i]) {
                        sprintf(buffer, "forbid %u in field %s", i + 1, field->name);
                        logReduction(buffer);

                        field->candidates[i] = 0;
                        field->candidatesLeft--;

                        assert(field->candidatesLeft > 0);

                        progress = 1;
                    }
                }
            }
        }
    }

    //    showAllCandidates();
    return progress;
}

//-------------------------------------------------------------------
// Verbiete eine Zahl in einer bestimmten Zelle
// Return-Wert:
//   1 ... Nummer wurde verboten
//   0 ... keine Aenderung, Nummer war bereits verboten

int forbidNumber(Field *field, unsigned n) {

    assert(n >= 1 && n <= MAX_NUMBER);

    if (field->candidates[n - 1]) {
        field->candidates[n - 1] = 0;
        field->candidatesLeft--;
        assert(field->candidatesLeft > 0);

        if (field->candidatesLeft == 1) {
            // nur noch eine einzige Zahl ist moeglich => ausfuellen!
            setUniqueNumber(field);
        }
        return 1;
    }
    return 0;
}

/**
 * checks if the field is unresolved and the given number is a possible 
 * candidate
 * 
 * @param field the field to check
 * @param n the number to check
 * @return 1 if the number is a possible candidate, 0 if it is not
 */
int fieldHasCandidate(Field *field, unsigned n) {
    return !field->value && (field->candidates[n - 1] == n);
}


//-------------------------------------------------------------------
// Setzt in dem Feld #f (das nur mehr eine Moeglichkeit aufweisen muss)
// die einzige Zahl, die in den Candidates gefunden wird.
// Es muss sichergestellt sein, dass nur mehr eine Zahl moeglich ist,
// hier wird das nicht mehr ueberprueft - der erste Candidate wird als 
// "einzig moegliche Zahl" behandelt.
// Return-Wert:
//   die fixierte Zahl

int setUniqueNumber(Field *field) {
    unsigned n;

    // field should not be solved already
    assert(!field->value);

    unsigned *candidates = field->candidates;
    for (n = 1; n <= MAX_NUMBER; n++) {
        if (candidates[n - 1]) {
            setValue(field, n);
            break;
        }
    }

    return n;
}

/**
 * collects all unresolved fields in which the given number is a possible 
 * candidate
 * 
 * @param container
 * @param n the number to look for as a candidate
 * @return vector of fields containing the given number as possible candidate
 */
FieldsVector *fieldsWithCandidate(Field **container, unsigned n) {
    int ix;
    Field *field;
    FieldsVector *found;
    FieldsVector *foundPtr;

    found = (FieldsVector *) xmalloc(sizeof (Field *) * (MAX_NUMBER + 1));

    foundPtr = found;
    for (ix = 0; ix < MAX_NUMBER; ix++) {
        field = *(container + ix);
        if (fieldHasCandidate(field, n)) {
            *foundPtr = field;
            foundPtr++;
        }
    }

    // terminate vector
    foundPtr = NULL;

    return found;
}

/**
 * checks if the number of fields in the FieldsVector and the number of numbers
 * in the numbers vector are equal.
 * 
 * @param fieldsVector vector or pointers to Fields, terminated with a NULL 
 *   pointer
 * @param numbers vector of numbers, terminated with 0
 * @return 1 both have equal length. 0 if they differ
 */
unsigned equalNumberOfFieldsAndCandidates(FieldsVector *fieldsVector, unsigned *numbers) {

    sprintf(buffer, "check if found fields are of tuple dimension of numbers ...");
    logVerbose(buffer);

    do {
        if (*fieldsVector == NULL && *numbers == 0) {
            logVerbose("YES!");
            return 1;
        }

        // if we are still here, then at least one of the vectors is not null.
        // However, if the other one is exhausted, then both vectors apparently
        // do not have the same length
        if (*fieldsVector == NULL || *numbers == 0) {
            logVerbose("   no ...");
            return 0;
        }

        fieldsVector++;
        numbers++;
    } while (1);
}


//-------------------------------------------------------------------
// Checkt die Anzahl der moeglichen Vorkommnisse einer Zahl in der
// Unit u.
// Liefert:
//   x ... x-Position des Feldes, in dem die Zahl n als einziges Feld
//         der ganzen Reihe vorkommen koennte oder
//   -1 ... Zahl koennte in der Zeile an mehreren Positionen vorkommen

int getUniquePositionInContainer(Field **container, unsigned n) {
    int pos;
    int unique;
    int foundPos;
    Field *field;

    assert(n >= 1 && n <= MAX_NUMBER);
    sprintf(buffer, "Looking for unique position of %u in container ...", n);
    logVerbose(buffer);

    for (pos = 0; pos < MAX_NUMBER; pos++) { // FIXME debugging output
        showCandidates(container[pos]);
    }

    unique = 0;
    foundPos = 0;
    for (pos = 0; pos < MAX_NUMBER; pos++) {
        field = container[pos];
        if ((field->value == n) || (!(field->value) && (field->candidates[n - 1] == n))) {
            sprintf(buffer, "Field %s can contain candidate %u", field->name, n);
            logVerbose(buffer);

            if (!unique) {
                unique = 1; // first occurrence in the current container
                foundPos = pos; // remember position, in case it is the only one
            } else {
                // what a pity, this is the second occurrence of this number in the container
                return -1; // => apparently no hidden single
            }
        }
    }
    if (unique) {
        return foundPos;
    }

    return -1;
}

/**
 * checks if the possible candidates for a field contain all of the given 
 * numbers. If the field is already solved, returns 0.
 * 
 * @param field pointer to field for which the candidates should be checked
 * @param numbers vector of numbers, terminated with 0
 * @return 1 if the field's candidates are a (strict or non-strict) superset of
 *   the given numbers vector. 0 if they are not or if the field is already
 *   solved.
 */
int fieldCandidatesContainAllOf(Field *field, unsigned *numbers) {

    if (field->value) {
        // already solved => nothing to do with the candidates
        return 0;
    }

    while (*numbers) {
        if (!field->candidates[*numbers - 1]) {
            // if any candidate is found which is not in "numbers", the field's
            // candidates are no subset of "numbers"
            sprintf(buffer, "number %u not found in candidates (%u)", *numbers, field->candidates[*numbers - 1]);
            logVerbose(buffer);
            return 0;
        }
        numbers++;
    }

    return 1;
}


//-------------------------------------------------------------------
// Checkt, ob alle Zellen mit einer Zahl befuellt sind, dann sind 
// wir naemlich fertig!
// Return-Wert:
//   1 ... fertig (in jeder Zelle steht eine Zahl)
//   0 ... noch nicht fertig

int isFinished() {
    int f;

    for (f = 0; f < NUMBER_OF_FIELDS; f++) {
        if (!fields[f].value)
            // ein leeres Feld gefunden => wir sind noch nicht fertig!
            return 0;
    }
    return 1;
}

/**
 * set all candidates for all fields initially by eliminating all candidates
 * in the neighborhood of solved fields. This is done for all fields.
 * 
 * The purpose of this function is to determine the candidates for all fields
 * which have an initial value (the fields' values have already been set 
 * before).
 */
void cleanUpCandidates() {
    int f;
    Field *field;

    for (f = 0; f < NUMBER_OF_FIELDS; f++) {
        field = fields + f;

        if (field->value) {
            sprintf(buffer, "Set value of field %s (#%d) to %u", field->name, f, field->value);
            logVerbose(buffer);
            setValue(field, field->value);
        }
    }

    // FIXME debugging output
    logVerbose("Initial candidates are:");
    showAllCandidates();
}
