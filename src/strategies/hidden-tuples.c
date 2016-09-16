/*
 * File:   hidden-tuples.c
 * Author: aurez
 *
 * Created on 2016-07-07
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
#include "numberlist.h"
#include "hidden-tuples.h"


// auxiliary functions
static Bool recurseHiddenTuples(unsigned maxLevel, Container *container, unsigned level, NumberList *includedCandidates, unsigned *candidatesLeft, FieldsVector *fieldsWithCandidates);
static Bool eliminateOtherCandidatesFromFields(FieldsVector *fields, unsigned *candidates);
static void populateCandidatesForHiddenTuples(unsigned *candidatesLeft, FieldsVector *allFields);
static Bool countDistinctFields(FieldsVector *fields, unsigned *candidates, size_t limit, FieldsVector *fieldsWithCandidates);

/**
 * find hidden tuples (pairs, triples, ...) in the same container
 * 
 * @return TRUE if something has changed, FALSE if not
 */
Bool findHiddenTuples() {
    Bool progress;
    Container *container;
    NumberList *includedCandidates;
    unsigned *candidatesLeft;
    FieldsVector *fieldsWithCandidates;


    logVerbose("[strategy] find hidden tuples ...");

    progress = FALSE;

    // allocate memory for strategy variables
    includedCandidates = createNumberList(MAX_TUPLE_DIMENSION);
    candidatesLeft = (unsigned *) xmalloc(sizeof (unsigned) * (maxNumber + 1));
    fieldsWithCandidates = (FieldsVector *) xmalloc(sizeof (Field *) * (maxNumber + 1));

    for (int dimension = 2; dimension <= MAX_TUPLE_DIMENSION; dimension++) {

        // go through all containers and find hidden tuples therein
        for (unsigned c = 0; c < numberOfContainers; c++) {
            container = &(allContainers[c]);
            sprintf(buffer, "-- next container: %s", container->name);
            logVerbose(buffer);

            progress |= findHiddenTuplesInContainer(container, dimension, includedCandidates, candidatesLeft, fieldsWithCandidates);
        }

        if (progress) {
            break;
        }
    }

    free(candidatesLeft);
    freeNumberList(includedCandidates);

    return progress;
}

/**
 * find hidden tuples (pairs, triples, ...) which share the same candidates.
 * For instance, if two fields have only the candidates 2 and 4, then 2 and 4
 * can be eliminated from all other fields in the same container.
 * Note: finds tuples of and only of the given dimension. For example,
 * if dimension == 3, then only hidden triples are found, but not hidden pairs 
 * (actually, hidden pairs will be identified as hidden triple with one "excessive
 * cell").
 * 
 * @param container vector of fields (=container) in which we look for hidden 
 *   tuples
 * @param dimension dimension of the tuple to be looked for. 2=pairs, 
 *   3=triples etc.
 * @param includedCandidates allocated buffer for the "found fields" during the
 *   algorithm, must be the size of the dimension (plus 1 for a NULL termination)
 * @param candidatesLeft allocated buffer for the vector of fields left (yet to be 
 *   examined), terminated with NULL, thus must be the size of the dimension
 *   plus 1 (for the NULL terminator)
 * @return progress flag: TRUE for "something has changed", FALSE for "no change"
 */
Bool findHiddenTuplesInContainer(Container *container, unsigned dimension, NumberList *includedCandidates, unsigned *candidatesLeft, FieldsVector *fieldsWithCandidates) {
    Bool progress;

    assert(dimension > 0 && dimension < maxNumber);

    progress = FALSE;

    // we are in level 0 of recursion: initialize numbers vector
    emptyNumberList(includedCandidates);

    // populate candidates to search for hidden tuples
    populateCandidatesForHiddenTuples(candidatesLeft, container->fields);

    if (recurseHiddenTuples(dimension, container, 1, includedCandidates, candidatesLeft, fieldsWithCandidates)) {
        // FIXME Optimierungsschritt: dieses gefundene hidden tuple merken, damit es nicht
        // in Zukunft jedesmal gefunden wird (aber ohne mehr etwas zu bewirken)
        progress = TRUE;
    } else {

        logVerbose("[1255] returned from recursion");
    }

    return progress;
}

/**
 * FIXME: see if description is valid since it has been copied from recurseNakedTuples
 *   with "naked" replace by "hidden"
 * recursively look for hidden tuples of the dimension maxLevel.
 * Note: finds tuples of and only of the given dimension. For example,
 * if dimension == 3, then only hidden triples are found, but not hidden pairs 
 * (actually, hidden pairs will be identified as hidden triple with one "excessive
 * cell").
 * 
 * @param maxLevel (beginning with 1) = dimension
 * @param container container in which we look for hidden tuples
 * @param level current recursion depth (starting with 1)
 * @param includedCandidates allocated buffer for the "found candidates" during 
 *   the algorithm, must be the size of the dimension (plus 1 for a NULL 
 *   termination)
 * @param candidatesLeft allocated buffer for the vector of candidates left 
 *   (yet to be examined), terminated with NULL, thus must be the size of the 
 *   dimension plus 1 (for the NULL terminator)
 * @return TRUE if a hidden tuple has been found, FALSE otherwise
 */
Bool recurseHiddenTuples(unsigned maxLevel, Container *container, unsigned level, NumberList *includedCandidates, unsigned *candidatesLeft, FieldsVector *fieldsWithCandidates) {
    unsigned *left;

    assert(level >= 1);
    //FIXME war fuer alten Algo, gehts fuer den neuen auch mit so etwas?    assert(level == 1 || (level >= 2 && numbers[level - 2] != 0));

    if (level > maxLevel) {
        // maximum recursion depth reached => nothing found
        sprintf(buffer, "maximum recursion depth of %u reached.", maxLevel);
        logVerbose(buffer);
        return FALSE;
    }

//    sc(container); // FIXME remove me

    sprintf(buffer, "Entering recursion level %u/%u ...", level, maxLevel);

    logVerbose(buffer);

    //    // prepare list terminations *after* current list item
    //    numbers[level] = 0;
    //    foundFields[level] = NULL;

    left = candidatesLeft;

    // go through all left fields
    while (*left) {
        pushToNumberList(includedCandidates, *left++);

        // on level 1, don't try to find nake "tuples", only start at level 2
        if (level == 1 || countDistinctFields(container->fields, includedCandidates->numbers, maxLevel, fieldsWithCandidates)) {
            // hm ... yes ... hm ... might contribute to a hidden tuple ...
            if (includedCandidates->count < maxLevel) {
                // recurse further
                if (recurseHiddenTuples(maxLevel, container, level + 1, includedCandidates, left, fieldsWithCandidates)) {
                    // propagate success flag all levels down
                    return TRUE;
                }
            } else {
                /*
                 *  found hidden tuple!
                 * we reached maximum recursion depth (= dimension) and the number
                 * of found fields for the given candidates is equal to or lower
                 * than the number of candidates
                 */

                // depending on whether some candidates could be eliminated, the
                // board has changed or not
                if (eliminateOtherCandidatesFromFields(fieldsWithCandidates, includedCandidates->numbers)) {
                    // something has changed! success, we actually found something!
                    return TRUE;
                } else {
                    // restore candidates list to previous iteration
                    popFromNumberList(includedCandidates);
                    return FALSE;
                }

            }
        }

        popFromNumberList(includedCandidates);
    }


    //    // take back extensions of the vectors from the current level
    //    numbers[level - 1] = 0;
    //    foundFields[level - 1] = NULL;

    sprintf(buffer, "leaving recursion level %d/%d, going back one level\n", level, maxLevel);
    logVerbose(buffer);

    return FALSE; // nothing found
}

/**
 * eliminate other candidates than the given ones from the given fields.
 * This is a utility function for the strategy "find hidden tuples", where
 * the candidates of the hidden tuple must be remain the only candidates
 * of the fields forming the hidden tuple.
 * 
 * @param fields the fields to be reduced to their "hidden tuple" candidates
 * @param candidates the candidates which the fields should be reduced to. All
 *   other candidates will be removed.
 * @return progress flag: 1 if something has changed (candidates eliminated) or
 *   0 if not
 */
Bool eliminateOtherCandidatesFromFields(FieldsVector *fields, unsigned *candidates) {
    unsigned *candidatesPtr;
    int progress;

    progress = FALSE;

    while (*fields) {

        candidatesPtr = candidates;
        for (unsigned n = 1; n <= maxNumber; n++) {

            while (*candidatesPtr && *candidatesPtr < n) {
                // "catch up" to loop variable
                candidatesPtr++;
            }

            if (*candidatesPtr != n) {
                // candidate should not be preserved => remove it
                progress |= removeCandidate(*fields, n);
            }
        }

        fields++;
    }

    return progress;
}

/**
 * aux function whose only purpose is to populate the candidates which the
 * algorithm should take into account when searching for hidden tuples.
 * The function takes all candidates first, and then removes certain candidates
 * because it is clear that they cannot play a role when searching for a 
 * hidden tuple of the given dimension.
 * 
 * Example: a field is already solved => ignore this candidate
 * 
 * The purpose of this function is to remove the number of candidates which must
 * be considered by the search algorithm (performance issue).
 * 
 * @param candidatesLeft list of relevant candidates for the algorithm (this is 
 *   the output = the result)
 * @param allFields list of fields from which the remaining candidates should be
 *   determined.
 *   Note that this is not NULL-terminated - instead, it contains maxNumber
 *   fields (= all fields of the container to be examined)
 */
void populateCandidatesForHiddenTuples(unsigned *candidatesLeft, FieldsVector *allFields) {
    Field *field;
    int i;

    // initialize left candidates with ALL possible candidates
    for (i = 0; i < maxNumber; i++) {
        candidatesLeft[i] = 1; // start with all candidates being possible
    }

    for (i = 0; i < maxNumber; i++) {
        field = *allFields++;

        if (!field) {
            // in case of NULL-terminated list allFields, exit if the 
            // terminating NULL is found (just a safeguard, since the loop stops
            // after maxNumber fields anyway)
            break;
        }

        /*
         * in case of a solved fields, remove its solution from the list of
         * possible candidates
         */
        if (field->value) {
            candidatesLeft[field->value - 1] = 0;
            continue;
        }
    }

    // compact the list of candidates
    unsigned *compact = candidatesLeft;
    for (int i = 0; i < maxNumber; i++) {
        if (candidatesLeft[i]) {
            *compact++ = i + 1;
        }
    }
    // terminate list of candidates
    *compact = 0;
}

/**
 * counts the number of fields which contain at least one of the given 
 * candidates and checks if the number of such fields is not greater than
 * the given limit.
 * 
 * @param fields list of fields to search for. CAUTION: since this is a field
 *   list of a container, it is NOT a NULL-terminated list!
 * @param fields list of candidates for which the fields should be counted
 * @param limit the maximum number of total distinct fields
 * @param fieldsWithCandidates vector of all fields containing the given 
 *   candidates (output of this function)
 * @return TRUE if successful, FALSE if not
 */
Bool countDistinctFields(FieldsVector *fields, unsigned *candidates, size_t limit, FieldsVector *fieldsWithCandidates) {
    size_t fieldsIndex;
    size_t count;
    FieldsVector *fieldsPtr;
    int i;
    unsigned *fieldsSet;

    // TODO could already be pre-allocated by the strategy (performance optimisation))
    // ... a strategy-local buffer so to speak ...
    fieldsSet = (unsigned *) xmalloc(sizeof (unsigned) * maxNumber);

    // initialize all candidate counters with 0    
    for (i = 0; i < maxNumber; i++) {
        fieldsSet[i] = 0;
    }

    count = 0;
    while (*candidates) {
        fieldsPtr = fields;
        fieldsIndex = 0;

        // iterate through all fields
        for (i = 0; i < maxNumber; i++) {

            // only analyse unsolved fields
            if (!(*fieldsPtr)->value) {
                // count 
                if ((*fieldsPtr)->candidates[*candidates - 1]) {

                    if (!fieldsSet[fieldsIndex]) {
                        if (count >= limit) {
                            // too many fields (adding this field to the
                            // set would exceed the given limit)
                            return FALSE;
                        }

                        fieldsSet[fieldsIndex]++;
                        count++;
                    }
                }
            }

            fieldsPtr++;
            fieldsIndex++;
        }

        candidates++;
    }

    // no break until now => must be success

    // build list of found fields by compacting the fieldsSet
    for (i = 0; i < maxNumber; i++) {
        if (fieldsSet[i]) {
            *fieldsWithCandidates++ = fields[i];
        }
    }
    // terminate list of candidates
    *fieldsWithCandidates = 0;

    return TRUE;
}