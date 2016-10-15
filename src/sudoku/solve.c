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
#include "grid.h"
#include "typedefs.h"
#include "util.h"
#include "logfile.h"
#include "fieldlist.h"
#include "hidden-singles.h"
#include "naked-tuples.h"
#include "pointing-tuples.h"
#include "hidden-tuples.h"

// maximum possible number of strategies (can be incremented any time, at the 
// moment I regard 10 strategies as a good maximum value)
#define MAX_NUMBER_OF_STRATEGIES 10




// prototypes
static Strategy **buildStrategies();

// the defined strategies
Strategy **strategies;

// number of errors in the algorithm
int errors;


/**
 * The working horse. Try to solve the Sudoku.
 * 
 * @return 1 ... Sudoku has been solved successfully. 0 ... algorithm got stuck,
 *   indefinite iteration cancelled.
 */
Bool solve() {
    int iteration;
    int progress; // flag: something has changed in the iteration
    Strategy **currentStrategy;
    
    strategies = buildStrategies();
    
    iteration = 0;
    errors = 0; // no errors yet

    cleanUpCandidates();

    sf((Field *)(fields + 1));
    
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
            progress |= ((*currentStrategy)->solver)();

            if (progress) {
                (*currentStrategy)->used++;
                
                /*
                 * no iterating to next strategy, begin again with the easiest 
                 * one for the next iteration
                 */
                break;
            }

            // no progress => try next strategy
            currentStrategy++;
        }

        if (isFinished()) {
            return TRUE;
        }

    } while (progress);

    showAllCandidates();

    // wir kommen hierher, weil die letzte Iteration keine einzige Aenderung gebracht
    // hat => wir bleiben stecken mit unseren Algorithmen. Ohne Aenderung in der
    // Implementierung ist dieses Sudoku nicht loesbar
    return FALSE;
}


/**
 * define the strategies
 * 
 * @return the list of Strategy "objects"
 */
Strategy **buildStrategies() {
    Strategy **strategies;
    Strategy **currentStrategy;

    // add stragies to list of strategies to be used
    // FIXME hardcoded maximum of 10 different strategies
    strategies = (Strategy **) xmalloc(sizeof (Strategy *) * (MAX_NUMBER_OF_STRATEGIES + 1));
    
    for (int i = 0; i < MAX_NUMBER_OF_STRATEGIES; i++) {
        strategies[i] = (Strategy *) xmalloc(sizeof (Strategy));
    }

    // build list of strategies
    // ------------------------
    
    currentStrategy = strategies;
    
    // check for solved cells
    (*currentStrategy)->name = strdup("check for solved cells");
    (*currentStrategy)->solver = &checkForSolvedCells;
    currentStrategy++;
    
    // check for solved cells
    (*currentStrategy)->name = strdup("find hidden singles");
    (*currentStrategy)->solver = &findHiddenSingles;
    currentStrategy++;
    
    // check for naked tuples
    (*currentStrategy)->name = strdup("find naked tuples");
    (*currentStrategy)->solver = &findNakedTuples;
    currentStrategy++;
    
    // check for pointing tuples
    (*currentStrategy)->name = strdup("find pointing tuples");
    (*currentStrategy)->solver = &findPointingTuples;
    currentStrategy++;

    // check for hidden tuples
    (*currentStrategy)->name = strdup("find hidden tuples");
    (*currentStrategy)->solver = &findHiddenTuples;
    currentStrategy++;

    *currentStrategy = (Strategy *)NULL; // terminate list of strategies

    // set all strategies  to "not yet used" initially
    currentStrategy = strategies;
    while (*currentStrategy) {
        (*currentStrategy)->used = 0;
        currentStrategy++;
    }
    
    return strategies;
}


/**
 * check for cells having only one candidate left and set their value (and
 * thus eliminate this value in neighboring fields)
 * 
 * @return TRUE ... something has changed, FALSE ... nothing changed
 */
Bool checkForSolvedCells() {
    int f;
    Field *field;
    int progress; // Flag: in einer Iteration wurde zumindest eine Erkenntnis gewonnen

    logVerbose("[strategy] check for solved cells ...");

    progress = FALSE;

    for (f = 0; f < numberOfFields; f++) {
        field = fields + f;

        if (field->candidatesLeft == 1 && !field->value) {
            setUniqueNumber(field);
            progress = TRUE;
        }

    }
    return progress;
}
