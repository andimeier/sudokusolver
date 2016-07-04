/* 
 * File:   log.c
 * Author: aurez
 *
 * Created on 04. April 2016, 21:01
 */
#include <assert.h>
#include "global.h"
#include "grid.h"
#include "log.h"

#define INIT_LOGSIZE    100
#define INCREMENT_LOGSIZE   100


static History *history;


/**
 * initializes the log
 */
void initLog() {
    history = (void **) xmalloc(sizeof (void *) * INIT_LOGSIZE);
    history->capacity = INIT_LOGSIZE;
    history->count = 0;
}

/**
 * 
 * @param entry a log entry.
 */
void writeLog(print *printFunc, void *info) {
    if (history->count >= history->capacity) {
        // allocate another block for further log entries
        history = (void **) realloc(history, sizeof (void *) * (history->capacity + INCREMENT_LOGSIZE));
        assert(history != NULL);

        history->capacity += INCREMENT_LOGSIZE;
    }

    // add log entry
    Entry *entry = (Entry *) xmalloc(sizeof (Entry));
    entry->printFunc = printFunc;
    entry->info = info;
    history->entries[history->count++] = &entry;
}

/**
 * calls the output function of the log entry
 */
void printLog() {
    for (unsigned i = 0; i < history->count; i++) {
        Entry *entry;

        entry = history->entries[i];
//        *(entry->printFunc) (entry->info);
    }
}
