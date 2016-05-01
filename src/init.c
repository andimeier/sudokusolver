/*
 * File:   init.c
 * Author: aurez
 *
 * Created on 01. May 2016, 21:42
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "solve.h"
#include "show.h"
#include "grid.h"
#include "typedefs.h"
#include "util.h"
#include "init.h"
#include "log.h"


static void getQuadrantField(int q, int position, int *x, int *y);
static void getQuadrantStart(int q, int *qx, int *qy);
static int getQuadrantNr(int x, int y);


/**
 * init the units
 */
void initFields() {
    fields = (Field *) xmalloc(sizeof (Field) * NUMBER_OF_FIELDS);

    // alloc candidates
    for (int f = 0; f < NUMBER_OF_FIELDS; f++) {
        unsigned *candidates = (unsigned *) xmalloc(sizeof (unsigned) * MAX_NUMBER);
        fields[f].candidates = candidates;
    }
}

/**
 * init the units
 */
void initUnits() {
    Unit *unit;

    // assuming a standard Sudoku, 
    // we have 3 units (row, column, box)
    unitDefs.units = (Unit *) xmalloc(sizeof (Unit) * 3);
    unitDefs.count = 3;

    // first unit: row
    unit = &(unitDefs.units[ROWS]);
    unit->name = strdup("row");
    unit->containers = MAX_NUMBER;
    unit->fields = (Field ***) xmalloc(sizeof (Field **) * unit->containers);
    for (int i = 0; i < unit->containers; i++) {
        unit->fields[i] = (Field **) xmalloc(sizeof (Field *) * MAX_NUMBER);
    }

    // second unit: column
    unit = &(unitDefs.units[COLS]);
    unit->name = strdup("column");
    unit->containers = MAX_NUMBER;
    unit->fields = (Field ***) xmalloc(sizeof (Field **) * unit->containers);
    for (int i = 0; i < unit->containers; i++) {
        unit->fields[i] = (Field **) xmalloc(sizeof (Field *) * MAX_NUMBER);
    }

    // third unit: box
    unit = &(unitDefs.units[BOXES]);
    unit->name = strdup("box");
    unit->containers = MAX_NUMBER;
    unit->fields = (Field ***) xmalloc(sizeof (Field **) * unit->containers);
    for (int i = 0; i < unit->containers; i++) {
        unit->fields[i] = (Field **) xmalloc(sizeof (Field *) * MAX_NUMBER);
    }
}

/**
 * free units memory
 */
void freeUnits() {

    for (int i = 0; i < unitDefs.count; i++) {
        free(unitDefs.units[i].name);
        for (int n = 0; n < unitDefs.units[i].containers; n++) {
            free(unitDefs.units[i].fields[n]);
        }
        free(unitDefs.units[i].fields);
    }
    free(unitDefs.units);
}

/**
 * free fields memory
 */
void freeFields() {

    for (int f = 0; f < NUMBER_OF_FIELDS; f++) {
        free(fields[f].candidates);
    }

    free(fields);
}

void initGrid() {
    int x, y;
    Field *field;
    Unit *unit;

    assert(unitDefs.count > 0);

    for (int f = 0; f < NUMBER_OF_FIELDS; f++) // FIXME debugging code

        // Initialisierung:
        // zunaechst sind ueberall alle Zahlen moeglich
        for (f = 0; f < NUMBER_OF_FIELDS; f++) {
            field = fields + f;

            x = f % MAX_NUMBER;
            y = f / MAX_NUMBER;

            for (int n = 0; n < MAX_NUMBER; n++) {
                field->candidates[n] = n + 1;
            }

            field->candidatesLeft = MAX_NUMBER;
            field->value = 0;
            field->initialValue = 0;

            int *unitPositions = (int *) xmalloc(sizeof (int) * unitDefs.count);

            unitPositions[ROWS] = y;
            unitDefs.units[ROWS].fields[y][x] = field;

            unitPositions[COLS] = x;
            unitDefs.units[COLS].fields[x][y] = field;

            unitPositions[BOXES] = getQuadrantNr(x, y);
            unitDefs.units[BOXES].fields[unitPositions[BOXES]][y] = field;

            field->unitPositions = unitPositions;

            // use the ROWS and COLS coordinates as the "name" of the field
            char *name = (char *) xmalloc(sizeof (char) * 4);
            sprintf(name, "%c%u", (char) (y + (int) 'A'), x + 1);
            field->name = name;
        }

    // fill units with pointers to the corresponding fields

    // rows
    unit = &(unitDefs.units[ROWS]);
    for (int row = 0; row < MAX_NUMBER; row++) {
        for (int ix = 0; ix < MAX_NUMBER; ix++) {
            field = fields + row * MAX_NUMBER + ix;
            assert(field->unitPositions[ROWS] == row);

            unit->fields[row][ix] = field;
        }
    }

    // cols
    unit = &(unitDefs.units[COLS]);
    for (int col = 0; col < MAX_NUMBER; col++) {
        for (int ix = 0; ix < MAX_NUMBER; ix++) {
            field = fields + ix * MAX_NUMBER + col;
            assert(field->unitPositions[COLS] == col);

            unit->fields[col][ix] = field;
        }
    }

    // boxes
    unit = &(unitDefs.units[BOXES]);
    for (int box = 0; box < MAX_NUMBER; box++) {
        for (int ix = 0; ix < MAX_NUMBER; ix++) {

            getQuadrantField(box, ix, &x, &y);
            field = fields + y * MAX_NUMBER + x;
            assert(field->unitPositions[BOXES] == box);
            assert(field->unitPositions[COLS] == x);
            assert(field->unitPositions[ROWS] == y);

            unit->fields[box][ix] = field;
        }
    }
}

/**
 * frees memory allocated for the grid fields
 */
void freeGrid() {
    for (int f = 0; f < NUMBER_OF_FIELDS; f++) {
        free(fields[f].unitPositions);
        free(fields[f].name);
    }
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
