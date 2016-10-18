/* 
 * File:   solvepath.c
 * Author: aurez
 *
 * Created on 04. April 2016, 21:01
 */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "grid.h"
#include "util.h"
#include "recorder.h"

#define INIT_RECORDER_STEPS    100
#define INCREMENT_RECORDER_STEPS   100

typedef struct {
    void *info;
    STEP_TYPE stepType;
    PrintFunc printFunc;
} Step;

typedef struct {
    Step **steps;
    size_t capacity;
    size_t count;
} History;


//static History *history;
static History hist;

// message buffer for the step messages to store their messages into
static char msgBuffer[2000];

/**
 * initializes the solve path recorder
 */
void initRecorder() {

    hist.steps = (Step **) xmalloc(sizeof (Step *) * INIT_RECORDER_STEPS);
    hist.capacity = INIT_RECORDER_STEPS;
    hist.count = 0;
}

/**
 * records one step along the solve path
 * 
 * @param stepType the "quality" of the solve step
 * @param printFunc the handler for transforming this info to human-readable
 *   output. This function must know the type of structure referenced by info.
 * @param info a solve step entry, as a pointer to some structure which this
 *   function does not need to know about. The interpreting function of the
 *   solve step must know the type of structure (the printFunc)
 */
void recordStep(STEP_TYPE stepType, PrintFunc printFunc, void *info) {

    if (hist.count >= hist.capacity) {
        // allocate another block for further solve step entries
        hist.steps = (Step **) realloc(hist.steps, sizeof (Step *) * (hist.capacity + INCREMENT_RECORDER_STEPS));
        assert(hist.steps != NULL);

        hist.capacity += INCREMENT_RECORDER_STEPS;
    }

    // add solve step entry
    Step *entry = (Step *) xmalloc(sizeof (Step));
    entry->printFunc = printFunc;
    entry->info = info;
    entry->stepType = stepType;
    hist.steps[hist.count++] = entry;
}

/**
 * marks the start of a "strategy findings" chapter in the solve path.
 * This registers the found "thing" which a strategy has yielded, but it is not
 * clear yet whether this finding results in any change on the board.
 * 
 * E.g. if a naked pair is found, this is a "strategy finding" of the strategy
 * "findNakedPairs". However, if this naked pair does not lead to any
 * candidate being removed, then it does not need to be recorded.
 * 
 * For such cases, the "start of a strategy finding" documents the start of 
 * such a chapter. When the chapter is finished and no reduction steps have
 * been recorded since, the solve path is rolled back to the point where the
 * "start of a strategy finding" has been recorded. In other words, the
 * strategy finding entry is discarded (rolled back).
 * 
 * @param printFunc
 * @param info
 */
void recordStartOfStrategyFinding(PrintFunc printFunc, void *info) {
    recordStep(STEP_STRATEGY_FINDING_START, printFunc, info);
}

/**
 * closes the "chapter" of a strategy finding.
 * 
 * If the last entry is a "strategy finding", then apparently this strategy
 * finding was not followed by any actual changes on the Sudoku board. 
 * Consequently, in this case, the entry is removed again.
 * 
 * @see recordStartOfStrategyFinding for details
 */
void recordEndOfStrategyFinding(void) {

    assert(hist.count > 0);

    /* check if last entry is a "strategy finding" entry (thus, not followed
     * by any board changes)
     */
    if ((hist.steps[hist.count - 1])->stepType == STEP_STRATEGY_FINDING_START) {
        // empty strategy finding => remove it
        hist.count--;
    } else {
        recordStep(STEP_STRATEGY_FINDING_END, NULL, NULL);
    }
}

/**
 * calls the output function of the solve step entry
 */
void printStep() {
    unsigned i;

    for (i = 0; i < hist.count; i++) {
        Step *entry;

        entry = hist.steps[i];

        if (entry->printFunc) {
            // let the print function fill the msgBuffer
            (*(entry->printFunc)) (msgBuffer, entry->stepType, (void *) (entry->info));
            printf(msgBuffer);
        } else {
            if (entry->stepType == STEP_STRATEGY_FINDING_END) {
                printf("---\n"); // mark end of stategy findings
            }
        }
    }
}

/**
 * releases the memory used for the solve path
 */
void releaseRecording() {

    free(hist.steps);
}