/* 
 * File:   container.h
 * Author: alex
 *
 * Created on 02. Mai 2016, 20:31
 */

#ifndef DIAGONAL_H
#define	DIAGONAL_H

#include "typedefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

    unsigned createDiagonalContainers(ContainerSet *containerSet);

    // aux functions, exposed for unit testing only
    int determineDiagonalContainer(unsigned x, unsigned y);

    
#ifdef	__cplusplus
}
#endif

#endif	/* DIAGONAL_H */

