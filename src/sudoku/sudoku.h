/* 
 * File:   sudoku.h
 * Author: alex
 *
 * Created on 16. Oktober 2016, 00:18
 */

#ifndef SUDOKU_H
#define	SUDOKU_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "typedefs.h"

    void setupSudoku(Parameters *parameters);
    Bool solveSudoku();
    void releaseSudoku();

#ifdef	__cplusplus
}
#endif

#endif	/* SUDOKU_H */

