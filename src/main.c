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
// As specified (rather obliquely) in convpng.ini
#define FONT_WHITE 2
#define FONT_GOLD 3
#define FONT_CYAN 4

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
#define MOVESIDE_TIMEOUT 5

#define CHANGE_BUF1 (1<<7)
#define CHANGE_BUF2 (1<<6)
#define TILE_FLASHING (1<<5)
#define TILE_HALFLINGS (1<<4)
#define TILE_TARGET_GEM (1<<3)

#define P1_GRIDLEFT 16
#define P2_GRIDLEFT 208
#define CENTER_GRIDLEFT 112

#define PALSWAP_AREA 192


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
	
uint16_t bgp1[] = {528,396,264,4228,404,272,140,8};                  //1PO: cyan/blue
uint16_t bgp2[] = {20876,16648,12420,8192,21008,16780,12552,8324};   //2PO: pink/lpink
uint16_t bgp3[] = {25088,20864,16640,12416,21012,16784,12556,8328};  //DBO: gold/lav
uint16_t bgp4[] = {528,396,264,132,516,388,260,128};                 //1PF: cyan/grn
uint16_t bgp5[] = {16652,12424,8196,4096,16784,12556,8328,4100};     //2PF: purp/lpur
uint16_t bgp6[] = {29312,25088,20864,16512,29056,24832,20608,16512}; //DBF: yell/orng
uint16_t bgp7[] = {20872,16644,12416,8192,21004,16776,12548,8320};   //Aracde mode

uint16_t *blockpal[] = {
	bgp1,bgp2,bgp3,bgp4,bgp5,bgp6,bgp7
};
	
	
	

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
uint8_t gridmatch(entity_t *e);  //returns number of blocks matched, mods cgrid


#include "font.h"            //Fontset to use
#include "gfx/tiles_gfx.h"   //gems_tiles_compressed, explosion_tiles_compressed
#include "gfx/sprites_gfx.h" //cursor_compressed, grid_compressed
#include "gfx/bg_gfx.h"      //Background stuffs

/* ---------------------- Put all your globals here ------------------------- */
gfx_rletsprite_t *gems_spr[gems_tiles_num];
gfx_rletsprite_t *explosion_spr[explosion_tiles_num];
gfx_rletsprite_t *cursor_spr;
gfx_sprite_t *grid_spr;
gfx_sprite_t *greentile;
gfx_sprite_t *cyantile;
uint8_t gamestate;
entity_t player1;
entity_t player2;
bool curbuf;
uint8_t main_timer;

gfx_rletsprite_t *bg_central;
gfx_rletsprite_t *bg_next;
gfx_rletsprite_t *bg_score;
gfx_rletsprite_t *bg_scoref;
gfx_rletsprite_t *bg_btm8d;
gfx_rletsprite_t *bg_btm5d;
gfx_rletsprite_t *bg_btm4d;
gfx_rletsprite_t *bg_btm3d;
gfx_rletsprite_t *bg_btm8df;
gfx_rletsprite_t *bg_btm5df;
gfx_rletsprite_t *bg_btm4df;
gfx_rletsprite_t *bg_btm3df;
gfx_rletsprite_t *bg_top8d;
gfx_rletsprite_t *bg_top5d;
gfx_rletsprite_t *bg_top4d;
gfx_rletsprite_t *bg_top3d;


uint8_t fallspeed[] = {30,25,20,15,15,10,7,5,4,3,2,1,1,1,2,1,1,1,1,1};


void main(void) {
	uint8_t i;
	kb_key_t kd,kc;
	options_t options;
	
	initgfx();
	gfx_FillScreen(0x01);
	
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

void palshift(gfx_sprite_t *sprite, uint8_t shiftby) {
	uint8_t x,y;
	int size;
	uint8_t *ptr;
	
	ptr = (uint8_t*) sprite;
	size = ptr[0]*ptr[1];
	ptr+=2;
	for(;size;--size,++ptr) ptr[0] +=shiftby;
}


gfx_rletsprite_t* decompAndAllocate(void* cmprsprite) {
	uint8_t i;
	void *baseimg,*flipimg,*img;
	
	baseimg = (void*) gfx_vbuffer;
	flipimg = (void*) (*gfx_vbuffer+32768+2);
	
	dzx7_Turbo(cmprsprite,baseimg);
	img = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)baseimg);
	gfx_FlipSpriteY((gfx_sprite_t*) baseimg, (gfx_sprite_t*)flipimg);
	
	return img;
}

void initgfx(void) {
	uint8_t i;
	void *baseimg,*flipimg;
	uint8_t *ptr;
	int loop;
	uint8_t *fontspacing;
	
	gfx_Begin();
	gfx_SetDrawBuffer();
	ti_CloseAll();
	//Font data abbreviated
	gfx_SetFontData(font-(32*8));
	fontspacing = malloc(128);
	for (i=0;i<128;i++) fontspacing[i]=8;
	gfx_SetFontSpacing(fontspacing);
	gfx_SetPalette(tiles_gfx_pal,sizeof tiles_gfx_pal,0);
	//Palette area at PALSWAP_AREA is initialized on game mode select
	
	baseimg = (void*) gfx_vbuffer;
	flipimg = (void*) (*gfx_vbuffer+32768+2);
	
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
	//Alloc to special area
	dzx7_Turbo(greentile_compressed,greentile = malloc(32*32+2));
	dzx7_Turbo(cyantile_compressed,cyantile = malloc(32*32+2));
	palshift(greentile,PALSWAP_AREA);
	palshift(cyantile,PALSWAP_AREA);
	//All those backers and transform
	bg_central = decompAndAllocate(bg_central_compressed);
	bg_next    = decompAndAllocate(bg_next_compressed);
	bg_score   = decompAndAllocate(bg_score_compressed);
	bg_scoref  = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)flipimg);
	bg_btm8d   = decompAndAllocate(bg_btm8d_compressed);
	bg_btm8df  = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)flipimg);
	bg_btm5d   = decompAndAllocate(bg_btm5d_compressed);
	bg_btm5df  = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)flipimg);
	bg_btm4d   = decompAndAllocate(bg_btm4d_compressed);
	bg_btm4df  = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)flipimg);
	bg_btm3d   = decompAndAllocate(bg_btm3d_compressed);
	bg_btm3df  = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)flipimg);
	bg_top8d   = decompAndAllocate(bg_top8d_compressed);
	bg_top5d   = decompAndAllocate(bg_top5d_compressed);
	bg_top4d   = decompAndAllocate(bg_top4d_compressed);
	bg_top3d   = decompAndAllocate(bg_top3d_compressed);
	
	
	
	
	
	
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

void drawSpriteAndClass(enum Difficulty diff, gfx_rletsprite_t *sprite, int x, int y, int offset) {
	char *strn;
	gfx_RLETSprite(sprite,x,y);
	if (diff == NOVICE) strn = "NOV";
	else if (diff == AMATEUR) strn = "AMA";
	else if (diff == PRO) strn = "PRO";
	else strn = "!!!";
	gfx_PrintStringXY(strn,x+3+offset,y+2);
}


//Magic numbers are for 32x32 tiles on a 320x240 display
//Some conversions were required since the original was a 320x224 display.
void drawgamebg(options_t *options) {
	int x,y;
	uint8_t x1,x2;
	uint8_t ix,iy;
	gfx_sprite_t *ptr;
	gfx_rletsprite_t *rptr;
	
	for(iy=0,y=-8; iy<8; ++iy,y+=32) {
		for(ix=0,x=0; ix<10; ++ix,x+=32) {
			if ((ix^iy)&1) ptr = greentile;
			else ptr = cyantile;
			gfx_Sprite(ptr,x,y);
		}
	}
	
	gfx_SetTextFGColor(FONT_GOLD);
	
	//Draw NEXT box
	if (options->players == PLAYER2) {
		x1 = 118;
		x2 = 182;
	} else {
		x1 = 86;
		x2 = 214;
	}
	gfx_RLETSprite(bg_next,x1,14);
	if (options->players != PLAYER1) gfx_RLETSprite(bg_next,x2,14);
		
	//Draw current score/timer box
	
	if (options->players == PLAYER2) {
		gfx_RLETSprite(bg_scoref,115,70); //P1
		gfx_RLETSprite(bg_score,158,94);  //P2
	} else {
		gfx_RLETSprite(bg_score,62,94);  //P1
		if (options->players == DOUBLES) {
			gfx_RLETSprite(bg_scoref,211,94);  //P2
		}
	}
		
	//Draw current score/time indicator box
	if (options->players == PLAYER2) {
		 gfx_RLETSprite(bg_central,134,126);
		 if (options->type == TYPE_FLASH) {
			gfx_RLETSprite(bg_top5d,115,117);
			gfx_RLETSprite(bg_btm5d,157,141);
			gfx_PrintStringXY("best",144,131);
		 } else {
			gfx_RLETSprite(bg_top8d,115,117);
			gfx_RLETSprite(bg_btm8d,133,141);
			gfx_PrintStringXY("score",140,131);
		 }
	} else {
		gfx_RLETSprite(bg_central,37,126);
		if (options->type == TYPE_FLASH) {
			gfx_RLETSprite(bg_btm5d,61,141);
			gfx_PrintStringXY("best",48,131);
		} else {
			gfx_RLETSprite(bg_btm8d,37,141);
			gfx_PrintStringXY("score",44,131);
		}
		if (options->players == DOUBLES) {
			gfx_RLETSprite(bg_central,230,126);
			if (options->type == TYPE_FLASH) {
				gfx_RLETSprite(bg_btm5df,212,141);
				gfx_PrintStringXY("best",240,131);
			} else {
				gfx_RLETSprite(bg_btm8df,212,141);
				gfx_PrintStringXY("score",236,131);
			}
		}
	}
	
	//Display current level
	if (options->players == PLAYER2) {
		gfx_RLETSprite(bg_central,134,166);
		gfx_RLETSprite(bg_top3d,115,157);
		gfx_RLETSprite(bg_btm3d,173,181);
		gfx_PrintStringXY("level",140,171);
	} else {
		gfx_RLETSprite(bg_central,38,166);
		gfx_RLETSprite(bg_btm3d,77,181);
		gfx_PrintStringXY("level",44,171);
		if (options->players == DOUBLES) {
			gfx_RLETSprite(bg_central,230,166);
			gfx_RLETSprite(bg_btm3df,212,181);
			gfx_PrintStringXY("level",236,171);
		}
	}
	
	//Display current jewels/class
	if (options->players == PLAYER2) {
		gfx_RLETSprite(bg_central,134,198);
		if (options->type == TYPE_FLASH) {
			gfx_PrintStringXY("class",140,203);
			drawSpriteAndClass(options->p1_class,bg_top3d,115,189,2);
			drawSpriteAndClass(options->p2_class,bg_btm3d,173,213,0);
		} else {
			gfx_PrintStringXY("jewels",136,203);
			gfx_RLETSprite(bg_top4d,115,189);
			gfx_RLETSprite(bg_btm4d,165,213);
		}
	} else {
		gfx_RLETSprite(bg_central,38,198);
		if (options->type == TYPE_FLASH) {
			gfx_PrintStringXY("class",44,203);
			drawSpriteAndClass(options->p1_class,bg_btm3d,77,213,0);
		} else {
			gfx_PrintStringXY("jewels",40,203);
			gfx_RLETSprite(bg_btm4d,69,213);
		}
		if (options->players == DOUBLES) {
			gfx_RLETSprite(bg_central,230,198);
			if (options->type == TYPE_FLASH) {
				gfx_PrintStringXY("class",236,203);
				drawSpriteAndClass(options->p2_class,bg_btm3df,212,213,0);
			} else {
				gfx_PrintStringXY("jewels",232,203);
				gfx_RLETSprite(bg_btm4df,212,213);
			}
		}
	}
	
	
//drawSpriteAndClass(diff,*sprite,x,y,offset)
	
	
	
	
	
	
}

void rungame(options_t *options) {
	uint8_t i,t,idx;
	uint8_t moveside_active,moveside_delay;
	uint8_t shuffle_active;
	uint8_t timer_p1,timer_p2;
	kb_key_t kd,kc;
	uint8_t *ptr;
	uint16_t *palptr;
	int longctr;
	uint8_t flash_active,flash_countdown;
	uint8_t score_active,score_countdown;
	uint8_t matches_found;
	uint8_t palette_offset;
	
RESTARTGAME:
	
	moveside_delay = MOVESIDE_TIMEOUT;
	flash_countdown = flash_active = score_active = shuffle_active = moveside_active = 0;
	//Generate game static background
	
	if (options->type == TYPE_ARCADE) {
		palptr = blockpal[6];
	} else {
		palptr = blockpal[(options->type-1)*2+options->players];
	}
	gfx_SetPalette(palptr,16,PALSWAP_AREA);
	
	palette_offset = 0;
	for(i=0;i<2;++i) {
		drawgamebg(options);
		redrawboard(options);
		gfx_SwapDraw(); 
	}
	//End generate game static background
	
	while (1) {
		kb_Scan();
		kc = kb_Data[1];
		kd = kb_Data[7];
		
		// DEBUGGING START - DEL KEY CYCLES GAME MODES
		if (kc&kb_Del) {
			keywait();
			if (!((++options->players) %= 3) || options->type == TYPE_ARCADE) {
				(++options->type) %= 3;
			}
			goto RESTARTGAME;
		}
		// DEBUGGING END -- REMOVE SECTION BETWEEN WHEN NO LONGER NEEDED
		
		if (kc&kb_Mode) { keywait(); ;return; } //For now, exit game immediately
		//Left/right debouncing no matter the mode
		if (kd&(kb_Left|kb_Right)) {
			if (moveside_active) {
				if (moveside_delay) {
					kd = 0;
					moveside_delay--;
				}
			} else {
				moveside_active = 1;
				moveside_delay = MOVESIDE_TIMEOUT;
			}
		} else moveside_active = 0;
		//2nd key debouncing
		if (kc&kb_2nd) {
			if (shuffle_active) kc &= ~kb_2nd;
			else shuffle_active = 1;
		} else shuffle_active = 0;
		
		if (player1.state == GM_PLAYSTART) {
			
			
			/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
		} else if (player1.state == GM_PLAYMATCH) {
			falldown(&player1);
			
			for (i=GRID_START,t=0;i<GRID_SIZE;++i) {
				t |= player1.cgrid[i] & (CHANGE_BUF1|CHANGE_BUF2|TILE_FLASHING);
			}
			if (t) player1.cur_delay = MATCH_TIMEOUT; //And reset timeout if change.
			//If flashing, wait until timeout to destroy gems fully.
			if (flash_active) {
				if (!--flash_countdown) {
					flash_active = 0;
					for (i=0; i<GRID_SIZE; ++i) {
						if (player1.cgrid[i] & TILE_FLASHING) {
							player1.cgrid[i] = CHANGE_BUF1|CHANGE_BUF2;
							player1.grid[i] = GRID_EXP1;
						}
					}
				}
			}
			
			//Check for matches, update grid for match anim, and update score
			//Check for level up, and then update drop_max accordingly
			//
			//If match timed out, check board for matches proceed to place new triad.
			if (!--(player1.cur_delay)) {
				matches_found = gridmatch(&player1);
				if (matches_found) {
					flash_active = 1;
					flash_countdown = 20;
					//Add scoring when it is the thing to do.
					//
				} else {
					//Check if anything is past the top before continuing
					for (i=t=0; i<GRID_START; ++i) {
						if (player1.grid[i] != GRID_EMPTY) t = 1;
					}
					if (t) {
						player1.state = GM_GAMEOVER;
						player1.triad_idx = GRID_SIZE-1; //Re-used for indexing
						continue;
					}
					//Emit triad object.
					player1.triad_idx = t = (player1.playerid==PLAYER1) ? 2 : 3;
					dbg_sprintf(dbgout,"Emitting triad objects: ");
					for (i=0;i<3;++i,t+=GRID_W) {
						player1.grid[t] = player1.next_triad[i];
						dbg_sprintf(dbgout,"%i, ",player1.next_triad[i]);
						player1.cgrid[t] = CHANGE_BUF1|CHANGE_BUF2;
						player1.next_triad[i] = 0;
					}
					dbg_sprintf(dbgout,"\n");
					player1.cur_delay = player1.drop_max;
					player1.stay_delay = LONG_TIMEOUT;
					player1.state = GM_PLAYMOVE;
				}
			}
			/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
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
			if (kc&kb_2nd) {
				t = player1.grid[player1.triad_idx+(2*GRID_W)];
				for (i=3,idx=player1.triad_idx+(2*GRID_W);i;--i,idx-=GRID_W) {
					player1.grid[idx] = player1.grid[idx-GRID_W];
					player1.cgrid[idx] |= CHANGE_BUF1|CHANGE_BUF2;
				}
				player1.grid[player1.triad_idx] = t;
			}
			//
			idx = player1.triad_idx;
			
			//Check if the spot below triad is empty or not on bottom row.
			if ((player1.grid[idx+(GRID_W*3)] == GRID_EMPTY) && (idx<GRID_TBTM)) {
				//If empty, reset stay_delay and check cur_delay if need to fall
				//dbg_sprintf(dbgout,"Condition 1: %i\n",player1.grid[idx+(GRID_W*3)] == GRID_EMPTY);
				//dbg_sprintf(dbgout,"Condition 2: %i\n",GRID_EMPTY && idx<GRID_TBTM);
				player1.stay_delay = LONG_TIMEOUT;
				if (!--(player1.cur_delay)) {
					falldown(&player1);
					if (player1.triad_idx >= GRID_START && !player1.next_triad[0]) {
						gentriad(&player1);
					}
					player1.cur_delay = player1.drop_max;
				}
			} else {
				player1.cur_delay = 1; //Make sure delay hovers at 1 in case
				//If not empty, make sure stay_delay is nonzero else do matching
				if (!--(player1.stay_delay)) {
					//Check to see if we didn't stop past the top. If so, game over.
					player1.cur_delay = LONG_TIMEOUT;
					player1.state = GM_PLAYMATCH;
					continue;
				}
			}

			
			/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
		} else if (player1.state == GM_GAMEOVER) {
			if (player1.triad_idx==255) break;  //nothing after anim. Kill rtn.
			if (main_timer&1) {  //Cycle every other frame
				for(i=0;i<6;++i,player1.triad_idx--) {
					player1.grid[player1.triad_idx] = GRID_EXP1;
					player1.cgrid[player1.triad_idx] = CHANGE_BUF1|CHANGE_BUF2;
				}
			}
		} else break; //Illegal value - stop playing the game
		//dbg_sprintf(dbgout,"State %i, cur timer %i, stay timer %i, index %i\n",player1.state,player1.cur_delay,player1.stay_delay,player1.triad_idx);
		redrawboard(options);
		gfx_SwapDraw();
	}
}

//Draw if any of the flags were set, but clear only mask_buf flag from dgrid
void drawgrid(entity_t *e,uint8_t mask_buf) {
	uint8_t tilestate,tileid,grididx,gridx,gridy,y;
	unsigned int x;
	
	main_timer++;
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
					if (!(tilestate&TILE_FLASHING) || main_timer&2) {
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
	i = e->triad_idx+GRID_BELOW;
//	dbg_sprintf(dbgout,"Cond1 %i, Cond2 %i\n",i<GRID_SIZE,e->grid[i]==GRID_EMPTY);
	if (i<GRID_SIZE && e->grid[i]==GRID_EMPTY) {
		e->triad_idx += GRID_W;
	}
	//dbg_sprintf(dbgout,"Falldown new idx %i\n",e->triad_idx);
	for (i=GRID_SIZE-1;i>GRID_W-1;--i) {
		if (e->grid[i] == GRID_EMPTY && e->grid[i-GRID_W] != GRID_EMPTY) {
			e->grid[i] = e->grid[i-GRID_W];
			e->grid[i-GRID_W] = GRID_EMPTY;
			e->cgrid[i] |= CHANGE_BUF1|CHANGE_BUF2;
			e->cgrid[i-GRID_W] |= CHANGE_BUF1|CHANGE_BUF2;
		}
	}
}

void movedir(entity_t *e, enum Direction dir) {
	int8_t i,idx,oldidx,temp;
	//Move idx to left or right if we aren't on a field boundary
	oldidx = idx = e->triad_idx;
	if ((dir<0 && idx%GRID_W) || (dir>0 && ((idx+1)%GRID_W))) {
		idx += dir;
		//dbg_sprintf(dbgout,"Current state %i\n",idx);
		//dbg_sprintf(dbgout,"Dir change to %i\n",dir);
	} else return;
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


//Using the TILE_FLASHING flag to mark and detect tiles that have matched.
uint8_t gridmatch(entity_t *e) {
	uint8_t i,t,matches,dist;
	uint8_t curidx,curx,cury;
	uint8_t tempidx,tempx,tempy;
	uint8_t limx,limy,limidx;
	
	//Ensure grid is free of anything flashing. This shouldn't happen, though.
	for(i=0;i<GRID_SIZE;++i) e->cgrid[i] &= ~TILE_FLASHING;
	
	//Outer loop - vertical traversal
	curidx = 0;
	for (cury=0,limy=GRID_H; limy; --limy,++cury) {
		//Inner(ish) loop - horizontal sweep
		for(curx=0,limx=GRID_W; limx; --limx,++curx,++curidx) {
			//If tile is empty
			t = e->grid[curidx];
			if (t==GRID_EMPTY || t<GRID_GEM1 || t>GRID_GEM6) continue;
			//dbg_sprintf(dbgout,"Mstate cx:%i cy:%i lx:%i ly:%i idx:%i\n",curx,cury,limx,limy,curidx);
			//Checking horizontal matches (towards right)
			for (limidx=limx,tempidx=curidx,dist=0;
			limidx; --limidx,++tempidx,++dist) {
				if (e->grid[tempidx] != t) break;
			}
			if (dist>2) {
				//if (!limidx) --tempidx;
				for (;dist;--dist) {
					--tempidx;
					//dbg_sprintf(dbgout,"Match at idx %i, dist %i\n",tempidx,dist);
					e->cgrid[tempidx] |= TILE_FLASHING;
				}
			}
			//Checking diagonal matches (towards bottom-right)
			for (limidx=((limy>limx)?limx:limy),tempidx=curidx,dist=0;
			limidx; --limidx,tempidx+=(GRID_W+1),++dist) {
				if (e->grid[tempidx] != t) break;
			}
			if (dist>2) {
				//if (!limidx) --tempidx;
				for (;dist;--dist) {
					tempidx-=(GRID_W+1);
					//dbg_sprintf(dbgout,"Match at idx %i, dist %i\n",tempidx,dist);
					e->cgrid[tempidx] |= TILE_FLASHING;
				}
			}
			//Checking vertical matches (towards bottom)
			for (limidx=limy,tempidx=curidx,dist=0;
			limidx; --limidx,tempidx+=GRID_W,++dist) {
				if (e->grid[tempidx] != t) break;
			}
			if (dist>2) {
				//if (!limidx) --tempidx;
				for (;dist;--dist) {
					tempidx-=GRID_W;
					//dbg_sprintf(dbgout,"Match at idx %i, dist %i\n",tempidx,dist);
					e->cgrid[tempidx] |= TILE_FLASHING;
				}
			}
			//Checking diagonal matches (towards bottom-left)
			for (limidx=((cury>curx+1)?curx+1:cury),tempidx=curidx,dist=0;
			limidx; --limidx,tempidx+=(GRID_W-1),++dist) {
				if (e->grid[tempidx] != t) break;
			}
			if (dist>2) {
				//if (!limidx) --tempidx;
				for (;dist;--dist) {
					tempidx-=(GRID_W-1);
					//dbg_sprintf(dbgout,"Match at idx %i, dist %i\n",tempidx,dist);
					e->cgrid[tempidx] |= TILE_FLASHING;
				}
			}
		}
	}
	
	//Check how many tiles have been set to flashing and return that number.
	for(matches=i=0; i<GRID_SIZE; ++i) {
		if (e->cgrid[i]&TILE_FLASHING) ++matches;
	}
	dbg_sprintf(dbgout,"Matches found: %i\n",matches);
	return matches;
}




