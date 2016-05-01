/* 
 * File:   gridutils.c
 * Author: aurez
 *
 * Created on 04. April 2016, 21:01
 */
#include <assert.h>
#include <stdio.h>
#include "global.h"
#include "gridutils.h"
#include "log.h"
#include "util.h"

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
