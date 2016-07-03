/* 
 * File:   solve.h
 * Author: aurez
 *
 * Created on 04. April 2016, 21:22
 */

#ifndef SOLVE_H
#define	SOLVE_H

#ifdef	__cplusplus
extern "C" {
#endif

    int solve();

    // strategies
    int checkForSolvedCells();

    // number of errors in the algorithm
    extern int errors;

#ifdef	__cplusplus
}
#endif


#endif	/* SOLVE_H */

