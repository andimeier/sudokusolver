/* 
 * File:   box.h
 * Author: alex
 *
 * Created on 2016-07-09
 */

#ifndef BOX_H
#define	BOX_H

#include "typedefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

    unsigned createBoxContainers(ContainerSet *containerSet);
    void setBoxDimensions(unsigned width, unsigned height);
    void determineBoxDimensions(unsigned maxNumber, unsigned *width, unsigned *height);
    unsigned getBoxWidth();
    unsigned getBoxHeight();
    
#ifdef	__cplusplus
}
#endif

#endif	/* BOX_H */

