/* 
 * File:   solve.c
 * Author: aurez
 *
 * Created on 04. April 2016, 21:01
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

/**
 * copies all numbers from one array to another.
 * Remember to free the allocated memory when you don't need it anymore
 * @param src
 * @param len
 * @return 
 */
int *unsigneddup(unsigned const *src, size_t len) {
    int *p = malloc(len * sizeof (unsigned));
    if (p == NULL) {
        exit(EXIT_FAILURE);
    }
    memcpy(p, src, len * sizeof (unsigned));
    return p;
}

void *xmalloc(size_t size) {
    void *value = malloc(size);
    if (value == NULL) {
        printf("virtual memory exhausted\n");
        exit(EXIT_FAILURE);
    }

    return value;
}

/**
 * get the length of an array of unsigned ints by counting all non-NULL
 * items until the terminating 0.
 * 
 * @param array array of unsigned values
 * @return the length of the array = the first index of NULL
 */
unsigned ulength(unsigned *array) {
    unsigned i;
    unsigned *ptr;

    ptr = array;
    i = 0;
    while (*ptr++) i++;

    return i;
}

/**
 * append a Field reference to a NULL-terminated list of references to Fields.
 * 
 * @param fields the already existing list of field references
 * @param newField the field reference to be added to the end of the list
 */
void appendField(Field **fields, Field *newField) {
    Field **ptr;
    
    ptr = fields;
    
    // search end of current list
    while (*ptr++);
    
    // extend list
    *ptr++ = newField;
    
    // terminate extended list
    *ptr = NULL;
}
