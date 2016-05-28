/* 
 * File:   log.c
 * Author: aurez
 *
 * Created on 04. April 2016, 21:01
 */
#include <stdio.h>
#include <string.h>
#include "grid.h"


static FILE *logfile;

// general buffer for string operations
char buffer[1000];

int verboseLogging;

    
void showCandidates(Field *field) {
    char candidates[MAX_NUMBER + 1];

    for (int i = 0; i < MAX_NUMBER; i++) {
        candidates[i] = (char) (field->candidates[i] + '0');
    }
    candidates[MAX_NUMBER] = '\0';

    printf("candidates for field %s are: %s\n", field->name, candidates);
}

void showAllCandidates() {
    char candidates[MAX_NUMBER + 1];
    Field *field;

    for (int f = 0; f < NUMBER_OF_FIELDS; f++) {
        field = fields + f;

        if (field->value) {
            printf("candidates for field %s ... value %u\n", field->name, field->value);
            continue;
        }

        // build candidates string
        for (int i = 0; i < MAX_NUMBER; i++) {
            candidates[i] = (char) (field->candidates[i] + '0');
        }
        candidates[MAX_NUMBER] = '\0';

        sprintf(buffer, "candidates for field %s are: %s\n", field->name, candidates);
        printlog(buffer);
    }
}

/**
 * log a "reduction" event (candidates could be removed)
 * @param msg
 */
void logReduction(char *msg) {
    printf("--- %s\n", msg);
}

/**
 * log a "found a number" event
 * @param msg
 */
void logNewNumber(char *msg) {
    printf("+++ %s\n", msg);
}

void openLogFile(char *outputFilename) {
    logfile = fopen(outputFilename, "w");
}

//-------------------------------------------------------------------

void printlog(char *text) {
    // printlog a message to printlog file or to stdout

    if (logfile) {
        fputs(text, logfile);
    } else {
        // no printlog file => write to stdout
        puts(text);
    }
}

//-------------------------------------------------------------------

void closeLogFile() {
    if (logfile)
        fclose(logfile);
}

