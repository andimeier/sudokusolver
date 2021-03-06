#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include "grid.h"
#include "logfile.h"
#include "util.h"
#include "row.h"
#include "container.h"

static char *getRowName(unsigned index);
//static int determineRowContainer(unsigned x, unsigned y);
static void fillContainerFields(unsigned containerIndex, FieldsVector *fields);

/**
 * get the name of a row container
 * 
 * @param index index of the row container = number of row
 * @return a newly allocated string representing the "label" of the row 
 *   container, e.g. "row 2"
 */
char *getRowName(unsigned index) {
    assert(index >= 0 && index < 26);

    sprintf(buffer, "row %c", (char) ('A' + index));
    return strdup(buffer);
}


/**
 * fills the given fields vector with the list of fields which are members
 * of this container
 * 
 * @param containerIndex the index of the container which should be filled
 * @param fields pre-allocated vector of fields which will be filled by this
 *   function
 */
void fillContainerFields(unsigned containerIndex, FieldsVector *fields) {
    unsigned x;
    unsigned y;

    y = containerIndex;
    for (x = 0; x < maxNumber; x++) {
        fields[x] = getFieldAt(x, y);
    }
}


/**
 * creates a container set for rows, along with all needed containers instances
 * of this type
 * 
 * @param the container set structure to be filled with data
 * @return the number of generated container children of this container set
 */
unsigned createRowContainers(ContainerSet *containerSet) {

    // delegate container creation to generic generator function
    createContainers(ROWS, strdup("row"), maxNumber, containerSet);

    containerSet->fillContainerFields = &fillContainerFields;
    containerSet->getContainerName = &getRowName;

    // maxNumber rows have been generated
    return maxNumber;
}

