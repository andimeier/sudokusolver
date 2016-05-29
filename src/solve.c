/*
 * File:   solve.c
 * Author: aurez
 *
 * Created on 04. April 2016, 21:01
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "solve.h"
#include "show.h"
#include "grid.h"
#include "typedefs.h"
#include "util.h"
#include "grid.h"
#include "log.h"

// search for pairs, triples and quadruples, not more
#define MAX_TUPLE_DIMENSION 4

typedef int (*strategy)(void);

// auxiliary functions
static int findNakedTuplesX(size_t dimension);
static unsigned recurseNakedTuples(unsigned maxLevel, Container *container, unsigned level, unsigned *numbers, FieldsVector *fieldsContainingCandidates);
static int compareCandidates(unsigned *c1, unsigned *c2);


int errors; // number of errors in the algorithm
int verboseLogging; // 0 ... no verbose logging, 1 ... log changes, 2 ... log even considerations

//-------------------------------------------------------------------
// check for cells having only one candidate left and set their value (and
// thus eliminate this value in neighboring fields)
// @return 1 ... something has changed, 0 ... nothing changed

int checkForSolvedCells() {
    int f;
    Field *field;
    int value;
    int progress; // Flag: in einer Iteration wurde zumindest eine Erkenntnis gewonnen

    if (verboseLogging == 2)
        printlog("[strategy] check for solved cells ...\n");

    progress = 0;

    showAllCandidates();

    for (f = 0; f < NUMBER_OF_FIELDS; f++) {
        field = fields + f;
        value = field->value;

        if (field->candidatesLeft == 1 && !field->value) {
            setUniqueNumber(field);
            progress = 1;
        }

    }
    return progress;
}

int findHiddenSingles() {
    int progress; // flag: something has changed
    Container *container;

    if (verboseLogging == 2)
        printlog("[strategy] find hidden singles ...\n");

    progress = 0;

    // search in all unit types (rows, cols, boxes, ...) for numbers which can 
    // only occur on one position within the unit (even if there would be
    // several candidates for this cell, but the other candidates can be
    // discarded in this case)


    for (unsigned c = 0; c < numberOfContainers; c++) {
        container = allContainers + c;

        for (unsigned n = 1; n <= MAX_NUMBER; n++) {
            Field **containerFields = container->fields;
            int pos = getUniquePositionInContainer(containerFields, n);
            if (pos != -1 && !containerFields[pos]->value) {
                // number can only occur in the position pos in this container
                if (verboseLogging) {
                    // TODO sprintf(buffer, "!!! Neue Erkenntnis 2a: In Zeile %d kann %d nur an Position %d vorkommen => (%d/%d) = %d!\n", y + 1, n, x + 1, y + 1, x + 1, n);
                    // TODO printlog(buffer);
                }
                setValue(containerFields[pos], n);

                Field *field = containerFields[pos];
                sprintf(buffer, "*** [hidden single] hidden single in unit %s, field %s: %u ... \n", container->name, field->name, n);
                printlog(buffer);

                progress = 1; // Flag "neue Erkenntnis" setzen
            }
        }
        container++;
    }


    // OBSOLETE VERSION:
    //    for (int u = 0; u < unitDefs.count; u++) {
    //        ContainerSet *unit = &(unitDefs.containerTypes[u]);
    //
    //        if (verboseLogging == 2) {
    //            sprintf(buffer, "??? Searching for: hidden singles in units of type %s ... \n", unit->name);
    //            printlog(buffer);
    //        }
    //
    //        for (int c = 0; c < unit->numberOfContainers; c++) {
    //            for (unsigned n = 1; n <= MAX_NUMBER; n++) {
    //                Container *container = &(unit->containers[c]);
    //                Field **containerFields = container->fields;
    //                int pos = getUniquePositionInContainer(containerFields, n);
    //                if (pos != -1 && !containerFields[pos]->value) {
    //                    // number can only occur in the position pos in this container
    //                    if (verboseLogging) {
    //                        // TODO sprintf(buffer, "!!! Neue Erkenntnis 2a: In Zeile %d kann %d nur an Position %d vorkommen => (%d/%d) = %d!\n", y + 1, n, x + 1, y + 1, x + 1, n);
    //                        // TODO printlog(buffer);
    //                    }
    //                    setValue(containerFields[pos], n);
    //
    //                    Field *field = containerFields[pos];
    //                    sprintf(buffer, "*** [hidden single] hidden single in unit %s, field %s: %u ... \n", container->name, field->name, n);
    //                    printlog(buffer);
    //
    //                    progress = 1; // Flag "neue Erkenntnis" setzen
    //                }
    //            }
    //        }
    //
    //    }

    return progress;
}

/**
 * find naked tuples (pairs, triples, ...) in the same container
 * @return 
 */
int findNakedTuples() {
    int progress;
    Container *container;

    if (verboseLogging == 2)
        printlog("[strategy] find naked tuples ...\n");

    progress = 0;

    for (int dimension = 2; dimension < MAX_TUPLE_DIMENSION; dimension++) {

        // go through all containers and find naked tuples therein
        for (int c = 0; c < numberOfContainerSets; c++) {
            container = &(allContainers[c]);
            sprintf(buffer, "-- next container: %s", container->name);
            printlog(buffer);
            progress |= findNakedTuplesInContainer(container, dimension);
        }

        if (progress)
            break;
    }

    return progress;
}

int findHiddenPairs() {
    int y;
    int cand;
    int progress;

    // http://programmers.stackexchange.com/questions/270930/sudoku-hidden-sets-algorithm

    progress = 0;

    // hidden pairs in rows
    for (y = 0; y < MAX_NUMBER; y++) {
        for (cand = 1; cand <= MAX_NUMBER; cand++) {
            // countCandidateInRow(cand, y);
        }

    }

    return progress;
}

int recurseHiddenTuples(unsigned maxLevel, FieldsVector *fields, unsigned level, unsigned *candidates, FieldsVector *fieldsContainingCandidates) {

    // make room for new candidate in the candidates vector
    candidates[level + 1] = 0;

    // add next number to numbers vector
    for (unsigned n = 1; n < MAX_NUMBER; n++) {
        candidates[level] = n;

        // check this combination of candidates (in the candidates vector) 
        // whether there are only length(candidates) Sudoku fields in which
        // the candidates can occur => in this case this would be a hidden
        // tuple of length length(candidates)

        // FIXME setFieldsContainingCandidates(fieldsContainingCandidates, n);

        // recurse further?
        // FIXME 
        //        if () {
        //        }
    }

    // "rollback" recursion
    candidates[level] = 0;

    return 0; // FIXME ????
}

/**
 * find pointing pairs or triples
 * 
 * @return progress flag: 1 for "something has changed", 0 for "no change"
 */
int findPointingTupels() {
    ContainerSet *unit;
    int progress; // flag: something has changed
    unsigned tuple[MAX_NUMBER];
    unsigned n;

    progress = 0;


    //    printf("[pii] starting findPointingTupels ...\n");
    //
    //    // search in all unit types (rows, cols, boxes, ...) for a tuple of numbers 
    //    // which form a "pointing tuple"
    //
    //    for (int u = 0; u < unitDefs.count; u++) {
    //        unit = &(unitDefs.containerTypes[u]);
    //        if (verboseLogging == 2) {
    //            sprintf(buffer, "??? Searching for: pointing tuples in units of type %s ... \n", unit->name);
    //            printlog(buffer);
    //        }
    //
    //        printf("container \"%s\" has %zu instances\n", unit->name, unit->numberOfContainers);
    //        for (int c = 0; c < unit->numberOfContainers; c++) {
    //            FieldsVector *container = unit->containers[c].fields;
    //
    //            showAllCandidates();
    //            show(0);
    //            printf("Alex\n");
    //            show(0);
    //            sudokuString(0);
    //
    //            printf("iterating into instance %d of container \"%s\"\n", c, unit->name); // FIXME debugging output
    //
    //            // check for pointing tuples in this container
    //            for (n = 1; n <= MAX_NUMBER; n++) {
    //                FieldsVector *fieldsVector;
    //
    //                // collect all fields which contain this candidate
    //                printf("get fields with candidate %u in unit type %s ... \n", n, unit->name); // FIXME debugging output
    //                fieldsVector = fieldsWithCandidate(container, n);
    //                printf("got fields:\n"); // FIXME debugging output
    //                Field **ptr = fieldsVector; // FIXME debugging variable
    //                while (*ptr) { // FIXME debugging output
    //                    //                    printf("ptr point to address %d\n", (int) *ptr);
    //                    printf("  candidate %u is possible in field %d/%d\n", n, (*ptr)->containerIndexes[ROWS], (*ptr)->containerIndexes[COLS]);
    //                    ptr++;
    //                }
    //
    //                printf("[hhh1]\n");
    //
    //                if (*fieldsVector == NULL) {
    //                    // candidate n not found in any free field => skip it
    //                    printf("[hhh2]\n");
    //                    continue;
    //                }
    //                printf("[hhh3]\n");
    //
    //                // for every unit type other than the current one, check if
    //                // all fields of the tuple share the same "other" unit instance.
    //                // If so, the candidate can be removed from all fields in the
    //                // "other" instance except for the fields in the tuple (in the
    //                // current container)
    //
    //                for (int u2 = 0; u2 < unitDefs.count; u2++) {
    //                    if (u2 == u) {
    //                        // only look in OTHER unit types
    //                        printf("[hhh4]\n");
    //                        continue;
    //                    }
    //
    //                    printf("[hhh5]\n");
    //                    printf("  look at unit %s ...\n", unitDefs.containerTypes[u2].name);
    //
    //                    //                    for (int f2 = 0; f2 < NUMBER_OF_FIELDS; f2++) { // FIXME debugging output
    //                    //                        printf("[1234] field #%d: in row %d, col %d, box %d\n", f2, fields[f2].unitPositions[ROWS], fields[f2].unitPositions[COLS], fields[f2].unitPositions[BOXES]);
    //                    //                    }
    //
    //
    //                    int containerIndex;
    //                    // check if all fields share the same instance of the "other unit"
    //                    Field **fieldsPtr;
    //                    fieldsPtr = fieldsVector;
    //                    containerIndex = (*fieldsPtr)->containerIndexes[u2];
    //                    printf("the tupel MIGHT be in %s #%d ...\n", unitDefs.containerTypes[u2].name, containerIndex);
    //                    fieldsPtr++;
    //                    while (*fieldsPtr) {
    //                        printf("[jjj2] check next position ...\n");
    //                        if ((*fieldsPtr)->containerIndexes[u2] != containerIndex) {
    //                            printf("[jjj3] FOUND SOMETHING => no tupel...\n");
    //                            break;
    //                        }
    //
    //                        fieldsPtr++;
    //                    }
    //
    //                    printf("[jjj4] finished\n");
    //
    //                    // found pointing tuple?
    //                    if (!(*fieldsPtr)) {
    //                        printf("[lkkk] Found pointing tuple in %s #%d ...) \n", unitDefs.containerTypes[u2].name, containerIndex);
    //
    //                        // prepare tuple
    //                        for (int i = 0; i < MAX_NUMBER; i++) {
    //                            tuple[i] = 0;
    //                        }
    //                        tuple[n - 1] = n;
    //
    //                        progress |= forbidNumbersInOtherFields(&(unitDefs.containerTypes[u2].containers[containerIndex]), tuple, fieldsVector);
    //                    }
    //                }
    //
    //                free(fieldsVector);
    //            }
    //        }
    //    }

    return progress;

}

/**
 * find naked tuples (pairs, triples, ...) which share the same candidates.
 * For instance, if two fields have only the candidates 2 and 4, then 2 and 4
 * can be eliminated from all other fields in the same container.
 * 
 * @param container vector of fields (=container) in which we look for naked 
 *   tuples
 * @param dimension dimension of the tupel to be looked for. 2=pairs, 
 *   3=triples etc.
 * @return progress flag: 1 for "something has changed", 0 for "no change"
 */
unsigned findNakedTuplesInContainer(Container *container, unsigned dimension) {
    unsigned progress;
    unsigned *numbers;
    FieldsVector *foundFields;

    assert(dimension > 0 && dimension < MAX_NUMBER);

    progress = 0;
    numbers = (unsigned *) xmalloc(sizeof (unsigned) * (dimension + 1));
    foundFields = (FieldsVector *) xmalloc(sizeof (FieldsVector) * (dimension + 1));

    // we are in level 0 of recursion: initialize numbers vector
    numbers[0] = 0;
    foundFields[0] = NULL;

    printf("starting recursion ...\n"); // DEBUG
    fflush(stdout);

    if (recurseNakedTuples(dimension, container, 1, numbers, foundFields)) {
        progress = 1;
    } else {
        printlog('[1244] returned from recursion');
    }

    free(foundFields);
    free(numbers);

    return progress;
}

/**
 * recursively look for naked tuples of the dimension maxLevel.
 * Note: finds tupels of the given dimension and below the given dimension.
 * For example, if dimension is 3, then naked triples are found, but naked pairs
 * as well.
 * 
 * @param maxLevel (beginning with 1)
 * @param container 
 * @param level (starting with 1)
 * @param numbers numbers vector to be searched for, terminated with 0
 * @param fieldsContainingCandidates vector of found fields, terminated with NULL
 * @return 1 if a naked tuple has been found, 0 otherwise
 */
unsigned recurseNakedTuples(unsigned maxLevel, Container *container, unsigned level, unsigned *numbers, FieldsVector *fieldsContainingCandidates) {

    if (level > maxLevel) {
        // maximum recursion depth reached => nothing found
        sprintf(buffer, "maximum recursion depth of %u reached.", level);
        printlog(buffer);
        return 0;
    }

    printf("Entering recursion level %u/%u ...\n", level, maxLevel);

    numbers[level] = 0;
    fieldsContainingCandidates[level] = NULL;

    // iterate through all numbers of this level, starting with number of 
    // previous iteration level plus 1. Thus, the numbers are in ascending
    // order and the numbers are not repeating themselves
    printf("start iteration level %d, numbers= (%u, %u, %u)\n", level, numbers[0], numbers[1], numbers[2]);
    printf("start iteration level %d with number %u\n", level, (level == 1) ? 1 : (numbers[level - 2] + 1));
    for (unsigned number = ((level == 1) ? 1 : (numbers[level - 2] + 1)); number <= MAX_NUMBER; number++) {
        // try next number
        numbers[level - 1] = number;
        sprintf(buffer, "number vector is now (%u, %u, %u), level=%u", numbers[0], numbers[1], numbers[1] ? numbers[2] : 0, level);
        printlog(buffer);

        // loop through all fields of the container
        for (unsigned i = 0; i < MAX_NUMBER; i++) {
            Field *field;

            field = container->fields[i];
            sprintf(buffer, "%s field #%u (level %u)", (level == 1) ? "looking at" : "comparing with", i, level);
            printlog(buffer);
            printlog("[ttt]");
            if (fieldCandidatesAreSubsetOf(field, numbers)) {
                sprintf(buffer, "yeah! field candidates of field #%u are a subset of the numbers %u, %u", i, numbers[0], numbers[1]);
                printlog(buffer);

                // append field to list of found fields
                FieldsVector *fieldPtr = fieldsContainingCandidates;
                while (*fieldPtr) {
                    fieldPtr++;
                }
                *fieldPtr++ = field;
                *fieldPtr = NULL; // terminate list of fields

                // check if we have found enough fields
                if (equalNumberOfFieldsAndCandidates(fieldsContainingCandidates, numbers)) {
                    unsigned progress;

                    /* found a naked tuple! But we only make progress if - based
                     * on this naked tuple - candidates can be eliminated... 
                     * let's see ...
                     */
                    progress = 0;

                    /* eliminate the found numbers of the naked tuple from
                     * all other field of the same container
                     * TODO and from other containers if all found fields 
                     * share the same other container
                     */
                    printf("GOT IT! forbidding numbers in other fields ...\n");
                    progress |= forbidNumbersInOtherFields(container, numbers, fieldsContainingCandidates);
                    printlog("[123a]");
                    return progress;
                }
            }
        }

        // no tuple of dimension "level" found => recurse further
        if (recurseNakedTuples(maxLevel, container, level + 1, numbers, fieldsContainingCandidates)) {
            printlog("recursion returned with progress flag of 1");
            // found a naked tuple! Instantly return
            sprintf(buffer, "recursion exited with 1, propagate exit from level %d", level);
            printlog(buffer);
            return 1;
        }

        // FIXME DEBUG
        printlog("recursion returned with progress flag of 0");
    }

    printf("leaving recursion level %d/%d, going back one level\n", level, maxLevel);
    return 0;
}


//FieldsVector **containersContainingAllFields(FieldsVector *fields) {
//}

/**
 * The working horse. Try to solve the Sudoku.
 * 
 * @return 1 ... Sudoku has been solved successfully. 0 ... algorithm got stuck,
 *   indefinite iteration cancelled.
 */
int solve() {
    int iteration;
    int progress; // flag: something has changed in the iteration
    strategy *strategies;
    strategy *currentStrategy;

    // add stragies to list of strategies to be used
    strategies = (strategy *) xmalloc(sizeof (strategy) * 10);
    currentStrategy = strategies;
    *currentStrategy++ = &checkForSolvedCells;
    *currentStrategy++ = &findHiddenSingles;
    *currentStrategy++ = &findNakedTuples;
    *currentStrategy++ = NULL; // terminate list of strategies


    iteration = 0;
    errors = 0; // no errors yet

    printf("[4sf]\n");

    printSvg(0);

    printf("[4s65f]\n");

    cleanUpCandidates();

    // main loop, only enter if Sudoku has been solved or if we got stuck and
    // are unable to solve the Sudoku
    do {
        iteration++;
        progress = 0; // nothing changed in this iteration (no wonder - has just started)
        if (verboseLogging == 2) {
            sprintf(buffer, "----- Beginne Iteration %d -----\n", iteration);
            printlog(buffer);
        }


        // loop through all strategies
        currentStrategy = strategies;
        while (currentStrategy) {
            progress |= (**currentStrategy)();

            if (progress) {
                // no iterating to next strategy
                break;
            }

            // no progress => try next strategy
            currentStrategy++;
        }

        if (isFinished()) {
            return 1;
        }

    } while (progress);

    showAllCandidates();

    free(strategies);

    // wir kommen hierher, weil die letzte Iteration keine einzige Aenderung gebracht
    // hat => wir bleiben stecken mit unserem Algorithmus. Ohne Aenderung in der
    // Implementierung ist dieses Sudoku nicht loesbar
    return 0;
}

/**
 * compare two lists of candidates and check if they are equal
 */
int compareCandidates(unsigned *c1, unsigned *c2) {
    for (int n = 0; n < MAX_NUMBER; n++) {
        if (c1[n] != c2[n])
            return 0;
    }

    // both are equal
    return 1;
}

