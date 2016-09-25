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

    typedef enum { INITIAL, SOLVED } FieldValueType;

    void printGrid(FieldValueType whichValue);

#ifdef __cplusplus
}
#endif

#endif /* PRINTGRID_H */

