/* 
 * File:   log.h
 * Author: ju
 *
 * Created on 05. April 2016, 00:33
 */

#ifndef LOG_H
#define	LOG_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct Entry;
    
    // pointer to "print log" function
    typedef char * (*print)(struct Entry *entry);
    
    typedef struct Entry {
        void *info;
        print *printFunc;
    } Entry;

    typedef struct History {
        Entry **entries;
        size_t capacity;
        size_t count;
    } History;
    
    void initLog();
    void writeLog(print *printFunc, void *info);
    void printLog();

#ifdef	__cplusplus
}
#endif

#endif	/* LOG_H */

