/*
 * File:   solve.c
 * Author: aurez
 *
 * Created on 04. April 2016, 21:01
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

// search for pairs, triples and quadruples, not more
#define MAX_TUPLE_DIMENSION 4

static int getUniquePositionInContainer(Field **container, unsigned n);

// auxiliary functions
static int setUniqueNumber(Field *field);

UnitDefs unitDefs;
Field *fields; // the fields of the game board


int errors; // number of errors in the algorithm
int verboseLogging; // 0 ... no verbose logging, 1 ... log changes, 2 ... log even considerations

/**
 * init the units
 */
void initFields() {
    fields = (Field *) malloc(sizeof (Field) * 81);
    if (fields == NULL) {
        exit(EXIT_FAILURE);
    }
}

/**
 * init the units
 */
void initUnits() {
    Unit *unit;

    // assuming a standard Sudoku, 
    // we have 3 units (row, column, box)
    unitDefs.units = (Unit *) malloc(sizeof (Unit) * 3);
    if (unitDefs.units == NULL) {
        exit(EXIT_FAILURE);
    }
    unitDefs.count = 3;

    // first unit: row
    unit = &(unitDefs.units[ROWS]);
    unit->name = strdup("row");
    unit->containers = 9;
    unit->fields = (Field ***) malloc(sizeof (Field **) * unit->containers);
    if (unit->fields == NULL) {
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < unit->containers; i++) {
        unit->fields[i] = (Field **) malloc(sizeof (Field *) * 9);
        if (unit->fields[i] == NULL) {
            exit(EXIT_FAILURE);
        }
    }

    // second unit: column
    unit = &(unitDefs.units[COLS]);
    unit->name = strdup("column");
    unit->containers = 9;
    unit->fields = (Field ***) malloc(sizeof (Field **) * unit->containers);
    if (unit->fields == NULL) {
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < unit->containers; i++) {
        unit->fields[i] = (Field **) malloc(sizeof (Field *) * 9);
        if (unit->fields[i] == NULL) {
            exit(EXIT_FAILURE);
        }
    }

    // third unit: box
    unit = &(unitDefs.units[BOXES]);
    unit->name = strdup("box");
    unit->containers = 9;
    unit->fields = (Field ***) malloc(sizeof (Field **) * unit->containers);
    if (unit->fields == NULL) {
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < unit->containers; i++) {
        unit->fields[i] = (Field **) malloc(sizeof (Field *) * 9);
        if (unit->fields[i] == NULL) {
            exit(EXIT_FAILURE);
        }
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
    free(fields);
}

void initGrid() {
    int f, x, y;
    Field *field;
    Unit *unit;

    assert(unitDefs.count > 0);

    // Initialisierung:
    // zunaechst sind ueberall alle Zahlen moeglich
    for (f = 0; f < 81; f++) {
        field = fields + y * 9 + x;

        x = f % 9;
        y = f / 9;

        for (int n = 0; n < 9; n++) {
            field->candidates[n] = n;
        }

        field->candidatesLeft = 9;
        field->value = 0;
        field->initialValue = 0;

        int *unitPositions = (int *) malloc(sizeof (int) * unitDefs.count);
        if (unitPositions == NULL) {
            exit(EXIT_FAILURE);
        }

        unitPositions[ROWS] = y;
        unitDefs.units[ROWS].fields[y][x] = field;

        unitPositions[COLS] = x;
        unitDefs.units[COLS].fields[x][y] = field;

        unitPositions[BOXES] = getQuadrantNr(x, y);
        unitDefs.units[BOXES].fields[unitPositions[BOXES]][y] = field;
        //            field = fields[y * 9 + x];

        field->unitPositions = unitPositions;
    }

    // fill units with pointers to the corresponding fields

    // rows
    unit = &(unitDefs.units[ROWS]);
    for (int row = 0; row < 9; row++) {
        for (int ix = 0; ix < 9; ix++) {
            field = &(fields[row * 9 + ix]);
            assert(field->unitPositions[ROWS] == row);

            unit->fields[row][ix] = field;
        }
    }

    // cols
    unit = &(unitDefs.units[COLS]);
    for (int col = 0; col < 9; col++) {
        for (int ix = 0; ix < 9; ix++) {
            field = &(fields[ix * 9 + col]);
            assert(field->unitPositions[COLS] == col);

            unit->fields[col][ix] = field;
        }
    }

    // boxes
    unit = &(unitDefs.units[BOXES]);
    for (int box = 0; box < 9; box++) {
        for (int ix = 0; ix < 9; ix++) {

            getQuadrantField(box, ix, &x, &y);
            field = &(fields[y * 9 + x]);
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
    for (int f = 0; f < 81; f++) {
        free(fields[f].unitPositions);
    }
}


//-------------------------------------------------------------------
// Checkt, ob alle Zellen mit einer Zahl befuellt sind, dann sind 
// wir naemlich fertig!
// Return-Wert:
//   1 ... fertig (in jeder Zelle steht eine Zahl)
//   0 ... noch nicht fertig

int isFinished() {
    int f;

    for (f = 0; f < 81; f++) {
        if (!fields[f].value)
            // ein leeres Feld gefunden => wir sind noch nicht fertig!
            return 0;
    }
    return 1;
}


//-------------------------------------------------------------------
// Setzt in dem Feld #f (das nur mehr eine Moeglichkeit aufweisen muss)
// die einzige Zahl, die in den Candidates gefunden wird.
// Es muss sichergestellt sein, dass nur mehr eine Zahl moeglich ist,
// hier wird das nicht mehr ueberprueft - der erste Candidate wird als 
// "einzig moegliche Zahl" behandelt.
// Return-Wert:
//   die fixierte Zahl

int setUniqueNumber(Field *field) {
    int n;

    if (field->value) {
        if (verboseLogging == 2) {
            // TODO sprintf(buffer, "FEHLER! HUCH! Obwohl schon ausgefuellt, wird das aufgerufen! (%d/%d) soll gesetzt werden, ist aber bereits %d!\n", f, fields[f].value);
            // TODO printlog(buffer);
            sprintf(buffer, "Fehler vor inc: %d\n", errors); //?DEBUG
            printlog(buffer);
        }
        errors++;
        if (verboseLogging) {
            sprintf(buffer, "Fehler nach inc: %d\n", errors); //?DEBUG
            printlog(buffer);
        }
    }

    unsigned *candidates = field->candidates;
    for (n = 1; n <= 9; n++)
        if (candidates[n - 1]) {
            if (verboseLogging == 2) {
                // TODO sprintf(buffer, "Aha, nur mehr eine Moeglichkeit in Feld (%d/%d) (possibilities: %s): %d\n", y + 1, x + 1, possibilities[y][x], n);
                // TODO printlog(buffer);
            }
            field->value = n;
            break;
        }

    return n;
}

//-------------------------------------------------------------------
// Checkt die Anzahl der moeglichen Vorkommnisse einer Zahl in der
// Unit u.
// Liefert:
//   x ... x-Position des Feldes, in dem die Zahl n als einziges Feld
//         der ganzen Reihe vorkommen koennte oder
//   -1 ... Zahl koennte in der Zeile an mehreren Positionen vorkommen

int getUniquePositionInContainer(Field **container, unsigned n) {
    int i;
    int unique;
    int pos;
    Field *field;

    assert(n >= 1 && n <= 9);

    unique = 0;
    pos = 0;
    for (i = 0; i < 9; i++) {
        field = container[i];
        if ((field->value == n) || (!(field->value) && (field->candidates[n - 1] == n))) {
            if (!unique) {
                unique = 1; // erstes gefundenes Vorkommen in der Reihe
                pos = i; // Position merken, falls sie eindeutig ist
            } else {
                // oje, das waere schon das 2. Vorkommen der Zahl in dieser Reihe
                return -1; // war wohl nix
            }
        }
    }
    if (unique)
        return pos;

    return -1;
}

/**
 * @return 1 if the field is in the field list, 0 if it is not
 */
int containsField(Field **list, Field * field) {
    for (int i = 0; list[i] != NULL; i++) {
        if (field == list[i])
            return 1;

    }
    return 0;
}

//-------------------------------------------------------------------
// "Isoliert" pairs/triples/quads in einem Quadranten: die candidates, die
// in diesen beiden Zellen moeglich sein, koennen im restlichen
// Quadranten nicht mehr vorkommen
// Return-Wert:
//   1 ... mind. 1 Nummer in der restlichen Spalte oder dem restlichen
//         Quadranten wurde verboten, wir "sind weitergekommen"
//   0 ... Isolieren der Zwillinge hat keine Aenderung im Sudoku bewirkt
// TODO im Moment gehen nur Zwillinge, trotz des Funktionsnamens!
//
// @param dontTouch ... NULL terminated list of Field pointers. These fields
//   will not be touched. In all other fields in the container, the given 
//   numbers will be removed as candidates

int forbidNumbersInOtherFields(Field **container, unsigned *n, Field **dontTouch) {
    int progress;
    Field *field;

    progress = 0; // nothing has changed yet
    if (verboseLogging == 2) {
        // TODO sprintf(buffer, "Isoliere Tupel (%d/%d) und (%d/%d): %s/%s\n", y1 + 1, x1 + 1, y2 + 1, x1 + 1, possibilities[y1][x1], possibilities[y2][x2]);
        // TODO printlog(buffer);
    }

    // walk through entire container
    for (int pos = 0; pos < 9; pos++) {
        field = container[pos];

        // don't touch the 'dontTouch' fields
        if (!containsField(dontTouch, field)) {
            // forbid the tuple numbers
            for (int i = 0; i < 9; i++) {
                if (n[i]) {
                    // was a candidate until now => remove candidate now
                    if (field->candidates[i - 1]) {
                        field->candidates[i - 1] = 0;
                        field->candidatesLeft--;
                        progress = 1;
                    }
                }
            }
        }
    }

    return progress;
}

//-------------------------------------------------------------------
// Verbiete eine Zahl in einer bestimmten Zelle
// Return-Wert:
//   1 ... Nummer wurde verboten
//   0 ... keine Aenderung, Nummer war bereits verboten

int forbidNumber(Field *field, unsigned n) {

    assert(n >= 1 && n <= 9);

    if (field->candidates[n - 1]) {
        if (verboseLogging == 2) {
            // TODO sprintf(buffer, "Vorher: (%d/%d) possibilities=%s\n", y + 1, x + 1, possibilities[y][x]);
            printlog(buffer);
        }
        field->candidates[n - 1] = 0;
        if (verboseLogging == 2) {
            // TODO sprintf(buffer, "Nachher: (%d/%d) possibilities=%s)\n", y + 1, x + 1, possibilities[y][x]);
            printlog(buffer);
        }
        field->candidatesLeft--;
        if (field->candidatesLeft == 1) {
            // nur noch eine einzige Zahl ist moeglich => ausfuellen!
            setUniqueNumber(field);
        }
        return 1;
    }
    return 0;
}

//-------------------------------------------------------------------
// check for solved cells and remove candidates from neighbor cells
// @return 1 ... something has changed, 0 ... nothing changed

int checkForSolvedCells() {
    int f;
    Field **container;
    Field *field;
    int value;
    int progress; // Flag: in einer Iteration wurde zumindest eine Erkenntnis gewonnen

    progress = 0;

    for (f = 0; f < 81; f++) {
        field = &(fields[f]);
        value = field->value;
        if (value) {
            // field contains a number => this number must not appear in
            // any other "neighboring" fields (fields within the same unit)

            // forbid number in other cells of the same unit

            for (int u = 0; u < unitDefs.count; u++) {
                Unit *unit = &(unitDefs.units[u]);
                container = unit->fields[field->unitPositions[u]];

                // go through all positions (numbers) of the container and 
                // forbid this number in all other fields of the container
                unsigned candidates[9];

                // build tuple to search for
                for (int i = 0; i < 9; i++) {
                    candidates[i] = 0;
                }
                candidates[value - 1] = value;

                Field * preserve[2];
                preserve[0] = field;
                preserve[1] = NULL;

                progress |= forbidNumbersInOtherFields(container, candidates, preserve);
            }
        }

    }
    return progress;
}

int findHiddenSingles() {
    int progress; // flag: something has changed

    progress = 0;

    // search in all unit types (rows, cols, boxes, ...) for numbers which can 
    // only occur on one position within the unit (even if there would be
    // several candidates for this cell, but the other candidates can be
    // discarded in this case)

    for (int u = 0; u < unitDefs.count; u++) {
        Unit *unit = &(unitDefs.units[u]);

        if (verboseLogging == 2) {
            sprintf(buffer, "??? Searching for: hidden singles in units of type %s ... \n", unit->name);
            printlog(buffer);
        }

        for (int c = 0; c < unit->containers; c++) {
            for (unsigned n = 1; n <= 9; n++) {
                Field **container = unit->fields[c];
                int pos = getUniquePositionInContainer(container, n);
                if (pos != -1 && !container[pos]->value) {
                    // number can only occur in the position pos in this container
                    if (verboseLogging) {
                        // TODO sprintf(buffer, "!!! Neue Erkenntnis 2a: In Zeile %d kann %d nur an Position %d vorkommen => (%d/%d) = %d!\n", y + 1, n, x + 1, y + 1, x + 1, n);
                        // TODO printlog(buffer);
                    }
                    container[pos]->value = n;
                    progress = 1; // Flag "neue Erkenntnis" setzen
                }
            }
        }

    }

    return progress;
}

/**
 * compare two lists of candidates and check if they are equal
 */
int compareCandidates(unsigned *c1, unsigned *c2) {
    for (int n = 0; n < 9; n++) {
        if (c1[n] != c2[n])
            return 0;
    }

    // both are equal
    return 1;
}

/**
 * find naked tuples (pairs, triples, ...) in the same container
 * @param dimension 2 for pairs, 3 for triples, etc.
 * @return 
 */
int findNakedTuples(size_t dimension) {
    Unit *unit;
    int progress; // flag: something has changed
    unsigned n1, n2;

    // allow for pairs, triples and quadruples
    assert(dimension <= MAX_TUPLE_DIMENSION);

    progress = 0;

    // search in all unit types (rows, cols, boxes, ...) for a tuple of numbers 
    // which can only occur on n = size of tuple positions within the unit.
    // So, even if there would be  several candidates for these cells, the 
    // tuple numbers must be distributed among these n fields, so we can discard
    // these tuple candidates in all other cells of the same container

    for (int u = 0; u < unitDefs.count; u++) {
        unit = &(unitDefs.units[u]);
        if (verboseLogging == 2) {
            sprintf(buffer, "??? Searching for: naked tuples of dimension %d in units of type %s ... \n", (int) dimension, unit->name);
            printlog(buffer);
        }

        for (int c = 0; c < unit->containers; c++) {
            for (unsigned n = 1; n <= 9; n++) {
                Field **container = unit->fields[c];

                // check for naked tuples in this container
                for (n1 = 1; n1 < 9; n1++) {
                    for (n2 = 1; n2 < 9; n2++) {
                        unsigned tuple[9];

                        // build tuple to search for
                        for (int i = 0; i < 9; i++) {
                            tuple[i] = 0;
                        }
                        tuple[n1 - 1] = n1;
                        tuple[n2 - 1] = n2;



                        // search for cells with exactly this tuple as
                        // candidates
                        Field * foundTupleFields[MAX_TUPLE_DIMENSION + 1];
                        int countTupleFound = 0;
                        for (int pos = 0; pos < 9; pos++) {
                            unsigned *candidates = container[pos]->candidates;
                            // check candidates if they match the tuple
                            if (compareCandidates(candidates, tuple)) {
                                foundTupleFields[countTupleFound++] = container[pos];
                                if (countTupleFound == dimension)
                                    // terminate list of field pointers
                                    foundTupleFields[countTupleFound] = NULL;
                                break;
                            }
                        }
                        if (countTupleFound == dimension) {
                            // we found "dimension" places in the container
                            // containing the tuple => these numbers must be
                            // distributed among these found fields => forbid
                            // these numbers in all other fields of the container
                            progress |= forbidNumbersInOtherFields(container, tuple, foundTupleFields);
                        }
                    }
                }
            }
        }
    }

    return progress;

}

/*
int findNakedPairsOBSOLETE() {
    int x, y, i, j;
    int q;
    int x1, x2, y1, y2, qx, qy;
    int progress; // Flag: in einer Iteration wurde zumindest eine Erkenntnis gewonnen

    progress = 0;

    // Suche nach Zwillingen in einem Quadranten (nicht unbedingt in der gleichen Zeile oder Spalte):
    // ----------------------------------------------------------------------------------------------
    // wenn zwei Felder in der gleichen Zeile die gleichen 2 moeglichen 
    // Zahlen haben, muessen jeweils eine dieser beiden Zahlen in 
    // jeweils einer dieser 2 Zellen stehen => damit koennen beide 
    // Zahlen im restlichen Quadranten nicht mehr vorkommen.
    // Wenn die beiden auch noch in der selben Zeile sind, kann auch 
    // in der restlichen Zeile keine dieser Zahlen mehr vorkommen.
    // Analog fuer Spalten.
    if (verboseLogging == 2) printlog("??? Searching for: twins ... \n");

    for (q = 0; q < 9; q++) {
        if (verboseLogging == 2) {
            sprintf(buffer, "Untersuche Quadrant %d auf Zwillinge ...\n", q + 1);
            printlog(buffer);
        }
        getQuadrantStart(q, &qx, &qy);
        // Vergleiche jedes Feld im Quadranten mit jedem anderen im selben Quadranten
        for (i = 0; i < 9; i++) {
            getQuadrantCell(i, &x1, &y1);
            x1 += qx;
            y1 += qy;
            for (j = i + 1; j < 9; j++) {
                getQuadrantCell(j, &x2, &y2);
                x2 += qx;
                y2 += qy;

                if ((nrOfPossibilities[y1][x1] == 2 && nrOfPossibilities[y2][x2] == 2)
                        && !strcmp(possibilities[y1][x1], possibilities[y2][x2])) {
                    // ja, wird haben Quadranten-Zwillinge => im restlichen Quadranten 
                    // koennen diese 2 Zahlen nicht mehr vorkommen!
                    if (verboseLogging == 2) {
                        sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 3a: Zwillinge! Feld (%d/%d) und Feld (%d/%d) sind im gleichen Quadranten und haben beide: %s\n", y1 + 1, x1 + 1, y2 + 1, x2 + 1, possibilities[y1][x1]);
                        printlog(buffer);
                    }
                    if (isolateBoxTuples(q, y1, x1, y2, x2))
                        progress = 1;
                }
            }
        }
    }

    if (verboseLogging) {
        printSvg(0);
    }


    // Suche nach Zwillingen in einer Zeile oder einer Spalte (nicht unbedingt in einem Quadranten):
    // ---------------------------------------------------------------------------------------------
    // wenn zwei Felder in der gleichen Zeile die gleichen 2 moeglichen 
    // Zahlen haben, muessen jeweils eine dieser beiden Zahlen in 
    // jeweils einer dieser 2 Zellen stehen => damit koennen beide 
    // Zahlen in der restlichen Zeile nicht mehr vorkommen

    // alle Zeilen durchgehen
    for (y = 0; y < 9; y++) {
        if (verboseLogging == 2) {
            sprintf(buffer, "Untersuche Reihe %d auf Zwillinge ...\n", y + 1);
            printlog(buffer);
        }
        // suche Zwillinge in dieser Reihe
        for (x1 = 0; x1 < 9; x1++) {
            for (x2 = x1 + 1; x2 < 9; x2++) {
                // vergleiche die beiden Zellen: sind es Zwillinge?
                if ((nrOfPossibilities[y][x1] == 2 && nrOfPossibilities[y][x2] == 2)
                        && !strcmp(possibilities[y][x1], possibilities[y][x2])) {
                    // ja, x1, x2 sind Zwillinge => in der restlichen Zeile
                    // koennen diese 2 Zahlen nicht mehr vorkommen!
                    if (verboseLogging == 2) {
                        sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 3b: Zwillinge! Feld (%d/%d) und Feld (%d/%d) haben beide: %s\n", y + 1, x1 + 1, y + 1, x2 + 1, possibilities[y][x1]);
                        printlog(buffer);
                    }
                    if (isolateRowTwins(y, x1, x2))
                        progress = 1;
                }
            }
        }
    }

    if (verboseLogging) {
        printSvg(0);
    }

    // alle Spalten durchgehen
    for (x = 0; x < 9; x++) {
        if (verboseLogging == 2) {
            sprintf(buffer, "Untersuche Spalte %d auf Zwillinge ...\n", x + 1);
            printlog(buffer);
        }
        // suche Zwillinge in dieser Spalte
        for (y1 = 0; y1 < 9; y1++) {
            for (y2 = y1 + 1; y2 < 9; y2++) {
                // vergleiche die beiden Zellen: sind es Zwillinge?
                if ((nrOfPossibilities[y1][x] == 2 && nrOfPossibilities[y2][x] == 2)
                        && !strcmp(possibilities[y1][x], possibilities[y2][x])) {
                    // ja, y1, y2 sind Zwillinge => in der restlichen Spalte
                    // koennen diese 2 Zahlen nicht mehr vorkommen!
                    if (verboseLogging == 2) {
                        sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 3c: Zwillinge! Feld (%d/%d) und Feld (%d/%d) haben beide: %s\n", y1 + 1, x + 1, y2 + 1, x + 1, possibilities[y1][x]);
                        printlog(buffer);
                    }
                    if (isolateColumnTwins(x, y1, y2))
                        progress = 1;
                }
            }
        }
    }

    return progress;
}
 */
int findHiddenPairs() {
    int y;
    int cand;
    int progress;

    // http://programmers.stackexchange.com/questions/270930/sudoku-hidden-sets-algorithm

    progress = 0;

    // hidden pairs in rows
    for (y = 0; y < 9; y++) {
        for (cand = 1; cand <= 9; cand++) {
            // countCandidateInRow(cand, y);
        }

    }

    return progress;
}