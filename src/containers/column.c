#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include "grid.h"
#include "logfile.h"
#include "util.h"
#include "column.h"
#include "container.h"

static char *getColumnName(unsigned index);
//static int determineColumnContainer(unsigned x, unsigned y);
static void fillContainerFields(unsigned containerIndex, FieldsVector *fields);

/**
 * get the name of a column container
 * 
 * @param index index of the column container = number of colum
 * @return a newly allocated string representing the "label" of the column
 *   container, e.g. "column 2"
 */
char *getColumnName(unsigned index) {
    assert(index >= 0 && index < 26);

    sprintf(buffer, "column %u", index + 1);
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
void fillContainerFields(unsigned containerIndex, FieldsVector * fields) {
    unsigned x;
    unsigned y;

    x = containerIndex;
    for (y = 0; y < maxNumber; y++) {
        fields[y] = getFieldAt(x, y);
    }
}

/**
 * creates a container set for columns, along with all needed containers 
 * instances of this type
 * 
 * @param the container set structure to be filled with data
 * @return the number of generated container children of this container set
 */
unsigned createColumnContainers(ContainerSet * containerSet) {

    // delegate container creation to generic generator function
    createContainers(COLS, strdup("column"), maxNumber, containerSet);

    containerSet->fillContainerFields = &fillContainerFields;
    containerSet->getContainerName = &getColumnName;

    // maxNumber columns have been generated
    return maxNumber;
}

