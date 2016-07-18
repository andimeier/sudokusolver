/* 
 * File:   naked-triples.h
 * Author: aurez
 *
 * Created on 2016-07-03 21:19
 */

#ifndef NAKED_TRIPLES_H
#define	NAKED_TRIPLES_H

#include "typedefs.h"
#include "fieldlist.h"

#ifdef	__cplusplus
extern "C" {
#endif

    int findNakedTuples();

    // auxiliary functions
    unsigned findNakedTuplesInContainer(Container *container, unsigned dimension, FieldList *includedFields, FieldsVector *fieldsLeft);
    
#ifdef	__cplusplus
}
#endif


#endif	/* NAKED_TRIPLES_H */

