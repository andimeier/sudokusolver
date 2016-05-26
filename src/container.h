/* 
 * File:   container.h
 * Author: alex
 *
 * Created on 02. Mai 2016, 20:31
 */

#ifndef CONTAINER_H
#define	CONTAINER_H

#include "typedefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

    void setupContainers();
    
    void getCoordinatesInBox(int q, int position, int *x, int *y);
    int getBoxNr(int x, int y);

    // get container index for the specified field coordinates
    int determineRowContainer(unsigned x, unsigned y);
    int determineColumnContainer(unsigned x, unsigned y);
    int determineBoxContainer(unsigned x, unsigned y);
    
#ifdef	__cplusplus
}
#endif

#endif	/* CONTAINER_H */

