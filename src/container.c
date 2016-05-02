/**
 * functions related to mapping fields into some sort of containers.
 * A container is any set of fields except a row or a column. Standard
 * containers are "box" (for standard Sudoku), "diagonal" (X-Sudoku) or
 * "color" (for color Sudoku).
 */
#include <assert.h>
#include "container.h"

void getBoxStartCoordinates(int q, int *qx, int *qy);
    


/**
 * sets up containers
 */
void setupContainers() {
//    typedef int (*myFuncDef)(int, int);
}


//-------------------------------------------------------------------
// Liefert zu dem x-ten Feld eines Quadranten dessen absolute x- und
// y-Koordinaten im Sudoku
// Parameter:
//   q ... Nummer des Quadranten (0..8)
//   position ... Position innerhalb des Quadranten (0..8, wobei 0..2
//     in der ersten Zeile des Quadranten sind)
//   x ... absolute X-Koordinate (0..8), wird zurueckgeliefert
//   y ... absolute Y-Koordinate (0..8), wird zurueckgeliefert

void getCoordinatesInBox(int q, int position, int *x, int *y) {
    int qx, qy;

    assert(q >= 0 && q < 9);
    assert(position >= 0 && position < 9);

    getBoxStartCoordinates(q, &qx, &qy);

    *x = qx + (position % 3);
    *y = qy + (position / 3);
}


//-------------------------------------------------------------------
// Rechnet aus Quadrantenkoordinaten in absolute Koordinaten um.
// Parameter:
//   q ... Nummer (0..8) des Quadranten
//   qx ... X-Koordinate des linken oberen Ecks des Quadranten
//   qx ... Y-Koordinate des linken oberen Ecks des Quadranten

void getBoxStartCoordinates(int q, int *qx, int *qy) {
    *qx = (q % 3) * 3;
    *qy = (q / 3) * 3;
}


//-------------------------------------------------------------------
// Liefert die Nummer des Quadranten, in dem das Feld mit den
// Koordinaten x/y steht
// Quadranten sind von 0 bis 8 durchnummeriert, dh so angeordnet:
// Q0 Q1 Q3
// Q3 Q4 Q5
// Q6 Q7 Q8
// Jeder Quadrant ist eine 3x3-Matrix

int getBoxNr(int x, int y) {

    assert(x >= 0 && x < 9);
    assert(y >= 0 && y < 9);

    return (y / 3) * 3 + (x / 3);
}
