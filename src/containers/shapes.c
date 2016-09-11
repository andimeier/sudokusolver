#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "grid.h"
#include "logfile.h"
#include "util.h"
#include "shapes.h"
#include "container.h"


static char *getShapeName(unsigned index);
static void fillContainerFields(unsigned containerIndex, FieldsVector *fields);
static unsigned determineShapeContainersCount(void);

static unsigned *shapes;

/**
 * get the name of a shape container
 * 
 * @param index index of the shape container = number of shape
 * @return a newly allocated string representing the "label" of the shape 
 *   container, e.g. "shape 2"
 */
char *getShapeName(unsigned index) {
    assert(index >= 0 && index < 26);

    sprintf(buffer, "shape %u", index + 1);
    return strdup(buffer);
}

/**
 * fills the given fields vector with the list of fields which are members
 * of this container
 * 
 * @param containerIndex the index of the container which should be filled
 * @param containerFields pre-allocated vector of fields which will be filled by this
 *   function
 */
void fillContainerFields(unsigned containerIndex, FieldsVector *containerFields) {
    unsigned i;
    
    for (i = 0; i < numberOfFields; i++) {
        if (shapes[i] == containerIndex) {
            *containerFields = &(fields[i]);
            containerFields++;
        }
    }
}

/**
 * return number of shape containers necessary to hold the Sudoku data.
 * In many cases (like this) the number of containers of this type will be
 * equal to maxNumber, but in some cases it might not, e.g. for diagonals
 * there would be only 2 containers.
 * @return the number of needed containers of this type
 */
unsigned determineShapeContainersCount(void) {
    return maxNumber;
}

/**
 * creates a container set for shapees, along with all needed containers 
 * instances of this type
 * 
 * @param the container set structure to be filled with data
 * @return the number of generated container children of this container set
 */
unsigned createShapeContainers(ContainerSet *containerSet) {
 
    // delegate container creation to generic generator function
    createContainers(SHAPES, strdup("shape"), maxNumber, containerSet);

    containerSet->fillContainerFields = &fillContainerFields;
    containerSet->getContainerName = &getShapeName;

    // maxNumber shapees have been generated
    return maxNumber;
}

/**
 * creates the shapes definitions
 * 
 * @param shapes the shape definitions, as an array of numbers representing the
 *   shape IDs
 */
void setShapes(unsigned *_shapes) {
    shapes = _shapes;
}