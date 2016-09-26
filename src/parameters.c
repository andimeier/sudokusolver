/**
 * Utility functions for parameter parsing
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <assert.h>
#include "logfile.h"
#include "parameters.h"


static CommandLineArgs arguments;

//// variables defined by getopt
//extern char *optarg;
//extern int optind, opterr, optopt;

/**
 * parse the command line arguments and provide them in a structure
 * 
 * @param argc
 * @param argv
 * @return parsed command line arguments or NULL on error
 */
CommandLineArgs *parseCommandLineArguments(int argc, char **argv) {
    int c;

    // set default values, if not overridden by command line arguments
    arguments.logLevel = LOGLEVEL_ERRORS;
    arguments.gametypeString = NULL;
    arguments.outputFilename = NULL; // filename of printlog file
    arguments.inputFilename = NULL;
    arguments.help = FALSE;

    // read command line arguments
    opterr = 0;

    while ((c = getopt(argc, argv, "hvVl:s:")) != -1)
        switch (c) {
            case 'v':
                arguments.logLevel = LOGLEVEL_VERBOSE;
                break;

            case 'V':
                arguments.logLevel = LOGLEVEL_VERBOSE;
                break;

            case 'l':
                arguments.outputFilename = optarg;
                break;

            case 'h':
                arguments.help = TRUE;
                break;

            case '?':
                if (optopt == 'l' || optopt == 's')
                    fprintf(stderr, "Option -%d requires an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                return NULL;

            default:
                abort(); // should never happen, if getopt works
        }

    // first positional parameter is the filename of the Sudoku definition file
    if (optind < argc) {
        arguments.inputFilename = argv[optind];
    }

    // if no sudoku is given
    if (!arguments.inputFilename) {
        fprintf(stderr, "Missing Sudoku filename\n");
        fprintf(stderr, "See usage page for details: -h");
        return NULL;
    }

    return &arguments;
}

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

/**
 * expands a value characters string by expanding ranges of characters by their
 * literal counterparts.
 * E.g. convert a string like "0-9a-h" to "0123456789abcdefgh"
 * 
 * @param valueChars
 * @param errorMsg the error message in case of an error (will be returned)
 * @return the value characters expanded or NULL on error
 */
char *parseValueChars(char *valueCharsString, char *errorMsg[]) {
    char buffer[27];
    char *ptrSrc;
    char *ptrDest;
    char c;
    unsigned len;

    // replace '-' with the expanded range of characters
    ptrSrc = valueCharsString;
    ptrDest = buffer;
    len = 0;

    if (*ptrSrc == '-') {
        *errorMsg = strdup("illegal string of characters: hyphen must be between two candidate characters, but is on start of string");
        return NULL;
    }

    /*
     * State Machine:
     * switch (c)
     *   case '-':
     *     if (pos == 0)
     *       ERROR(must not start with hyphen)
     *     else
     *       if (!range)
     *         fromChar = lastChar;
     *         range = TRUE
     *       else
     *         ERROR(two hyphens)
     *   case '0'-'9', 'a'-'z', 'A'-'Z'
     *     if (range)
     *        if (from...to == lower...lower || upper...upper || digit...digit)
     *          explodeRande
     *          range = FALSE
     *        else
     *          ERROR(mixed range not allowed)
     *     else
     *        copyChar
     * if (range)
     *   ERROR(must not end with hyphen)
     */ 
    
    while (*ptrSrc) {
        if (*ptrSrc == '-') {
            // interpret as a range of characters
            c = *(ptrSrc - 1) + 1;
            ptrSrc++; // navigate to "upper boundary" character
            if (*ptrSrc == '\0') {
                *errorMsg = strdup("illegal string of characters: hyphen must be between two candidate characters, but is at end of string");
                return NULL;
            }
            while (c <= *ptrSrc) {
                if (len >= 26) {
                    *errorMsg = strdup("illegal string of characters (more than 26 candidates)");
                    return NULL;
                }
                *ptrDest = c;
                ptrDest++;
                c++;
                len++;
            }
            ptrSrc++;

        } else {
            if (len >= 26) {
                *errorMsg = strdup("illegal string of characters (more than 26 candidates)");
                return NULL;
            }
            *ptrDest = *ptrSrc;
            ptrDest++;
            ptrSrc++;
            len++;
        }
    }
    *ptrDest = '\0'; // terminate destination buffer

    return strdup(buffer);
}
