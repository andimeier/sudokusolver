/* 
 * File:   global.h
 * Author: ju
 *
 * Created on 04. April 2016, 21:31
 */

#ifndef GRID_H
#define	GRID_H

#include "typedefs.h"

#ifdef	__cplusplus
extern "C" {
#endif


// index number of the unit types, used in *units
#define ROWS 0
#define COLS 1
#define BOXES 2

// number of possible numbers
#define MAX_NUMBER 9
#define NUMBER_OF_FIELDS MAX_NUMBER * MAX_NUMBER
    
    // the fields of the game board
    extern Field *fields;

    // units are: rows, cols, boxes. Within one unit, the numbers 1...9 must
    // appear. For a standard Sudoku, there are 3 units (rows, cols, boxes).
    // More exotic Sudokus may use more unit, e.g. color units.
    extern struct UnitDefs unitDefs;



#ifdef	__cplusplus
}
#endif

#endif	/* GRID_H */

