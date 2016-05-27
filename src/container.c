/**
 * functions related to mapping fields into some sort of containers.
 * A container is any set of fields except a row or a column. Standard
 * containers are "row", "column" and "box" (for standard Sudoku), 
 * "diagonal" (X-Sudoku) or "color" (for color Sudoku).
 */
#include <assert.h>
#include "container.h"
#include "grid.h"
#include "log.h"

static void getBoxStartCoordinates(int q, int *qx, int *qy);
static void createContainers(char *name, size_t numberOfInstances, char *instanceNames[], ContainerSet *containerType);

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

//-------------------------------------------------------------------
// Liefert zu dem x-ten Feld eines Quadranten dessen absolute x- und
// y-Koordinaten im Sudoku
// Parameter:
//   q ... Nummer des Quadranten (0..8)
//   position ... Position innerhalb des Quadranten (0..8, wobei 0..2
//     in der ersten Zeile des Quadranten sind)
//   x ... absolute X-Koordinate (0..8), wird zurueckgeliefert
//   y ... absolute Y-Koordinate (0..8), wird zurueckgeliefert

void getCoordinatesInBox(int q, int position, int *x, int *y) {
    int qx, qy;

    assert(q >= 0 && q < 9);
    assert(position >= 0 && position < 9);

    getBoxStartCoordinates(q, &qx, &qy);

    *x = qx + (position % 3);
    *y = qy + (position / 3);
}


//-------------------------------------------------------------------
// Rechnet aus Quadrantenkoordinaten in absolute Koordinaten um.
// Parameter:
//   q ... Nummer (0..8) des Quadranten
//   qx ... X-Koordinate des linken oberen Ecks des Quadranten
//   qx ... Y-Koordinate des linken oberen Ecks des Quadranten

void getBoxStartCoordinates(int q, int *qx, int *qy) {
    *qx = (q % 3) * 3;
    *qy = (q / 3) * 3;
}


//-------------------------------------------------------------------
// Liefert die Nummer des Quadranten, in dem das Feld mit den
// Koordinaten x/y steht
// Quadranten sind von 0 bis 8 durchnummeriert, dh so angeordnet:
// Q0 Q1 Q3
// Q3 Q4 Q5
// Q6 Q7 Q8
// Jeder Quadrant ist eine 3x3-Matrix

int getBoxNr(int x, int y) {

    assert(x >= 0 && x < 9);
    assert(y >= 0 && y < 9);

    return (y / 3) * 3 + (x / 3);
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
 * @param the container set struct to be filled with data
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
    createContainers(strdup("row"), MAX_NUMBER, instanceNames, containerSet);

    containerSet->getContainerIndex = &determineRowContainer;

    // MAX_NUMBER rows have been generated
    return MAX_NUMBER;
}

/**
 * creates a container set for columns, along with all needed containers 
 * instances of this type
 * 
 * @param the container type struct to be filled with data
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
    createContainers(strdup("column"), MAX_NUMBER, instanceNames, containerSet);

    containerSet->getContainerIndex = &determineColumnContainer;

    // MAX_NUMBER columns have been generated
    return MAX_NUMBER;
}

/**
 * creates a container set for boxes, along with all needed containers 
 * instances of this type
 * 
 * @param the container type struct to be filled with data
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
    createContainers(strdup("box"), MAX_NUMBER, instanceNames, containerSet);

    containerSet->getContainerIndex = &determineBoxContainer;

    // MAX_NUMBER boxes have been generated
    return MAX_NUMBER;
}

void createContainers(char *name, size_t numberOfInstances, char *instanceNames[], ContainerSet *containerSet) {

    containerSet->name = name;
    containerSet->numberOfContainers = numberOfInstances;
    containerSet->containers = (Container *) xmalloc(sizeof (Container) * (numberOfInstances + 1));
    for (int i = 0; i < numberOfInstances; i++) {
        Container *container = &(containerSet->containers[i]);
        container->name = instanceNames[i];
        container->fields = (Field **) xmalloc(sizeof (Field *) * MAX_NUMBER);
    }

    // numberOfInstances children have been generated
    return numberOfInstances;
}

