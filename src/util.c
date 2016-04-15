/* 
 * File:   solve.c
 * Author: aurez
 *
 * Created on 04. April 2016, 21:01
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "util.h"


FILE *logfile;


//-------------------------------------------------------------------
// Rechnet aus Quadrantenkoordinaten in Koordinaten, relativ zum
// Quadrant-Ursprung, um.
// Parameter:
//   n ... Nummer der Zelle innerhalb des Quadranten, in dieser Reihenfolge:
//         012
//         345
//         678
//   qx ... X-Koordinate (0..2), relativ zum Quadranten, wird zurueckgeliefert
//   qy ... Y-Koordinate (0..2), relativ zum Quadranten, wird zurueckgeliefert
// Return-Wert:
//   absolute X-Koordinate (0..8) der Zelle

void getQuadrantCell(int n, int *qx, int *qy) {
    *qx = n % 3;
    *qy = n / 3;
}

//-------------------------------------------------------------------
// Rechnet aus Quadrantenkoordinaten in absolute Koordinaten um.
// Parameter:
//   q ... Nummer (0..8) des Quadranten
//   qx ... X-Koordinate (0..2), relativ zum Quadranten
// Return-Wert:
//   absolute X-Koordinate (0..8) der Zelle

int getAbsoluteX(int q, int qx) {
    return (q % 3) * 3 + qx;
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

void getQuadrantField(int q, int position, int *x, int *y) {
    int qx, qy;

    assert(q >= 0 && q < 9);
    assert(position >= 0 && position < 9);

    getQuadrantStart(q, &qx, &qy);

    *x = qx + (position % 3);
    *y = qy + (position / 3);
}


//-------------------------------------------------------------------
// Rechnet aus Quadrantenkoordinaten in absolute Koordinaten um.
// Parameter:
//   q ... Nummer (0..8) des Quadranten
//   qy ... Y-Koordinate (0..2), relativ zum Quadranten
// Return-Wert:
//   absolute Y-Koordinate (0..8) der Zelle

int getAbsoluteY(int q, int qy) {
    return (q / 3) * 3 + qy;
}

//-------------------------------------------------------------------
// Rechnet aus Quadrantenkoordinaten in absolute Koordinaten um.
// Parameter:
//   q ... Nummer (0..8) des Quadranten
//   qx ... X-Koordinate des linken oberen Ecks des Quadranten
//   qx ... Y-Koordinate des linken oberen Ecks des Quadranten

void getQuadrantStart(int q, int *qx, int *qy) {
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

int getQuadrantNr(int x, int y) {

    assert(x >= 0 && x < 9);
    assert(y >= 0 && y < 9);

    return (y / 3) * 3 + (x / 3);
}

void openLogFile(char *outputFilename) {
    logfile = fopen(outputFilename, "w");
}

//-------------------------------------------------------------------

void printlog(char *text) {
    // printlog a message to printlog file or to stdout

    if (logfile) {
        fputs(text, logfile);
    } else {
        // no printlog file => write to stdout
        puts(text);
    }
}

//-------------------------------------------------------------------

void closeLogFile() {
    if (logfile)
        fclose(logfile);
}

/**
 * copies all numbers from one array to another
 * @param src
 * @param len
 * @return 
 */
int *unsigneddup(unsigned const *src, size_t len) {
    int *p = malloc(len * sizeof (unsigned));
    if (p == NULL) {
        exit(EXIT_FAILURE);
    }
    memcpy(p, src, len * sizeof (unsigned));
    return p;
}
