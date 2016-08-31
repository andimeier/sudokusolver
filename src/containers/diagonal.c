#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include "grid.h"
#include "logfile.h"
#include "util.h"
#include "diagonal.h"
#include "container.h"


static char *getDiagonalName(unsigned index);
//static int determineDiagonalContainer(unsigned x, unsigned y);
static void fillContainerFields(unsigned containerIndex, FieldsVector *fields);
static unsigned determineDiagonalContainersCount(void);

/**
 * get the name of a diagonal container
 * 
 * @param index index of the diagonal container = number of diagonal
 * @return a newly allocated string representing the "label" of the diagonal
 *   container, e.g. "falling diagonal"
 */
char *getDiagonalName(unsigned index) {
    assert(index >= 0 && index <= 1);

    switch (index) {
        case 0:
            return strdup("falling diagonal");
            break;
        case 1:
            return strdup("ascending diagonal");
            break;
    }

    assert(0); // we should never get here
}

///**
// * determines the index of the diagonal container which contains the field on 
// * the given Sudoku coordinates
// * 
// * @param x X coordinate (starting with 0) of the specified field
// * @param y Y coordinate (starting with 0) of the specified field
// * @return index of the diagonal container which contains the specified field, 
// *   or -1 if no such container contains the specified field (a field that 
// *   does not lie on one of the two diagonals)
// */
//int determineDiagonalContainer(unsigned x, unsigned y) {
//    assert(x >= 0 && x < maxNumber);
//    assert(y >= 0 && y < maxNumber);
//
//    if (x == y) {
//        return 0; // on falling diagonal
//    }
//    if (x + y + 1 == maxNumber) {
//        return 1; // on ascending diagonal
//    }
//    return -1;
//}

/**
 * fills the given fields vector with the list of fields which are members
 * of this container
 * 
 * @param containerIndex the index of the container which should be filled
 * @param fields pre-allocated vector of fields which will be filled by this
 *   function
 */
void fillContainerFields(unsigned containerIndex, FieldsVector *fields) {
    unsigned n;
    unsigned x;
    unsigned y;

    for (n = 0; n < maxNumber; n++) {
        x = n;
        y = (containerIndex == 0) ?
                x : // index 0 ... falling diagonal
                maxNumber - 1 - x; // index 1 ... ascending diagonal 

        fields[n] = getFieldAt(x, y);
    }
}

/**
 * return number of diagonal containers necessary to hold the Sudoku data.
 * There are only 2 diagonals.
 * @return the number of needed containers of this type
 */
unsigned determineDiagonalContainersCount(void) {
    return 2;
}

/**
 * creates a container set for diagonals, along with all needed containers 
 * instances of this type
 * 
 * @param the container set structure to be filled with data
 * @return the number of generated container children of this container set
 */
unsigned createDiagonalContainers(ContainerSet *containerSet) {
    char **instanceNames;

    instanceNames = (char **) xmalloc(sizeof (char *) * 2);

    instanceNames[0] = strdup("falling diagonal");
    instanceNames[1] = strdup("asending diagonal");

    // check that the number of instance names is equal to the containers
    // count stated by the auxiliary count function
    assert(2 == determineDiagonalContainersCount());

    // delegate container creation to generic generator function
    createContainers(DIAGONALS, strdup("diagonals"), 2, instanceNames, containerSet);

    containerSet->fillContainerFields = &fillContainerFields;
    containerSet->getContainerName = &getDiagonalName;

    // 2 diagonals have been generated
    return 2;
}
