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
    Bool containsField(Field **list, Field * field);
    Bool fieldCandidatesAreSubsetOf(Field *field, unsigned *numbers);
    int countDistinctCandidates(FieldsVector *fields, size_t limit);
    FieldsVector *fieldsContainingCandidate(FieldsVector *fieldsWithCandidate, FieldsVector *fields, unsigned candidate);
    Container *getCommonContainer(FieldsVector *fieldsWithCandidate, size_t ContainerSetIndex); // or NULL if fields have no common container
    unsigned *getCandidates(Field *field);

#ifdef	__cplusplus
}
#endif

#endif	/* UTIL_H */

