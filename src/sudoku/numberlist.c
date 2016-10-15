/* 
 * File:   numberlist.c
 * Author: aurez
 *
 * Created on 2016-07-07
 */
#include <stdlib.h>
#include <stddef.h>
#include "util.h"
#include "numberlist.h"

/**
 * create number list of given length and initialize it (with 0)
 * 
 * @param capacity number of elements to be stored in the list
 * @return list the initialized number list
 */
NumberList *createNumberList(size_t capacity) {
    NumberList *list;

    list = (NumberList *) xmalloc(sizeof (NumberList));
    list->numbers = (unsigned *) xmalloc(sizeof (unsigned) * (capacity + 1)); // leave room for list terminator

    // initialization
    list->count = 0;
    list->numbers[0] = 0;

    return list;
}

/**
 * empty number list, but keep memory allocation as is
 * 
 * @param list number list to be emptied
 */
void emptyNumberList(NumberList *list) {
    list->count = 0;
    list->numbers[0] = 0;
}

/**
 * free memory for the given number list
 * 
 * @param list list to be de-allocated
 */
void freeNumberList(NumberList *list) {
    free(list->numbers);
    free(list);
}

/**
 * pushes a new number onto the list of numbers
 * 
 * @param list number list to be appended to
 * @param newNumber new number to be appended to the list
 */
void pushToNumberList(NumberList *list, unsigned newNumber) {
    list->numbers[list->count++] = newNumber;
    list->numbers[list->count] = 0;
}

/**
 * pops the last element from the list of numbers
 * 
 * @param includedFields the number list 
 * @return the last, "popped" element
 */
unsigned popFromNumberList(NumberList *list) {
    unsigned lastField;

    list->count--;
    lastField = list->numbers[list->count];
    list->numbers[list->count] = 0;

    return lastField;
}