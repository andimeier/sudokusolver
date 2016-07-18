#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include "global.h"
#include "grid.h"
#include "logfile.h"
#include "util.h"
#include "box.h"
#include "container.h"


static char *getBoxName(unsigned index);
static int determineBoxContainer(unsigned x, unsigned y);
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

/**
 * determines the index of the box container which contains the field on the
 * given Sudoku coordinates
 * 
 * @param x X coordinate (starting with 0) of the specified field
 * @param y Y coordinate (starting with 0) of the specified field
 * @return index of the box container which contains the specified field, 
 *   or -1 if no such container contains the specified field (which is not
 *   possible with box containers, but might be possible for other types of
 *   containers)
 */
int determineBoxContainer(unsigned x, unsigned y) {
    assert(x >= 0 && x < MAX_NUMBER);
    assert(y >= 0 && y < MAX_NUMBER);
    assert(boxHeight > 0);
    assert(boxWidth > 0);

    return (y / boxHeight) * (MAX_NUMBER / boxWidth) + (x / boxWidth);
}

/**
 * return number of box containers necessary to hold the Sudoku data.
 * In many cases (like this) the number of containers of this type will be
 * equal to MAX_NUMBER, but in some cases it might not, e.g. for diagonals
 * there would be only 2 containers.
 * @return the number of needed containers of this type
 */
unsigned determineBoxContainersCount(void) {
    return MAX_NUMBER;
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
    assert(MAX_NUMBER == 9 || MAX_NUMBER == 4);
    
    if (MAX_NUMBER == 9) {
        boxWidth = 3;
        boxHeight = 3;
    }

    if (MAX_NUMBER == 4) {
        boxWidth = 2;
        boxHeight = 2;
    }


    instanceNames = (char **) xmalloc(sizeof (char *) * MAX_NUMBER);

    for (i = 0; i < MAX_NUMBER; i++) {
        sprintf(buffer, "box %u", i + 1);
        instanceNames[i] = strdup(buffer);
    }

    // check that the number of instance names is equal to the containers
    // count stated by the auxiliary count function
    assert(i == determineBoxContainersCount());

    // delegate container creation to generic generator function
    createContainers(BOXES, strdup("box"), MAX_NUMBER, instanceNames, containerSet);

    containerSet->getContainerIndex = &determineBoxContainer;
    containerSet->getContainerName = &getBoxName;

    // MAX_NUMBER boxes have been generated
    return MAX_NUMBER;
}
