/* 
 * File:   numberlist.h
 * Author: ju
 *
 * Created on 2016-07-07
 */

#ifndef NUMBERLIST_H
#define	NUMBERLIST_H

#include "typedefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

    /*
     * list of numbers (list of variable length)
     */
    typedef struct NumberList {
        size_t count; // number of elements in the list
        unsigned *numbers; // list of numbers
    } NumberList;


    NumberList *createNumberList(size_t capacity);
    void emptyNumberList(NumberList *list);
    void freeNumberList(NumberList *list);
    void pushToNumberList(NumberList *list, unsigned newNumber);
    unsigned popFromNumberList(NumberList *list);

#ifdef	__cplusplus
}
#endif

#endif	/* NUMBERLIST_H */

