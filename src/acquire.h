/* 
 * File:   acquire.h
 * Author: aurez
 *
 * Created on 01. Mai 2016, 22:04
 */

#ifndef ACQUIRE_H
#define	ACQUIRE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "gametype.h"

    /*
     * entire set of the Sudoku input parameters. All parameters which are
     * read from the Sudoku file or from other sources (e.g. as switches from
     * the command line) are included in this structure.
     */
    typedef struct {
        GameType gameType;
        unsigned maxNumber;
        unsigned numberOfFields;
        unsigned boxWidth;
        unsigned boxHeight;
        unsigned *initialValues;
        unsigned *shapes;
    } Parameters;

    Parameters *readSudoku(char *inputFilename);
    //    int parseSudokuString(char *sudoku);

#ifdef	__cplusplus
}
#endif

#endif	/* ACQUIRE_H */

