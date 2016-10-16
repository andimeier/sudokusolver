/* 
 * File:   log.c
 * Author: aurez
 *
 * Created on 04. April 2016, 21:01
 */
#include <assert.h>
#include <stdlib.h>
#include "grid.h"
#include "util.h"
#include "log.h"

#define INIT_LOGSIZE    100
#define INCREMENT_LOGSIZE   100

typedef struct Entry {
    void *info;
    PrintFunc printFunc;
} Entry;

typedef struct History {
    Entry **entries;
    size_t capacity;
    size_t count;
} History;


//static History *history;
static History hist;

/**
 * initializes the log
 */
void initLog() {
    //    void *f;

    hist.entries = (Entry **) xmalloc(sizeof (Entry *) * INIT_LOGSIZE);
    hist.capacity = INIT_LOGSIZE;
    hist.count = 0;
    //    return;
    //    
    //    history = (History *) xmalloc(sizeof (History));
    //    f = (void *) xmalloc(sizeof (Entry *) * INIT_LOGSIZE);
    //    history->entries = (Entry **) f;
    //    history->entries = (Entry **) xmalloc(sizeof (Entry *) * INIT_LOGSIZE);
    //    history->capacity = INIT_LOGSIZE;
    //    history->count = 0;
}

/**
 * 
 * @param entry a log entry.
 */
void writeLog(PrintFunc printFunc, void *info) {
    if (hist.count >= hist.capacity) {
        // allocate another block for further log entries
        hist.entries = (Entry **) realloc(hist.entries, sizeof (Entry *) * (hist.capacity + INCREMENT_LOGSIZE));
        assert(hist.entries != NULL);

        hist.capacity += INCREMENT_LOGSIZE;
    }

    // add log entry
    Entry *entry = (Entry *) xmalloc(sizeof (Entry));
    entry->printFunc = printFunc;
    entry->info = info;
    hist.entries[hist.count++] = entry;
}

/**
 * calls the output function of the log entry
 */
void printLog() {
    unsigned i;

    for (i = 0; i < hist.count; i++) {
        Entry *entry;

        entry = hist.entries[i];
        (*(entry->printFunc)) ((void *) (entry->info));
    }
}
