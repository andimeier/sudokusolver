/*
 * File:   hidden-singles.c
 * Author: aurez
 *
 * Created on 2016-07-03
 */
#include <stdio.h>
#include "grid.h"
#include "typedefs.h"
#include "util.h"
#include "logfile.h"
#include "fieldlist.h"
#include "hidden-singles.h"
#include "recorder.h"

// solve path recorder: record naked tuple
typedef struct {
    Container *container;
    Field *field;
    unsigned candidate;
} StepFoundHiddenSingle;


static void recordFoundHiddenSingleStart(Container *container, Field *field, unsigned candidate);
static void recordFoundHiddenSingleEnd();
static void printFoundHiddenSingle(char *msgBuffer, STEP_TYPE stepType, void *info);

/**
 * strategy "find hidden singles"
 * 
 * @return TRUE if something has changed, FALSE if not
 */
Bool findHiddenSingles() {
    Bool progress; // flag: something has changed
    Container *container;

    logVerbose("[strategy] find hidden singles ...");

    progress = FALSE;

    // search in all unit types (rows, cols, boxes, ...) for numbers which can 
    // only occur on one position within the unit (even if there would be
    // several candidates for this cell, but the other candidates can be
    // discarded in this case)

    for (unsigned c = 0; c < numberOfContainers; c++) {
        container = &(allContainers[c]);

        for (unsigned n = 1; n <= maxNumber; n++) {
            Field **containerFields = container->fields;
            int pos = getUniquePositionInContainer(containerFields, n);
            if (pos != -1 && !containerFields[pos]->value) {
                Field *field = containerFields[pos];

                // number can only occur in the position pos in this container
                recordFoundHiddenSingleStart(container, field, n);

                setValue(field, n);
 
                recordFoundHiddenSingleEnd();
                
                sprintf(buffer, "*** [hidden single] hidden single in unit %s, field %s: %u ... ", container->name, field->name, n);
                logVerbose(buffer);

                progress = TRUE; // set flag "changes!"
            }
        }
        container++;
    }

    return progress;
}

/**
 * records the start of the strategy finding "naked tuple"
 * 
 * @param container
 * @param field
 * @param candidate
 */
void recordFoundHiddenSingleStart(Container *container, Field *field, unsigned candidate) {
    StepFoundHiddenSingle *info;

    info = (StepFoundHiddenSingle *) xmalloc(sizeof (StepFoundHiddenSingle));

    info->container = container;
    info->field = field;
    info->candidate = candidate;
    
    recordStartOfStrategyFinding(printFoundHiddenSingle, (void *) info);
}

void recordFoundHiddenSingleEnd() {
    recordEndOfStrategyFinding();
}


void printFoundHiddenSingle(char *msgBuffer, STEP_TYPE stepType, void *info) {
    StepFoundHiddenSingle *infoStruct;
    
    infoStruct = (StepFoundHiddenSingle *) info;
       
    sprintf(msgBuffer, "found hidden single in %s:\n"
            "candidate %u can only occur in %s\n", 
            infoStruct->container->name,
            infoStruct->candidate,
            infoStruct->field->name);
}