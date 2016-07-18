/* 
 * File:   pointing-tuples.h
 * Author: aurez
 *
 * Created on 2016-07-03
 */

#ifndef POINTING_TUPLES_H
#define	POINTING_TUPLES_H

#include "typedefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

    // strategies
    int findPointingTuples();

    // auxiliary functions
    unsigned findPointingTuplesInContainer(Container *container, FieldsVector *fieldsWithCandidate);
    
#ifdef	__cplusplus
}
#endif


#endif	/* POINTING_TUPLES_H */

