/* 
 * File:   container.h
 * Author: alex
 *
 * Created on 02. Mai 2016, 20:31
 */

#ifndef CONTAINER_H
#define	CONTAINER_H

#ifdef	__cplusplus
extern "C" {
#endif

    void setupContainers();
    
    void getCoordinatesInBox(int q, int position, int *x, int *y);
    int getBoxNr(int x, int y);

#ifdef	__cplusplus
}
#endif

#endif	/* CONTAINER_H */

