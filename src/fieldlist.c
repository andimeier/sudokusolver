/* 
 * File:   fieldlist.c
 * Author: aurez
 *
 * Created on 2016-06-15
 */
#include <stdlib.h>
#include "util.h"
#include "fieldlist.h"

/**
 * create field list of given length and initialize it (with 0)
 * 
 * @param capacity number of elements to be stored in the list
 * @return list the initialized field list
 */
FieldList *createFieldList(size_t capacity) {
    FieldList *list;

    list = (FieldList *) xmalloc(sizeof (FieldList));
    list->fields = (Field **) xmalloc(sizeof (Field *) * (capacity + 1)); // leave room for list terminator

    // initialization
    list->count = 0;
    list->fields[0] = NULL;

    return list;
}

/**
 * empty field list, but keep memory allocation as is
 * 
 * @param list field list to be emptied
 */
void emptyFieldList(FieldList *list) {
    list->count = 0;
    list->fields[0] = NULL;
}

/**
 * free memory for the given field list
 * 
 * @param list list to be de-allocated
 */
void freeFieldList(FieldList *list) {
    free(list->fields);
    free(list);
}

/**
 * pushes a new field onto the list of fields
 * 
 * @param list field list to be appended to
 * @param newField new field to be appended to the list
 */
void pushToFieldList(FieldList *list, Field *newField) {
    list->fields[list->count++] = newField;
    list->fields[list->count] = NULL;
}

/**
 * pops the last element from the list of fields
 * 
 * @param includedFields the field list 
 * @return the last, "popped" element
 */
Field *popFromFieldList(FieldList *list) {
    Field *lastField;

    list->count--;
    lastField = list->fields[list->count];
    list->fields[list->count] = NULL;

    return lastField;
}