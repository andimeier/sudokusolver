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
    int findNakedTuples(size_t dimension);
    int findHiddenPairsAndTriples(); // not implemented yet
    int findNakedQuads(); // not implemented yet
    int findHiddenQuads(); // not implemented yet
    int findPointingTupels();
    int boxLineReduction(); // not implemented yet

    // number of errors in the algorithm
    extern int errors;


#ifdef	__cplusplus
}
#endif


#endif	/* SOLVE_H */

