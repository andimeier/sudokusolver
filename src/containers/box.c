#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "grid.h"
#include "logfile.h"
#include "util.h"
#include "box.h"
#include "container.h"


static char *getBoxName(unsigned index);
static void fillContainerFields(unsigned containerIndex, FieldsVector *fields);

static unsigned boxWidth = 0;
static unsigned boxHeight = 0;

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

    boxStartX = containerIndex % (maxNumber / boxWidth) * boxWidth;
    boxStartY = containerIndex / (maxNumber / boxWidth) * boxHeight;

    for (n = 0; n < maxNumber; n++) {
        boxX = n % boxWidth;
        boxY = n / boxWidth;

        fields[n] = getFieldAt(boxStartX + boxX, boxStartY + boxY);
    }
}

/**
 * creates a container set for boxes, along with all needed containers 
 * instances of this type
 * 
 * @param the container set structure to be filled with data
 * @return the number of generated container children of this container set
 */
unsigned createBoxContainers(ContainerSet *containerSet) {

    // if box dimensions are not yet set manually, try to determine the
    // dimensions automatically, based on the "side length" of the Sudoku
    if (!boxWidth) {
        determineBoxDimensions(maxNumber, &boxWidth, &boxHeight);
    }

    if (!boxWidth) {
        sprintf(buffer, "unable to determine box shapes on a %zux%zu Sudoku", maxNumber, maxNumber);
        logError(buffer);
        exit(EXIT_FAILURE);
    }

    sprintf(buffer, "box size: %u x %u", boxWidth, boxHeight);
    logAlways(buffer);

    // sanity check
    if (boxWidth * boxHeight != maxNumber) {
        sprintf(buffer, "illegal box size: a %u x %u box does not hold exactly %zu numbers", boxWidth, boxHeight, maxNumber);
        logError(buffer);
        exit(EXIT_FAILURE);
    }

    // delegate container creation to generic generator function
    createContainers(BOXES, strdup("box"), maxNumber, containerSet);

    containerSet->fillContainerFields = &fillContainerFields;
    containerSet->getContainerName = &getBoxName;

    // maxNumber boxes have been generated
    return maxNumber;
}

/**
 * determines box height and box width from the maxNumber
 * 
 * If maxNumber is a square number, then width/height are the square root
 * (standard case), otherwise we have rectangular shaped boxes
 * 
 * @param maxNumber maximum number (= side length of the Sudoku)
 * @param *width [out] return the calculated width of the boxes
 * @param *height [out] return the calculated height of the boxes
 */
void determineBoxDimensions(unsigned maxNumber, unsigned *width, unsigned *height) {
    unsigned w = 0;
    unsigned h = 0;

    switch (maxNumber) {
        case 9:
            w = 3;
            h = 3;
            break;
        case 6:
            w = 3;
            h = 2;
            break;
        case 4:
            w = 2;
            h = 2;
            break;
        default:
            // unknown Sudoku size
            w = 0;
            h = 0;
    }

    *width = w;
    *height = h;
}

/**
 * sets the box dimensions
 * 
 * will be called if the dimensions should not be determined automatically,
 * but set manually
 * 
 * @param width the width of the boxes
 * @param height the height of the boxes
 */
void setBoxDimensions(unsigned width, unsigned height) {
    boxWidth = width;
    boxHeight = height;
}

/**
 * returns the box width (and computes it if not set already)
 * 
 * @return box width
 */
unsigned getBoxWidth() {
    if (!boxWidth) {
        determineBoxDimensions(maxNumber, &boxWidth, &boxHeight);
    }
    return boxWidth;
}

/**
 * returns the box height (and computes it if not set already)
 * 
 * @return box height
 */
unsigned getBoxHeight() {
    if (!boxHeight) {
        determineBoxDimensions(maxNumber, &boxWidth, &boxHeight);
    }
    return boxHeight;
}