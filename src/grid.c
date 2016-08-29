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
#include "grid.h"
#include "log.h"
#include "logfile.h"
#include "util.h"
#include "container.h"
#include "gametype.h"

// function prototypes
static void initContainerSets();
static void initFields();
static void initContainers();
static void freeFields();
static void freeContainers();

// static functions
void printLogSetUniqueNumber(void *info);
void printLogRemoveCandidate(void *info);


// grid geometry, initialize with 0 to indicate the state of being not initialized
unsigned sudokuType = STANDARD_SUDOKU; // type of Sudoku (e.g. X_SUDOKU))
size_t maxNumber = 0;
size_t numberOfFields = 0; // == maxNumber^2


// grid variables
Field *fields; // the fields of the game board
Container *allContainers; // all containers of the game board
ContainerSet *containerSets; // all container types (e.g. [row, column, box])
size_t numberOfContainerSets;
size_t numberOfContainers;

// aux variable

/*
 * pointer to container currently be setting up, can be used for additional
 * debugging info if something goes wrong during container setup
 */
static Container *settingUpContainer;

typedef struct EntrySolveField {
    char *fieldName;
    unsigned number;
} EntrySolveField;

typedef struct EntryRemoveCandidate {
    char *fieldName;
    unsigned removedCandidate;
} EntryRemoveCandidate;

/**
 * set default values (which might be overridden by command line parameters
 * or other settings)
 */
void setDefaults() {
    setSudokuType(STANDARD_SUDOKU);
    dimensionGrid(9); // max number of 9
}

/**
 * set the type of Sudoku, e.g. X_SUDOKU
 * 
 * @param type type of the Sudoku type constants, e.g. X_SUDOKU
 */
void setSudokuType(unsigned type) {
    sudokuType = type;
}

/**
 * dimensions the Sudoku grid
 * 
 * @param maxNumber number of numbers = side length of the Sudoku grid
 */
void dimensionGrid(size_t _maxNumber) {
    maxNumber = _maxNumber;
    numberOfFields = maxNumber * maxNumber;
}

/**
 * 
 * @param gametype
 */
void setupGrid() {

    // assure that the grid has been dimensioned already
    assert(maxNumber > 1);

    initContainerSets();
    initFields();
    initContainers();
}

void releaseGrid() {
    freeContainers();
    freeFields();
}

/**
 * just allocate memory for fields, do not perform complete setup
 * 
 * @param numberOfFields total number of fields to be allocated
 */
void allocateFields(size_t numberOfFields) {
    fields = (Field *) xmalloc(sizeof (Field) * numberOfFields);
}

/**
 * determine the container types (depend on the game type) and provide
 * some basic global variables needed for initFields
 */
void initContainerSets() {
    unsigned *containerTypes;
    ContainerSet *containerSetPtr;

    // assuming a standard Sudoku, 
    // we have 3 types of containers (row, column, box)
    containerTypes = getContainerTypes(sudokuType);
    numberOfContainerSets = ulength(containerTypes);

    assert(numberOfContainerSets > 0);
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
}

/**
 * init empty fields, just the memory for the field and candidates, but
 * with no data, except the field name. This will be done in initGrid().
 */
void initFields() {
    Field *field;
    unsigned *candidates;
    int i;

    for (int f = 0; f < numberOfFields; f++) {
        field = fields + f;

        field->x = f % maxNumber;
        field->y = f / maxNumber;

        // allocate candidates
        candidates = (unsigned *) xmalloc(sizeof (unsigned) * maxNumber);
        // allow all candidates for the field
        for (int n = 0; n < maxNumber; n++) {
            candidates[n] = n + 1;
        }

        field->candidates = candidates;
        field->candidatesLeft = maxNumber;


        // allocate containerIndexes and referenced containers
        field->containerIndexes = (int **) xmalloc(sizeof (int *) * numberOfContainerSets);
        field->containers = (Container ***) xmalloc(sizeof (Container **) * numberOfContainerSets);
        for (i = 0; i < numberOfContainerSets; i++) {
            /* for each containerSet, reserve space for maxNumber containers, 
             * as if the field could be member in all (=maxNumber) container.
             * In most cases, each field will only be part of ONE container, 
             * e.g. one row. However, in some edge cases, a field could be 
             * member of more than one container of a type, e.g. the center 
             * field in the container type "diagonal" is part of both diagonal
             * containers. To cover cases like that, the containerIndexes is 
             * no index, but a vector of indexes, terminated with -1. In most
             * cases, there will be only one member, in other cases there will
             * be no member (e.g. the field A2 is not part of any diagonal 
             * container). In other cases there will be more than one member (as
             * described above with the center field with diagonals).
             * Allocate maxNumber + 1 to leave a slot for the terminator (-1).
             */
            field->containerIndexes[i] = (int *) xmalloc(sizeof (int) * (maxNumber + 1));

            // init containerIndexes with an empty list by registering
            // terminator as only member (at the moment)
            field->containerIndexes[i][0] = -1;

            // the same operations for referenced containers ...
            // -------------------------------------------------

            field->containers[i] = (Container **) xmalloc(sizeof (Container *) * (maxNumber + 1));

            // init containerIndexes with an empty list by registering
            // terminator as only member (at the moment)
            field->containers[i][0] = NULL;
        }

        // use the ROWS and COLS coordinates as the "name" of the field
        // reserve space for coordinates up to "Z26" (a theoretical limit of
        // a 26-number-Sudoku)
        sprintf(field->name, "%c%u", (char) (field->y + (int) 'A'), field->x + 1);
    }
    logVerbose("Finished initializing fields");
}

/**
 * free fields memory
 */
void freeFields() {

    for (int f = 0; f < numberOfFields; f++) {

        free(fields[f].candidates);
    }

    free(fields);
}

/**
 * init the containers, but with no link to containing 
 * fields. This will be done later in initGrid().
 */
void initContainers() {
    unsigned index;

    // init and populate "all containers" vector
    allContainers = (Container *) xmalloc(sizeof (Container) * (numberOfContainers));
    Container *containerPtr = allContainers;

    /*
     * go through all container sets and generate all child containers for each
     * container set in turn
     */
    for (unsigned set = 0; set < numberOfContainerSets; set++) {
        ContainerSet *containerSet = &(containerSets[set]);

        // generate the corresponding child containers
        for (unsigned containerIndex = 0; containerIndex < containerSet->numberOfContainers; containerIndex++) {

            containerPtr->name = containerSet->getContainerName(containerIndex);
            containerPtr->type = containerSet->type;

            settingUpContainer = containerPtr;


            // reserve space for a NULL terminator at the end of the container's field list
            containerPtr->fields = (FieldsVector *) xmalloc(sizeof (FieldsVector) * (maxNumber + 1));

            /*
             * pre-fill container fields (+1 extra field as terminator) with 
             * NULL in order to be able to check whether really maxNumber 
             * fields have been assigned
             */
            for (index = 0; index <= maxNumber; index++) {
                containerPtr->fields[maxNumber] = NULL;
            }

            // fill the field of the container
            (containerSet->fillContainerFields)(containerIndex, containerPtr->fields);

            // link fields to containers: containerIndexes and containers
            // ----------------------------------------------------------

            // register the field's indexes within the containers of this type
            for (index = 0; index < maxNumber; index++) {
                Field *field = containerPtr->fields[index];
                int *containerIndexes = field->containerIndexes[set];
                Container **containers = field->containers[set];

                // fast forward to end of the lists
                while (*containerIndexes != -1) {
                    containerIndexes++;

                    // both containerIndexes and containers must correlate
                    // to each other, thus having the same number of members.
                    // So we can fast-forward both at simultaneously
                    containers++;
                }

                // append containerIndex to existing list of containerIndexes
                *containerIndexes++ = index;
                *containerIndexes = -1; // terminate list

                *containers++ = containerPtr;
                *containers = NULL; // terminate list
            }

            // link to container set
            containerSet->containers[containerIndex] = containerPtr;

            // FIXME debugging code
            sprintf(buffer, "name of container: %s", containerPtr->name);
            logVerbose(buffer);

            containerPtr++;
        }
    }

    settingUpContainer = NULL;

    // sanity check: check that each and every container contains exactly
    // maxNumber fields
    for (index = 0; index < numberOfContainers; index++) {
        Container *container = &(allContainers[index]);

        // check container
        for (unsigned fieldIndex = 0; fieldIndex < maxNumber; fieldIndex++) {
            // all maxNumber fields of the container must be initialized,
            // i.e. must point to a field
            assert(container->fields[fieldIndex]);
        }

        // check terminating NULL field pointer
        assert(container->fields[maxNumber] == NULL);
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

/**
 * sets the value of a field and eliminates this number from all candidates
 * of neighboring fields
 * 
 * @param field pointer to the Field structure
 * @param value the number to be set as result field value
 */
void setValue(Field *field, unsigned value) {
    Container **containers;
    Field *otherField;

    assert(value <= maxNumber);

    if (field->correctSolution) {
        assert(value == field->correctSolution);
    }

    field->value = value;

    // check if the number does not occur in any neighbors in any containers
    for (int containerSetIndex = 0; containerSetIndex < numberOfContainerSets; containerSetIndex++) {
        containers = field->containers[containerSetIndex];

        while (*containers) {
            Container *container;

            container = *containers;
            for (int pos = 0; pos < maxNumber; pos++) {
                otherField = container->fields[pos];
                if (otherField != field) {
                    /* 
                     * check that the number to which "our" field should be set
                     * is not an already solved number in the "other" field (in 
                     * the same container, each number must be unique)
                     */
                    assert(otherField->value != value);
                }
            }
            containers++;
        }
    }

    // remove all candidates from this field
    unsigned *candidates = field->candidates;
    for (unsigned n = 1; n <= maxNumber; n++) {

        candidates[n - 1] = (n == value) ? value : 0;
    }

    forbidNumberInNeighbors(field, value);
}

/**
 * forbids a number in all neighbor fields of the given field. This is used
 * e.g. after setting the value of a field to eliminate this number from all
 * neighbors. The "neighbors" are determined in all containers containing
 * the specified field. No logging of removed candidates occurs.
 * 
 * @param field
 * @param n the number to be forbidden in neighboring fields
 */
void forbidNumberInNeighbors(Field *field, unsigned n) {
    Container **containers;
    unsigned numbers[2];

    assert(n <= maxNumber);

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
    for (unsigned containerSetIndex = 0; containerSetIndex < numberOfContainerSets; containerSetIndex++) {
        for (containers = field->containers[containerSetIndex]; *containers; containers++) {
            forbidNumbersInOtherFields(*containers, numbers, preserve);
        }
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
    unsigned candidate;
    unsigned *candidates;

    //    showAllCandidates();

    progress = 0; // nothing has changed yet

    // walk through entire container
    for (int pos = 0; pos < maxNumber; pos++) {
        field = container->fields[pos];

        // ignore solved fields
        if (field->value) {
            continue;
        }

        // don't touch the 'dontTouch' fields
        if (!containsField(dontTouch, field)) {
            // forbid the tuple numbers
            candidates = n;
            while (*candidates) {
                candidate = *candidates;

                // was a candidate until now => remove candidate now
                if (!field->value && isCandidate(field, candidate)) {

                    sprintf(buffer, "forbid %u in field %s", candidate, field->name);
                    logReduction(buffer);
                    logVerbose("Before forbidding ...");
                    showCandidates(field);
                    sprintf(buffer, "Before - candidatesLeft = %u", field->candidatesLeft);
                    logVerbose(buffer);

                    progress = removeCandidate(field, candidate);
                }

                // go to next candidate to be forbidden
                candidates++;
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

    assert(n >= 1 && n <= maxNumber);

    if (field->candidates[n - 1]) {

        if (field->correctSolution) {
            assert(n != field->correctSolution);
        }

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
    for (n = 1; n <= maxNumber; n++) {
        if (candidates[n - 1]) {
            solveField(field, n);

            break;
        }
    }

    return n;
}

/**
 * solves the field with the given value
 * 
 * @param field
 * @param n
 */
void solveField(Field *field, unsigned n) {

    // field should not be solved already

    assert(!field->value);
    assert(field->candidatesLeft == 1);
    assert(field->candidates[n - 1]);

    EntrySolveField *info = (EntrySolveField *) xmalloc(sizeof (EntrySolveField));

    info->fieldName = field->name;
    info->number = n;

    writeLog(printLogSetUniqueNumber, info);

    setValue(field, n);
}

void printLogSetUniqueNumber(void *info) {

    EntrySolveField *infoStruct;

    infoStruct = (EntrySolveField *) info;

    printf("--- LOG: set value of field %s to %u\n", infoStruct->fieldName, infoStruct->number);
}

/**
 * collects all unresolved fields in which the given number is a possible 
 * candidate
 * 
 * @param container
 * @param n the number to look for as a candidate
 * @return vector of fields containing the given number as possible candidate
 */
FieldsVector *fieldsWithCandidate(FieldsVector *fields, unsigned n) {
    int ix;
    Field *field;
    FieldsVector *found;
    FieldsVector *foundPtr;

    found = (FieldsVector *) xmalloc(sizeof (Field *) * (maxNumber + 1));

    foundPtr = found;
    for (ix = 0; ix < maxNumber; ix++) {
        field = *(fields + ix);
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
    do {
        if (*fieldsVector == NULL && *numbers == 0) {
            return 1;
        }

        // if we are still here, then at least one of the vectors is not null.
        // However, if the other one is exhausted, then both vectors apparently
        // do not have the same length
        if (*fieldsVector == NULL || *numbers == 0) {
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

    assert(n >= 1 && n <= maxNumber);
    sprintf(buffer, "Looking for unique position of %u in container ...", n);
    logVerbose(buffer);

    for (pos = 0; pos < maxNumber; pos++) { // FIXME debugging output
        showCandidates(container[pos]);
    }

    unique = 0;
    foundPos = 0;
    for (pos = 0; pos < maxNumber; pos++) {
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

    for (f = 0; f < numberOfFields; f++) {
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

    for (f = 0; f < numberOfFields; f++) {
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

/**
 * checks if the given number is still a valid candidate for the field
 * 
 * @param field the field from which a candidate shall be determined
 * @param candidate the candidate to be determined
 * @return 1 if the candidate is still valid for this field, or 0 if it is not
 */
int isCandidate(Field *field, unsigned candidate) {
    return field->candidates[candidate - 1];
}

/**
 * removes one candidate from a field. 
 * 
 * @param field the field from which a candidate shall be removed
 * @param candidate the candidate to be removed
 * @return 1 if the candidate has been removed, or 0 if nothing has changed
 *   (candidate has already been removed before)
 */
int removeCandidate(Field *field, unsigned candidate) {
    unsigned *c;

    c = field->candidates + candidate - 1;

    if (*c) {

        if (field->correctSolution) {
            assert(candidate != field->correctSolution);
        }

        *c = 0;
        field->candidatesLeft--;

        // TODO log removal of candidate
        EntryRemoveCandidate *info = (EntryRemoveCandidate *) xmalloc(sizeof (EntryRemoveCandidate));
        info->fieldName = strdup(field->name);
        info->removedCandidate = (*c + 1);
        writeLog(printLogRemoveCandidate, info);

        if (field->candidatesLeft == 0) {
            sprintf(buffer, "no candidates left on field %s", field->name);
            logError(buffer);
            exit(EXIT_FAILURE);
        }

        return 1;
    }

    return 0;
}

void printLogRemoveCandidate(void *info) {

    EntryRemoveCandidate *infoStruct;

    infoStruct = (EntryRemoveCandidate *) info;

    printf("--- LOG: field %s: remove candidate %u\n", infoStruct->fieldName, infoStruct->removedCandidate);
}

/**
 * identifies the field on the given coordinates 
 * @param x X coordinate, in the range (0 ... maxNumber-1)
 * @param y Y coordinate, in the range (0 ... maxNumber-1)
 * @return the field on the given coordinates
 */
Field *getFieldAt(unsigned x, unsigned y) {
 
    // sanity check
    if (!(x >= 0 && x < maxNumber) && settingUpContainer) {
        sprintf(buffer, "setting up container %s seems to be buggy (requested field at %u / %u)", settingUpContainer->name, x, y);
        logError(buffer);
        exit(EXIT_FAILURE);
    }
    if (!(y >= 0 && y < maxNumber) && settingUpContainer) {
        sprintf(buffer, "setting up container %s seems to be buggy (requested field at %u / %u)", settingUpContainer->name, x, y);
        logError(buffer);
        exit(EXIT_FAILURE);
    }
    
    assert(x >= 0 && x < maxNumber);
    assert(y >= 0 && y < maxNumber);

    return (fields + y * maxNumber + x);
}