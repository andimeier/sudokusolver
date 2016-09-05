/**
 * functions related to different type of Sudoku games
 */
#include <assert.h>
#include <string.h>
#include "grid.h"
#include "gametype.h"
#include "logfile.h"
#include "util.h"

/**
 * returns the game type string
 * 
 * @param gameType the Sudoku type, e.g. X_SUDOKU
 * @return the string describing the game type, e.g. "X-Sudoku"
 */
char *getGameTypeString(GameType gameType) {
    char *string;

    switch (gameType) {
        case STANDARD_SUDOKU:
            string = strdup("Standard Sudoku");
            break;

        case X_SUDOKU:
            string = strdup("X Sudoku");
            break;

        case COLOR_SUDOKU:
            string = strdup("Color Sudoku");
            break;
            
        case JIGSAW_SUDOKU:
            string = strdup("Jigsaw Sudoku");
            break;
            
        default:
            string = strdup("UNKNOWN GAME TYPE");
    }
    
    return string;
}

/**
 * determines the types of containers needed for a specific game type
 * 
 * @param gameType the Sudoku type, e.g. X_SUDOKU
 * @return the list of container types needed for this Sudoku type
 */
ContainerType *getContainerTypes(GameType gameType) {
    ContainerType *containerTypes;
    size_t numberOfContainerTypes;
    ContainerType *ptr;

    switch (gameType) {
        case STANDARD_SUDOKU:
            logVerbose("Game type: Standard Sudoku");

            numberOfContainerTypes = 3;
            containerTypes = (unsigned *) xmalloc(sizeof (ContainerType) * (numberOfContainerTypes + 1));

            ptr = containerTypes;
            *ptr++ = ROWS;
            *ptr++ = COLS;
            *ptr++ = BOXES;
            *ptr = 0;
            break;

        case X_SUDOKU:
            logVerbose("Game type: X Sudoku");

            numberOfContainerTypes = 4;
            containerTypes = (unsigned *) xmalloc(sizeof (ContainerType) * (numberOfContainerTypes + 1));

            ptr = containerTypes;
            *ptr++ = ROWS;
            *ptr++ = COLS;
            *ptr++ = BOXES;
            *ptr++ = DIAGONALS;
            *ptr = 0;
            break;
            
        case COLOR_SUDOKU:
            break;
    }

    return containerTypes;
}
