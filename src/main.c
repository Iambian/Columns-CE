/*
 *--------------------------------------
 * Program Name: Columns CE
 * Author: rawrf.
 * License: rawrf.
 * Description: rawrf.
 *--------------------------------------
*/

#define VERSION_INFO 0.1

#define TRANSPARENT_COLOR 0xF8
#define GRIDSTART_X 5
#define GRIDSTART_Y 5

#define GRID_W 6
#define GRID_H 15
#define GRID_HSTART 2
#define GRID_START GRID_HSTART*GRID_W
#define GRID_SIZE GRID_W*GRID_H
#define GRID_OBJ_CHANGE_MASK 0x80
#define GRID_EMPTY 0x00
#define GRID_GEM1 0x01
#define GRID_GEM2 0x02
#define GRID_GEM3 0x03
#define GRID_GEM4 0x04
#define GRID_GEM5 0x05
#define GRID_GEM6 0x06
#define GRID_EXP1 0x07
#define GRID_EXP2 0x08
#define GRID_EXP3 0x09
#define GRID_EXP4 0x0A
#define GRID_EXP5 0x0B
#define GRID_EXP6 0x0C
#define GRID_EXP7 0x0D


/* Keep these headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

/* Standard headers (recommended) */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <graphx.h>
#include <debug.h>
#include <keypadc.h>
#include <graphx.h>
#include <decompress.h>
#include <fileioc.h>

void keywait() { while (kb_AnyKey()); }
void waitanykey() {	keywait(); 	while (!kb_AnyKey()); keywait(); }
/* Put your function prototypes here */
void gentriad(uint8_t maxtypes);
void exchtriad(uint8_t x_left,uint8_t y_top); //exchanges to the right

#include "gfx/tiles_gfx.h"   //gems_tiles_compressed, explosion_tiles_compressed
#include "gfx/sprites_gfx.h" //cursor_compressed, grid_compressed
/* Put all your globals here */
gfx_rletsprite_t *gems_spr[gems_tiles_num];
gfx_rletsprite_t *explosion_spr[explosion_tiles_num];
gfx_rletsprite_t *cursor_spr;
gfx_sprite_t *grid_spr;

uint8_t grid[GRID_SIZE];  //Top two columns are not shown
uint8_t curtriad[3];
uint8_t nexttriad[3];


void main(void) {
    uint8_t i,t,y,gx,gy,gridstart_y,falldelay,curfall,maxfall;
	uint8_t triadx,triady,max_blocktypes;
	int x,gridstart_x;
	void *baseimg;
	kb_key_t kd,kc;
	
	/* Initialize game environment */
	gfx_Begin();
	gfx_SetDrawBuffer();
	gfx_SetTransparentColor(TRANSPARENT_COLOR);
	ti_CloseAll();

	/* Initialize game assets */
	baseimg = (void*) gfx_vbuffer;
	for (i=0;i<gems_tiles_num;i++) {
		dzx7_Turbo(gems_tiles_compressed[i],baseimg);
		gems_spr[i] = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)baseimg);
	}
	for (i=0;i<explosion_tiles_num;i++) {
		dzx7_Turbo(explosion_tiles_compressed[i],baseimg);
		explosion_spr[i] = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)baseimg);
	}
	dzx7_Turbo(cursor_compressed,baseimg);
	cursor_spr = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)baseimg);
	grid_spr = malloc(16*16+2);
	dzx7_Turbo(grid_compressed,grid_spr);
	
	/* Initialize game variables */
	kd = kc = 0;
	gfx_FillScreen(0x09);
	memset(grid,GRID_OBJ_CHANGE_MASK,GRID_SIZE);
	gridstart_x = GRIDSTART_X;
	gridstart_y = GRIDSTART_Y;
	maxfall = 30;
	curfall = 0;
	falldelay = 45; //1.5x maxfall
	max_blocktypes = 6;  //All six gems. Modes changeable from 4 to 6.
	triadx = 2;
	triady = 0;
	gentriad(max_blocktypes);
	
	while (1) {
		kb_Scan();
		kc = kb_Data[1];
		kd = kb_Data[7];
		//BEGIN TEST MODE
		//Move triad
		t = triadx+triady*GRID_W+(GRID_W*2-1); //left side of block
		if (kd&kb_Left && triadx && !(grid[t]&(~GRID_OBJ_CHANGE_MASK))) {
			exchtriad(triadx-1,triady);
			triadx--;
		}
		t+=2;  //right side of block
		if (kd&kb_Right && triadx<5 && !(grid[t]&(~GRID_OBJ_CHANGE_MASK))) {
			exchtriad(triadx,triady);
			triadx++;
		}
		if (kd&kb_Down && curfall > 1) curfall = 1;
		//Check for timed falling
		if (!curfall) {
			//Check if current triad can fall. If so, then increment triady
			if (triady<(GRID_H-3) && !(grid[triadx+triady*GRID_W+(GRID_W*3)]&~(GRID_OBJ_CHANGE_MASK))) {
				triady++;
			}
			for (i = GRID_SIZE-GRID_W;i;) {
				i--;
				if (grid[i] && !(grid[i+GRID_W]&(~GRID_OBJ_CHANGE_MASK))) {
					grid[i+GRID_W] = grid[i] | GRID_OBJ_CHANGE_MASK;
					grid[i] = GRID_EMPTY | GRID_OBJ_CHANGE_MASK;
				}
			}
			curfall = maxfall;
		} else curfall--;
		if (kc&kb_Mode) break;
		//Check if there's been any change in the map within 1.5x max timing.
		//If not, generate new triad
		if (!falldelay) {
			triadx = 2;
			triady = 0;
			gentriad(max_blocktypes);
			falldelay = (maxfall>>1) + maxfall;
		} else {
			t = 0;
			for (i=0;i<GRID_SIZE;i++) t |= grid[i];
			if (t&GRID_OBJ_CHANGE_MASK) falldelay = (maxfall>>1) + maxfall;
			falldelay--;	
		}
		
		//render all changes to the playing field
		for (i=GRID_START,y=gridstart_y,gy=GRID_HSTART;gy<GRID_H;y+=16,gy++) {
			for (x=gridstart_x,gx=0;gx<GRID_W;x+=16,gx++) {
				t = grid[i];
				if (t & 0x80) {
					t = t & 0x7F;
					gfx_Sprite_NoClip(grid_spr,x,y);
					if (t>=GRID_GEM1 && t<=GRID_GEM6) {
						gfx_RLETSprite_NoClip(gems_spr[t-GRID_GEM1],x,y);
					} else if (t>=GRID_EXP1 && t<=GRID_EXP7) {
						gfx_RLETSprite_NoClip(explosion_spr[t-GRID_EXP1],x,y);
					} else {
						//bork bork borked.
					}
					grid[i] = t;
				}
				i++;
			}
		}
		//Clear all changes in nonvisible area of board:
		for (i=0;i<(GRID_W*GRID_HSTART);i++) grid[i] = grid[i]&(~GRID_OBJ_CHANGE_MASK);
		
		gfx_BlitBuffer();
		//END TEST MODE
	}
	keywait();
	gfx_End();
}

/* Put other functions here */
void gentriad(uint8_t maxtypes) {
	uint8_t i,t;
	for (t=2,i=0;i<3;i++,t+=GRID_W) { 
		grid[t] = GRID_GEM1+(randInt(0,maxtypes-1)|GRID_OBJ_CHANGE_MASK);
	}
}

void exchtriad(uint8_t x_left,uint8_t y_top) {
	uint8_t i,t,blk;
	t =  x_left+y_top*GRID_W;
	for (i=0;i<3;i++,t+=GRID_W) {
		blk = grid[t]|GRID_OBJ_CHANGE_MASK;
		grid[t] = grid[t+1]|GRID_OBJ_CHANGE_MASK;
		grid[t+1] = blk;
	}
}
