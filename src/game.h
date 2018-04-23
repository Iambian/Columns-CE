#ifndef __columns_game__
#define __columns_game__

#include <stdint.h>
#include "types.h"

void initGameState(options_t *options);
void runGame(options_t *options);  //Starts the game session

extern uint16_t bgp7[];

#endif

