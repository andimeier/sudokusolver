/*
 * File:   naked-triples.c
 * Author: aurez
 *
 * Created on 2016-07-03
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "show.h"
#include "grid.h"
#include "typedefs.h"
#include "util.h"
#include "grid.h"
#include "logfile.h"
#include "solve.h"
#include "fieldlist.h"
#include "naked-tuples.h"
#include "recorder.h"
#include "util.h"

/*
 * Optimisations:
 * 
 * find naked tuples:
 * - skip fields with candidatesLeft > maxTupleDimensionToLookFor
 */

// solve path recorder: record naked tuple

typedef struct {
    Container *container;
    unsigned dimension; // dimension of the naked tuple, 2 == pair etc.
    FieldsVector *fields;
    unsigned *candidates;
} StepFoundNakedTuple;


// auxiliary functions
static Bool recurseNakedTuples(unsigned maxLevel, Container *container, unsigned level, FieldList *includedFields, FieldsVector *fieldsLeft);
static Bool eliminateFieldsCandidatesFromOtherFields(Container *container, FieldsVector *fields);
static void populateFieldsForNakedTuples(FieldsVector *relevantFields, FieldsVector *allFields, unsigned dimension);

// aux functions for the solve path recorder
void recordFoundNakedTupleStart(unsigned dimension, Container *container, FieldsVector *fields);
void recordFoundNakedTupleEnd();
void printFoundNakedTuple(char *msgBuffer, STEP_TYPE stepType, void *info);

/**
 * find naked tuples (pairs, triples, ...) in the same container
 * 
 * @return TRUE if something has changed, FALSE if not
 */
Bool findNakedTuples() {
    Bool progress;
    Container *container;
    FieldList *includedFields;
    FieldsVector *fieldsLeft;

    logVerbose("[strategy] find naked tuples ...");

    progress = FALSE;

    // allocate memory for strategy variables
    includedFields = createFieldList(MAX_TUPLE_DIMENSION);
    fieldsLeft = (FieldsVector *) xmalloc(sizeof (Field *) * (maxNumber + 1));

    for (int dimension = 2; dimension <= MAX_TUPLE_DIMENSION; dimension++) {

        // go through all containers and find naked tuples therein
        for (unsigned c = 0; c < numberOfContainers; c++) {
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

/**
 * find naked tuples (pairs, triples, ...) which share the same candidates.
 * For instance, if two fields have only the candidates 2 and 4, then 2 and 4
 * can be eliminated from all other fields in the same container.
 * Note: finds tuples of and only of the given dimension. For example,
 * if dimension == 3, then only naked triples are found, but not naked pairs 
 * (actually, naked pairs will be identified as naked triple with one "excessive
 * cell").
 * 
 * @param container vector of fields (=container) in which we look for naked 
 *   tuples
 * @param dimension dimension of the tuple to be looked for. 2=pairs, 
 *   3=triples etc.
 * @param includedFields allocated buffer for the "found fields" during the
 *   algorithm, must be the size of the dimension (plus 1 for a NULL termination)
 * @param fieldsLeft allocated buffer for the vector of fields left (yet to be 
 *   examined), terminated with NULL, thus must be the size of the dimension
 *   plus 1 (for the NULL terminator)
 * @return progress flag: TRUE for "something has changed", FALSE for "no change"
 */
Bool findNakedTuplesInContainer(Container *container, unsigned dimension, FieldList *includedFields, FieldsVector *fieldsLeft) {
    Bool progress;

    assert(dimension > 0 && dimension < maxNumber);

    progress = FALSE;

    // we are in level 0 of recursion: initialize numbers vector
    emptyFieldList(includedFields);

    // populate fields to search for naked tuples
    populateFieldsForNakedTuples(fieldsLeft, container->fields, dimension);

    if (recurseNakedTuples(dimension, container, 1, includedFields, fieldsLeft)) {
        // FIXME Optimierungsschritt: dieses gefundene naked tuple merken, damit es nicht
        // in Zukunft jedesmal gefunden wird (aber ohne mehr etwas zu bewirken)
        progress = TRUE;
    } else {

        logVerbose("[1244] returned from recursion");
    }

    return progress;
}

/**
 * recursively look for naked tuples of the dimension maxLevel.
 * Note: finds tuples of and only of the given dimension. For example,
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
 * @return TRUE if a naked tuple has been found, FALSE otherwise
 */
Bool recurseNakedTuples(unsigned maxLevel, Container *container, unsigned level, FieldList *includedFields, FieldsVector *fieldsLeft) {
    FieldsVector *left;
    Bool result;

    assert(level >= 1);
    //FIXME war fuer alten Algo, gehts fuer den neuen auch mit so etwas?    assert(level == 1 || (level >= 2 && numbers[level - 2] != 0));

    if (level > maxLevel) {
        // maximum recursion depth reached => nothing found
        sprintf(buffer, "maximum recursion depth of %u reached.", maxLevel);
        logVerbose(buffer);
        return FALSE;
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
                    return TRUE;
                }
            } else {
                /*
                 *  found naked tuple!
                 * we reached maximum recursion depth (= dimension) and the number
                 * of found candidates with a maximum total sum of "dimension"
                 * candidates is maximum "dimension"
                 */

                recordFoundNakedTupleStart(level, container, includedFields->fields);

                // depending on whether some candidates could be eliminated, the
                // board has changed or not
                if (eliminateFieldsCandidatesFromOtherFields(container, includedFields->fields)) {
                    // something has changed! success, we actually found something!
                    result = TRUE;
                } else {
                    // restore field list to previous iteration
                    popFromFieldList(includedFields);
                    result = FALSE;
                }

                recordFoundNakedTupleEnd();

                return result;
            }
        }

        popFromFieldList(includedFields);
    }


    //    // take back extensions of the vectors from the current level
    //    numbers[level - 1] = 0;
    //    foundFields[level - 1] = NULL;

    sprintf(buffer, "leaving recursion level %d/%d, going back one level\n", level, maxLevel);
    logVerbose(buffer);

    return FALSE; // nothing found
}

/**
 * eliminate any candidates of the given fields in the rest of the container.
 * This is a utility function for the strategy "find naked tuples", where
 * the candidates of the naked tuple must be removed from all other fields in
 * the container except from the ones forming the naked tuple.
 * 
 * @param container the container in which candidates should be eliminated
 * @param fields the fields of which the candidates should be taken. These 
 *   candidates are forbidden in all of the *other* fields of the container
 * @return progress flag: TRUE if something has changed (candidates eliminated) or
 *   FALSE if not
 */
Bool eliminateFieldsCandidatesFromOtherFields(Container *container, FieldsVector *fields) {
    unsigned *candidates;
    Bool progress;
    FieldsVector *fieldsPtr;

    // add 1 item as a zero termination in the purely theoretical case that a
    // naked "nonuple" was found (all possible numbers of a Sudoku)
    candidates = (unsigned *) xmalloc(sizeof (unsigned) * (maxNumber + 1));
    for (int i = 0; i < maxNumber; i++) {
        candidates[i] = 0;
    }

    // find out all candidates
    fieldsPtr = fields;
    while (*fieldsPtr) {

        // retrieve all candidates from the field
        for (int i = 0; i < maxNumber; i++) {
            candidates[i] |= (*fieldsPtr)->candidates[i];
        }
        fieldsPtr++;
    }

    // compact candidates to fit the parameter for "forbidNumbersInOtherFields"
    unsigned *compact = candidates;
    for (int i = 0; i < maxNumber; i++) {
        if (candidates[i]) {
            *compact++ = candidates[i];
        }
    }
    // terminate list of candidates
    *compact = 0;

    // forbid candidates in all other fields
    progress = forbidCandidatesInOtherFields(container, candidates, fields);

    free(candidates);

    return progress;
}

/**
 * aux function whose only purpose is to populate the fields in which the
 * algorithm should search for naked tuples.
 * The function takes all fields first, and then removes certain fields
 * because it is clear that they cannot play a role when searching for a 
 * naked tuple of the given dimension.
 * 
 * Example: a field which is already solved, is filtered out. A field containing
 * 4 or more candidates is filtered out when searching for a naked triple.
 * 
 * The purpose of this function is to remove the number of fields which must
 * be considered by the search algorithm (performance issue).
 * 
 * @param relevantFields list of fields with the relevant fields (this is the
 *   output = the result)
 * @param allFields container from which all or some fields form the result.
 *   Note that this is not NULL-terminated - instead, it contains maxNumber
 *   fields (= all fields of the container to be examined)
 * @param dimension the dimension of the tuples to search for. E.g. 2 means
 *   "search for naked pairs", 3 means "naked triples" etc.
 */
void populateFieldsForNakedTuples(FieldsVector *relevantFields, FieldsVector *allFields, unsigned dimension) {
    Field *field;
    int i;

    for (i = 0; i < maxNumber; i++) {
        field = *allFields++;

        if (!field) {
            // in case of NULL-terminated list allFields, exit if the 
            // terminating NULL is found (just a safeguard, since the loop stops
            // after maxNumber fields anyway)
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

/**
 * records the start of the strategy finding "naked tuple"
 * 
 * @param dimension
 * @param container
 * @param fields
 */
void recordFoundNakedTupleStart(unsigned dimension, Container *container, FieldsVector *fields) {
    StepFoundNakedTuple *info;

    info = (StepFoundNakedTuple *) xmalloc(sizeof (StepFoundNakedTuple));

    info->container = container;
    info->dimension = dimension;
    info->fields = (FieldsVector *) xmalloc(dimension * sizeof(Field *));
    memcpy(info->fields, fields, dimension * sizeof(Field *));
    info->candidates = getCandidates(fields[0]);
    
    recordStartOfStrategyFinding(printFoundNakedTuple, (void *) info);
}

void recordFoundNakedTupleEnd() {
    recordEndOfStrategyFinding();
}

void printFoundNakedTuple(char *msgBuffer, STEP_TYPE stepType, void *info) {
    StepFoundNakedTuple *infoStruct;
    
    infoStruct = (StepFoundNakedTuple *) info;
       
    sprintf(msgBuffer, "found naked tuple of dimension %u in %s:\n"
            "fields %s/%s share the same candidates: [ %u, %u ]\n", 
            infoStruct->dimension, 
            infoStruct->container->name,
            infoStruct->fields[0]->name, infoStruct->fields[1]->name,
            infoStruct->candidates[0], infoStruct->candidates[1]);
}