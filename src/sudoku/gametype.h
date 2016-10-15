/* 
 * File:   gametype.h
 * Author: alex
 *
 * functions related to different types of Sudoku games (Standard Sudoku,
 * X-Sudoku, Color Sudoku, ...)
 * 
 * Created on 02. Mai 2016, 20:31
 */

#ifndef GAMETYPE_H
#define	GAMETYPE_H

#include "typedefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

    ContainerType *getContainerTypes(GameType gameType);
    char *getGameTypeString(GameType gameType);
    void setGameType(GameType gameType);
    GameType getGameType();
    GameType parseGameTypeString(char *gameTypeString);

#ifdef	__cplusplus
}
#endif

#endif	/* FAMETYPE_H */

