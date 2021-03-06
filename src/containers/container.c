/**
 * functions related to mapping fields into some sort of containers.
 * A container is any set of fields except a row or a column. Standard
 * containers are "row", "column" and "box" (for standard Sudoku), 
 * "diagonal" (X-Sudoku) or "color" (for color Sudoku).
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include "container.h"
#include "grid.h"
#include "logfile.h"
#include "util.h"
#include "row.h"
#include "column.h"
#include "box.h"
#include "diagonal.h"
#include "shape.h"
#include "color.h"
#include "typedefs.h"


/**
 * fills the container set placeholder with the characteristics of the 
 * specified type of container set
 * 
 * @param containerSet the container set structure to be filled
 * @param containerType, e.g. ROWS, COLS or BOXES
 */
void setContainerSet(ContainerSet *containerSet, ContainerType containerType) {

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
        case DIAGONALS:
            createDiagonalContainers(containerSet);
            break;
        case SHAPES:
            createShapeContainers(containerSet);
            break;
        case COLORS:
            createColorContainers(containerSet);
            break;
    }
}

/**
 * generic function for creating a container set
 * 
 * @param type the type of container set, e.g. ROWS or COLS or BOXES
 * @param name name of the container set, e.g. "row"
 * @param numberOfInstances the number of container instance (child containers).
 *   For a container set ROWS, this is the number of rows.
 * @param containerSet the container set structure to be filled with data
 */
void createContainers(ContainerType type, char *name, size_t numberOfInstances, ContainerSet *containerSet) {

    containerSet->name = name;
    containerSet->type = type;
    containerSet->numberOfContainers = numberOfInstances;
    containerSet->containers = (Container **) xmalloc(sizeof (Container *) * (numberOfInstances + 1));
}
