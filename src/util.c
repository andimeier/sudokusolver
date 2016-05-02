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
    if (value == 0) {
        printf("virtual memory exhausted\n");
        exit(EXIT_FAILURE);
    }
    
    return value;
}