/*
 *--------------------------------------
 * Program Name: Columns CE
 * Author: rawrf.
 * License: rawrf.
 * Description: rawrf.
 *--------------------------------------
*/

#define VERSION_INFO 0.1

enum GameState {GM_TITLE=0,GM_MAINMENU,
				GM_ARCADEOPTIONS, //This is an in-field menu
				GM_GAMEMENU,GM_GAMEOPTIONS,GM_GAMEPREVIEW,
				GM_PLAYSTART,GM_PLAYMATCH,GM_PLAYMOVE,  //Used in gameplay only
				GM_OPTIONS};
enum GameType { TYPE_ARCADE = 0, TYPE_ORIGINAL, TYPE_FLASH };
enum Players {PLAYER1 = 0, PLAYER2, DOUBLES };
enum Difficulty { NOVICE = 3, AMATEUR, PRO,
				  EASIEST, EASY, NORMAL, HARD};


#define TRANSPARENT_COLOR 0xF8
#define GRIDSTART_X 5
#define GRIDSTART_Y 5

#define GRID_W 6
#define GRID_H 15
#define GRID_HSTART 2
#define GRID_START GRID_HSTART*GRID_W
#define GRID_SIZE GRID_W*GRID_H
#define TILE_W 16
#define TILE_H 16

#define GRID_OBJ_CHANGE_MASK 0x80
#define CHANGE_BUF1 (1<<7)
#define CHANGE_BUF2 (1<<6)
#define TILE_FLASHING (1<<5)
#define TILE_HALFLINGS

#define P1_GRIDLEFT 16
#define P2_GRIDLEFT 208
#define CENTER_GRIDLEFT 112


#define GRID_EMPTY 0x00
#define GRID_GEM1 0x01
#define GRID_GEM2 0x02
#define GRID_GEM3 0x03
#define GRID_GEM4 0x04
#define GRID_GEM5 0x05
#define GRID_GEM6 0x06
//Insert other gems here


#define GRID_EXP1 0x07
#define GRID_EXP2 0x08
#define GRID_EXP3 0x09
#define GRID_EXP4 0x0A
#define GRID_EXP5 0x0B
#define GRID_EXP6 0x0C
#define GRID_EXP7 0x0D
//Not an actual sprite code. 


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

typedef struct entity_t {
	enum Players playerid; //
	unsigned int grid_top; //pixel position. Best use 16 here.
	uint8_t grid_left;     //px pos. 112 in single, 16/208 in 2player
	uint8_t triad_idx;     //X+Y*GRID_WIDTH, where X,Y is topmost block in triad
	unsigned int level;    //Player's level
	unsigned int score;    //player's current score
	uint8_t combo;         //Player's current combo
	uint8_t matches;       //9 match cycles (matchlen not considered) is level++
	enum GameState state;  //GM_PLAYMATCH or GM_PLAYMOVE
	uint8_t drop_speed;    //Cycles to delay
	uint8_t drop_max;      //Maximum speed to drop at. Decided by level
	uint8_t max_types;     //3,4,5
	
	uint8_t next_triad[3]; //next 3 blocks, top to bottom.
	uint8_t grid[GRID_SIZE];  //Gem/explosion IDs
	uint8_t cgrid[GRID_SIZE]; //Board state flags (changing, flashing, etc)
} entity_t;

typedef struct options_t {
	enum GameType type;
	enum Players players;
	enum Difficulty p1_class;
	enum Difficulty p2_class;
	bool time_trial;
	uint8_t p1_level;  //doubles as column height in Flash columns mode
	uint8_t p2_level;
	uint8_t bgm;
} options_t;
	
	
	
	
	

enum GameState {GM_TITLE=0,GM_MAINMENU,
				GM_ARCADEOPTIONS, //This is an in-field menu
				GM_GAMEMENU,GM_GAMEOPTIONS,GM_GAMEPREVIEW,
				GM_PLAYSTART,GM_PLAYMATCH,GM_PLAYMOVE,  //Used in gameplay only
				GM_OPTIONS};
enum GameType { TYPE_ARCADE = 0, TYPE_ORIGINAL, TYPE_FLASH };
enum Players {PLAYER1 = 0, PLAYER2, DOUBLES };
enum Difficulty { NOVICE = 0, AMATEUR, PRO };

void keywait(void) { while (kb_AnyKey()); }
void waitanykey(void) {	keywait(); 	while (!kb_AnyKey()); keywait(); }
/* Put your function prototypes here */
void initgfx(void);
void initgamestate(options_t *options);
void gentriad(entity_t entity);
void rungame(options_t *options);
void redrawboard(options_t *options);




void exchtriad(uint8_t x_left,uint8_t y_top); //exchanges to the right
void redrawgrid(uint8_t *gridvar, int gridstart_x, int gridstart_y);
void gridfall(uint8_t *gridvar);
uint8_t markmatches(uint8_t *gridvar, uint8_t *dgridvar);


#include "gfx/tiles_gfx.h"   //gems_tiles_compressed, explosion_tiles_compressed
#include "gfx/sprites_gfx.h" //cursor_compressed, grid_compressed

/* ---------------------- Put all your globals here ------------------------- */
gfx_rletsprite_t *gems_spr[gems_tiles_num];
gfx_rletsprite_t *explosion_spr[explosion_tiles_num];
gfx_rletsprite_t *cursor_spr;
gfx_sprite_t *grid_spr;
uint8_t gamestate;
entity_t player1;
entity_t player2;
bool curbuf;
uint8_t main_timer;



void main(void) {
	uint8_t i;
	kb_key_t kd,kc;
	options_t options;
	
	initgfx();
	gfx_FillScreen(0x09);
	
	/* Initialize game variables */
	
	options.type = TYPE_ORIGINAL;
	options.players = PLAYER1;
	options.p1_class = NOVICE;
	options.p2_class = EASY;
	options.time_trial = false;
	options.p1_level = 0;
	options.p2_level = 0;
	options.bgm = 0;
	
	initgamestate(&options);
	rungame(&options);
	
	keywait();
	gfx_End();
	}
}



void redrawgrid(uint8_t *gridvar, int gridstart_x, int gridstart_y) {
	uint8_t i,gx,gy,y,t;
	int x;
	for (i=GRID_START,y=gridstart_y,gy=GRID_HSTART;gy<GRID_H;y+=16,gy++) {
		for (x=gridstart_x,gx=0;gx<GRID_W;x+=16,gx++) {
			t = grid[i];
			if (t & GRID_OBJ_CHANGE_MASK) {
				t = t & (~GRID_OBJ_CHANGE_MASK);
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
}

void gridfall(uint8_t *gridvar) {
	uint8_t i;
	for (i = GRID_SIZE-GRID_W;i;) {
		i--;
		if (grid[i] && !(grid[i+GRID_W]&(~GRID_OBJ_CHANGE_MASK))) {
			grid[i+GRID_W] = grid[i] | GRID_OBJ_CHANGE_MASK;
			grid[i] = GRID_EMPTY | GRID_OBJ_CHANGE_MASK;
		}
	}
}
//On intializing a search, ignore re-matched (changed) bit only for initial
//but do direct match against masked initial for all else on the chain
//Re-using GRID_OBJ_CHANGE_MASK for matched objects

uint8_t markmatches(uint8_t *gridvar,uint8_t *dgridvar) {
	uint8_t i,t,k,x,y,gx,gy,tx,ty,ti,ct;
	
	for (i=y=0;y<(GRID_H-3);y++) {
		for(x=0;x<GRID_W;x++,i++) {
			//Get current gem or skip iteration if not a gem
			t = gridvar[i]&(~GRID_OBJ_CHANGE_MASK);
			if (t == GRID_EMPTY || t >= GRID_EXP1) continue;
			//Matching to the right
			tx = x;
			ti = i;
			ct = 0;
			while (++tx<GRID_W) {
				ti++;
				if (gridvar[ti]!=t) break;
				ct++;
			}
			if (ct>2) {
				tx=x;ti=i;
				while (++tx<GRID_W) {
					ti++;
					if (gridvar[ti]!=t) break;
					gridvar[ti] = gridvar[ti]|GRID_OBJ_CHANGE_MASK;
				}
			}
			//Matching to the bottom-right
			
		}
	}
	for (ct=i=0;i<GRID_SIZE;i++) {
		if (GRID_OBJ_CHANGE_MASK&gridvar[i]) {
			ct++;
			dgridvar[i] = 1;
		} else dgridvar[i] = 0;
	}
	return ct;
}

/* ========================================================================== */

void initgfx(void) {
	uint8_t i;
	void *baseimg = (void*) gfx_vbuffer;
	
	gfx_Begin();
	gfx_SetDrawBuffer();
	gfx_SetTransparentColor(TRANSPARENT_COLOR);
	ti_CloseAll();

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
}

	
void initgamestate(options_t *options) {
	uint8_t t,x1,x2;
	//Clear entity memory
	memset(&player1,0,sizeof player1);
	memset(&player2,0,sizeof player2);
	//Set player IDs
	player1.playerid = PLAYER1;
	player1.playerid = PLAYER2;
	//Set grid top position
	player1.grid_top = player2.grid_top = 16;
	//Set grid left position
	if (options->players == PLAYER2){
		x1 = P1_GRIDLEFT;
		x2 = P2_GRIDLEFT;
	}
	else {
		x2 = x1 = CENTER_GRIDLEFT;
	}
	player1.grid_left = x1;
	player2.grid_left = x2;
	//Set game state
	player1.state = player2.state = GM_PLAYMATCH;  //guarantees triad placement
	//Set maximum gem types
	if (options->type == TYPE_ARCADE) {
		player1.max_types = 5;
		player2.max_types = 5;
	} else {
		player1.max_types = (uint8_t) options->p1_class;
		player2.max_types = (uint8_t) options->p2_class;
	}
	//Preselect game level
	if (options->type == TYPE_FLASH) {
		//Pregen board based on level
	} else {
		player1.level = options->p1_level;
		player2.level = options->p2_level;
	}
	//Set up dgrid to force initial render
	memset(player1->dgrid,CHANGE_BUF1|CHANGE_BUF2,GRID_SIZE);
	memset(player2->dgrid,CHANGE_BUF1|CHANGE_BUF2,GRID_SIZE);
}


void gentriad(entity_t *entity) {
	uint8_t *triad,i;
	triad = entity->next_triad;
	for (i=0;i<3;i++,triad++) *triad = GRID_GEM1 + randInt(0,entity->max_types);
}

void rungame(options_t *options) {
	uint8_t i;
	uint8_t timer_p1,timer_p2;
	
	//Generate game static background
	
	
	
	
	
	
	
	
}

//Draw if any of the flags were set, but clear only mask_buf flag from dgrid
void drawgrid(entity_t *e,uint8_t mask_buf, uint8_t mask_scr) {
	uint8_t tilestate,tileid,grididx,gridx,gridy,y;
	unsigned int x;
	
	screeny = e->grid_top;
	grididx = GRID_START;
	for (gridy = GRID_HSTART; gridy < GRID_H; gridy++) {
		x = e->grid_left;
		for (gridx = 0 ; gridx < GRID_W; gridx++) {
			tilestate = e->dgrid[grididx];
			if (tilestate & (CHANGE_BUF1 | CHANGE_BUF2 | TILE_FLASHING)) {
				tilestate &= ~mask_buf; //Acknowledge render
				gfx_Sprite_NoClip(grid_spr,x,y);
				tileid = e->grid[grididx];
				if (tileid >= GRID_EXP1 && tileid <= GRID_EXP7) {
					gfx_TransparentSprite_NoClip(explosion_spr[GRID_EXP1-tileid],x,y);
					if (++tileid >GRID_EXP7) tileid = GRID_EMPTY;
					tilestate |= mask_buf; //Reverse acknowledgement.
				} else if (tileid >= GRID_GEM1 && tileid <=GRID_GEM6) {
					if (!(tilestate&TILE_FLASHING) || main_timer&1) {
						gfx_TransparentSprite(gems_spr[GRID_GEM1-tileid],x,y);
					}
				}
			}
			x += TILE_W;
			grididx++;
		}
		y += TILE_H;
	}
}

void redrawboard(options_t *options) {
	uint8_t i,mask_buf,mask_scr;
	
	if (curbuf) {
		mask_buf = CHANGE_BUF1;
		mask_scr = CHANGE_BUF2;
	} else {
		mask_buf = CHANGE_BUF2;
		mask_scr = CHANGE_BUF1;
	}
	
	
	
	
	
	curbuf = !curbuf;
}




