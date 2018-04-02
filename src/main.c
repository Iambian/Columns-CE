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


#define GRIDSTART_X 5
#define GRIDSTART_Y 5
// As specified (rather obliquely) in convpng.ini
#define FONT_WHITE 2
#define FONT_GOLD 3
#define FONT_CYAN 4
//original NOV 50, arcade 35,

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

#define UPDATE_SCORE ((1<<0)|(1<<4))
#define UPDATE_LEVEL ((1<<1)|(1<<5))
#define UPDATE_JEWELS ((1<<2)|(1<<6))
#define UPDATE_NEXT ((1<<3)|(1<<7))

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

typedef struct numsprite_t {
	gfx_rletsprite_t *sprite;
	int ypos;
	int xpos;
} numsprite_t;

typedef struct entity_t {
	enum Players playerid; //
	unsigned int grid_top; //pixel position. Best use 16 here.
	uint8_t grid_left;     //px pos. 112 in single, 16/208 in 2player
	uint8_t triad_idx;     //X+Y*GRID_WIDTH, where X,Y is topmost block in triad
	unsigned int level;    //Player's level
	uint8_t score[8];      //player's current score (in digits) LSB-first
	uint8_t scoreadd[5];   //5 digits to add to score[3:8]
	unsigned int jewels;   //Number of jewels total player has blown up (digits)
	uint8_t combo;         //Player's current combo
	uint8_t matches;       //9 match cycles (matchlen not considered) is level++
	enum GameState state;  //GM_PLAYMATCH or GM_PLAYMOVE
	uint8_t cur_delay;     //Unified delay cycles
	uint8_t drop_max;      //Maximum speed to drop at. Decided by level
	uint8_t max_types;     //3,4,5
	uint8_t stay_delay;    //from MATCH_TIMEOUT to 0 once triad rests on surface
	
	uint8_t next_triad[3]; //next 3 blocks, top to bottom.
	uint8_t grid[GRID_SIZE];  //Gem/explosion IDs
	uint8_t cgrid[GRID_SIZE]; //Board state flags (changing, flashing, etc)
	numsprite_t nums[5];      //Up to five digits
	int scoreybase;           //Y position of base scorebox
	uint8_t scorefallthrough; //Countdown frm 16 for drop. Higher vals ignored
	uint8_t updating;         //Flags based on UPDATE_XXXX defines
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

uint16_t num0[] = {12300,29725,30365,30621}; //Combo x1 (initial)
uint16_t num1[] = {272,797,30621,30621}; //Combo x2
uint16_t num2[] = {384,776,30621,30621}; //Combo x3
uint16_t num3[] = {25096,30464,30621,30621}; //Combo x4
uint16_t num4[] = {20608,29952,30476,30621}; //Combo x5
uint16_t num5[] = {12288,29828,30224,30621}; //Combo x6

uint16_t *numpal[] = {
	num0,num1,num2,num3,num4,num5
};

char *mainmenu[] = {" select "," arcade"," menu"," options"," quit"};
uint8_t mainmenustate[] = {GM_ARCADEOPTIONS,GM_GAMEMENU,GM_OPTIONS,255};

#define BG_CENT 0
#define BG_NEXT 1
//The arrow points to the right
#define BG_SCORE 2
#define BG_SCOREF 3
#define BG_B8D 4
#define BG_B8DF 5
#define BG_B5D 6
#define BG_B5DF 7
#define BG_B4D 8
#define BG_B4DF 9
#define BG_B3D 10
#define BG_B3DF 11
#define BG_T8D 12
#define BG_T5D 13
#define BG_T4D 14
#define BG_T3D 15

#define SOBJ_NEXT 0
#define SOBJ_SCOREMAIN 1
#define SOBJ_LEVELMAIN 2
#define SOBJ_JEWELSMAIN 3
#define SOBJ_CURSCORE 4
#define SOBJ_SCORESUB 5
#define SOBJ_LEVELSUB 6
#define SOBJ_JEWELSSUB 7

#define PMODE_NORMAL 0
#define PMODE_FLASH 1
#define PMODE_SPANE 0
#define PMODE_DPANE 1 

#define TYPE_USE2PANE
//[normal/flash][single/dual panes][playerid][objidx][x,y,spriteType]
uint8_t posarr[2][2][2][8][3]= {
	{	//Normal (not-flash) mode. Includes arcade mode.
		{	//Single pane mode (1p,doubles)
			{	//Player 1
				{	86,14,BG_NEXT     //Next
				},{	38,126,BG_CENT    //Scoremain
				},{	38,166,BG_CENT    //Levelmain
				},{	38,198,BG_CENT    //Jewelmain
				},{	62,94,BG_SCORE    //Curscore
				},{	38,141,BG_B8D     //Scoresub
				},{	77,181,BG_B3D     //Levelsub
				},{	69,213,BG_B4D     //Jewlesub
				}
			},{	//Player 2
				{	214,14,BG_NEXT     //Next
				},{	230,126,BG_CENT    //Scoremain
				},{	230,166,BG_CENT    //Levelmain
				},{	230,198,BG_CENT    //Jewelmain
				},{	211,94,BG_SCOREF   //Curscore
				},{	213,141,BG_B8DF    //Scoresub
				},{	212,181,BG_B3DF    //Levelsub
				},{	212,213,BG_B4DF    //Jewlesub
				}
			}
		},{	//Dual pane mode
			{	//Player 1
				{	118,14,BG_NEXT     //Next
				},{	134,126,BG_CENT    //Scoremain
				},{	134,166,BG_CENT    //Levelmain
				},{	134,198,BG_CENT    //Jewelmain
				},{	115,70,BG_SCOREF   //Curscore
				},{	117,117,BG_T8D     //Scoresub
				},{	115,157,BG_T3D     //Levelsub
				},{	115,189,BG_T4D     //Jewlesub
				}
			},{	//Player 2
				{	182,14,BG_NEXT     //Next
				},{	134,126,BG_CENT    //Scoremain
				},{	134,166,BG_CENT    //Levelmain
				},{	134,198,BG_CENT    //Jewelmain
				},{	158,94,BG_SCORE    //Curscore
				},{	134,141,BG_B8D     //Scoresub
				},{	173,181,BG_B3D     //Levelsub
				},{	165,213,BG_B4D     //Jewlesub
				}
			}
		}
	},{	//Flash columns mode
		{	//Single pane mode (1p,doubles)
			{	//Player 1
				{	86,14,BG_NEXT     //Next
				},{	38,126,BG_CENT    //Bestmain
				},{	38,166,BG_CENT    //Levelmain
				},{	38,198,BG_CENT    //Classmain
				},{	62,94,BG_SCORE    //Curtime
				},{	61,141,BG_B5D     //Bestsub
				},{	77,181,BG_B3D     //Levelsub
				},{	77,213,BG_B3D     //Classsub
				}
			},{	//Player 2
				{	214,14,BG_NEXT     //Next
				},{	229,126,BG_CENT    //Bestmain
				},{	230,166,BG_CENT    //Levelmain
				},{	230,198,BG_CENT    //Classmain
				},{	211,94,BG_SCOREF   //Curtime
				},{	212,141,BG_B5DF    //Bestsub
				},{	212,181,BG_B3DF    //Levelsub
				},{	212,213,BG_B3DF    //Classsub
				}
			}
		},{	//Dual pane mode
			{	//Player 1
				{	118,14,BG_NEXT     //Next
				},{	134,126,BG_CENT    //Bestmain
				},{	134,166,BG_CENT    //Levelmain
				},{	134,198,BG_CENT    //Classmain
				},{	115,70,BG_SCOREF   //Curtime
				},{	115,117,BG_T5D     //Bestsub
				},{	115,157,BG_T3D     //Levelsub
				},{	115,189,BG_T3D     //Classsub
				}
			},{	//Player 2
				{	182,14,BG_NEXT     //Next
				},{	134,126,BG_CENT    //Bestmain
				},{	134,166,BG_CENT    //Levelmain
				},{	134,198,BG_CENT    //Classmain
				},{	158,94,BG_SCORE    //Curtime
				},{	157,141,BG_B5D     //Bestsub
				},{	173,181,BG_B3D     //Levelsub
				},{	173,213,BG_B3D     //Classsub
				}
			}
		}
	}
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

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
#include "font.h"            //Fontset to use
#include "gfx/tiles_gfx.h"   //gems_tiles_compressed, explosion_tiles_compressed
#include "gfx/sprites_gfx.h" //cursor_compressed, grid_compressed
#include "gfx/bg_gfx.h"      //Background stuffs
#include "gfx/score_gfx.h"   //Scores
#include "gfx/title_gfx.h"   //Title graphics. Large enough to have own gravity

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

gfx_rletsprite_t *titlebanner;

gfx_rletsprite_t *bg_central;
gfx_rletsprite_t *bg_next;
gfx_rletsprite_t *bg_score;
gfx_rletsprite_t *bg_scoref;
gfx_rletsprite_t *bg_btm8d;
gfx_rletsprite_t *bg_btm8df;
gfx_rletsprite_t *bg_btm5d;
gfx_rletsprite_t *bg_btm5df;
gfx_rletsprite_t *bg_btm4d;
gfx_rletsprite_t *bg_btm4df;
gfx_rletsprite_t *bg_btm3d;
gfx_rletsprite_t *bg_btm3df;
gfx_rletsprite_t *bg_top8d;
gfx_rletsprite_t *bg_top5d;
gfx_rletsprite_t *bg_top4d;
gfx_rletsprite_t *bg_top3d;
gfx_rletsprite_t *bgspr[16];
gfx_rletsprite_t *scorenum1[score_tiles_num]; //0-9 and colon.
gfx_rletsprite_t *scorenum2[score_tiles_num]; //0-9 and colon.

gfx_sprite_t *menutile;
gfx_sprite_t *menuborder;
gfx_rletsprite_t *p1sprite;
gfx_rletsprite_t *p2sprite;


uint8_t fallspeed[] = {30,25,20,15,15,10,7,5,4,3,2,1,1,1,2,1,1,1,1,1};
uint8_t numbuf[6];

void* setnewtitle() {
	if (randInt(0,1)) return title1_compressed;
	else return title2_compressed;
}
int randI(int imin,int imax) {  //Because apparently randInt is actually a macro.
	return randInt(imin,imax);
}

void main(void) {
	uint8_t i,y,rebuilding,gamestate;
	kb_key_t kd,kc;
	options_t options;
	void *titleptr;
	uint8_t curopt,maxopt;
	uint8_t debounce;
	
	//Randomize the RNG
	asm("	LD A,R");  //Grab semirandom value from register R
	asm("	LD B,A");  //And seed _RandInt by running it R times.
	asm("__loop13487:");
	asm("	PUSH BC");
	asm("		LD BC,255");
	asm("		PUSH BC");
	asm("		LD BC,0");
	asm("		PUSH BC");
	asm("		CALL _randI");
	asm("		POP BC");
	asm("		POP BC");
	asm("	POP BC");
	asm("	DJNZ __loop13487");

	initgfx();
	gfx_FillScreen(0x01);
	
	/* Initialize game variables */
	titleptr = setnewtitle();
	rebuilding = 2;
	gamestate = 0;
	debounce = 0;
	curopt = 0;
	maxopt = 0;
	
	options.type = TYPE_ORIGINAL;
	options.players = PLAYER1;
	options.p1_class = NOVICE;
	options.p2_class = EASY;
	options.time_trial = false;
	options.p1_level = 0;
	options.p2_level = 0;
	options.bgm = 0;
	
	initgamestate(&options);
	
	while (1) {
		kb_Scan();
		kc = kb_Data[1];
		kd = kb_Data[7];
		if (kc|kd) {
			if (debounce) kc = kd = 0;
			debounce = 1;
		} else debounce = 0;
		
		if (gamestate == GM_TITLE) {
			if (rebuilding) {
				rebuilding--;
				dzx7_Turbo(titleptr,gfx_vbuffer);
				gfx_RLETSprite(titlebanner,48,16);
				gfx_SwapDraw();
			} else {
				if (kc&kb_Mode) break;
				if (kc) gamestate = GM_MAINMENU;
				curopt = 0;
				//handle keys here including moving on to next.
			}
		} else if (gamestate == GM_MAINMENU) { //Menu on the title screen
			if (kc&kb_Mode) break;
			gfx_SetTextFGColor(2);
			gfx_SetColor(2);
			gfx_FillRectangle_NoClip(112,137,96,69); //8px taller, downward
			gfx_SetTextBGColor(1);
			gfx_SetColor(1);
			gfx_FillRectangle_NoClip(115,141,90,62); //8px taller, downward
			for (i=0,y=136; i<5; ++i,y+=14) {
				gfx_PrintStringXY(mainmenu[i],128,y);
			}
			gfx_SetTextXY(120,148+(14*curopt));
			gfx_PrintChar(']');
			if (kd|kc) {
				if (kd&kb_Up) curopt--;
				if (kd&kb_Down) curopt++;
				curopt &= 3;
				if (kc&kb_2nd) gamestate = mainmenustate[curopt];
			}
			gfx_SwapDraw();
		} else if (gamestate == GM_GAMEMENU) { //Select origina/flash and players
			break;
			
		} else if (gamestate == GM_GAMEOPTIONS) { //Class/height/match,bgm, etc
			break;
			
			
		} else if (gamestate == GM_GAMEPREVIEW) {  //Preview of selected options b4 starting
			break;
			
		} else if (gamestate == GM_ARCADEOPTIONS) {
			//Later replace with actual thingies.
			rungame(&options);
			debounce = 1;
			gamestate = GM_TITLE;
			rebuilding = 2;
			
		} else if (gamestate == GM_OPTIONS) {
			//Options menu for arcade mode / other settings
			break;
		} else break;
	}
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
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */

void initgfx(void) {
	uint8_t i,j;
	void *baseimg,*flipimg;
	uint8_t *ptr;
	int loop;
	uint8_t *fontspacing;
	
	gfx_Begin();
	gfx_SetDrawBuffer();
	ti_CloseAll();
	
	baseimg = (void*) gfx_vbuffer;
	flipimg = (void*) (*gfx_vbuffer+32768+2);

	//Title banner and graphics
	dzx7_Turbo(banner_compressed,baseimg);
	titlebanner = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)baseimg);
	//Menu graphics
	dzx7_Turbo(menutile_compressed,menutile = malloc(menutile_size));
	dzx7_Turbo(menuborder_compressed,menuborder = malloc(menuborder_size));
	p1sprite = decompAndAllocate(p1_compressed);
	p2sprite = decompAndAllocate(p2_compressed);
	//Font data abbreviated
	gfx_SetFontData(font-(32*8));
	fontspacing = malloc(128);
	for (i=0;i<128;i++) fontspacing[i]=8;
	gfx_SetFontSpacing(fontspacing);
	gfx_SetPalette(tiles_gfx_pal,sizeof tiles_gfx_pal,0);
	//Palette area at PALSWAP_AREA is initialized on game mode select
	
	for (i=0;i<gems_tiles_num;i++) {
		gems_spr[i] = decompAndAllocate(gems_tiles_compressed[i]);
	}
	for (i=0;i<explosion_tiles_num;i++) {
		explosion_spr[i] = decompAndAllocate(explosion_tiles_compressed[i]);
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
	bgspr[0]  = bg_central = decompAndAllocate(bg_central_compressed);
	bgspr[1]  = bg_next    = decompAndAllocate(bg_next_compressed);
	bgspr[2]  = bg_score   = decompAndAllocate(bg_score_compressed);
	bgspr[3]  = bg_scoref  = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)flipimg);
	bgspr[4]  = bg_btm8d   = decompAndAllocate(bg_btm8d_compressed);
	bgspr[5]  = bg_btm8df  = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)flipimg);
	bgspr[6]  = bg_btm5d   = decompAndAllocate(bg_btm5d_compressed);
	bgspr[7]  = bg_btm5df  = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)flipimg);
	bgspr[8]  = bg_btm4d   = decompAndAllocate(bg_btm4d_compressed);
	bgspr[9]  = bg_btm4df  = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)flipimg);
	bgspr[10] = bg_btm3d   = decompAndAllocate(bg_btm3d_compressed);
	bgspr[11] = bg_btm3df  = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)flipimg);
	bgspr[12] = bg_top8d   = decompAndAllocate(bg_top8d_compressed);
	bgspr[13] = bg_top5d   = decompAndAllocate(bg_top5d_compressed);
	bgspr[14] = bg_top4d   = decompAndAllocate(bg_top4d_compressed);
	bgspr[15] = bg_top3d   = decompAndAllocate(bg_top3d_compressed);
	
	for (i=0;i<score_tiles_num;i++) {
		//Player 1 digits
		dzx7_Turbo(score_tiles_compressed[i],baseimg);
		for(j=0,ptr=((uint8_t*)baseimg)+2;j<(8*16);j++,ptr++) {
			if (ptr[0] == 12) ptr[0] = 0;  //12 is transparent color in tilepal
			else ptr[0] += PALSWAP_AREA+8; //Else shift everything up to numpal
		}
		scorenum1[i] = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)baseimg);
		//Player2 digits
		dzx7_Turbo(score_tiles_compressed[i],baseimg);
		for(j=0,ptr=((uint8_t*)baseimg)+2;j<(8*16);j++,ptr++) {
			if (ptr[0] == 12) ptr[0] = 0;   //12 is transparent color in tilepal
			else ptr[0] += PALSWAP_AREA+12; //Else shift everything up to numpal
		}
		scorenum2[i] = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)baseimg);
	}
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


void drawbgspriteobj(uint8_t isflash, uint8_t idx, uint8_t *ptr) {
	uint8_t x,y,id;
	char *s;
	x = ptr[0];
	y = ptr[1];
	id= ptr[2];
	gfx_RLETSprite(bgspr[id],x,y);
	s = NULL;
	if (id==BG_CENT){
		if (idx == SOBJ_SCOREMAIN) {
			if (isflash) s = "best";
			else         s = "score";
		} else if (idx == SOBJ_LEVELMAIN) {
			s = "level";
		} else if (idx == SOBJ_JEWELSMAIN) {
			if (isflash) s = "class";
			else         s = "jewels";
		}
		if (s!=NULL) {
			gfx_PrintStringXY(s,x+((52-gfx_GetStringWidth(s))>>1),y+5);
		}
	}
}


//Magic numbers are for 32x32 tiles on a 320x240 display
//Some conversions were required since the original was a 320x224 display.
void drawgamebg(options_t *options) {
	int x,y;
	uint8_t x1,x2;
	uint8_t ix,iy;
	uint8_t mode,panes;
	gfx_sprite_t *ptr;
	uint8_t i;
	
	for(iy=0,y=-8; iy<8; ++iy,y+=32) {
		for(ix=0,x=0; ix<10; ++ix,x+=32) {
			if ((ix^iy)&1) ptr = greentile;
			else ptr = cyantile;
			gfx_Sprite(ptr,x,y);
		}
	}
	
	gfx_SetTextFGColor(FONT_GOLD);
	
	mode = options->type == TYPE_FLASH; //1 for flash, 0 for anything else.
	panes = ((options->players == PLAYER2) || (options->type == TYPE_ARCADE));
	for (i=0;i<8;i++) {
		drawbgspriteobj(mode, i, &(posarr[mode][panes][0][i][0]));
		if (options->players == DOUBLES || panes) 
			drawbgspriteobj(mode, i, &(posarr[mode][panes][1][i][0]));
	}
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
	int tempscore,x,y,templevel;
	
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
	player1.updating = UPDATE_SCORE|UPDATE_LEVEL|UPDATE_JEWELS;
	if (options->type == TYPE_ARCADE || options->players == PLAYER2)
		player2.updating = UPDATE_SCORE|UPDATE_LEVEL|UPDATE_JEWELS;;
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
					//Add to score now.
					for (i=t=0;i<8;i++) {
						if (i<5) {
							t = player1.score[i] + player1.scoreadd[i] + t;
							player1.scoreadd[i] = 0;
						}
						player1.score[i] = t%10;
						t = t/10;
					}
					player1.updating |= (UPDATE_SCORE|UPDATE_JEWELS);
					if (options->type != TYPE_FLASH) {
						templevel = (options->type == TYPE_ARCADE) ? 35 : 50;
						templevel = player1.jewels / templevel;
						if (templevel != player1.level) {
							player1.level = templevel;
							player1.updating |= UPDATE_LEVEL;
						}
					}
					for (i=0; i<GRID_SIZE; ++i) {
						if (player1.cgrid[i] & TILE_FLASHING) {
							player1.cgrid[i] = CHANGE_BUF1|CHANGE_BUF2;
							player1.grid[i] = GRID_EXP1;
						}
					}
				}
			}

			//If match timed out, check board for matches proceed to place new triad.
			if (!--(player1.cur_delay)) {
				//Check if anything is past the top before continuing
				for (i=t=0; i<GRID_START; ++i) {
					if (player1.grid[i] != GRID_EMPTY) t = 1;
				}
				if (t) {
					player1.state = GM_GAMEOVER;
					player1.triad_idx = GRID_SIZE-1; //Re-used for indexing
					continue;
				}
				
				matches_found = gridmatch(&player1);				
				if (matches_found) {
					flash_active = 1;
					flash_countdown = 28;
					player1.jewels += matches_found;
					if (player1.jewels > 9999) player1.jewels = 9999; //limit
					//Calculate score -- Not perfect but serviceable.
					player1.combo++;
					i = (matches_found+2)/3;
					i = (i>3)?3:i;  //Lim 3
					tempscore = ((int)i) * (player1.level+1) * (player1.combo) * 30;
					//Extract digits.
					for (i=0,ptr=numbuf; i<5; i++,ptr++) {
						player1.scoreadd[i] = ptr[0] = tempscore%10;
						tempscore /= 10;
					}
					//Load digits to buffer.
					y = 111;  //constant in all cases except 1P:2P
					if (player1.playerid == PLAYER1) {
						if (options->players == PLAYER2) {
							y = 87;
							x = 120;  //player 1 in 2P split screen mode
						} else {
							x = 64;   //Player 1 in 1P or doubles single column
						}
					} else {
						if (options->players == PLAYER2) {
							x = 160;
						} else {
							x = 216;
						}
					}
					//Process digits into sprites then buffer them.
					player1.scorefallthrough = 40;
					t = 0;
					for (i=0,ptr=numbuf+4;i<5;i++,ptr--) {
						y -= 16;
						//dbg_sprintf(dbgout,"Digit pos (%i,%i)\n",x,y);
						if (!(t || ptr[0])) {
							player1.nums[i].sprite = NULL;
						} else {
							if (player1.playerid == PLAYER1) {
								player1.nums[i].sprite = scorenum1[ptr[0]];
							} else {
								player1.nums[i].sprite = scorenum2[ptr[0]];
							}
							t++;
							//dbg_sprintf(dbgout,"Digit %i: %i\n",i,ptr[0]);
						}
						player1.nums[i].ypos = y;
						player1.nums[i].xpos = x;
						x += 8;
					}
					//Pick correct palette for said sprite
					if (player1.combo>6) palptr = num5;
					else palptr = numpal[player1.combo-1];
					gfx_SetPalette(palptr,8,PALSWAP_AREA + 16);
				} else {
					player1.combo = 0;
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
			//keep updating NEXT until a triad is finally generated.
			if (!player1.next_triad[0]) player1.updating |= UPDATE_NEXT;
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
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
void printuint(int num, int x,int y,uint8_t digits) {
	uint8_t i,t;
	uint8_t buf[8];
	
	for (i=0;i<digits;i++) {
		buf[i] = num%10;
		num = num/10;
	}
	gfx_SetTextXY(x,y);
	t=0;
	for (i=digits;i;i--) {
		if (i==1) t++;
		if (!(t||buf[i-1])) gfx_PrintChar(' ');
		else                gfx_PrintChar(buf[i-1]+'0'),++t;
		//dbg_sprintf(dbgout,"* Emit digit %i: %i\n",i,buf[i-1]);
	}
}

void drawscore(entity_t *e, options_t *opt) {
	uint8_t i,j,*ptr,t;
	int x,y;
	int x1,y1,x2,y2;
	numsprite_t *num;
	uint8_t isflash,isdual,imgid;
	char *s;
	
	
	//ONLY IN NON-FLASH MODE.
	//TODO: FLASH MODE TIMER, COMBO COLORS FOR SCORE MODE
	if (e->scorefallthrough) {
		e->scorefallthrough--;
		y = 111;
		if (e->playerid == PLAYER1 && opt->players == PLAYER2) y = 87;

		x1 = (e->nums[0]).xpos;
		x2 = ((e->nums[4]).xpos)+8;
		y1 = y - 16;
		y2 = y;
		gfx_SetColor(1);  //black
		gfx_FillRectangle(x1,y1,40,16);
		gfx_SetClipRegion(x1,y1,x2,y2);
		for (i=0;i<5;i++) {
			num = &(e->nums[i]);
			//dbg_sprintf(dbgout,"Digit %i pos (%i,%i)\n",i,num->xpos,num->ypos);
			if (num->sprite != NULL) {
				if ((num->ypos < y1) || (e->scorefallthrough < 5)) {
					num->ypos += 4;
				}
			 gfx_RLETSprite(num->sprite,num->xpos,num->ypos);
			}
		}
		gfx_SetClipRegion(0,0,320,240);
	}
	if (e->updating) {
		gfx_SetTextFGColor(FONT_GOLD);
		gfx_SetTextBGColor(1);
		isflash = opt->type == TYPE_FLASH;
		isdual = ((opt->players == PLAYER2) || (opt->type == TYPE_ARCADE));
		
		if (e->updating&UPDATE_NEXT) {
			ptr = &(posarr[isflash][isdual][e->playerid][0][0]);
			x = ptr[0]+2;
			y = ptr[1]+2;
			for (i=0,j=e->triad_idx;i<3;i++,j+=GRID_W) {
				if (e->next_triad[0]) {
					t = e->next_triad[i]-GRID_GEM1;
				} else {
					t = e->grid[j]-GRID_GEM1;
				}
				gfx_RLETSprite_NoClip(gems_spr[t],x,y);
				y+=16;
			}
		}
		for (i=SOBJ_SCORESUB;i<(SOBJ_JEWELSSUB+1);i++) {
			ptr = &(posarr[isflash][isdual][e->playerid][i][0]);
			x = ptr[0]+2;
			y = ptr[1]+2;
			imgid = ptr[2];
			switch (imgid) {
				case BG_B8DF:
				case BG_B5DF:
				case BG_B4DF:
				case BG_B3DF:
				case BG_T8D:
				case BG_T5D:
				case BG_T4D:
				case BG_T3D: x +=2; break;
				default: break;
			}
			t = 0;
			if (i == SOBJ_SCORESUB && (e->updating&UPDATE_SCORE)) {
				if (!isflash) {
					gfx_SetTextXY(x,y);
					for (j=8;j;j--) {
						if (j==1) ++t;
						if (!(t||(e->score[j-1]))) {
							gfx_PrintChar(' ');
						}
						else {
							gfx_PrintChar((e->score[j-1])+0x030);
							++t;
						}
					}
				} else {
					//Print time remain
				}
			} else if (i == SOBJ_LEVELSUB && (e->updating&UPDATE_LEVEL)) {
				//Pring levels
				printuint(e->level,x,y,3);
			} else if (i == SOBJ_JEWELSSUB && (e->updating&UPDATE_JEWELS)) {
				//Print jewels/class
				dbg_sprintf(dbgout,"Updating jewels: %i\n",e->jewels);
				if (isflash) {
					switch (e->max_types) {
						case NOVICE: s = "NOV"; break;
						case AMATEUR:s = "AMA"; break;
						case PRO:    s = "PRO"; break;
						default:     s = "!!!"; break;
					}
					gfx_PrintStringXY(s,x,y);
				} else {
					printuint(e->jewels,x,y,4);
				}
			}
		}
		e->updating >>= 4;
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
	drawscore(&player1,options);
	if (options->players != PLAYER1) { 
		drawgrid(&player2,mask_buf);
		drawscore(&player2,options);
	}
	
	
		
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




