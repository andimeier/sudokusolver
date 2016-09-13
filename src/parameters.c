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

// variables defined by getopt
extern char *optarg;
extern int optind, opterr, optopt;


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
