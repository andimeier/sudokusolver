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


/**
 * tries to solve a given sudoku
 * 
 * @param parameters the Sudoku parameters (geometry, initial values, ...)
 * @return success flag: TRUE if the Sudoku could be solved successfully, 
 *   FALSE if not
 */
Bool solveSudoku(Parameters *parameters) {

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

    // start working horse
    return solve();
}
