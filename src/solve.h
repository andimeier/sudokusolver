/* 
 * File:   solve.h
 * Author: aurez
 *
 * Created on 04. April 2016, 21:22
 */

#ifndef SOLVE_H
#define SOLVE_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct Strategy {
        char *name;
        int (*solver)(void);
        int used; // flag if this strategy had to be used during solving
    } Strategy;


    int solve();

    // strategies
    int checkForSolvedCells();

    // number of errors in the algorithm
    extern int errors;

    // the defined strategies
    extern Strategy **strategies;


#ifdef __cplusplus
}
#endif


#endif /* SOLVE_H */

