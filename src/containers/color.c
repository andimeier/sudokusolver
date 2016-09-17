#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "grid.h"
#include "logfile.h"
#include "util.h"
#include "color.h"
#include "container.h"
#include "box.h"


static char *getColorName(unsigned index);
static void fillContainerFields(unsigned containerIndex, FieldsVector *fields);

static unsigned boxWidth = 0;
static unsigned boxHeight = 0;

/**
 * get the name of a color container
 * 
 * @param index index of the color container = number of color
 * @return a newly allocated string representing the "label" of the color
 *   container, e.g. "color 2"
 */
char *getColorName(unsigned index) {
    assert(index >= 0 && index < 26);

    sprintf(buffer, "color %u", index + 1);
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
    unsigned n;
    unsigned colorX;
    unsigned colorY;

    // determine position of first field of this color (in box 1)
    colorX = containerIndex % boxWidth;
    colorY = containerIndex / boxWidth;

    for (n = 0; n < maxNumber; n++) {
        fields[n] = getFieldAt(colorX, colorY);
        sprintf(buffer, "color container %u: add field at %u/%u", containerIndex, colorX, colorY);
        logVerbose(buffer);
        
        colorX += boxWidth;
        if (colorX >= maxNumber) {
            colorX -= maxNumber;
            colorY += boxHeight;
        }
    }
}

/**
 * creates a container set for colors, along with all needed containers 
 * instances of this type
 * 
 * @param the container set structure to be filled with data
 * @return the number of generated container children of this container set
 */
unsigned createColorContainers(ContainerSet *containerSet) {

    // if box dimensions are not yet set manually, try to determine the
    // dimensions automatically, based on the "side length" of the Sudoku
    if (!boxWidth) {
        determineBoxDimensions(maxNumber, &boxWidth, &boxHeight);
    }

    // delegate container creation to generic generator function
    createContainers(COLORS, strdup("color"), maxNumber, containerSet);

    containerSet->fillContainerFields = &fillContainerFields;
    containerSet->getContainerName = &getColorName;

    // maxNumber colors have been generated
    return maxNumber;
}
