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
				GM_PLAYSTART,GM_PLAYMATCH,GM_PLAYMOVE, GM_GAMEOVER,
				GM_OPTIONS};
enum GameType { TYPE_ARCADE = 0, TYPE_ORIGINAL, TYPE_FLASH };
enum Players {PLAYER1 = 0, PLAYER2, DOUBLES };
enum Difficulty { NOVICE = 3, AMATEUR, PRO,
				  EASIEST, EASY, NORMAL, HARD};
enum Direction {DIR_LEFT = -1,DIR_RIGHT = 1};


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
#define GRID_TBTM (GRID_SIZE-(GRID_W*3))
#define GRID_BELOW (GRID_W*3)

#define MATCH_TIMEOUT 10
#define LONG_TIMEOUT 30

#define CHANGE_BUF1 (1<<7)
#define CHANGE_BUF2 (1<<6)
#define TILE_FLASHING (1<<5)
#define TILE_HALFLINGS (1<<4)
#define TILE_TARGET_GEM (1<<3)

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
	uint8_t cur_delay;     //Unified delay cycles
	uint8_t drop_max;      //Maximum speed to drop at. Decided by level
	uint8_t max_types;     //3,4,5
	unsigned int scoreadd; //Increase score by 1/16 of this much
	uint8_t scorecycle;    //Loop from 16 to 1. Final score on 1. Inactive on 0.
	uint8_t stay_delay;    //from MATCH_TIMEOUT to 0 once triad rests on surface
	
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
	
	
	
	
	

void keywait(void) { while (kb_AnyKey()); }
void waitanykey(void) {	keywait(); 	while (!kb_AnyKey()); keywait(); }
/* Put your function prototypes here */
void initgfx(void);
void initgamestate(options_t *options);
void gentriad(entity_t *entity);
void rungame(options_t *options);
void redrawboard(options_t *options);
void falldown(entity_t *e);
void movedir(entity_t *e, enum Direction dir);



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

uint8_t fallspeed[] = {30,25,20,15,15,10,7,5,4,3,2,1,1,1,2,1,1,1,1,1};


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
	curbuf = 1;
}

	
void initgamestate(options_t *options) {
	uint8_t t,x1,x2,dropmax;
	//Clear entity memory
	memset(&player1,0,sizeof player1);
	memset(&player2,0,sizeof player2);
	//Set player IDs
	player1.playerid = PLAYER1;
	player2.playerid = PLAYER2;
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
		player1.drop_max = fallspeed[options->p1_level];
		player2.drop_max = fallspeed[options->p2_level];
		//Pregen board based on level
	} else {
		player1.level = options->p1_level;
		player2.level = options->p2_level;
		player1.drop_max = player2.drop_max = LONG_TIMEOUT;
	}
	//Set up dgrid to force initial render
	memset(player1.cgrid,CHANGE_BUF1|CHANGE_BUF2,GRID_SIZE);
	memset(player2.cgrid,CHANGE_BUF1|CHANGE_BUF2,GRID_SIZE);
	//Set initial delay cycles
	player1.cur_delay = player2.cur_delay = MATCH_TIMEOUT;
	//Initialize triad
	gentriad(&player1);
}


void gentriad(entity_t *entity) {
	uint8_t *triad,i,t;
	triad = entity->next_triad;
	for (i=0;i<3;i++) triad[i] = GRID_GEM1 + randInt(0,entity->max_types);
}

void rungame(options_t *options) {
	uint8_t i,t,idx;
	uint8_t timer_p1,timer_p2;
	kb_key_t kd,kc;
	uint8_t *ptr;
	int longctr;
	
	//Generate game static background
	asm(" ld bc,51200");
	asm(" ld hl,0D40000h");
	asm(" ld de,090909h");
	asm("loclbl9001:");
	asm(" ld (hl),de");
	asm(" inc hl");
	asm(" inc hl");
	asm(" inc hl");
	asm(" dec bc");
	asm(" ld a,b");
	asm(" or a,c");
	asm(" jr nz,loclbl9001");
	for(i=0;i<2;++i) { redrawboard(options); gfx_SwapDraw(); }
	//End generate game static background
	
	while (1) {
		kb_Scan();
		kc = kb_Data[1];
		kd = kb_Data[7];
		
		if (kc&kb_Mode) { keywait(); ;return; } //For now, exit game immediately
		
		if (player1.state == GM_PLAYSTART) {
			
			
		} else if (player1.state == GM_PLAYMATCH) {
			falldown(&player1);
			//Check if there has been any change in the visible grid.
			
			//TODO: ADD DEBUG DISPLAY TO CHECK FOR CURRENT STATE OF CGRID BUFFER
			//      ON EACH DELAY TIMEOUT (NEED TO FALL DOWN)
			
			for (i=GRID_START,t=0;i<GRID_SIZE;++i) {
				t |= player1.cgrid[i] & (CHANGE_BUF1|CHANGE_BUF2);
			}
			if (t) player1.cur_delay = MATCH_TIMEOUT; //And reset timeout if change.
			//Check for matches, update grid for match anim, and update score
			//Check for level up, and then update drop_max accordingly
			//
			//If match timed out, proceed to place new triad.
			if (!--(player1.cur_delay)) {
				t = (player1.playerid==PLAYER1) ? 2 : 3;
				dbg_sprintf(dbgout,"Emitting triad objects: ");
				for (i=0;i<3;++i,t+=GRID_W) {
					player1.grid[t] = player1.next_triad[i];
					dbg_sprintf(dbgout,"%i, ",player1.next_triad[i]);
					player1.cgrid[t] = CHANGE_BUF1|CHANGE_BUF2;
					player1.next_triad[i] = 0;
				}
				dbg_sprintf(dbgout,"\n");
				player1.cur_delay = player1.drop_max;
				player1.state = GM_PLAYMOVE;
			}
			
		} else if (player1.state == GM_PLAYMOVE) {
			//If push down, speed things up everywhere.
			if (kd&kb_Down) {
				player1.stay_delay = 1;
				player1.cur_delay = 1;
			}
			//Check and handle any left/right motions.
			if (kd&kb_Left) movedir(&player1,DIR_LEFT);
			if (kd&kb_Right) movedir(&player1,DIR_RIGHT);
			//Check if swap (2nd)
			//
			idx = player1.triad_idx;
			//Check if the spot below triad is empty or not on bottom row.
			if (player1.grid[idx+(GRID_W*3)] == GRID_EMPTY && idx<GRID_TBTM) {
				//If empty, reset stay_delay and check cur_delay if need to fall
				player1.stay_delay = MATCH_TIMEOUT;
				if (!--(player1.cur_delay)) {
					falldown(&player1);
					player1.cur_delay = player1.drop_max;
				}
			} else {
				player1.cur_delay = 1; //Make sure delay hovers at 1 in case
				//Check to see if we didn't stop past the top. If so, game over.
				if (idx<GRID_W*2) {
					player1.state = GM_GAMEOVER;
					continue;
				}
				//If not empty, make sure stay_delay is nonzero else do matching
				if (!--(player1.stay_delay)) {
					player1.cur_delay = LONG_TIMEOUT;
					player1.state = GM_PLAYMATCH;
					continue;
				}
			}
		} else if (player1.state == GM_GAMEOVER) {
			break;  //For now, exit game
		
		} else break; //Illegal value - stop playing the game
		//dbg_sprintf(dbgout,"State %i, cur timer %i, stay timer %i\n",player1.state,player1.cur_delay,player1.stay_delay);
		redrawboard(options);
		gfx_SwapDraw();
	}
}

//Draw if any of the flags were set, but clear only mask_buf flag from dgrid
void drawgrid(entity_t *e,uint8_t mask_buf) {
	uint8_t tilestate,tileid,grididx,gridx,gridy,y;
	unsigned int x;
	
	y = e->grid_top;
	grididx = GRID_START;
	for (gridy = GRID_HSTART; gridy < GRID_H; gridy++) {
		x = e->grid_left;
		for (gridx = 0 ; gridx < GRID_W; gridx++) {
			tilestate = e->cgrid[grididx];
			if (tilestate & (CHANGE_BUF1 | CHANGE_BUF2 | TILE_FLASHING)) {
				tilestate &= ~mask_buf; //Acknowledge render
				gfx_Sprite_NoClip(grid_spr,x,y);
				tileid = e->grid[grididx];
				if (tileid >= GRID_EXP1 && tileid <= GRID_EXP7) {
					gfx_RLETSprite_NoClip((gfx_rletsprite_t*)explosion_spr[tileid-GRID_EXP1],x,y);
					if (++tileid >GRID_EXP7) tileid = GRID_EMPTY;
					tilestate |= mask_buf; //Reverse acknowledgement.
				} else if (tileid >= GRID_GEM1 && tileid <=GRID_GEM6) {
					if (!(tilestate&TILE_FLASHING) || main_timer&1) {
						gfx_RLETSprite((gfx_rletsprite_t*)gems_spr[tileid-GRID_GEM1],x,y);
					}
				}
				e->grid[grididx] = tileid;
			}
			e->cgrid[grididx] = tilestate;
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
	
	drawgrid(&player1,mask_buf);
	if (options->players == PLAYER2) drawgrid(&player2,mask_buf);
	
	// Put code here which handles the scoreboard depending on the game type
	// and who's got the score.
	//
	
	curbuf = !curbuf;
}

//Known problem: Does not properly transfer grid properties on falldown.
//This problem affects TILE_TARGET_GEM in Flash Columns mode if the target is
//not on the bottom row. We should also implement half drop support eventually.
void falldown(entity_t *e) {
	uint8_t i;
	for (i=GRID_SIZE-1;i>GRID_W;--i) {
		if (e->grid[i] == GRID_EMPTY && e->grid[i-GRID_W] != GRID_EMPTY) {
			e->grid[i] = e->grid[i-GRID_W];
			e->grid[i-GRID_W] = GRID_EMPTY;
			e->cgrid[i] |= CHANGE_BUF1|CHANGE_BUF1;
			e->cgrid[i-GRID_W] |= CHANGE_BUF1|CHANGE_BUF1;
		}
	}
	if (e->triad_idx<GRID_TBTM && e->cgrid[i+GRID_BELOW]!=GRID_EMPTY) {
		e->triad_idx += GRID_W;
	}
}

void movedir(entity_t *e, enum Direction dir) {
	int8_t i,idx,oldidx,temp;
	//Move idx to left or right if we aren't on a field boundary
	oldidx = idx = e->triad_idx;
	if ((dir<0 && !(idx%GRID_W)) || (dir>0 && !((idx-1)%GRID_W))) {
		i =+ dir;
	}
	//Do not change anything else if we tried to move on top of something else.
	if (e->grid[idx+(GRID_W*2)] != GRID_EMPTY) return;
	//Begin exchanging.
	e->triad_idx = idx;
	for (i=0;i<3;++i) {
		temp = e->grid[idx];
		e->grid[idx] = e->grid[oldidx];
		e->grid[oldidx] = temp;
		temp = e->cgrid[oldidx];
		e->cgrid[oldidx] = e->cgrid[idx] | CHANGE_BUF1 | CHANGE_BUF2;
		e->cgrid[idx] = temp | CHANGE_BUF1 | CHANGE_BUF2;
		idx += GRID_W;
		oldidx += GRID_W;
	}
	//If temp has the TILE_HALFLINGS flag set, force the tile below that to
	//update if idx is not past the bottom of the screen
	if (idx<GRID_SIZE) e->cgrid[idx] |= CHANGE_BUF1 | CHANGE_BUF2;
}





