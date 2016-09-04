/* 
 * File:   squiggle.h
 * Author: alex
 *
 * Created on 2016-07-09
 */

#ifndef SQUIGGLE_H
#define	SQUIGGLE_H

#include "typedefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

    unsigned createBoxContainers(ContainerSet *containerSet);
    void setBoxDimensions(unsigned width, unsigned height);

#ifdef	__cplusplus
}
#endif

#endif	/* SQUIGGLE_H */

