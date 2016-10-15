#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
#include "solve.h"
#include "grid.h"
#include "acquire.h"
#include "parameters.h"
#include "sudoku.h"
#include "summary.h"

static void printUsage();

/**
 * main function, orchestrates in a very high-level form the flow through the
 * application.
 * 
 * @param argc
 * @param argv
 * @return EXIT_SUCCESS on no error (regardless whether the Sudoku has been 
 *   solved or not. EXIT_FAILURE on user or application error.
 */
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

    // START (solve Sudoku)
    // ====================

    result = solveSudoku(parameters);

    
    // print result
    // ============
    
    printSummary(result, parameters->candidate0);
    

    closeLogFile();

    free(strategies); // FIXME where to put that?

    releaseGrid();

    exit(EXIT_SUCCESS);
}


/**
 * prints the usage screen
 */
void printUsage() {
    // print program usage

    puts("Usage: ");
    puts(" sudoku-solver [ -l LOGFILE -v -h ] SUDOKU_FILE");
    puts("");
    puts("Parameters:");
    puts("");
    puts("  -l LOGFILE  printlog into LOGFILE (filename) instead of stdout");
    puts("  -v          verbose logging");
    puts("  -h          this help screen");
    puts("  SUDOKU_FLIE the Sudoku input file");
}
