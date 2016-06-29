/* 
 * File:   solve.h
 * Author: aurez
 *
 * Created on 04. April 2016, 21:22
 */

#ifndef SOLVE_H
#define	SOLVE_H

#include "typedefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

    int solve();

    // strategies
    int checkForSolvedCells();
    int findHiddenSingles();
    int findNakedTuples();
    int findHiddenPairsAndTriples(); // not implemented yet
    int findPointingTuples();
    int boxLineReduction(); // not implemented yet

    // number of errors in the algorithm
    extern int errors;

    // auxiliary functions
    unsigned findNakedTuplesInContainer(Container *container, unsigned dimension, FieldList *includedFields, FieldsVector *fieldsLeft);
    unsigned findPointingTuplesInContainer(Container *container, FieldsVector *fieldsWithCandidate);
    

    
#ifdef	__cplusplus
}
#endif


#endif	/* SOLVE_H */

