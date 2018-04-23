#ifndef __columns_gfx__
#define __columns_gfx__

#include <stdint.h>

#include "defs.h"

void initGraphics(void);

extern gfx_rletsprite_t *gems_spr[];
extern gfx_rletsprite_t *explosion_spr[];
extern gfx_rletsprite_t *cursor_spr;
extern gfx_sprite_t *grid_spr;
extern gfx_sprite_t *greentile;
extern gfx_sprite_t *cyantile;
extern gfx_rletsprite_t *titlebanner;
extern gfx_rletsprite_t *bg_central;
extern gfx_rletsprite_t *bg_next;
extern gfx_rletsprite_t *bg_score;
extern gfx_rletsprite_t *bg_scoref;
extern gfx_rletsprite_t *bg_btm8d;
extern gfx_rletsprite_t *bg_btm8df;
extern gfx_rletsprite_t *bg_btm5d;
extern gfx_rletsprite_t *bg_btm5df;
extern gfx_rletsprite_t *bg_btm4d;
extern gfx_rletsprite_t *bg_btm4df;
extern gfx_rletsprite_t *bg_btm3d;
extern gfx_rletsprite_t *bg_btm3df;
extern gfx_rletsprite_t *bg_top8d;
extern gfx_rletsprite_t *bg_top5d;
extern gfx_rletsprite_t *bg_top4d;
extern gfx_rletsprite_t *bg_top3d;
extern gfx_rletsprite_t *bgspr[];
extern gfx_rletsprite_t *scorenum1[]; //0-9 and colon.
extern gfx_rletsprite_t *scorenum2[]; //0-9 and colon.
extern gfx_sprite_t *menutile;
extern gfx_sprite_t *menuborder;
extern gfx_rletsprite_t *p1sprite;
extern gfx_rletsprite_t *p2sprite;
extern gfx_rletsprite_t *downarrow;
extern gfx_rletsprite_t *flashgems[6][8];
extern gfx_rletsprite_t *magicgems[6];
extern gfx_rletsprite_t *arcadeselect;
extern gfx_rletsprite_t *gameoverspr[8][2];
extern gfx_rletsprite_t *youdiditspr[13][2];
extern uint8_t gameoverchr[];
extern uint8_t youdiditchr[];
extern uint8_t gameoverpos[];
extern uint8_t youdiditpos[];
extern gfx_sprite_t *hsborder;
extern gfx_sprite_t *hsborderf;


#endif