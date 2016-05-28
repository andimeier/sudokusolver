/**
 * functions related to different type of Sudoku games
 */
#include <assert.h>
#include "grid.h"
#include "gametype.h"
#include "log.h"
#include "util.h"

        
unsigned *getContainerTypes(unsigned gameType) {
    unsigned *containerTypes;

    assert(gameType >= GAME_STANDARD_SUDOKU && gameType <= GAME_COLOR_SUDOKU);

    switch (gameType) {
        case GAME_STANDARD_SUDOKU:
            printlog("Game type: Standard Sudoku");
            containerTypes = (unsigned *) xmalloc(sizeof (unsigned) * (3 + 1));

            unsigned *ptr = containerTypes;
            *ptr++ = ROWS;
            *ptr++ = COLS;
            *ptr++ = BOXES;
            *ptr = 0;
            break;
    }

    return containerTypes;
}
