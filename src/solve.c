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
#include "global.h"
#include "solve.h"
#include "show.h"
#include "grid.h"
#include "typedefs.h"
#include "util.h"
#include "grid.h"
#include "log.h"
#include "fieldlist.h"

/*
 * Optimisations:
 * 
 * find naked tuples:
 * - skip fields with candidatesLeft > maxTupleDimensionToLookFor
 */

#define ZZDEBUG_SEGFAULT

// search for pairs, triples and quadruples, not more
#define MAX_TUPLE_DIMENSION 3

typedef int (*strategy)(void);

// auxiliary functions
static unsigned recurseNakedTuples(unsigned maxLevel, Container *container, unsigned level, FieldList *includedFields, FieldsVector *fieldsLeft);
static int eliminateFieldsCandidatesFromOtherFields(Container *container, FieldsVector *fields);
static void populateFieldsForNakedTuples(FieldsVector *relevantFields, FieldsVector *allFields, unsigned dimension);
static int compareCandidates(unsigned *c1, unsigned *c2);

// number of errors in the algorithm
int errors;

// FIXME remove this test counter
int testCounter = 0;

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

    printSvg(0);

    cleanUpCandidates();

    // main loop, only enter if Sudoku has been solved or if we got stuck and
    // are unable to solve the Sudoku
    do {
        iteration++;
        progress = 0; // nothing changed in this iteration (no wonder - has just started)
        sprintf(buffer, "----- Beginne Iteration %d -----", iteration);
        logVerbose(buffer);


        // loop through all strategies
        currentStrategy = strategies;
        while (*currentStrategy) {
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
    // hat => wir bleiben stecken mit unseren Algorithmen. Ohne Aenderung in der
    // Implementierung ist dieses Sudoku nicht loesbar
    return 0;
}

void printFoundFields(FieldsVector *foundFields) {
    FieldsVector *f;

    f = foundFields;

    buffer[0] = '\0';
    while (*f) {
        strcat(buffer, "-");
        strcat(buffer, (*f)->name);
        f++;
    }
    logVerbose(buffer);
}

/**
 * check for cells having only one candidate left and set their value (and
 * thus eliminate this value in neighboring fields)
 * 
 * @return 1 ... something has changed, 0 ... nothing changed
 */
int checkForSolvedCells() {
    int f;
    Field *field;
    int progress; // Flag: in einer Iteration wurde zumindest eine Erkenntnis gewonnen

    logVerbose("[strategy] check for solved cells ...");

    progress = 0;

    showAllCandidates();

    for (f = 0; f < NUMBER_OF_FIELDS; f++) {
        field = fields + f;

        if (field->candidatesLeft == 1 && !field->value) {
            setUniqueNumber(field);
            progress = 1;
        }

    }
    return progress;
}

/**
 * strategy "find hidden singles"
 * 
 * @return 1 if something has changed, 0 if not
 */
int findHiddenSingles() {
    int progress; // flag: something has changed
    Container *container;

    logVerbose("[strategy] find hidden singles ...");

    progress = 0;

    // search in all unit types (rows, cols, boxes, ...) for numbers which can 
    // only occur on one position within the unit (even if there would be
    // several candidates for this cell, but the other candidates can be
    // discarded in this case)


    for (unsigned c = 0; c < numberOfContainers; c++) {
        container = &(allContainers[c]);

        for (unsigned n = 1; n <= MAX_NUMBER; n++) {
            Field **containerFields = container->fields;
            int pos = getUniquePositionInContainer(containerFields, n);
            if (pos != -1 && !containerFields[pos]->value) {
                // number can only occur in the position pos in this container
                setValue(containerFields[pos], n);

                Field *field = containerFields[pos];
                sprintf(buffer, "*** [hidden single] hidden single in unit %s, field %s: %u ... ", container->name, field->name, n);
                logVerbose(buffer);

                progress = 1; // Flag "neue Erkenntnis" setzen
            }
        }
        container++;
    }


    // OBSOLETE VERSION:
    //    for (int u = 0; u < unitDefs.count; u++) {
    //        ContainerSet *unit = &(unitDefs.containerTypes[u]);
    //
    //
    //        for (int c = 0; c < unit->numberOfContainers; c++) {
    //            for (unsigned n = 1; n <= MAX_NUMBER; n++) {
    //                Container *container = &(unit->containers[c]);
    //                Field **containerFields = container->fields;
    //                int pos = getUniquePositionInContainer(containerFields, n);
    //                if (pos != -1 && !containerFields[pos]->value) {
    //                    // number can only occur in the position pos in this container
    //                    setValue(containerFields[pos], n);
    //
    //                    Field *field = containerFields[pos];
    //                    sprintf(buffer, "*** [hidden single] hidden single in unit %s, field %s: %u ... ", container->name, field->name, n);
    //                    logVerbose(buffer);
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
 * 
 * @return 1 if something has changed, 0 if not
 */
int findNakedTuples() {
    int progress;
    Container *container;
    FieldList *includedFields;
    FieldsVector *fieldsLeft;

    logVerbose("[strategy] find naked tuples ...");

    progress = 0;

    // allocate memory for strategy variables
    includedFields = createFieldList(MAX_TUPLE_DIMENSION);
    fieldsLeft = (FieldsVector *) xmalloc(sizeof (Field *) * (MAX_NUMBER + 1));

    for (int dimension = 2; dimension <= MAX_TUPLE_DIMENSION; dimension++) {

        // go through all containers and find naked tuples therein
        for (unsigned c = 0; c < numberOfContainers; c++) {
            //        for (unsigned c = 0; c < 2; c++) {
            container = &(allContainers[c]);
            sprintf(buffer, "-- next container: %s", container->name);
            logVerbose(buffer);

            progress |= findNakedTuplesInContainer(container, dimension, includedFields, fieldsLeft);
        }

        if (progress) {
            break;
        }
    }

    free(fieldsLeft);
    freeFieldList(includedFields);

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
    //ContainerSet *unit;
    int progress; // flag: something has changed
    //unsigned tuple[MAX_NUMBER];
    //unsigned n;

    progress = 0;


    //    printf("[pii] starting findPointingTupels ...\n");
    //
    //    // search in all unit types (rows, cols, boxes, ...) for a tuple of numbers 
    //    // which form a "pointing tuple"
    //
    //    for (int u = 0; u < unitDefs.count; u++) {
    //        unit = &(unitDefs.containerTypes[u]);
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
 * Note: finds tupels of and only of the given dimension. For example,
 * if dimension == 3, then only naked triples are found, but not naked pairs 
 * (actually, naked pairs will be identified as naked triple with one "excessive
 * cell").
 * 
 * @param container vector of fields (=container) in which we look for naked 
 *   tuples
 * @param dimension dimension of the tupel to be looked for. 2=pairs, 
 *   3=triples etc.
 * @param includedFields allocated buffer for the "found fields" during the
 *   algorithm, must be the size of the dimension (plus 1 for a NULL termination)
 * @param fieldsLeft allocated buffer for the vector of fields left (yet to be 
 *   examined), terminated with NULL, thus must be the size of the dimension
 *   plus 1 (for the NULL terminator)
 * @return progress flag: 1 for "something has changed", 0 for "no change"
 */
unsigned findNakedTuplesInContainer(Container *container, unsigned dimension, FieldList *includedFields, FieldsVector *fieldsLeft) {
    unsigned progress;

    assert(dimension > 0 && dimension < MAX_NUMBER);

    progress = 0;

    // we are in level 0 of recursion: initialize numbers vector
    emptyFieldList(includedFields);

    testCounter++;
    sprintf(buffer, "~~~ testCounter: %d ~~~", testCounter);
    logVerbose(buffer);

    // populate fields to search for naked tuples
    populateFieldsForNakedTuples(fieldsLeft, container->fields, dimension);

    if (recurseNakedTuples(dimension, container, 1, includedFields, fieldsLeft)) {
        // FIXME Optimierungsschritt: dieses gefundene naked tuple merken, damit es nicht
        // in Zukunft jedesmal gefunden wird (aber ohne mehr etwas zu bewirken)
        progress = 1;
    } else {

        logVerbose("[1244] returned from recursion");
    }

    return progress;
}

/**
 * recursively look for naked tuples of the dimension maxLevel.
 * Note: finds tupels of and only of the given dimension. For example,
 * if dimension == 3, then only naked triples are found, but not naked pairs 
 * (actually, naked pairs will be identified as naked triple with one "excessive
 * cell").
 * 
 * @param maxLevel (beginning with 1) = dimension
 * @param container container in which we look for naked tuples
 * @param level current recursion depth (starting with 1)
 * @param includedFields allocated buffer for the "found fields" during the
 *   algorithm, must be the size of the dimension (plus 1 for a NULL termination)
 * @param fieldsLeft allocated buffer for the vector of fields left (yet to be 
 *   examined), terminated with NULL, thus must be the size of the dimension
 *   plus 1 (for the NULL terminator)
 * @return 1 if a naked tuple has been found, 0 otherwise
 */
unsigned recurseNakedTuples(unsigned maxLevel, Container *container, unsigned level, FieldList *includedFields, FieldsVector *fieldsLeft) {
    FieldsVector *left;

    assert(level >= 1);
    //FIXME war fuer alten Algo, gehts fuer den neuen auch mit so etwas?    assert(level == 1 || (level >= 2 && numbers[level - 2] != 0));

    if (level > maxLevel) {
        // maximum recursion depth reached => nothing found
        sprintf(buffer, "maximum recursion depth of %u reached.", maxLevel);
        logVerbose(buffer);
        return 0;
    }

    sprintf(buffer, "Entering recursion level %u/%u ...", level, maxLevel);

    logVerbose(buffer);

    //    // prepare list terminations *after* current list item
    //    numbers[level] = 0;
    //    foundFields[level] = NULL;

    left = fieldsLeft;

    // go through all left fields
    while (*left) {
        pushToFieldList(includedFields, *left++);

        // on level 1, don't try to find nake "tuples", only start at level 2
        if (level == 1 || countDistinctCandidates(includedFields->fields, maxLevel)) {
            // hm ... yes ... hm ... might contribute to a naked tuple ...
            if (includedFields->count < maxLevel) {
                // recurse further
                if (recurseNakedTuples(maxLevel, container, level + 1, includedFields, left)) {
                    // propagate success flag all levels down
                    return 1;
                }
            } else {
                /*
                 *  found naked tuple!
                 * we reached maximum recursion depth (= dimension) and the number
                 * of found candidates with a maximum total sum of "dimension"
                 * candidates is maximum "dimension"
                 */

                // depending on whether some candidates could be eliminated, the
                // board has changed or not
                return eliminateFieldsCandidatesFromOtherFields(container, includedFields->fields);
            }
        }

        popFromFieldList(includedFields);
    }


    //    // take back extensions of the vectors from the current level
    //    numbers[level - 1] = 0;
    //    foundFields[level - 1] = NULL;

    sprintf(buffer, "leaving recursion level %d/%d, going back one level\n", level, maxLevel);
    logVerbose(buffer);

    return 0; // nothing found
}


/**
 * compare two lists of candidates and check if they are equal
 * 
 * @param c1 first list of unsigned numbers (candidates)
 * @param c2 second list of unsigned numbers (candidates)
 * @return 1 if both are equal, 0 if they are not
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

int compareCandidates(unsigned *c1, unsigned *c2) {
    for (int n = 0; n < MAX_NUMBER; n++) {
        if (c1[n] != c2[n])
            return 0;
    }

    // both are equal
    return 1;
}
#pragma GCC diagnostic pop

/**
 * eliminate any candidates of the given fields in the rest of the container.
 * This is a utility function for the strategy "find naked tuples", where
 * the candidates of the naked tuple must be removed from all other fields in
 * the container except from the ones forming the naked tuple.
 * 
 * @param container the container in which candidates should be eliminated
 * @param fields the fields of which the candidates should be taken. These 
 *   candidates are forbidden in all of the *other* fields of the container
 * @return progress flag: 1 if something has changed (candidates eliminated) or
 *   0 if not
 */
int eliminateFieldsCandidatesFromOtherFields(Container *container, FieldsVector *fields) {
    unsigned *candidates;
    int progress;

    // add 1 item as a zero termination in the purely theoretical case that a
    // naked "nonuple" was found (all possible numbers of a Sudoku)
    candidates = (unsigned *) xmalloc(sizeof (unsigned) * (MAX_NUMBER + 1));
    for (int i = 0; i < MAX_NUMBER; i++) {
        candidates[i] = 0;
    }

    // find out all candidates
    while (*fields) {

        // retrieve all candidates from the field
        for (int i = 0; i < MAX_NUMBER; i++) {
            candidates[i] |= (*fields)->candidates[i];
        }
        fields++;
    }

    // compact candidates to fit the parameter for "forbidNumbersInOtherFields"
    unsigned *compact = candidates;
    for (int i = 0; i < MAX_NUMBER; i++) {
        if (candidates[i]) {
            *compact++ = candidates[i];
        }
    }
    // terminate list of candidates
    *compact = 0;

    // forbid candidates in all other fields
    progress = forbidNumbersInOtherFields(container, candidates, fields);

    free(candidates);

    return progress;
}

/**
 * aux function whose only purpose is to populate the fields in which the
 * algorithm should search for naked tuples
 * 
 * @param relevantFields list of fields with the relevant fields (this is the
 *   output = the result)
 * @param allFields container from which all or some fields form the result.
 *   Note that this is not NULL-terminated - instead, it contains MAX_NUMBER
 *   fields (= all fields of the container to be examined)
 * @param dimension the dimension of the tuples to search for. E.g. 2 means
 *   "search for naked pairs", 3 means "naked triples" etc.
 */
void populateFieldsForNakedTuples(FieldsVector *relevantFields, FieldsVector *allFields, unsigned dimension) {
    Field *field;

    int i;
    for (i = 0; i < MAX_NUMBER; i++) {
        field = *allFields++;

        if (!field) {
            // in case of NULL-terminated list allFields, exit if the 
            // terminating NULL is found (safeguard))
            break;
        }

        /*
         * ignore solved fields
         */
        if (field->value) {
            continue;
        }

        /*
         * ignore field if it has more than dimension candidates => this 
         * cannot be part of a naked tuple by definition
         */
        if (field->candidatesLeft > dimension) {
            continue;
        }

        // still here? No exclusion criterion matched? Then add this field to
        // the result list
        *relevantFields++ = field;
    }

    // terminate fields vector
    *relevantFields = NULL;
}