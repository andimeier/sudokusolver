#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include "grid.h"
#include "logfile.h"
#include "util.h"
#include "box.h"
#include "container.h"


static char *getBoxName(unsigned index);
//static int determineBoxContainer(unsigned x, unsigned y);
static void fillContainerFields(unsigned containerIndex, FieldsVector *fields);
static unsigned determineBoxContainersCount(void);


static unsigned boxWidth;
static unsigned boxHeight;

/**
 * get the name of a box container
 * 
 * @param index index of the box container = number of box
 * @return a newly allocated string representing the "label" of the box 
 *   container, e.g. "box 2"
 */
char *getBoxName(unsigned index) {
    assert(index >= 0 && index < 26);

    sprintf(buffer, "box %u", index + 1);
    return strdup(buffer);
}

///**
// * determines the index of the box container which contains the field on the
// * given Sudoku coordinates
// * 
// * @param x X coordinate (starting with 0) of the specified field
// * @param y Y coordinate (starting with 0) of the specified field
// * @return index of the box container which contains the specified field, 
// *   or -1 if no such container contains the specified field (which is not
// *   possible with box containers, but might be possible for other types of
// *   containers)
// */
//int determineBoxContainer(unsigned x, unsigned y) {
//    assert(x >= 0 && x < maxNumber);
//    assert(y >= 0 && y < maxNumber);
//    assert(boxHeight > 0);
//    assert(boxWidth > 0);
//
//    return (y / boxHeight) * (maxNumber / boxWidth) + (x / boxWidth);
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
    unsigned boxStartX;
    unsigned boxStartY;
    unsigned boxX;
    unsigned boxY;

    boxStartX = (containerIndex % 3) * 3;
    boxStartY = (containerIndex / 3) * 3;
    
    for (n = 0; n < maxNumber; n++) {
        boxX = n % 3;
        boxY = n / 3;

        fields[n] = getFieldAt(boxStartX + boxX, boxStartY + boxY);
    }
}


/**
 * return number of box containers necessary to hold the Sudoku data.
 * In many cases (like this) the number of containers of this type will be
 * equal to maxNumber, but in some cases it might not, e.g. for diagonals
 * there would be only 2 containers.
 * @return the number of needed containers of this type
 */
unsigned determineBoxContainersCount(void) {
    return maxNumber;
}

/**
 * creates a container set for boxes, along with all needed containers 
 * instances of this type
 * 
 * @param the container set structure to be filled with data
 * @return the number of generated container children of this container set
 */
unsigned createBoxContainers(ContainerSet *containerSet) {
    char **instanceNames;
    unsigned i;

    // FIXME: at the moment, only two dimensions are possible:
    assert(maxNumber == 9 || maxNumber == 4);
    
    if (maxNumber == 9) {
        boxWidth = 3;
        boxHeight = 3;
    }

    if (maxNumber == 4) {
        boxWidth = 2;
        boxHeight = 2;
    }


    instanceNames = (char **) xmalloc(sizeof (char *) * maxNumber);

    for (i = 0; i < maxNumber; i++) {
        sprintf(buffer, "box %u", i + 1);
        instanceNames[i] = strdup(buffer);
    }

    // check that the number of instance names is equal to the containers
    // count stated by the auxiliary count function
    assert(i == determineBoxContainersCount());

    // delegate container creation to generic generator function
    createContainers(BOXES, strdup("box"), maxNumber, instanceNames, containerSet);

    containerSet->fillContainerFields = &fillContainerFields;
    containerSet->getContainerName = &getBoxName;

    // maxNumber boxes have been generated
    return maxNumber;
}
