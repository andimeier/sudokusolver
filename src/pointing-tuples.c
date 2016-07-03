/*
 * File:   pointing-tuples.c
 * Author: aurez
 *
 * Created on 2016-07-03
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "global.h"
#include "show.h"
#include "grid.h"
#include "typedefs.h"
#include "util.h"
#include "grid.h"
#include "log.h"
#include "fieldlist.h"
#include "pointing-tuples.h"

// aux functions for pointing tuples
static int eliminateCandidatesFromOtherFields(Container *container, FieldsVector *fields, unsigned candidate);


/**
 * find pointing pairs or triples or whatever dimension. Pointing tuples
 * are a number of fields in a container which share the same container of 
 * another container type. Example: if in a box the number 3 can occur in two
 * fields of the box AND these two fields share also the same row, then all 
 * other ocurrences of the number 3 in the rest of the row can be eliminated.
 * 
 * @return progress flag: 1 for "something has changed", 0 for "no change"
 */
int findPointingTuples() {
    //ContainerSet *unit;
    int progress; // flag: something has changed
    Container *container;
    FieldsVector *fieldsBuffer;
    //unsigned tuple[MAX_NUMBER];
    //unsigned n;

    progress = 0;


    printf("[pii] starting findPointingTuples ...\n");

    fieldsBuffer = (FieldsVector *) xmalloc(sizeof (Field *) * (MAX_NUMBER + 1));

    // search in all containers (be it rows, cols, boxes, ...) for a tuple of 
    // numbers which form a "pointing tuple"

    // go through all containers and find naked tuples therein
    for (unsigned c = 0; c < numberOfContainers; c++) {
        container = &(allContainers[c]);
        sprintf(buffer, "-- next container: %s", container->name);
        logVerbose(buffer);

        progress |= findPointingTuplesInContainer(container, fieldsBuffer);
    }

    return progress;

}

/**
 * find pointing pairs or triples or whatever dimension in the specified
 * container. 
 * 
 * @param container container in which we look for pointing tuples
 * @param fieldsWithCandidate fields containing a candidate. This is a 
 *   pre-allocated buffer to be used by this strategy. Performance issue, so 
 *   that not every iteration has to allocate buffer, but a "common" buffer is
 *   used
 * @return progress flag: 1 for "something has changed", 0 for "no change"
 */
unsigned findPointingTuplesInContainer(Container *container, FieldsVector *fieldsWithCandidate) {
    unsigned progress;
    ContainerSet *containerSet;
    Container *commonContainer;
    FieldsVector *fieldsWithCandidatePtr;

    progress = 0;

    /*
     * strategy: for each container (for the given container in this function),
     * check all "free" numbers whether all occurrences of such candidate is
     * in the same container of another container type. If so, eliminate this
     * candidate in the other container in all other fields than the fields
     * of the "first" container.
     */

    // try all numbers
    for (unsigned n = 1; n < MAX_NUMBER; n++) {
        // TODO optimisation step: skip numbers which are already solved in the
        // container

        // gather all fields of the container which contain this candidate
        fieldsContainingCandidate(fieldsWithCandidate, container->fields, n);
        fieldsWithCandidatePtr = fieldsWithCandidate;

        if (*fieldsWithCandidatePtr) {
            // iterate through all container types and see if all fields share
            // the same "other" container

            for (size_t set = 0; set < numberOfContainerSets; set++) {
                containerSet = &(containerSets[set]);

                // TODO optimisation: some combination of container sets are not
                // relevant. E.g. tuples in a row can never "point" to the same
                // row for all fields. Fields in a row can never share the same
                // diagonal. Fields in a column can never share the same 
                // diagonal. So these combinations of container sets can safely
                // be skipped. But these combinations (or the allowed 
                // combinations) must be defined somewhere ...

                // skip current container (container does not need to be 
                // compared with itself), we only want to look in "other"
                // containers
                if (containerSet->type == container->type) {
                    continue;
                }

                // check if all fields are in the same "other" container
                commonContainer = getCommonContainer(fieldsWithCandidate, set);

                if (commonContainer) {
                    // all fields are in the same "other" container => try to
                    // eliminate this candidate in the other container.
                    // Depending on whether some candidates could be eliminated, the
                    // board has changed or not
                    return eliminateCandidatesFromOtherFields(commonContainer, fieldsWithCandidate, n);
                }
            }
        }
    }

    return progress;
}


/**
 * eliminate the given candidate of the given fields in the rest of the container.
 * This is a utility function for the strategy "find pointing tuples", where
 * a found candidate of a pointing tuple must be removed from all other fields in
 * the container except from the ones forming the pointing tuple.
 * 
 * @param container the container in which candidates should be eliminated
 * @param fields the fields to be preserved. The candidates of these fields are
 *   not touched
 * @param candidate the candidate to be removed from the other fields
 * @return progress flag: 1 if something has changed (candidates eliminated) or
 *   0 if not
 */
int eliminateCandidatesFromOtherFields(Container *container, FieldsVector *fields, unsigned candidate) {
    unsigned *candidates;
    int progress;

    candidates = (unsigned *) xmalloc(sizeof (unsigned) * 2);
    candidates[0] = candidate;
    candidates[1] = 0;

    // forbid candidates in all other fields
    progress = forbidNumbersInOtherFields(container, candidates, fields);

    free(candidates);

    return progress;
}
