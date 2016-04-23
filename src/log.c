/* 
 * File:   show.c
 * Author: aurez
 *
 * Created on 04. April 2016, 21:01
 */
#include <stdio.h>
#include <string.h>
#include "global.h"

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

        for (int i = 0; i < MAX_NUMBER; i++) {
            candidates[i] = (char) (field->candidates[i] + '0');
        }
        candidates[MAX_NUMBER] = '\0';

        printf("candidates for field %s are: %s\n", field->name, candidates);
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
