/* 
 * File:   global.h
 * Author: ju
 *
 * Created on 04. April 2016, 21:31
 */

#ifndef GLOBAL_H
#define	GLOBAL_H

#ifdef	__cplusplus
extern "C" {
#endif

    // index number of the unit types, used in *units
    #define ROWS 0
    #define COLS 1
    #define BOXES 2

    typedef struct Field {
        unsigned *unitPosition;
        unsigned candidates[9];
        unsigned initialValue;
        unsigned candidatesLeft;
        unsigned value;
    };
    
    extern Field fields[81]; // the fields of the game board
    // units are: rows, cols, boxes. Within one unit, the numbers 1...9 must
    // appear. For a standard Sudoku, there are 3 units (rows, cols, boxes).
    // More exotic Sudokus may use more unit, e.g. color units.
    extern Field **units;
    
    extern char buffer[1000]; // general buffer for string operations

    extern int errors; // number of errors in the algorithm
    extern int verboseLogging; // 0 ... no verbose logging, 1 ... log changes, 2 ... log even considerations

    extern char *svgFilename; // filename of SVG file

    

#ifdef	__cplusplus
}
#endif

#endif	/* GLOBAL_H */

