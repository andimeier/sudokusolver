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
static unsigned determineColumnContainersCount(void);

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


///**
// * determines the index of the column container which contains the field on the
// * given Sudoku coordinates
// * 
// * @param x X coordinate (starting with 0) of the specified field
// * @param y Y coordinate (starting with 0) of the specified field
// * @return index of the column container which contains the specified field, 
// *   or -1 if no such container contains the specified field (which is not
// *   possible with column containers, but might be possible for other types of
// *   containers)
// */
//int determineColumnContainer(unsigned x, unsigned y) {
//    return x;
//}

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
 * return number of column containers necessary to hold the Sudoku data.
 * In many cases (like this) the number of containers of this type will be
 * equal to maxNumber, but in some cases it might not, e.g. for diagonals
 * there would be only 2 containers.
 * @return the number of needed containers of this type
 */
unsigned determineColumnContainersCount(void) {
    return maxNumber;
}

/**
 * creates a container set for columns, along with all needed containers 
 * instances of this type
 * 
 * @param the container set structure to be filled with data
 * @return the number of generated container children of this container set
 */
unsigned createColumnContainers(ContainerSet * containerSet) {
    char **instanceNames;
    unsigned i;

    instanceNames = (char **) xmalloc(sizeof (char *) * maxNumber);

    for (i = 0; i < maxNumber; i++) {
        sprintf(buffer, "column %u", i + 1);
        instanceNames[i] = strdup(buffer);
    }

    // check that the number of instance names is equal to the containers
    // count stated by the auxiliary count function
    assert(i == determineColumnContainersCount());

    // delegate container creation to generic generator function
    createContainers(COLS, strdup("column"), maxNumber, instanceNames, containerSet);

    containerSet->fillContainerFields = &fillContainerFields;
    containerSet->getContainerName = &getColumnName;

    // maxNumber columns have been generated
    return maxNumber;
}

