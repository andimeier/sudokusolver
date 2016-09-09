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
 * parse box dimension string, e.g "3x3"
 *
 * @param boxDimensionString box dimension string, e.g. "3x3"
 */
void parseBoxDimensionString(char *boxDimensionString, unsigned *width, unsigned *height) {
    sscanf(boxDimensionString, "%ux%u", width, height);
}
