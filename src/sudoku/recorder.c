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
 * @param entry a solve step entry.
 */
void recordStep(PrintFunc printFunc, void *info) {

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
    hist.steps[hist.count++] = entry;
}

/**
 * calls the output function of the solve step entry
 */
void printStep() {
    unsigned i;

    for (i = 0; i < hist.count; i++) {
        Step *entry;

        entry = hist.steps[i];

        // let the print function fill the msgBuffer
        (*(entry->printFunc)) (msgBuffer, (void *) (entry->info));
        printf(msgBuffer);
    }
}


/**
 * releases the memory used for the solve path
 */
void releaseRecording() {

    free(hist.steps);
}