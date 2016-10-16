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

    // pointer to "print log" function
    typedef void (*PrintFunc)(void *);
    
    void initLog();
    void writeLog(PrintFunc printFunc, void *info);
    void printLog();

#ifdef	__cplusplus
}
#endif

#endif	/* LOG_H */

