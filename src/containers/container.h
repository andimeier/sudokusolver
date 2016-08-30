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

    void setContainerSet(ContainerSet *containerSet, ContainerType containerType);
    void createContainers(ContainerType type, char *name, size_t numberOfInstances, char *instanceNames[], ContainerSet *containerSet);

#ifdef	__cplusplus
}
#endif

#endif	/* CONTAINER_H */

