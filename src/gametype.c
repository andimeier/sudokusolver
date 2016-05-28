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

    // FIXME debugging test code
    char *alex;
    
    alex = (char *) xmalloc(sizeof (char) * 5);
    strcpy(alex, "Alex");
    printf("alex2: [%s]\n", alex);




    assert(gameType >= GAME_STANDARD_SUDOKU && gameType <= GAME_COLOR_SUDOKU);

    switch (gameType) {
        case GAME_STANDARD_SUDOKU:
            containerTypes = (unsigned *) xmalloc(sizeof (unsigned) * (3 + 1));

            sprintf(buffer, "allocated memory for container types: (%u, %u, %u, %u)", containerTypes[0], containerTypes[1], containerTypes[2], containerTypes[3]);
            printlog(buffer);

            unsigned *ptr = containerTypes;
            printlog("writing ROWS");
            *ptr++ = ROWS;
            printlog("writing COLS");
            *ptr++ = COLS;
            printlog("writing BOXES");
            *ptr++ = BOXES;
            printlog("written everything");
            *ptr = 0;
            break;
    }

    return containerTypes;
}
