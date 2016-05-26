/**
 * File:   grid.c
 * Author: aurez
 *
 * functions for creating the Sudoku grid, releasing (freeing) it. And some
 * utility functions.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grid.h"
#include "log.h"
#include "util.h"
#include "container.h"

// function prototypes
void initFields();
void initUnits();
void initGrid();
void freeUnits();
void freeGrid();
void freeFields();


UnitDefs unitDefs;
Field *fields; // the fields of the game board
Container **allContainers; // all containers of the game board

void setupGrid() {
    initFields();
    initUnits();
    initGrid();
}

void releaseGrid() {
    freeUnits();
    freeGrid();
    freeFields();
}

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
 * init the units (containers)
 */
void initUnits() {
    Unit *unit;
    unsigned numberOfContainers;

    numberOfContainers = 0;

    // assuming a standard Sudoku, 
    // we have 3 units (row, column, box)
    unitDefs.units = (Unit *) xmalloc(sizeof (Unit) * 3);
    unitDefs.count = 3;

    // first unit: row
    unit = &(unitDefs.units[ROWS]);
    unit->name = strdup("row");
    unit->containers = MAX_NUMBER;
    unit->theContainers = (Container *) xmalloc(sizeof (Container) * unit->containers);
    for (int i = 0; i < unit->containers; i++) {
        Container *container = &(unit->theContainers[i]);
        sprintf(buffer, "row %c", (char) ('A' + i));
        container->name = strdup(buffer);
        container->type = ROWS;
        container->fields = (Field **) xmalloc(sizeof (Field *) * MAX_NUMBER);
        numberOfContainers++;
    }

    // second unit: column
    unit = &(unitDefs.units[COLS]);
    unit->name = strdup("column");
    unit->containers = MAX_NUMBER;
    unit->theContainers = (Container *) xmalloc(sizeof (Container) * unit->containers);
    for (int i = 0; i < unit->containers; i++) {
        Container *container = &(unit->theContainers[i]);
        sprintf(buffer, "column %u", i + 1);
        container->name = strdup(buffer);
        container->type = COLS;
        container->fields = (Field **) xmalloc(sizeof (Field *) * MAX_NUMBER);
        numberOfContainers++;
    }

    for (int c = 0; c < unitDefs.units[COLS].containers; c++) {
        // FIXME debugging output:
        sprintf(buffer, "COLS, next container, name=%s", unitDefs.units[COLS].theContainers[c].name);
        printlog(buffer);
    }

    // third unit: box
    unit = &(unitDefs.units[BOXES]);
    unit->name = strdup("box");
    unit->containers = MAX_NUMBER;
    unit->theContainers = (Container *) xmalloc(sizeof (Container) * unit->containers);
    for (int i = 0; i < unit->containers; i++) {
        Container *container = &(unit->theContainers[i]);
        sprintf(buffer, "box %u", i + 1);
        container->name = strdup(buffer);
        container->type = BOXES;
        container->fields = (Field **) xmalloc(sizeof (Field *) * MAX_NUMBER);
        numberOfContainers++;
    }

    // init and populate "all containers" vector
    allContainers = (Container **) xmalloc(sizeof (Container *) * (numberOfContainers + 1));
    Container **containersPtr = allContainers;

    sprintf(buffer, "name of first container ever (should be 'row A') is: %s",
            unitDefs.units[0].theContainers[0].name);
    printlog(buffer);

    for (int i = 0; i < unitDefs.count; i++) {
        Container *unitContainer;

        sprintf(buffer, "populating container type %d ...", i);
        printlog(buffer);

        unitContainer = unitDefs.units[i].theContainers;

        // FIXME gehe durch alle Units und packe alle gefundenen Container
        // in den ContainerVector ...
        for (int c = 0; c < unitDefs.units[i].containers; c++) {
            // FIXME debugging output:
            sprintf(buffer, "name (should be: row A): %s", unitContainer[c].name);
            printlog(buffer);

            sprintf(buffer, "  populating container type %d, number %d,  ...", i, c);
            printlog(buffer);
            *containersPtr = &(unitContainer[c]);
            containersPtr++;
        }
    }
    *containersPtr = NULL;

    for (int c = 0; c < unitDefs.units[COLS].containers; c++) {
        // FIXME debugging output:
        sprintf(buffer, "COLS, next container, name=%s", unitDefs.units[COLS].theContainers[c].name);
        printlog(buffer);
    }

    // DEBUG FIXME remove me, just debugging output:
    Container **ptr = allContainers;
    int i = 0;
    while (*ptr) {
        sprintf(buffer, "container #%d: %s", i, (*ptr)->name);
        printlog(buffer);
        ptr++;
        i++;
    }
    // end of DEBUG FIXME
}

/**
 * free units memory
 */
void freeUnits() {

    for (int i = 0; i < unitDefs.count; i++) {
        free(unitDefs.units[i].name);
        for (int n = 0; n < unitDefs.units[i].containers; n++) {
            free(unitDefs.units[i].theContainers[n].name);
            free(unitDefs.units[i].theContainers[n].fields);
        }
        free(unitDefs.units[i].theContainers);
    }
    free(unitDefs.units);

    free(allContainers);

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

    // Initialisierung:
    // zunaechst sind ueberall alle Zahlen moeglich
    for (int f = 0; f < NUMBER_OF_FIELDS; f++) {
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
        unitDefs.units[ROWS].theContainers[y].fields[x] = field;

        unitPositions[COLS] = x;
        unitDefs.units[COLS].theContainers[x].fields[y] = field;

        unitPositions[BOXES] = getBoxNr(x, y);
        unitDefs.units[BOXES].theContainers[unitPositions[BOXES]].fields[y] = field;

        field->unitPositions = unitPositions;

        // use the ROWS and COLS coordinates as the "name" of the field
        // reserve space for coordinates up to "Z26" (a theoretical limit of
        // a 26-number-Sudoku)
        char *name = (char *) xmalloc(sizeof (char) * 4);
        sprintf(name, "%c%u", (char) (y + (int) 'A'), x + 1);
        field->name = name;
    }

    // fill units with pointers to the corresponding fields

    // rows
    unit = &(unitDefs.units[ROWS]);
    for (int row = 0; row < unit->containers; row++) {
        for (int ix = 0; ix < MAX_NUMBER; ix++) {
            field = fields + row * MAX_NUMBER + ix;
            assert(field->unitPositions[ROWS] == row);

            unit->theContainers[row].fields[ix] = field;
        }
    }

    // cols
    unit = &(unitDefs.units[COLS]);
    for (int col = 0; col < unit->containers; col++) {
        for (int ix = 0; ix < MAX_NUMBER; ix++) {
            field = fields + ix * MAX_NUMBER + col;
            assert(field->unitPositions[COLS] == col);

            unit->theContainers[col].fields[ix] = field;
        }
    }

    // boxes
    unit = &(unitDefs.units[BOXES]);
    for (int box = 0; box < unit->containers; box++) {
        for (int ix = 0; ix < MAX_NUMBER; ix++) {

            getCoordinatesInBox(box, ix, &x, &y);
            field = fields + y * MAX_NUMBER + x;
            assert(field->unitPositions[BOXES] == box);
            assert(field->unitPositions[COLS] == x);
            assert(field->unitPositions[ROWS] == y);

            unit->theContainers[box].fields[ix] = field;
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

    // remove all candidates from this field
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
    Field **containerFields;

    assert(n <= MAX_NUMBER);

    sprintf(buffer, "Forbid number %u in neighbors of field %s ...\n", n, field->name);
    printlog(buffer);

    // forbid number in all other "neighboring fields"
    for (int u = 0; u < unitDefs.count; u++) {
        printf("[6hshhs]\n");
        Unit *unit = &(unitDefs.units[u]);
        printf("[6hshhs++]\n");
        containerFields = unit->theContainers[field->unitPositions[u]].fields;

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

        forbidNumbersInOtherFields(containerFields, candidates, preserve);
    }
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

    //    showAllCandidates();

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

    //    showAllCandidates();
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

    // field should not be solved already
    assert(!field->value);

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
 * checks if the number of fields in the FieldsVector and the number of numbers
 * in the numbers vector are equal.
 * 
 * @param fieldsVector vector or pointers to Fields, terminated with a NULL 
 *   pointer
 * @param numbers vector of numbers, terminated with 0
 * @return 1 both have equal length. 0 if they differ
 */
unsigned equalNumberOfFieldsAndCandidates(FieldsVector *fieldsVector, unsigned *numbers) {

    printf("check if found fields are of tuple dimension of numbers ...\n");
    printf("fieldsVector: (%d, %d, %d), numbers: (%u, %u, %u)\n", fieldsVector[0], fieldsVector[1], fieldsVector[2], numbers[0], numbers[1], numbers[2]);

    do {
        if (*fieldsVector == NULL && *numbers == 0) {
            printf("YES!\n");
            return 1;
        }

        // if we are still here, then at least one of the vectors is not null.
        // However, if the other one is exhausted, then both vectors apparently
        // do not have the same length
        if (*fieldsVector == NULL || *numbers == 0) {
            printf("   no ...\n");
            return 0;
        }

        fieldsVector++;
        numbers++;
    } while (1);
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
 * checks if the possible candidates for a field are a subset of the given 
 * numbers. If the field is already solved, returns 0.
 * 
 * @param field pointer to field for which the candidates should be checked
 * @param numbers vector of numbers, terminated with 0
 * @return 1 if the field's candidates are a (strict or non-strict) subset of
 *   the given numbers vector. 0 if they are not or if the field is already
 *   solved.
 */
int fieldCandidatesAreSubsetOf(Field *field, unsigned *numbers) {
    unsigned *numbersPtr;
    int found;

    printlog("[6jj]");
    if (field->value) {
        // already solved => nothing to do with the candidates
        return 0;
    }

    printlog("[6jj-1]");
    for (int i = 0; i < MAX_NUMBER; i++) {
        printlog("[6jj-ii]");
        if (field->candidates[i]) {

            // check if field candidate is in the numbers vector
            found = 0;
            numbersPtr = numbers;
            while (*numbersPtr) {
                if (*numbersPtr == field->candidates[i]) {
                    found = 1;
                    break;
                }
                numbersPtr++;
            }

            if (!found) {
                // found a field candidate which is not in the given list of
                // numbers
                printlog("[6jj-return0]");
                return 0;
            }
        }
    }
    printlog("[6jj-return1]");
    return 1;
}

/**
 * checks if the possible candidates for a field contain all of the given 
 * numbers. If the field is already solved, returns 0.
 * 
 * @param field pointer to field for which the candidates should be checked
 * @param numbers vector of numbers, terminated with 0
 * @return 1 if the field's candidates are a (strict or non-strict) superset of
 *   the given numbers vector. 0 if they are not or if the field is already
 *   solved.
 */
int fieldCandidatesContainAllOf(Field *field, unsigned *numbers) {

    if (field->value) {
        // already solved => nothing to do with the candidates
        return 0;
    }

    while (*numbers) {
        if (!field->candidates[*numbers - 1]) {
            // if any candidate is found which is not in "numbers", the field's
            // candidates are no subset of "numbers"
            printf("OJE! number %u not found in candidates (%u)\n", *numbers, field->candidates[*numbers - 1]);
            return 0;
        }
        numbers++;
    }

    return 1;
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

/**
 * set all candidates for all fields initially.
 * The purpose of this function is to determine the candidates for all fields
 * initially (the fields' values have already been set before).
 */
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
