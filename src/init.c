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
#include "global.h"
#include "typedefs.h"
#include "util.h"
#include "init.h"
#include "log.h"

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
