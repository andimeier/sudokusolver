/* 
 * File:   gametype.h
 * Author: alex
 *
 * functions related to different types of Sudoku games (Standard Sudoku,
 * X-Sudoku, Color Sudoku, ...)
 * 
 * Created on 02. Mai 2016, 20:31
 */

#ifndef GAMETYPE_H
#define	GAMETYPE_H

#include "typedefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

    /*
     * game types:
     * STANDARD_SUDOKU: standard Sudoku with rows, columns and boxes
     * X_SUDOKU: X-Sudoku with rows, columns, boxes and 2 diagonals
     * COLOR_SUDOKU: with rows, columns, boxes and colors
     */
    typedef enum {
        STANDARD_SUDOKU,
        X_SUDOKU,
        COLOR_SUDOKU,
        JIGSAW_SUDOKU
    } GameType;

    ContainerType *getContainerTypes(GameType gameType);
    char *getGameTypeString(GameType gameType);

#ifdef	__cplusplus
}
#endif

#endif	/* FAMETYPE_H */

