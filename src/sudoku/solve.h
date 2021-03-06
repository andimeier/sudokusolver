/* 
 * File:   solve.h
 * Author: aurez
 *
 * Created on 04. April 2016, 21:22
 */

#ifndef SOLVE_H
#define SOLVE_H

#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

    // search for pairs, triples and quadruples, not more
#define MAX_TUPLE_DIMENSION 3

    typedef struct Strategy {
        char *name;
        Bool (*solver)(void);
        int used; // flag if this strategy had to be used during solving
    } Strategy;


    Bool solve();

    // strategies
    Bool checkForSolvedCells();

    // number of errors in the algorithm
    extern int errors;

    // the defined strategies
    extern Strategy **strategies;


#ifdef __cplusplus
}
#endif


#endif /* SOLVE_H */

