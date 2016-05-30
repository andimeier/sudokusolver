/**
 * functions related to mapping fields into some sort of containers.
 * A container is any set of fields except a row or a column. Standard
 * containers are "row", "column" and "box" (for standard Sudoku), 
 * "diagonal" (X-Sudoku) or "color" (for color Sudoku).
 */
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "global.h"
#include "container.h"
#include "grid.h"
#include "log.h"
#include "util.h"

static void createContainers(unsigned type, char *name, size_t numberOfInstances, char *instanceNames[], ContainerSet *containerType);

static unsigned createRowContainers(ContainerSet *containerSet);
static unsigned createColumnContainers(ContainerSet *containerSet);
static unsigned createBoxContainers(ContainerSet *containerSet);

static unsigned boxWidth;
static unsigned boxHeight;

/**
 * sets up containers sets
 */
void setupContainerSets() {
    //    typedef int (*myFuncDef)(int, int);

    // at the moment, no other dimensions are possible than a 9x9 Sudoku
    assert(MAX_NUMBER == 9);

    if (MAX_NUMBER == 9) {
        boxWidth = 3;
        boxHeight = 3;
    }

}

/**
 * fills the container set placeholder with the characteristics of the 
 * specified type of container set
 * 
 * @param containerSet the container set structure to be filled
 * @param containerType, e.g. ROWS, COLS or BOXES
 */
void setContainerSet(ContainerSet *containerSet, unsigned containerType) {

    assert(containerType >= ROWS && containerType <= BOXES);

    switch (containerType) {
        case ROWS:
            createRowContainers(containerSet);
            break;
        case COLS:
            createColumnContainers(containerSet);
            break;
        case BOXES:
            createBoxContainers(containerSet);
            break;
    }
}

/**
 * get the name of a row container
 * 
 * @param index index of the row container = number of row
 * @return a newly allocated string representing the "label" of the row 
 *   container, e.g. "row 2"
 */
char *getRowName(unsigned index) {
    assert(index >= 0 && index < 26);
    
    sprintf(buffer, "row %u", index + 1);
    return strdup(buffer);
}

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
 * determines the index of the row container which contains the field on the
 * given Sudoku coordinates
 * 
 * @param x X coordinate (starting with 0) of the specified field
 * @param y Y coordinate (starting with 0) of the specified field
 * @return index of the row container which contains the specified field, 
 *   or -1 if no such container contains the specified field (which is not
 *   possible with row containers, but might be possible for other types of
 *   containers)
 */
int determineRowContainer(unsigned x, unsigned y) {
    return y;
}

/**
 * determines the index of the column container which contains the field on the
 * given Sudoku coordinates
 * 
 * @param x X coordinate (starting with 0) of the specified field
 * @param y Y coordinate (starting with 0) of the specified field
 * @return index of the column container which contains the specified field, 
 *   or -1 if no such container contains the specified field (which is not
 *   possible with column containers, but might be possible for other types of
 *   containers)
 */
int determineColumnContainer(unsigned x, unsigned y) {
    return x;
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

    return (y / boxHeight) * 3 + (x / boxWidth);
}

/**
 * return number of row containers necessary to hold the Sudoku data.
 * In many cases (like this) the number of containers of this type will be
 * equal to MAX_NUMBER, but in some cases it might not, e.g. for diagonals
 * there would be only 2 containers.
 * @return the number of needed containers of this type
 */
unsigned determineRowContainersCount(void) {
    return MAX_NUMBER;
}

/**
 * return number of row containers necessary to hold the Sudoku data.
 * In many cases (like this) the number of containers of this type will be
 * equal to MAX_NUMBER, but in some cases it might not, e.g. for diagonals
 * there would be only 2 containers.
 * @return the number of needed containers of this type
 */
unsigned determineColumnContainersCount(void) {
    return MAX_NUMBER;
}

/**
 * return number of row containers necessary to hold the Sudoku data.
 * In many cases (like this) the number of containers of this type will be
 * equal to MAX_NUMBER, but in some cases it might not, e.g. for diagonals
 * there would be only 2 containers.
 * @return the number of needed containers of this type
 */
unsigned determineBoxContainersCount(void) {
    return MAX_NUMBER;
}

/**
 * creates a container set for rows, along with all needed containers instances
 * of this type
 * 
 * @param the container set structure to be filled with data
 * @return the number of generated container children of this container set
 */
unsigned createRowContainers(ContainerSet *containerSet) {
    char **instanceNames;
    unsigned i;

    instanceNames = (char **) xmalloc(sizeof (char *) * MAX_NUMBER);

    for (i = 0; i < MAX_NUMBER; i++) {
        sprintf(buffer, "row %c", (char) ('A' + i));
        instanceNames[i] = strdup(buffer);
    }

    // check that the number of instance names is equal to the containers
    // count stated by the auxiliary count function
    assert(i == determineRowContainersCount());

    // delegate container creation to generic generator function
    createContainers(ROWS, strdup("row"), MAX_NUMBER, instanceNames, containerSet);

    containerSet->getContainerIndex = &determineRowContainer;
    containerSet->getContainerName = &getRowName;

    // MAX_NUMBER rows have been generated
    return MAX_NUMBER;
}

/**
 * creates a container set for columns, along with all needed containers 
 * instances of this type
 * 
 * @param the container set structure to be filled with data
 * @return the number of generated container children of this container set
 */
unsigned createColumnContainers(ContainerSet *containerSet) {
    char **instanceNames;
    unsigned i;

    instanceNames = (char **) xmalloc(sizeof (char *) * MAX_NUMBER);

    for (i = 0; i < MAX_NUMBER; i++) {
        sprintf(buffer, "column %u", i + 1);
        instanceNames[i] = strdup(buffer);
    }

    // check that the number of instance names is equal to the containers
    // count stated by the auxiliary count function
    assert(i == determineColumnContainersCount());

    // delegate container creation to generic generator function
    createContainers(COLS, strdup("column"), MAX_NUMBER, instanceNames, containerSet);

    containerSet->getContainerIndex = &determineColumnContainer;
    containerSet->getContainerName = &getColumnName;

    // MAX_NUMBER columns have been generated
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

/**
 * generic function for creating a container set
 * 
 * @param type the type of container set, e.g. ROWS or COLS or BOXES
 * @param name name of the container set, e.g. "row"
 * @param numberOfInstances the number of container instance (child containers).
 *   For a container set ROWS, this is the number of rows.
 * @param instanceNames list of strings representing the name of each child 
 *   container, respectively
 * @param containerSet the container set structure to be filled with data
 */
void createContainers(unsigned type, char *name, size_t numberOfInstances, char *instanceNames[], ContainerSet *containerSet) {

    containerSet->name = name;
    containerSet->type = type;
    containerSet->numberOfContainers = numberOfInstances;
    containerSet->containers = (Container **) xmalloc(sizeof (Container *) * (numberOfInstances + 1));
}
