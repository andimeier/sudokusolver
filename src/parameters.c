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

    while ((c = getopt(argc, argv, "hvl:")) != -1)
        switch (c) {
            case 'v':
                arguments.logLevel = LOGLEVEL_VERBOSE;
                break;

            case 'l':
                arguments.outputFilename = optarg;
                break;

            case 'h':
                arguments.help = TRUE;
                break;

            case '?':
                if (optopt == 'l')
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
    if (!arguments.inputFilename && !arguments.help) {
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
    char expandedValueChars[27];
    char *ptrSrc;
    char *ptrDest;
    char c;
    char fromChar;
    char toChar;
    unsigned len;
    Bool processingRange;

    // initialize errorMsg with an empty string
    *errorMsg[0] = '\0';
    
    // init state machine variables
    ptrSrc = valueCharsString;
    ptrDest = expandedValueChars;
    len = 0;
    processingRange = FALSE;



    if (*ptrSrc == '-') {
        return NULL;
    }

    while (*ptrSrc) {
        c = *ptrSrc;

        if (c == '-') {
            if (len == 0) {
                *errorMsg = strdup("illegal string of characters: hyphen must be between two candidate characters, but is on start of string");
            } else {
                if (processingRange) {
                    // already in range, detected a second hyphen ... oops
                    *errorMsg = strdup("two hyphens");
                    break;
                } else {
                    // start of range definition
                    fromChar = *(ptrSrc - 1);
                    processingRange = TRUE;
                }
            }
        } else if (isdigit(c) || islower(c) || isupper(c)) {
            if (processingRange) {

                // "toChar" detected
                if ((isdigit(fromChar) && isdigit(c)) ||
                        (islower(fromChar) && islower(c)) ||
                        (isupper(fromChar) && isupper(c))) {
                    toChar = c;
                    if (toChar < fromChar) {
                        sprintf(buffer, "range must be defined in ascending order, but was defined as %c-%c", fromChar, toChar);
                        *errorMsg = strdup(buffer);
                        break;
                    }

                    // explode range
                    
                    /*
                     * start with the next character after fromChar, because
                     * fromChar has already been copied
                     */
                    c = fromChar + 1; 
                    while (c <= toChar) {
                        //                        if (addChar(ptrDest, c)) {
                        //                            len++;
                        //                        }
                        if (len >= 26) {
                            *errorMsg = strdup("illegal string of characters (more than 26 candidates)");
                            return NULL;
                        }
                        *ptrDest = c;
                        ptrDest++;
                        len++;
                        c++;
                    }

                    // range has been exploded, reset "range mode"
                    processingRange = FALSE;

                } else {
                    sprintf(buffer, "mixed range (%c-%c) not allowed", fromChar, c);
                    *errorMsg = strdup(buffer);
                    break;
                }
            } else {
                // not in range processing mode
                *ptrDest = c;
                ptrDest++;
                len++;
            }
        } else {
            sprintf(buffer, "illegal character in set of candidates: %c (only digits and letters are allowed)", c);
            *errorMsg = strdup(buffer);
            break;
        }

        /*
         * State Machine:
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


        if (*errorMsg[0] != '\0') {
            break;
        }
        
        ptrSrc++;
    }

    // are we in the middle of a range when we suddenly hit the end of the string?
    if (processingRange) {
        *errorMsg = strdup("illegal string of characters: hyphen must be between two candidate characters, but is at end of string");
    }

    if (*errorMsg[0] != '\0') {
        return NULL;
    }

    *ptrDest = '\0'; // terminate destination buffer

    return strdup(expandedValueChars);

    /*
     * State Machine:
     * switch (c)
     *   case '-':
     *     if (pos == 0)
     *       ERROR(must not start with hyphen)
     *errorMsg = strdup("illegal string of characters: hyphen must be between two candidate characters, but is on start of string");
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
}
