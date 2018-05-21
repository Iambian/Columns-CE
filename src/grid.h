#ifndef __columns_grid__
#define __columns_grid__

#include <stdint.h>
#include "types.h"

void refreshgrid(entity_t *e);
void drawgrid(entity_t *e,uint8_t mask_buf);
void falldown(entity_t *e);
void movedir(entity_t *e, enum Direction dir);
int8_t gridmatch(entity_t *e);
void genflashgrid(entity_t *e);
void trimgrid(entity_t *e, uint8_t height);

#endif

