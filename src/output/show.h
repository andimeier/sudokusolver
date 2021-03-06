/* 
 * File:   show.h
 * Author: ju
 *
 * Created on 05. April 2016, 00:33
 */

#ifndef SHOW_H
#define SHOW_H

#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

    struct Field;

    void printSudokuString(FieldValueType valueType, Bool candidate0);
    char *position(Field *field);

    void printInvolvedStrategies();
    
    // aux functions, mainly for debugging purposes
    void sf(Field *field);
    void sc(Container *container);
    void sfv(FieldsVector *fields, size_t limit);
    void sfs(FieldsVector *fields, size_t limit); // identical to (alias of)) sfv

    // filename of SVG file
    extern char *svgFilename;


#ifdef __cplusplus
}
#endif

#endif /* SHOW_H */

