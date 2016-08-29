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

void printUsage();
unsigned parseGametypeString(char *gametypeString);

int main(int argc, char **argv) {
    int result;
    int c;
    char *outputFilename = NULL; // filename of printlog file
    char *inputFilename = NULL;
    char *sudokuString = NULL;
    char *gametypeString = NULL;

    // if the Sudoku is wider than 26 numbers, we have a memory allocation issue
    // with the field->name (what is right of "Z26"?)
    assert(maxNumber <= 26);

    // read command line arguments
    opterr = 0;

    while ((c = getopt(argc, argv, "f:hvVl:s:t:")) != -1)
        switch (c) {
            case 'v':
                logLevel = LOGLEVEL_VERBOSE;
                break;
            case 'V':
                logLevel = LOGLEVEL_VERBOSE;
                break;
            case 's':
                svgFilename = optarg;
                break;
            case 'l':
                outputFilename = optarg;
                break;
            case 'f':
                inputFilename = optarg;
                break;
            case 't':
                gametypeString = optarg;
                break;
            case 'h':
                printUsage();
                return 0;
                break;
            case '?':
                if (optopt == 'l' || optopt == 's')
                    fprintf(stderr, "Option -%d requires an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                return 1;
            default:
                abort();
        }

    // first positional parameter is a Sudoku string
    if (optind < argc) {
        sudokuString = argv[optind];
    }

    for (int i = optind; i < argc; i++) {
        //  process positional parameters
    }

    // if no sudoku is given
    if (!sudokuString && !inputFilename) {
        fprintf(stderr, "No Sudoku data given. Please either specify a file with -f or a Sudoku string.\n");
        fprintf(stderr, "See usage page for details: -h");
        exit(EXIT_FAILURE);
    }

    if (outputFilename) {
        openLogFile(outputFilename);
    }



    // START
    // =====

    /*
     * start with a default Sudoku grid, can be overridden when the command
     * line parameters or Sudoku file parameters are actually used
     */
    setDefaults();

    /*
     * override defaults with settings passed by command line parameters
     */
    if (gametypeString) {
        setSudokuType(parseGametypeString(gametypeString));
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
    if (inputFilename && !readSudoku(inputFilename)) {
        exit(EXIT_FAILURE);
    }

    // try to parse Sudoku string
    if (sudokuString && !importSudoku(sudokuString)) {
        exit(EXIT_FAILURE);
    }


    /*
     * now that we definitely know the geometry and characteristics of the
     * game board, initialize the game
     */
    setupGrid();

    if (logLevel >= LOGLEVEL_VERBOSE) {
        logVerbose("Initial Sudoku:");
        show(0);
    }

    initLog();

    result = solve();

    printLog();

    sprintf(buffer, "Gametype: %s", getGameTypeString(sudokuType));
    logAlways(buffer);

    show(1);
    printSvg(1);

    logAlways("");
    printGrid(SOLVED);
    logAlways("");
    
    if (result) {
        logAlways("-----------------------------------------------");
        logAlways("         SUDOKU HAS BEEN SOLVED!");
        logAlways("-----------------------------------------------");
        printSudokuString(1);

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
        printSudokuString(0);

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
    puts(" sudoku-solver -l LOGFILE -s SVGFILE -v -h [ SUDOKU_STRING ]");
    puts("");
    puts("Parameters:");
    puts("");
    puts("  -f SUDOKUFILE  the Sudoku input file containing 9 lines of 9 number each.");
    puts("              This represents the starting state of the Sudoku.");
    puts("  -l LOGFILE  printlog into LOGFILE (filename) instead of stdout");
    puts("  -s SVGFILE  write SVG representation of Sudoku grid into SVG files, each iteration will write");
    puts("              another SVG file with a numeric suffix appended. The SVG file without an additional");
    puts("              numeric suffix is the final, solved grid.");
    puts("              For example, when the parameter -s test.svg is specified, you will end up with SVG");
    puts("              files of test.svg.1, test.svg.2, test.svg.3 etc. plus the final grid, stored in the");
    puts("              file test.svg (without additional suffix).");
    puts("  -t          game type, can be s(tandard), c(olor) or x (X-Sudoku). If not specified, standard");
    puts("              is assumed");
    puts("  -v          verbose logging");
    puts("  -V          very verbose logging");
    puts("  -h          this help screen");
    puts("  SUDOKU_STRING a Sudoku in the one-string format. If given, overrides the -f setting.");
}
