/**
 * Utility functions for parameter parsing
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "logfile.h"
#include "parameters.h"

/**
 * converts a string to lower case.
 *
 * @param str the string to be lowercased
 */
void toLowerStr(char *str) {
    int i;

    for (i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

/**
 * parses the game type from the command line and tries to find out which
 * game type has to be chosen. Game types are "standard", "x" (X-Sudoku) or
 * "color" (color Sudoku).
 *
 * @param gametypeString
 * @return
 */
GameType parseGametypeString(char *gametypeString) {
    GameType gametype;

    if (!strncmp(gametypeString, "standard", strlen(gametypeString))) {
        gametype = STANDARD_SUDOKU;
        logVerbose("Game type: Standard Sudoku");
    } else if (!strncmp(gametypeString, "x", strlen(gametypeString))) {
        gametype = X_SUDOKU;
        logVerbose("Game type: X-Sudoku");
    } else if (!strncmp(gametypeString, "color", strlen(gametypeString))) {
        gametype = COLOR_SUDOKU;
        logVerbose("Game type: Color Sudoku");
    } else {
        sprintf(buffer, "unnknown game type: %s (must be \"standard\", \"x\" or \"color\")", gametypeString);
        logError(buffer);
        exit(EXIT_FAILURE);
    }

    return gametype;
}

/**
 * parse box dimension string, e.g "3x3"
 *
 * @param boxDimensionString box dimension string, e.g. "3x3"
 */
void parseBoxDimensionString(char *boxDimensionString, unsigned *width, unsigned *height) {
    sscanf(boxDimensionString, "%ux%u", width, height);
}
