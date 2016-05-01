/* 
 * File:   util.h
 * Author: ju
 *
 * Created on 04. April 2016, 21:57
 */

#ifndef UTIL_H
#define	UTIL_H

#ifdef	__cplusplus
extern "C" {
#endif

    int *unsigneddup(unsigned const *src, size_t len);
    void *xmalloc(size_t size);
            
#ifdef	__cplusplus
}
#endif

#endif	/* UTIL_H */

