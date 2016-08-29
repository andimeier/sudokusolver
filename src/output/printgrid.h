/* 
 * File:   printgrid.h
 * Author: ju
 *
 * Created on 05. April 2016, 00:33
 */

#ifndef PRINTGRID_H
#define PRINTGRID_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum { INITIAL, SOLVED } FieldValue;

    void printGrid(FieldValue whichValue);

#ifdef __cplusplus
}
#endif

#endif /* PRINTGRID_H */

