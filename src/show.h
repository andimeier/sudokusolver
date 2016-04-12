/* 
 * File:   show.h
 * Author: ju
 *
 * Created on 05. April 2016, 00:33
 */

#ifndef SHOW_H
#define	SHOW_H

#include "typedefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

    struct Field;

    void show(int showInit);
    void printSvg(int finalVersion);
    char *position(Field *field);

#ifdef	__cplusplus
}
#endif

#endif	/* SHOW_H */

