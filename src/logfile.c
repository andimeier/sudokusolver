/* 
 * File:   log.c
 * Author: aurez
 *
 * Created on 04. April 2016, 21:01
 */
#include <stdio.h>
#include <string.h>
#include "grid.h"
#include "logfile.h"


static void printlog(char *text);


static FILE *logfile;


// general buffer for string operations
char buffer[1000];

static char buffer2[1000];

unsigned logLevel = LOGLEVEL_ERRORS; // 0 ... no logging, 1 ... solved cells, 2 ... changes, 9 ... debug

/**
 * show the remaining candidates for the specified field
 * 
 * @param field the field for which the candidates should be printed
 */
void showCandidates(Field *field) {
    char candidates[2 * maxNumber + 1];
    char *str;

    str = candidates;

    // default: no candidates left, print "-"
    *str = '-';
    *(str + 1) = '\0';

    for (int i = 0; i < maxNumber; i++) {
        if (field->candidates[i]) {
            if (str != candidates) {
                // not the first candidate => append comma-separated
                *str++ = ',';
            }
            *str++ = (char) (field->candidates[i] + '0');
            *str = '\0';
        }
    }

    sprintf(buffer, "candidates for field %s are: %s", field->name, candidates);
    logVerbose(buffer);
}

/**
 * show the remaining candidates of all fields
 */
void showAllCandidates() {
    Field *field;

    for (int f = 0; f < numberOfFields; f++) {
        field = fields + f;

        if (field->value) {
            sprintf(buffer, "candidates for field %s ... value %u", field->name, field->value);
            logVerbose(buffer);

            continue;
        }

        showCandidates(field);
    }
}

/**
 * log a "reduction" event (candidates could be removed)
 * 
 * @param msg
 */
void logReduction(char *msg) {
    if (logLevel >= LOGLEVEL_CHANGES) {
        sprintf(buffer2, "[REDUCTION] --- %s", msg);
        printlog(buffer2);
    }
}

/**
 * log a "found a number" event
 * 
 * @param msg
 */
void logNewNumber(char *msg) {
    if (logLevel >= LOGLEVEL_SOLVED_CELLS) {
        sprintf(buffer2, "+++ %s", msg);
        printlog(buffer2);
    }
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
 * log a message to printlog file or to stdout
 *
 * @param text text to be logged. A newline character will be appended.
 */
void logVerbose(char *text) {

    if (logLevel < LOGLEVEL_VERBOSE)
        return;

    printlog(text);
}

/**
 * log a message to printlog file or to stdout
 *
 * @param text text to be logged. A newline character will be appended.
 */
void logError(char *text) {

    if (logLevel < LOGLEVEL_ERRORS)
        return;

    printlog(text);
}

/**
 * log a message to printlog file or to stdout
 *
 * @param text text to be logged. A newline character will be appended.
 */
void logAlways(char *text) {
    printlog(text);
}

/**
 * close the log file
 */
void closeLogFile() {
    if (logfile)
        fclose(logfile);
}

