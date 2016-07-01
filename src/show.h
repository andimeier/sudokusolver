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

    void show(int showInit);
    void sudokuString(int showInit);
    void printSvg(int finalVersion);
    char *position(Field *field);

    // aux functions, mainly for debugging purposes
    void showField(Field *field, int showContainers, int appendLf);
    void sf(Field *field);
    void showContainer(Container *container);
    void sc(Container *container);
    void showFieldsVector(FieldsVector *fields, int indent);
    void sfv(FieldsVector *fields);

    // filename of SVG file
    extern char *svgFilename;


#ifdef __cplusplus
}
#endif

#endif /* SHOW_H */

