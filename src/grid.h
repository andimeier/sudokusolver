/* 
 * File:   grid.h
 * Author: aurez
 *
 * Created on 04. April 2016, 21:57
 */

#ifndef GRID_H
#define GRID_H

#include "typedefs.h"
#include "gametype.h"
#include "acquire.h"

#ifdef __cplusplus
extern "C" {
#endif


    // the fields of the game board
    extern Field *fields;

    /*
     *  container types are: rows, cols, boxes. Within one unit, the numbers 
     * 1...9 must appear. For a standard Sudoku, there are 3 units (rows, cols,
     * boxes). More exotic Sudokus may use more units, e.g. color units or 
     * diagonals.
     */
    extern ContainerSet *containerSets; // all container types (e.g. [row, column, box])

    // all containers of the game board
    extern Container *allContainers;

    extern GameType sudokuType; // type of Sudoku (e.g. X_SUDOKU))
    extern size_t maxNumber;
    extern size_t numberOfFields;
    extern size_t numberOfContainerSets;
    extern size_t numberOfContainers;

    // init functions
    void initSudoku(Parameters *parameters);
    void setDefaults();
    void setSudokuType(GameType type);
    void dimensionGrid(size_t maxNumber);
    void allocateFields(size_t numberOfFields);
    void setupGrid();
    void releaseGrid();

    Bool isValidSudoku();
    
    // utility functions
    void setValue(Field *field, unsigned value);
    void forbidNumberInNeighbors(Field *field, unsigned n);
    int forbidNumbersInOtherFields(Container *container, unsigned *n, Field **dontTouch);
    int forbidNumber(Field *field, unsigned n);
    int fieldHasCandidate(Field *field, unsigned n);
    int setUniqueNumber(Field *field);
    FieldsVector *fieldsWithCandidate(FieldsVector *fields, unsigned n);
    unsigned equalNumberOfFieldsAndCandidates(FieldsVector *fieldsVector, unsigned *numbers);
    int getUniquePositionInContainer(Field **container, unsigned n);
    int isFinished();
    void cleanUpCandidates();
    int isCandidate(Field *field, unsigned candidate);
    int removeCandidate(Field *field, unsigned candidate);
    void solveField(Field *field, unsigned n);
    Field *getFieldAt(unsigned x, unsigned y);

#ifdef __cplusplus
}
#endif

#endif /* GRID_H */

