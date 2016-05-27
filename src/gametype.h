/* 
 * File:   type.h
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

    // standard Sudoku with rows, columns and boxes
#define GAME_STANDARD_SUDOKU    1

    // X-Sudoku with rows, columns, boxes and 2 diagonals
#define GAME_X_SUDOKU  2

    // color Sudoku with rows, columns, boxes and colors
#define GAME_COLOR_SUDOKU  3

    unsigned *getContainerTypes(unsigned gameType);

#ifdef	__cplusplus
}
#endif

#endif	/* FAMETYPE_H */

