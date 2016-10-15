#include <stdio.h>
#include "summary.h"
#include "grid.h"
#include "printgrid.h"
#include "solve.h"
#include "log.h"
#include "logfile.h"
#include "show.h"
#include "acquire.h"

/**
 * prints the summary (result) of the solving process.
 * If the Sudoku has been solved, prints the solved Sudoku and the used 
 * strategies.
 * 
 * @param hasBeenSolved
 * @param candidate0 TRUE if '0' is a valid candidate, FALSE if not
 */
void printSummary(Bool hasBeenSolved, Bool candidate0) {

    printLog();

    sprintf(buffer, "Gametype: %s", getGameTypeString(sudokuType));
    logAlways(buffer);

    logAlways("");
    printGrid(SOLVED);
    logAlways("");

    if (hasBeenSolved) {
        logAlways("-----------------------------------------------");
        logAlways("         SUDOKU HAS BEEN SOLVED!");
        logAlways("-----------------------------------------------");
        printSudokuString(SOLVED, candidate0);

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
        printSudokuString(INITIAL, candidate0);

        // print the strategies involved
        printInvolvedStrategies();
    }

    if (errors) {
        sprintf(buffer, "%d ERRORS occurred!", errors);
        logError(buffer);
    }

}