/*
 * File:   hidden-singles.c
 * Author: aurez
 *
 * Created on 2016-07-03
 */
#include <stdio.h>
#include "global.h"
#include "grid.h"
#include "typedefs.h"
#include "util.h"
#include "logfile.h"
#include "fieldlist.h"
#include "hidden-singles.h"

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

    return progress;
}
