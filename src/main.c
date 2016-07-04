#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
#include "global.h"
#include "solve.h"
#include "grid.h"
#include "util.h"
#include "show.h"
#include "logfile.h"
#include "acquire.h"

void printUsage();

int main(int argc, char **argv) {
    int result;
    int c;
    char *outputFilename = NULL; // filename of printlog file
    char *inputFilename = NULL;
    char *sudoku = NULL;

    // if the Sudoku is wider than 26 numbers, we have a memory allocation issue
    // with the field->name (what is right of "Z26"?)
    assert(MAX_NUMBER <= 26);

    // read command line arguments
    opterr = 0;

    while ((c = getopt(argc, argv, "f:hvVl:s:")) != -1)
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

    // FIXME hardcoded example sudoku just to make the exec work without parameters (for GDB))
    if (!inputFilename) {
        if (MAX_NUMBER == 9) {
            inputFilename = strdup("examples/pointing-pair.sudoku");
        } else if (MAX_NUMBER == 4) {
            inputFilename = strdup("examples/4x4-naked-pair.sudoku");
        }
//        logLevel = LOGLEVEL_VERBOSE;
    }

    //FIXME remove this diabling of output buffering, it is only for testing purposes
    setvbuf(stdout, NULL, _IONBF, 0);

    // first positional parameter is a Sudoku string
    if (optind < argc) {
        sudoku = argv[optind];
    }

    for (int i = optind; i < argc; i++) {
        //  process positional parameters
    }

    // if no sudoku is given
    if (!sudoku && !inputFilename) {
        fprintf(stderr, "No Sudoku data given. Please either specify a file with -f or a Sudoku string.\n");
        fprintf(stderr, "See usage page for details: -h");
        exit(EXIT_FAILURE);
    }

    if (outputFilename) {
        openLogFile(outputFilename);
    }

    setupGrid();

    if (inputFilename && !readSudoku(inputFilename)) {
        exit(EXIT_FAILURE);
    }

    if (sudoku && !importSudoku(sudoku)) {
        exit(EXIT_FAILURE);
    }

    if (logLevel >= LOGLEVEL_VERBOSE) {
        logVerbose("Initial Sudoku:");
        show(0);
    }

    for (int f = 0; f < NUMBER_OF_FIELDS; f++) {
        sprintf(buffer, "[1234-2] field #%d: in %s, %s, %s", f, fields[f].containers[0]->name, fields[f].containers[1]->name, fields[f].containers[2]->name);
        logVerbose(buffer);
    }

    result = solve();

    show(1);
    printSvg(1);

    if (result) {
        logAlways("-----------------------------------------------");
        logAlways("         FERTIG, SUDOKU WURDE GELOEST!");
        logAlways("-----------------------------------------------");
        sudokuString(1);
    } else {

        int numbersFound = 0;
        for (int f = 0; f < NUMBER_OF_FIELDS; f++)
            if (fields[f].value)
                numbersFound++;

        logAlways("-----------------------------------------------");
        logAlways("      Sudoku konnte nicht geloest werden!");
        sprintf(buffer, "      %d von %d Zellen wurden gefunden.", numbersFound, NUMBER_OF_FIELDS);
        logAlways(buffer);
        logAlways("-----------------------------------------------");
        sudokuString(0);
    }

    if (errors) {
        sprintf(buffer, "Es sind %d FEHLER aufgetreten!", errors);
        logError(buffer);
    }


    closeLogFile();

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
    puts("  -v          verbose logging");
    puts("  -V          very verbose logging");
    puts("  -h          this help screen");
    puts("  SUDOKU_STRING a Sudoku in the one-string format. If given, overrides the -f setting.");
}

