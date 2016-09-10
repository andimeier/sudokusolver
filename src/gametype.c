/**
 * functions related to different type of Sudoku games
 */
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "grid.h"
#include "gametype.h"
#include "logfile.h"
#include "util.h"

static GameType gameType;
static char *gameTypeString;

/**
 * returns the game type string
 * 
 * @param gameType the Sudoku type, e.g. X_SUDOKU
 * @return the string describing the game type, e.g. "X-Sudoku"
 */
char *getGameTypeString(GameType gameType) {
    return gameTypeString;
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

        case JIGSAW_SUDOKU:
            logVerbose("Game type: Jigsaw Sudoku");

            numberOfContainerTypes = 3;
            containerTypes = (unsigned *) xmalloc(sizeof (ContainerType) * (numberOfContainerTypes + 1));

            ptr = containerTypes;
            *ptr++ = ROWS;
            *ptr++ = COLS;
            *ptr++ = SHAPES;
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

/**
 * parses the game type from the command line and tries to find out which
 * game type has to be chosen. Game types are "standard", "x" (X-Sudoku) or
 * "color" (color Sudoku).
 * The determined game type will be set (registered) as well.
 *
 * @param gametypeString
 * @return
 */
GameType parseGameTypeString(char *gametypeString) {

    if (!strncmp(gametypeString, "standard", strlen(gametypeString))) {
        setGameType(STANDARD_SUDOKU);
        logVerbose("Game type: Standard Sudoku");

    } else if (!strncmp(gametypeString, "x", strlen(gametypeString))) {
        setGameType(X_SUDOKU);
        logVerbose("Game type: X-Sudoku");

    } else if (!strncmp(gametypeString, "color", strlen(gametypeString))) {
        setGameType(COLOR_SUDOKU);
        logVerbose("Game type: Color Sudoku");

    } else if (!strncmp(gametypeString, "jigsaw", strlen(gametypeString))) {
        setGameType(JIGSAW_SUDOKU);
        logVerbose("Game type: Color Sudoku");

    } else {
        sprintf(buffer, "unnknown game type: %s (must be \"standard\", \"x\" or \"color\" or \"jigsaw\")", gametypeString);
        gameTypeString = strdup("UNKNOWN GAME TYPE");
        logError(buffer);
        exit(EXIT_FAILURE);
    }

    return gameType;
}

/**
 * sets the game type
 * 
 * @param gameType the game type to be set
 */
void setGameType(GameType _gameType) {

    gameType = _gameType;

    switch (gameType) {
        case STANDARD_SUDOKU:
            gameTypeString = strdup("Standard Sudoku");
            break;

        case X_SUDOKU:
            gameTypeString = strdup("X Sudoku");
            break;

        case COLOR_SUDOKU:
            gameTypeString = strdup("Color Sudoku");
            break;
            
        case JIGSAW_SUDOKU:
            gameTypeString = strdup("Jigsaw Sudoku");
            break;
            
        default:
            gameTypeString = strdup("UNKNOWN GAME TYPE");
    }
}

/**
 * gets the game type
 * 
 * @return game type
 */
GameType getGameType() {
    return gameType;
}
