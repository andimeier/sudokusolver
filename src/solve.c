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
#include "global.h"
#include "solve.h"
#include "show.h"
#include "grid.h"
#include "typedefs.h"
#include "util.h"
#include "grid.h"
#include "logfile.h"
#include "fieldlist.h"
#include "hidden-singles.h"
#include "naked-tuples.h"
#include "pointing-tuples.h"

// search for pairs, triples and quadruples, not more
#define MAX_TUPLE_DIMENSION 3

typedef int (*strategy)(void);

// number of errors in the algorithm
int errors;


/**
 * The working horse. Try to solve the Sudoku.
 * 
 * @return 1 ... Sudoku has been solved successfully. 0 ... algorithm got stuck,
 *   indefinite iteration cancelled.
 */
int solve() {
    int iteration;
    int progress; // flag: something has changed in the iteration
    strategy *strategies;
    strategy *currentStrategy;

    // add stragies to list of strategies to be used
    strategies = (strategy *) xmalloc(sizeof (strategy) * 10);
    currentStrategy = strategies;
    *currentStrategy++ = &checkForSolvedCells;
    *currentStrategy++ = &findHiddenSingles;
    *currentStrategy++ = &findNakedTuples;
    *currentStrategy++ = &findPointingTuples;
    *currentStrategy++ = NULL; // terminate list of strategies


    iteration = 0;
    errors = 0; // no errors yet

    printSvg(0);

    cleanUpCandidates();

    // main loop, only enter if Sudoku has been solved or if we got stuck and
    // are unable to solve the Sudoku
    do {
        iteration++;
        progress = 0; // nothing changed in this iteration (no wonder - has just started)
        sprintf(buffer, "----- Beginne Iteration %d -----", iteration);
        logVerbose(buffer);


        // loop through all strategies
        currentStrategy = strategies;
        while (*currentStrategy) {
            progress |= (**currentStrategy)();

            if (progress) {
                // no iterating to next strategy
                break;
            }

            // no progress => try next strategy
            currentStrategy++;
        }

        if (isFinished()) {
            return 1;
        }

    } while (progress);

    showAllCandidates();

    free(strategies);

    // wir kommen hierher, weil die letzte Iteration keine einzige Aenderung gebracht
    // hat => wir bleiben stecken mit unseren Algorithmen. Ohne Aenderung in der
    // Implementierung ist dieses Sudoku nicht loesbar
    return 0;
}

void printFoundFields(FieldsVector *foundFields) {
    FieldsVector *f;

    f = foundFields;

    buffer[0] = '\0';
    while (*f) {
        strcat(buffer, "-");
        strcat(buffer, (*f)->name);
        f++;
    }
    logVerbose(buffer);
}

/**
 * check for cells having only one candidate left and set their value (and
 * thus eliminate this value in neighboring fields)
 * 
 * @return 1 ... something has changed, 0 ... nothing changed
 */
int checkForSolvedCells() {
    int f;
    Field *field;
    int progress; // Flag: in einer Iteration wurde zumindest eine Erkenntnis gewonnen

    logVerbose("[strategy] check for solved cells ...");

    progress = 0;

    for (f = 0; f < NUMBER_OF_FIELDS; f++) {
        field = fields + f;

        if (field->candidatesLeft == 1 && !field->value) {
            setUniqueNumber(field);
            progress = 1;
        }

    }
    return progress;
}


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
