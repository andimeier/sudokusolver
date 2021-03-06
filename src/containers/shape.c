#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "grid.h"
#include "logfile.h"
#include "util.h"
#include "shape.h"
#include "container.h"


static char *getShapeName(unsigned index);
static void fillContainerFields(unsigned containerIndex, FieldsVector *fields);

static char *shapeIds;
static char *shapes;

/**
 * get the name of a shape container
 * 
 * @param index index of the shape container = number of shape
 * @return a newly allocated string representing the "label" of the shape 
 *   container, e.g. "shape 2"
 */
char *getShapeName(unsigned index) {
    assert(index >= 0 && index < 26);

    sprintf(buffer, "shape %c", shapeIds[index]);
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
    char shapeId;
    
    shapeId = shapeIds[containerIndex];
    
    assert(shapeId != '\0');
    
    for (i = 0; i < numberOfFields; i++) {
        if (shapes[i] == shapeId) {
            *containerFields = &(fields[i]);
            containerFields++;
        }
    }
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

    // maxNumber shapes have been generated
    return maxNumber;
}

/**
 * creates the shapes definitions
 * 
 * @param shapeIds the shape IDs as a null-terminated string
 * @param shapes the shape definitions, as an array of numbers representing the
 *   shape IDs
 */
void setShapes(char *_shapeIds, char *_shapes) {
    shapes = _shapes;
    shapeIds = _shapeIds;
}
