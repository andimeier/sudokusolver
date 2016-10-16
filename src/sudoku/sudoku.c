/*
 * File:   sudoku.c
 * Author: aurez
 */
#include <stdlib.h>
#include "sudoku.h"
#include "grid.h"
#include "logfile.h"
#include "log.h"
#include "solve.h"
#include "summary.h"

static Parameters *parameters;
static Bool hasBeenSolved;

/**
 * allocates the necessary resources for the given Sudoku
 * 
 * @param parameters the Sudoku parameters (geometry, initial values, ...)
 */
void setupSudoku(Parameters *_parameters) {

    // register parameters
    parameters = _parameters;
    
    // dimension Sudoku and allocate fields
    initSudoku(parameters);

    /*
     * now that we definitely know the geometry and characteristics of the
     * game board, initialize the game
     */
    setupGrid();
}


/**
 * tries to solve a given sudoku
 * 
 * @return success flag: TRUE if the Sudoku could be solved successfully, 
 *   FALSE if not
 */
Bool solveSudoku() {

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

    // start working horse
    hasBeenSolved = solve();
    
    return hasBeenSolved;
}

/**
 * prints the summary
 */
void printSudokuSummary() {
    printSummary(hasBeenSolved, parameters->candidate0);
}


/**
 * release all resources allocated for this Sudoku
 */
void releaseSudoku() {
}
