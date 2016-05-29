/* 
 * File:   log.c
 * Author: aurez
 *
 * Created on 04. April 2016, 21:01
 */
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "grid.h"
#include "log.h"


static FILE *logfile;

// general buffer for string operations
char buffer[1000];

/*
 * log level: 
 *   0 ... no verbose logging
 *   1 ... log changes
 *   2 ... log even considerations
 */
int verboseLogging; 


/**
 * show the remaining candidates for the specified field
 * 
 * @param field the field for which the candidates should be printed
 */   
void showCandidates(Field *field) {
    char candidates[MAX_NUMBER + 1];

    for (int i = 0; i < MAX_NUMBER; i++) {
        candidates[i] = (char) (field->candidates[i] + '0');
    }
    candidates[MAX_NUMBER] = '\0';

    printf("candidates for field %s are: %s\n", field->name, candidates);
}

/**
 * show the remaining candidates of all fields
 */
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
 * 
 * @param msg
 */
void logReduction(char *msg) {
    printf("--- %s\n", msg);
}

/**
 * log a "found a number" event
 * 
 * @param msg
 */
void logNewNumber(char *msg) {
    printf("+++ %s\n", msg);
}


/**
 * open log file for writing
 * 
 * @param outputFilename filename of log file (file will be overwritten if it 
 *   exists already)
 */
void openLogFile(char *outputFilename) {
    logfile = fopen(outputFilename, "w");
}


/**
 * log a message to printlog file or to stdout
 *
 * @param text text to be logged. A newline character will be appended.
 */
void printlog(char *text) {

    if (logfile) {
        fputs(text, logfile);
    } else {
        // no printlog file => write to stdout
        puts(text);
    }
}


/**
 * close the log file
 */
void closeLogFile() {
    if (logfile)
        fclose(logfile);
}

