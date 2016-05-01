/* 
 * File:   init.h
 * Author: alex
 *
 * Created on 01. Mai 2016, 21:41
 */

#ifndef INIT_H
#define	INIT_H

#ifdef	__cplusplus
extern "C" {
#endif

    void initFields();
    void initUnits();
    void initGrid();
    void freeUnits();
    void freeGrid();
    void freeFields();

#ifdef	__cplusplus
}
#endif

#endif	/* INIT_H */

