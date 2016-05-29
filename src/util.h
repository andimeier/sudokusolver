/* 
 * File:   util.h
 * Author: ju
 *
 * Created on 04. April 2016, 21:57
 */

#ifndef UTIL_H
#define	UTIL_H

#include "typedefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

    int *unsigneddup(unsigned const *src, size_t len);
    void *xmalloc(size_t size);
    unsigned ulength(unsigned *array);
    void appendField(Field **fields, Field *newField);
    int containsField(Field **list, Field * field);
    int fieldCandidatesAreSubsetOf(Field *field, unsigned *numbers);

#ifdef	__cplusplus
}
#endif

#endif	/* UTIL_H */

