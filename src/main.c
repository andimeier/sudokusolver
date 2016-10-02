#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
#include "solve.h"
#include "grid.h"
#include "util.h"
#include "show.h"
#include "logfile.h"
#include "log.h"
#include "gametype.h"
#include "acquire.h"
#include "printgrid.h"
#include "parameters.h"
#include "shape.h"
#include "box.h"

static void printUsage();

int main(int argc, char **argv) {
    int result;
    Parameters *parameters;
    CommandLineArgs *args;

    // command line parsing
    // ====================

    args = parseCommandLineArguments(argc, argv);

    if (!args) {
        // error in command line argument processing
        exit(EXIT_FAILURE);
    }

    if (args->help) {
        // help requested
        printUsage();
        exit(EXIT_SUCCESS);
    }

    // init phase
    // ==========

    setLogLevel(args->logLevel);

    if (args->outputFilename) {
        openLogFile(args->outputFilename);
    }

    /*
     * start with a default Sudoku grid, will be overridden by the 
     * actual parameters
     */
    setDefaults();

    /*
     * read Sudoku and fill the starting numbers
     * and possibly read grid parameters which override the default settings.
     * So, the settings which are defined in the Sudoku file will always
     * override any default or settings via command line parameter.
     * 
     * The basic geometry of the board will be implicitly set by the given
     * numbers. 
     */
    // try to load Sudoku from file
    parameters = readSudoku(args->inputFilename);
    if (!parameters) {
        exit(EXIT_FAILURE);
    }


    // dimension Sudoku and allocate fields
    initSudoku(parameters);


    /*
     * now that we definitely know the geometry and characteristics of the
     * game board, initialize the game
     */
    setupGrid();

    /*
     * check if the initial board is valid
     */
    if (!isValidSudoku()) {
        logError("invalid Sudoku");
        exit(EXIT_FAILURE);
    }

    /*
     * start logging
     */
    initLog();

    // START (solve Sudoku)
    // ====================

    result = solve();

    printLog();

    sprintf(buffer, "Gametype: %s", getGameTypeString(sudokuType));
    logAlways(buffer);

    printSvg(1);

    logAlways("");
    printGrid(SOLVED);
    logAlways("");

    if (result) {
        logAlways("-----------------------------------------------");
        logAlways("         SUDOKU HAS BEEN SOLVED!");
        logAlways("-----------------------------------------------");
        printSudokuString(SOLVED, parameters->candidate0);

        // print the strategies involved
        printInvolvedStrategies();

    } else {

        unsigned numbersFound = 0;
        for (int f = 0; f < numberOfFields; f++)
            if (fields[f].value)
                numbersFound++;

        logAlways("-----------------------------------------------");
        logAlways("      Sudoku could not be solved!");
        sprintf(buffer, "      Found %u of %zu cells.", numbersFound, numberOfFields);
        logAlways(buffer);
        logAlways("-----------------------------------------------");
        printSudokuString(INITIAL, parameters->candidate0);

        // print the strategies involved
        printInvolvedStrategies();
    }

    if (errors) {
        sprintf(buffer, "%d ERRORS occurred!", errors);
        logError(buffer);
    }


    closeLogFile();

    free(strategies); // FIXME where to put that?

    releaseGrid();

    exit(EXIT_SUCCESS);
}




//-------------------------------------------------------------------

void printUsage() {
    // print program usage

    puts("Usage: ");
    puts(" sudoku-solver -l LOGFILE -s SVGFILE -v -h SUDOKU_FILE");
    puts("");
    puts("Parameters:");
    puts("");
    puts("  -l LOGFILE  printlog into LOGFILE (filename) instead of stdout");
    puts("  -s SVGFILE  write SVG representation of Sudoku grid into SVG files, each iteration will write");
    puts("              another SVG file with a numeric suffix appended. The SVG file without an additional");
    puts("              numeric suffix is the final, solved grid.");
    puts("              For example, when the parameter -s test.svg is specified, you will end up with SVG");
    puts("              files of test.svg.1, test.svg.2, test.svg.3 etc. plus the final grid, stored in the");
    puts("              file test.svg (without additional suffix).");
    puts("  -v          verbose logging");
    puts("  -V          very verbose logging");
    puts("  -h          this help screen");
    puts("  SUDOKU_STRING a Sudoku in the one-string format. If given, overrides the -f setting.");
}

