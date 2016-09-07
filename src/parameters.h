/* 
 * File:   parameters.h
 * Author: aurez
 *
 * Created on 01. Mai 2016, 22:04
 */

#ifndef PARAMETERS_H
#define	PARAMETERS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "gametype.h"

    void toLowerStr(char *str);
    GameType parseGametypeString(char *gametypeString);
    void parseBoxDimensionString(char *boxDimensionString, unsigned *width, unsigned *height);

#ifdef	__cplusplus
}
#endif

#endif	/* PARAMETERS_H */

