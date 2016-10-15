/* 
 * File:   fieldlist.h
 * Author: ju
 *
 * Created on 04. April 2016, 21:57
 */

#ifndef FIELDLIST_H
#define	FIELDLIST_H

#include "typedefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

    /*
     * list of fields (list of variable length)
     */
    typedef struct FieldList {
        size_t count; // number of elements in the list
        Field **fields; // list of fields
    } FieldList;


    FieldList *createFieldList(size_t capacity);
    void emptyFieldList(FieldList *list);
    void freeFieldList(FieldList *list);
    void pushToFieldList(FieldList *list, Field *newField);
    Field *popFromFieldList(FieldList *list);

#ifdef	__cplusplus
}
#endif

#endif	/* FIELDLIST_H */

