/* 
 * File:   hidden-tuples.h
 * Author: aurez
 *
 * Created on 2016-07-07
 */

#ifndef HIDDEN_TUPLES_H
#define	HIDDEN_TUPLES_H

#include "numberlist.h"

#ifdef	__cplusplus
extern "C" {
#endif

    // strategies
    int findHiddenTuples();

    // auxiliary functions
    unsigned findHiddenTuplesInContainer(Container *container, unsigned dimension, NumberList *includedCandidates, unsigned *candidatesLeft, FieldsVector *fieldsWithCandidates);
    
#ifdef	__cplusplus
}
#endif


#endif	/* HIDDEN_TUPLES_H */

