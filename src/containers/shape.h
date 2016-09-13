/* 
 * File:   shape.h
 * Author: alex
 *
 * Created on 2016-07-09
 */

#ifndef SHAPE_H
#define	SHAPE_H

#include "typedefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

    unsigned createShapeContainers(ContainerSet *containerSet);
    void setShapes(char *_shapeIds, char *_shapes);

#ifdef	__cplusplus
}
#endif

#endif	/* SHAPE_H */

