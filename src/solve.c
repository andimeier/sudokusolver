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
#include "log.h"

// search for pairs, triples and quadruples, not more
#define MAX_TUPLE_DIMENSION 4

static int getUniquePositionInContainer(Field **container, unsigned n);

// auxiliary functions
static int setUniqueNumber(Field *field);
static FieldsVector *fieldsWithCandidate(FieldsVector *container, unsigned n);
static int fieldHasCandidate(Field *field, unsigned n);
static unsigned recurseNakedTuples(unsigned maxLevel, FieldsVector *container, unsigned level, unsigned *numbers, FieldsVector *fieldsContainingCandidates);
static int fieldCandidatesSubsetOf(Field *field, unsigned *numbers);
static unsigned equalNumberOfFieldsAndCandidates(FieldsVector *fieldsVector, unsigned *numbers);
static int forbidNumbersInOtherFields(Field **container, unsigned *n, Field **dontTouch);
static void forbidNumberInNeighbors(Field *field, unsigned n);
static void setValue(Field *field, unsigned value);

UnitDefs unitDefs;
Field *fields; // the fields of the game board


int errors; // number of errors in the algorithm
int verboseLogging; // 0 ... no verbose logging, 1 ... log changes, 2 ... log even considerations

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
// Checkt, ob alle Zellen mit einer Zahl befuellt sind, dann sind 
// wir naemlich fertig!
// Return-Wert:
//   1 ... fertig (in jeder Zelle steht eine Zahl)
//   0 ... noch nicht fertig

int isFinished() {
    int f;

    for (f = 0; f < NUMBER_OF_FIELDS; f++) {
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
    unsigned n;

    if (field->value) {
        if (verboseLogging == 2) {
            sprintf(buffer, "Ouch! Already containing a value, but \"setUniqueNumber\" is called! Field %s is already %u!\n", field->name, field->value);
            printlog(buffer);
        }
        errors++;
    }

    unsigned *candidates = field->candidates;
    for (n = 1; n <= MAX_NUMBER; n++) {
        if (candidates[n - 1]) {
            if (verboseLogging == 2) {
                // TODO sprintf(buffer, "Aha, nur mehr eine Moeglichkeit in Feld %s (possibilities: %s): %d\n", field->name, possibilities[y][x], n);
                // TODO printlog(buffer);
            }
            setValue(field, n);
            break;
        }
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
    int pos;
    int unique;
    int foundPos;
    Field *field;

    assert(n >= 1 && n <= MAX_NUMBER);
    printf("Looking for unique position of %u in container ...\n", n);

    for (pos = 0; pos < MAX_NUMBER; pos++) { // FIXME debugging output
        showCandidates(container[pos]);
    }

    unique = 0;
    foundPos = 0;
    for (pos = 0; pos < MAX_NUMBER; pos++) {
        field = container[pos];
        if ((field->value == n) || (!(field->value) && (field->candidates[n - 1] == n))) {
            printf("Field %d/%d can contain candidate %u\n", field->unitPositions[ROWS], field->unitPositions[COLS], n);
            if (!unique) {
                unique = 1; // first occurrence in the current container
                foundPos = pos; // remember position, in case it is the only one
            } else {
                // what a pity, this is the second occurrence of this number in the container
                return -1; // => apparently no hidden single
            }
        }
    }
    if (unique) {
        return foundPos;
    }

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
// "Isoliert" pairs/triples/quads in einem Container: die candidates, die
// in diesen beiden Zellen moeglich sein, koennen im restlichen
// Container nicht mehr vorkommen
// Return-Wert:
//   1 ... mind. 1 Nummer in der restlichen Spalte oder dem restlichen
//         Quadranten wurde verboten, wir "sind weitergekommen"
//   0 ... Isolieren der Zwillinge hat keine Aenderung im Sudoku bewirkt
// TODO im Moment gehen nur Zwillinge, trotz des Funktionsnamens!
//
// @param n ... tuple of candidates to be removed from "other fields". This
//   is a vector of MAX_NUMBER numbers, each position stands for the respective
//   candidate, e.g. an array of [ 0, 2, 0, 0, 5, 6, 0, 0, 0 ] means that
//   the candidates 2, 5 and 6 shall be removed from all "other fields" (fields
//   other than those in parameter dontTouch)
// @param dontTouch ... NULL terminated list of Field pointers. These fields
//   will not be touched. In all other fields in the container, the given 
//   numbers will be removed as candidates

int forbidNumbersInOtherFields(Field **container, unsigned *n, Field **dontTouch) {
    int progress;
    Field *field;

    printf("forbid number in container\n");
    showAllCandidates();

    progress = 0; // nothing has changed yet
    if (verboseLogging == 2) {
        // TODO sprintf(buffer, "Isoliere Tupel (%d/%d) und (%d/%d): %s/%s\n", y1 + 1, x1 + 1, y2 + 1, x1 + 1, possibilities[y1][x1], possibilities[y2][x2]);
        // TODO printlog(buffer);
    }

    // walk through entire container
    for (int pos = 0; pos < MAX_NUMBER; pos++) {
        field = container[pos];

        // don't touch the 'dontTouch' fields
        if (!containsField(dontTouch, field)) {
            // forbid the tuple numbers
            for (int i = 0; i < MAX_NUMBER; i++) {
                if (n[i]) {
                    // was a candidate until now => remove candidate now
                    if (!field->value && field->candidates[i]) {
                        sprintf(buffer, "forbid %u in field %s\n", i + 1, field->name);
                        logReduction(buffer);

                        field->candidates[i] = 0;
                        field->candidatesLeft--;
                        progress = 1;
                    }
                }
            }
        }
    }

    showAllCandidates();

    return progress;
}

//-------------------------------------------------------------------
// Verbiete eine Zahl in einer bestimmten Zelle
// Return-Wert:
//   1 ... Nummer wurde verboten
//   0 ... keine Aenderung, Nummer war bereits verboten

int forbidNumber(Field *field, unsigned n) {

    assert(n >= 1 && n <= MAX_NUMBER);

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
// set all candidates for all fields initially

void initCandidates() {
    int f;
    Field *field;

    for (f = 0; f < NUMBER_OF_FIELDS; f++) {
        field = fields + f;

        if (field->value) {
            sprintf(buffer, "Set value of field %s (#%d) to %u\n", field->name, f, field->value);
            printlog(buffer);
            setValue(field, field->value);
        }
    }

    printlog("Initial candidates are:\n");
    showAllCandidates();
}

//-------------------------------------------------------------------
// check for cells having only one candidate left and set their value (and
// then eliminate this value in neighboring fields)
// @return 1 ... something has changed, 0 ... nothing changed

int checkForSolvedCells() {
    int f;
    Field *field;
    int value;
    int progress; // Flag: in einer Iteration wurde zumindest eine Erkenntnis gewonnen

    progress = 0;

    showAllCandidates();

    for (f = 0; f < NUMBER_OF_FIELDS; f++) {
        field = fields + f;
        value = field->value;

        if (field->candidatesLeft == 1 && !field->value) {
            setUniqueNumber(field);
            progress = 1;
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
            for (unsigned n = 1; n <= MAX_NUMBER; n++) {
                Field **container = unit->fields[c];
                int pos = getUniquePositionInContainer(container, n);
                if (pos != -1 && !container[pos]->value) {
                    // number can only occur in the position pos in this container
                    if (verboseLogging) {
                        // TODO sprintf(buffer, "!!! Neue Erkenntnis 2a: In Zeile %d kann %d nur an Position %d vorkommen => (%d/%d) = %d!\n", y + 1, n, x + 1, y + 1, x + 1, n);
                        // TODO printlog(buffer);
                    }
                    setValue(container[pos], n);

                    Field *field = container[pos];
                    sprintf(buffer, "*** [hidden single] hidden single in unit %s, field %d/%d: %u ... \n", unit->name, field->unitPositions[ROWS], field->unitPositions[COLS], n);
                    printlog(buffer);

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
    for (int n = 0; n < MAX_NUMBER; n++) {
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

    printf("- yeah: ...\n");

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

        printf("container \"%s\" has %zu instances\n", unit->name, unit->containers);
        for (int c = 0; c < unit->containers; c++) {
            Field **container = unit->fields[c];

            printf("iterating into instance %d of container \"%s\"\n", c, unit->name);

            // check for naked tuples in this container
            for (n1 = 1; n1 <= MAX_NUMBER; n1++) {
                for (n2 = 1; n2 <= MAX_NUMBER; n2++) {
                    unsigned tuple[MAX_NUMBER];

                    printf("??? [kdb] Searching for: naked tuples of dimension %d (%u, %u) in units of type %s, #%d ... \n", (int) dimension, n1, n2, unit->name, c);

                    // build tuple to search for
                    for (int i = 0; i < MAX_NUMBER; i++) {
                        tuple[i] = 0;
                    }
                    tuple[n1 - 1] = n1;
                    tuple[n2 - 1] = n2;


                    printf("tuple: (%u, %u)\n", tuple[2], tuple[7]);

                    // search for cells with exactly this tuple as
                    // candidates
                    Field * foundTupleFields[MAX_TUPLE_DIMENSION + 1];
                    int countTupleFound = 0;
                    for (int pos = 0; pos < MAX_NUMBER; pos++) {
                        unsigned *candidates = container[pos]->candidates;
                        // check candidates if they match the tuple
                        if (tuple[2] == 3 && tuple[7] == 8 && u == 2 && c == 0) {
                            printf("[jhh1] {pass %d of %d} checking position %d of box 0 ... \n", pos, MAX_NUMBER, pos);
                            showCandidates(container[pos]);
                        }
                        if (tuple[3] == 4 && tuple[6] == 7 && u == 2 && c == 5) {
                            printf("[jhh2] {pass %d of %d} checking position %d of box 0 ... \n", pos, MAX_NUMBER, pos);
                            showCandidates(container[pos]);
                        }
                        if (compareCandidates(candidates, tuple)) {
                            printf("YEAH found!\n");
                            foundTupleFields[countTupleFound++] = container[pos];
                            if (countTupleFound == dimension) {
                                // terminate list of field pointers
                                foundTupleFields[countTupleFound] = NULL;
                                break;
                            }
                        } else {
                            printf("oooh not found!\n");
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
    for (y = 0; y < MAX_NUMBER; y++) {
        for (cand = 1; cand <= MAX_NUMBER; cand++) {
            // countCandidateInRow(cand, y);
        }

    }

    return progress;
}

int recurseHiddenTuples(unsigned maxLevel, FieldsVector *fields, unsigned level, unsigned *candidates, FieldsVector *fieldsContainingCandidates) {

    // make room for new candidate in the candidates vector
    candidates[level + 1] = 0;

    // add next number to numbers vector
    for (unsigned n = 1; n < MAX_NUMBER; n++) {
        candidates[level] = n;

        // check this combination of candidates (in the candidates vector) 
        // whether there are only length(candidates) Sudoku fields in which
        // the candidates can occur => in this case this would be a hidden
        // tuple of length length(candidates)

        // FIXME setFieldsContainingCandidates(fieldsContainingCandidates, n);

        // recurse further?
        // FIXME 
        //        if () {
        //        }
    }

    // "rollback" recursion
    candidates[level] = 0;

    return 0; // FIXME ????
}

/**
 * find pointing pairs or triples
 * 
 * @return progress flag: 1 for "something has changed", 0 for "no change"
 */
int findPointingTupels() {
    Unit *unit;
    int progress; // flag: something has changed
    unsigned tuple[MAX_NUMBER];
    unsigned n;

    progress = 0;


    printf("[pii] starting findPointingTupels ...\n");

    // search in all unit types (rows, cols, boxes, ...) for a tuple of numbers 
    // which form a "pointing tuple"

    for (int u = 0; u < unitDefs.count; u++) {
        unit = &(unitDefs.units[u]);
        if (verboseLogging == 2) {
            sprintf(buffer, "??? Searching for: pointing tuples in units of type %s ... \n", unit->name);
            printlog(buffer);
        }

        printf("container \"%s\" has %zu instances\n", unit->name, unit->containers);
        for (int c = 0; c < unit->containers; c++) {
            FieldsVector *container = unit->fields[c];

            showAllCandidates();
            show(0);
            printf("Alex\n");
            show(0);
            sudokuString();

            printf("iterating into instance %d of container \"%s\"\n", c, unit->name); // FIXME debugging output

            // check for pointing tuples in this container
            for (n = 1; n <= MAX_NUMBER; n++) {
                FieldsVector *fieldsVector;

                // collect all fields which contain this candidate
                printf("get fields with candidate %u in unit type %s ... \n", n, unit->name); // FIXME debugging output
                fieldsVector = fieldsWithCandidate(container, n);
                printf("got fields:\n"); // FIXME debugging output
                Field **ptr = fieldsVector; // FIXME debugging variable
                while (*ptr) { // FIXME debugging output
                    //                    printf("ptr point to address %d\n", (int) *ptr);
                    printf("  candidate %u is possible in field %d/%d\n", n, (*ptr)->unitPositions[ROWS], (*ptr)->unitPositions[COLS]);
                    ptr++;
                }

                printf("[hhh1]\n");

                if (*fieldsVector == NULL) {
                    // candidate n not found in any free field => skip it
                    printf("[hhh2]\n");
                    continue;
                }
                printf("[hhh3]\n");

                // for every unit type other than the current one, check if
                // all fields of the tuple share the same "other" unit instance.
                // If so, the candidate can be removed from all fields in the
                // "other" instance except for the fields in the tuple (in the
                // current container)

                for (int u2 = 0; u2 < unitDefs.count; u2++) {
                    if (u2 == u) {
                        // only look in OTHER unit types
                        printf("[hhh4]\n");
                        continue;
                    }

                    printf("[hhh5]\n");
                    printf("  look at unit %s ...\n", unitDefs.units[u2].name);

                    //                    for (int f2 = 0; f2 < NUMBER_OF_FIELDS; f2++) { // FIXME debugging output
                    //                        printf("[1234] field #%d: in row %d, col %d, box %d\n", f2, fields[f2].unitPositions[ROWS], fields[f2].unitPositions[COLS], fields[f2].unitPositions[BOXES]);
                    //                    }


                    int containerIndex;
                    // check if all fields share the same instance of the "other unit"
                    Field **fieldsPtr;
                    fieldsPtr = fieldsVector;
                    containerIndex = (*fieldsPtr)->unitPositions[u2];
                    printf("the tupel MIGHT be in %s #%d ...\n", unitDefs.units[u2].name, containerIndex);
                    fieldsPtr++;
                    while (*fieldsPtr) {
                        printf("[jjj2] check next position ...\n");
                        if ((*fieldsPtr)->unitPositions[u2] != containerIndex) {
                            printf("[jjj3] FOUND SOMETHING => no tupel...\n");
                            break;
                        }

                        fieldsPtr++;
                    }

                    printf("[jjj4] finished\n");

                    // found pointing tuple?
                    if (!(*fieldsPtr)) {
                        printf("[lkkk] Found pointing tuple in %s #%d ...) \n", unitDefs.units[u2].name, containerIndex);

                        // prepare tuple
                        for (int i = 0; i < MAX_NUMBER; i++) {
                            tuple[i] = 0;
                        }
                        tuple[n - 1] = n;

                        progress |= forbidNumbersInOtherFields(unitDefs.units[u2].fields[containerIndex], tuple, fieldsVector);
                    }
                }

                free(fieldsVector);
            }
        }
    }

    return progress;

}

/**
 * collects all unresolved fields in which the given number is a possible 
 * candidate
 * 
 * @param container
 * @param n the number to look for as a candidate
 * @return vector of fields containing the given number as possible candidate
 */
FieldsVector *fieldsWithCandidate(Field **container, unsigned n) {
    int ix;
    Field *field;
    FieldsVector *found;
    FieldsVector *foundPtr;

    found = (FieldsVector *) xmalloc(sizeof (Field *) * (MAX_NUMBER + 1));

    foundPtr = found;
    for (ix = 0; ix < MAX_NUMBER; ix++) {
        field = *(container + ix);
        if (fieldHasCandidate(field, n)) {
            *foundPtr = field;
            foundPtr++;
        }
    }

    // terminate vector
    foundPtr = NULL;

    return found;
}

/**
 * checks if the field is unresolved and the given number is a possible 
 * candidate
 * 
 * @param field the field to check
 * @param n the number to check
 * @return 1 if the number is a possible candidate, 0 if it is not
 */
int fieldHasCandidate(Field *field, unsigned n) {
    return !field->value && (field->candidates[n - 1] == n);
}

/**
 * find naked tuples (pairs, triples, ...) which share the same candidates.
 * For instance, if two fields have only the candidates 2 and 4, then 2 and 4
 * can be eliminated from all other fields in the same container.
 * 
 * @param container vector of fields (=container) in which we look for naked 
 *   tuples
 * @param dimension dimension of the tupel to be looked for. 2=pairs, 
 *   3=triples etc.
 * @return progress flag: 1 for "something has changed", 0 for "no change"
 */
unsigned findNakedTuplesInContainer(FieldsVector *container, unsigned dimension) {
    unsigned progress;
    unsigned *numbers;
    FieldsVector *foundFields;

    assert(dimension > 0 && dimension < MAX_NUMBER);

    progress = 0;
    numbers = (unsigned *) xmalloc(sizeof (unsigned) * (dimension + 1));
    foundFields = (FieldsVector *) xmalloc(sizeof (FieldsVector) * (dimension + 1));

    // we are in level 0 of recursion: initialize numbers vector
    numbers[0] = 0;
    foundFields[0] = NULL;

    if (recurseNakedTuples(dimension, container, 1, numbers, foundFields)) {
        progress = 1;
    }

    free(foundFields);
    free(numbers);

    return progress;
}

/**
 * recursively look for naked tuples of the dimension maxLevel.
 * Note: finds tupels of the given dimension and below the given dimension.
 * For example, if dimension is 3, then naked triples are found, but naked pairs
 * as well.
 * @param maxLevel
 * @param fields vector of found fields, terminated with NULL
 * @param level
 * @param numbers numbers vector to be searched for, terminated with 0
 * @param fieldsContainingCandidates
 * @return 1 if a naked tuple has been found, 0 otherwise
 */
unsigned recurseNakedTuples(unsigned maxLevel, FieldsVector *container, unsigned level, unsigned *numbers, FieldsVector *fieldsContainingCandidates) {

    if (level > maxLevel) {
        // maximum recursion depth reached => nothing found
        return 0;
    }

    // iterate through all numbers of this level
    numbers[level] = 0;
    fieldsContainingCandidates[level] = NULL;
    for (unsigned number = 0; number < MAX_NUMBER; number++) {
        numbers[level - 1] = number;

        for (unsigned i = 0; i < MAX_NUMBER; i++) {
            Field *field;

            field = container[i];
            if (fieldCandidatesSubsetOf(field, numbers)) {
                fieldsContainingCandidates[level - 1] = field;

                // check if we have found enough fields
                if (equalNumberOfFieldsAndCandidates(fieldsContainingCandidates, numbers)) {
                    unsigned progress;

                    // found a naked tuple!
                    progress = 0;

                    // eliminate the found numbers of the naked tuple from
                    // all other field of the same container
                    // TODO and from other containers if all found fields 
                    // share the same other container
                    progress |= forbidNumbersInOtherFields(container, numbers, fieldsContainingCandidates);
                    return progress;
                }
            }
        }

        // no tuple of dimension "level" found => recurse further
        return recurseNakedTuples(maxLevel, container, level + 1, numbers, fieldsContainingCandidates);
    }

    return 0;
}

/**
 * checks if the possible candidates for a field are a subset of the candidates
 * given in the parameter
 * 
 * @param field pointer to field for which the candidates should be checked
 * @param numbers vector of numbers, terminated with 0
 * @return 1 if the field's candidates are a (strict or non-strict) subset of
 *   the given numbers vector. 0 if they are not.
 */
int fieldCandidatesSubsetOf(Field *field, unsigned *numbers) {

    while (*numbers) {
        if (!field->candidates[*numbers]) {
            // if any candidate is found which is not in "numbers", the field's
            // candidates are no subset of "numbers"
            return 0;
        }
        numbers++;
    }

    return 1;
}

/**
 * checks if the number of fields in the FieldsVector and the number of numbers
 * in the numbers vector are equal.
 * 
 * @param fieldsVector vector or pointers to Fields, terminated with a NULL 
 *   pointer
 * @param numbers vector of numbers, terminated with 0
 * @return 1 both have equal length. 0 if they differ
 */
unsigned equalNumberOfFieldsAndCandidates(FieldsVector *fieldsVector, unsigned *numbers) {

    do {
        if (*fieldsVector == NULL && *numbers == 0) {
            return 1;
        }

        // if we are still here, then at least one of the vectors is not null.
        // However, if the other one is exhausted, then both vectors apparently
        // do not have the same length
        if (*fieldsVector == NULL || *numbers == 0) {
            return 0;
        }

        fieldsVector++;
        numbers++;
    } while (1);
}




//FieldsVector **containersContainingAllFields(FieldsVector *fields) {
//}

/**
 * sets the value of a field and eliminates this number from all candidates
 * of neighboring fields
 * 
 * @param field pointer to the Field structure
 * @param value the number to be set as result field value
 */
void setValue(Field *field, unsigned value) {
    assert(value <= MAX_NUMBER);

    field->value = value;

    unsigned *candidates = field->candidates;
    for (unsigned n = 1; n <= MAX_NUMBER; n++) {
        candidates[n - 1] = (n == value) ? value : 0;
    }

    forbidNumberInNeighbors(field, value);
}

/**
 * forbids a number in all neighbor fields of the given field. This is used
 * e.g. after setting the value of a field to eliminate this number from all
 * neighbors.
 * 
 * @param field
 * @param n
 */
void forbidNumberInNeighbors(Field *field, unsigned n) {
    Field **container;

    assert(n <= MAX_NUMBER);

    sprintf(buffer, "Forbid number %u in neighbors of field %s ...\n", n, field->name);
    printlog(buffer);

    // forbid number in all other "neighboring fields"
    for (int u = 0; u < unitDefs.count; u++) {
        printf("[6hshhs]\n");
        Unit *unit = &(unitDefs.units[u]);
        printf("[6hshhs++]\n");
        container = unit->fields[field->unitPositions[u]];

        // go through all positions (numbers) of the container and 
        // forbid this number in all other fields of the container
        unsigned candidates[MAX_NUMBER];

        // build tuple to search for
        for (int i = 0; i < MAX_NUMBER; i++) {
            candidates[i] = 0;
        }
        candidates[n - 1] = n;

        // preserve candidate in "our" field only
        Field * preserve[2];
        preserve[0] = field;
        preserve[1] = NULL;

        forbidNumbersInOtherFields(container, candidates, preserve);
    }
}