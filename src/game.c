#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <keypadc.h>
#include <graphx.h>
#include <decompress.h>
#include <fileioc.h>

#include "defs.h"
#include "types.h"
#include "main.h"
#include "menu.h"
#include "gfx.h"
#include "grid.h"


/* ----------------------- Define your constants here ------------------------*/
uint8_t curbuf;
entity_t player1;
entity_t player2;
uint8_t numbuf[6];

#define MAX_SPEED 64
uint8_t fallspeed[] = {
  //000,001,002,003,004,005,006,007,008,009
	2  ,4  ,6  ,12 ,20 ,28 ,12 ,20 ,28 ,32 ,
  //010,011,012,013,014,015,016,017,018,019
	48 ,28 ,32 ,48 ,64 ,2  ,2  ,2  ,2  ,2  , 
  //020,021,022,023,024,025,026,027,028,029
	2  ,2  ,2  ,2  ,2  ,2  ,2  ,2  ,2  ,2  ,
	
	
};

uint16_t bgp1[] = {528,396,264,4228,404,272,140,8};                  //1PO: cyan/blue
uint16_t bgp2[] = {20876,16648,12420,8192,21008,16780,12552,8324};   //2PO: pink/lpink
uint16_t bgp3[] = {25088,20864,16640,12416,21012,16784,12556,8328};  //DBO: gold/lav
uint16_t bgp4[] = {528,396,264,132,516,388,260,128};                 //1PF: cyan/grn
uint16_t bgp5[] = {16652,12424,8196,4096,16784,12556,8328,4100};     //2PF: purp/lpur
uint16_t bgp6[] = {29312,25088,20864,16512,29056,24832,20608,16512}; //DBF: yell/orng
uint16_t bgp7[] = {20872,16644,12416,8192,21004,16776,12548,8320};   //Aracde mode

//blockpal[options->type == TYPE_FLASH][options->players]
uint16_t *blockpal[2][3] = {
	{bgp1,bgp2,bgp3},
	{bgp4,bgp5,bgp6}
};

uint16_t num0[] = {12300,29725,30365,30621}; //Combo x1 (initial)
uint16_t num1[] = {272,797,30621,30621}; //Combo x2
uint16_t num2[] = {384,776,30621,30621}; //Combo x3
uint16_t num3[] = {25096,30464,30621,30621}; //Combo x4
uint16_t num4[] = {20608,29952,30476,30621}; //Combo x5
uint16_t num5[] = {12288,29828,30224,30621}; //Combo x6

#define NEN_PREV 0x1D
#define NEN_END 0x1E
#define NEN_NEXT 0x1F
char nentrydisp[] = {
	//0 1   2   3   4    5   6   7  8   9   A   b   c   d   e    f
	' ','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
	'P','Q','R','S','T','U','V','W','X','Y','Z','?','.','<','=','>'
};
char nentrymap[] = {
	' ','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
	'p','q','r','s','t','u','v','w','x','y','z','?','.','!','!','!'
};


uint16_t *numpal[] = {
	num0,num1,num2,num3,num4,num5
};

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


/* ----------------------- Function prototypes goes here ---------------------*/

void initGameState(options_t *opt);
void runGame(options_t *options);  //Starts the game session

/* ----------------------- Function prototypes goes here ---------------------*/
















void setTimeToMagic(entity_t *e, uint8_t target) {
	e->timetomagic = randInt(target-5,target+5);
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


void drawbgspriteobj(options_t *opt, uint8_t idx, uint8_t *ptr) {
	uint8_t x,y,id;
	char *s;
	x = ptr[0];
	y = ptr[1];
	id= ptr[2];
	gfx_RLETSprite(bgspr[id],x,y);
	s = NULL;
	if (id==BG_CENT){
		if (idx == SOBJ_SCOREMAIN) {
			if (opt->type == TYPE_FLASH) {
				if (opt->time_trial) s = "match";
				else                 s = "best";
			}	else                 s = "score";
		} else if (idx == SOBJ_LEVELMAIN) {
			s = "level";
		} else if (idx == SOBJ_JEWELSMAIN) {
			if (opt->type == TYPE_FLASH) s = "class";
			else                         s = "jewels";
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
		drawbgspriteobj(options, i, &(posarr[mode][panes][0][i][0]));
		if (options->players == DOUBLES || panes) 
			drawbgspriteobj(options, i, &(posarr[mode][panes][1][i][0]));
	}
}

void drawarcademenu(entity_t *e,uint8_t showcursor) {
	uint8_t i,t,y,oldy;
	int x;
	
	x = e->grid_top;
	oldy = y = e->grid_left+48;
	
	gfx_SetColor(MENU_GOLD);
	gfx_FillRectangle_NoClip(x,y,96,96);
	gfx_SetColor(MENU_MAROON);
	gfx_FillRectangle_NoClip(x+2,y+2,(96-4),(96-4));
	if (e->menuoption==0) {
		t = 8;
		y += 24;
	} else if (e->menuoption==1) {
		t = 16;
		y += 48;
	} else {
		t = 16;
		y += 72;
	}
	gfx_SetColor(MENU_LAVENDER);
	if (showcursor)	gfx_FillRectangle_NoClip(x+2,y,(96-4),t);
	gfx_RLETSprite(arcadeselect,x+6,oldy+6);
	gfx_SetTextBGColor(BG_TRANSPARENT);
	gfx_SetTextFGColor(FONT_GOLD);
	gfx_PrintStringXY("time ",x+24,oldy+104);
	gfx_PrintUInt(e->secondsleft-2,2);
	refreshgrid(e);
}


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
	gfx_rletsprite_t *rspr;
	uint16_t *palptr;
	
	if (opt->type == TYPE_FLASH || ( opt->time_trial && (opt->type == TYPE_ORIGINAL))
		&& (e->state > GM_PLAYSTART2)) {
		//Select X
		if (player1.playerid == PLAYER1) {
			if (opt->players == PLAYER2 || opt->type == TYPE_ARCADE) {
				x = 120;  //player 1 in 2P split screen mode
			} else {
				x = 64;   //Player 1 in 1P or doubles single column
			}
		} else {
			if (opt->players == PLAYER2 || opt->type == TYPE_ARCADE) {
				x = 160;
			} else {
				x = 216;
			}
		}
		//Select Y
		y = 95;
		if (e->playerid == PLAYER1 && 
			(opt->players == PLAYER2 || opt->type == TYPE_ARCADE)) y = 71;
		//Select number color
		t = (e->playerid == PLAYER1) ? 16 : 24;
		if (opt->time_trial && e->secondsleft < (59+1)) palptr = num5;
		else                                            palptr = num1;
		gfx_SetPalette(palptr,8,PALSWAP_AREA + t);
		//Begin rendering
		gfx_FillRectangle(x,y,40,16);
		if (opt->type == TYPE_FLASH) {
			ptr = &e->score[4];
		} else {
			ptr = &e->tttimer[4];
		}
		for (t=i=0;i <5; ++i,--ptr) {
			if (i==1) ++t;
			if ((t||(*ptr)) && ((*ptr)<=0x0A)) {
				if (e->playerid == PLAYER1) rspr = scorenum1[*ptr];
				else                        rspr = scorenum2[*ptr];
				gfx_RLETSprite_NoClip(rspr,x,y);
				++t;
			}
			x += 8;
		}
	} else if (e->scorefallthrough) {
		e->scorefallthrough--;

		y = 111;
		if (e->playerid == PLAYER1 && 
			(opt->players == PLAYER2 || opt->type == TYPE_ARCADE)) y = 87;
		x1 = (e->nums[0]).xpos;
		x2 = ((e->nums[4]).xpos)+8;
		y1 = y - 16;
		y2 = y;
		gfx_SetColor(1);  //black
		gfx_FillRectangle(x1,y1+1,40,16);
		gfx_SetClipRegion(x1,y1+1,x2,y2);
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
			gfx_SetColor(BG_BLACK);
			gfx_FillRectangle_NoClip(x,y,16,16*3);
			if (e->state == GM_PLAYMATCH || e->state == GM_PLAYMOVE) { //workaround for TT disp bug if triad is above top on game end
				for (i=0,j=e->triad_idx;i<3;i++,j+=GRID_W) {
					if (e->next_triad[0]) {
						t = e->next_triad[i];
					} else {
						t = e->grid[j];
					}
					if (t>=GRID_GEM1 && t<=GRID_GEM6) rspr = gems_spr[t-GRID_GEM1];
					else if (t>=GRID_MAG1 && t<=GRID_MAG6) rspr = magicgems[t-GRID_MAG1];
					else continue;  //prevents display of nondisplayable objects
					gfx_RLETSprite_NoClip(rspr,x,y);
					y+=16;
				}
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
				gfx_SetTextXY(x,y);
				if (!isflash) {
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
					//Print current best time
					s = (char*) getScorePtr(opt);
					gfx_PrintString(&s[4]);
				}
			} else if (i == SOBJ_LEVELSUB && (e->updating&UPDATE_LEVEL)) {
				//Pring levels
				printuint(e->level+e->baselevel,x,y,3);
			} else if (i == SOBJ_JEWELSSUB && (e->updating&UPDATE_JEWELS)) {
				//Print jewels/class
				//dbg_sprintf(dbgout,"Updating jewels: %i\n",e->jewels);
				if (isflash) {
					switch (e->max_types) {
						case 3:  s = "NOV"; break;
						case 4:  s = "AMA"; break;
						case 5:  s = "PRO"; break;
						default: s = "!!!"; break;
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
	
	
	if (options->players == PLAYER2 || options->type == TYPE_ARCADE) { 
		drawgrid(&player2,mask_buf);
		drawscore(&player2,options);
	}
	
	
		
	curbuf = !curbuf;
}


//##############################################################################
//##############################################################################
//##############################################################################
//##############################################################################

void initGameState(options_t *opt) {
	uint8_t t,tt,x1,x2,dropmax;
	uint8_t i,j,i1,i2,idx;
	uint8_t retrygem,retryboard;
	
	//Clear entity memory
	memset(&player1,0,sizeof player1);
	memset(&player2,0,sizeof player2);
	//Set player IDs
	player1.playerid = PLAYER1;
	player2.playerid = PLAYER2;
	//Set grid top position
	player1.grid_top = player2.grid_top = 16;
	//Set grid left position
	if (opt->type == TYPE_ARCADE || opt->players == PLAYER2) {
		player1.grid_left = P1_GRIDLEFT;
		player2.grid_left = P2_GRIDLEFT;
	} else {
		player2.grid_left = player1.grid_left = CENTER_GRIDLEFT;
	}
	//Set game state
	player1.state = player2.state = GM_PLAYSTART;
	//Set maximum gem types
	if (opt->type == TYPE_ARCADE) {
		player1.max_types = 5;
		player2.max_types = 5;
	} else {
		player1.max_types = 3 + ((uint8_t) opt->p1_class);
		player2.max_types = 3 + ((uint8_t) opt->p2_class);
	}
	//Set up dgrid to force initial render
	memset(player1.cgrid,CHANGE_BUF1|CHANGE_BUF2,GRID_SIZE);
	memset(player2.cgrid,CHANGE_BUF1|CHANGE_BUF2,GRID_SIZE);
	
	if (opt->type == TYPE_FLASH) {
		genflashgrid(&player1);
		memcpy(player2.grid,player1.grid,GRID_SIZE);
		
		trimgrid(&player1,2+opt->p1_level);
		trimgrid(&player2,2+opt->p2_level);
		
		player1.cgrid[GRID_SIZE-3] |= TILE_TARGET_GEM;
		player2.cgrid[GRID_SIZE-3] |= TILE_TARGET_GEM;
	} else {
		//### TODO: CREATE CALL FOR SETTING DROP_MAX IF LEVEL NOT 0
		player1.baselevel = opt->p1_level;
		player2.baselevel = opt->p2_level;
	}
	//Preselect game level
	player1.drop_max = fallspeed[player1.baselevel];
	player2.drop_max = fallspeed[player2.baselevel];
	//Set initial delay cycles
	player1.cur_delay = player2.cur_delay = MATCH_TIMEOUT;
	//Set initial index for triad generation. This position is always closest
	//to the player's score.
	if (opt->type == TYPE_ARCADE || opt->players == PLAYER2) {
		x1=3;
		x2=2;
	} else {
		x1=2;
		x2=3;
	}
	player1.start_idx = x1;
	player2.start_idx = x2;
	//Initialize state for magic gem deployment
	setTimeToMagic(&player1,TIME_TO_MAGIC_DEFAULT);
	setTimeToMagic(&player2,TIME_TO_MAGIC_DEFAULT);
	//Initialize score counters for flashcolumns and time trial modes
	if (opt->type == TYPE_FLASH) {
		player1.score[2] = 0x0A;
		player2.score[2] = 0x0A;
	} else if (opt->time_trial && (opt->type == TYPE_ORIGINAL)) {
		player1.tttimer[2] = 0x0A;
		player1.tttimer[3] = 3;
		player2.tttimer[2] = 0x0A;
		player2.tttimer[3] = 3;
	}
	//Initialize triad
	gentriad(&player1);
}



uint8_t scoreCmpSub(options_t *opt, char *oldscore, uint8_t *curscore) {
	uint8_t iold,icur;
	uint8_t oldtemp;
	uint8_t curtemp;
	uint8_t newgt;
	uint8_t totaltemp;
	
	totaltemp=newgt=0;
	//old is B-E chr start at +0, cur is L-E uint8 start at +(isFlash)?4:7
	for (iold=0,icur=(opt->type==TYPE_FLASH)?4:7; icur!=255; ++iold,--icur) {
		oldtemp = oldscore[iold];
		curtemp = curscore[icur];
		if (oldtemp==':') continue; //Skip test of semicolon object.
		if (oldtemp==' ') oldtemp = 0;
		else if (oldtemp) oldtemp = oldtemp-'0';
		if (curtemp == oldtemp) continue; //Keep going if nums match
		//On an non-matching number, it's decision time. Kill loop immediately after.
		
		if (opt->type==TYPE_FLASH) {
			if (curtemp<oldtemp) ++newgt;
		} else {
			if (curtemp>oldtemp) ++newgt;
		}
		break;
	}
	return newgt;
}

//Returns rank 1-9 on arcade mode, or just 1 if you made a high score, else 0.
uint8_t scoreCmp(options_t *opt) {
	uint8_t i,t;
	
	if (opt->type == TYPE_ARCADE) {
		for (i=0;i<9;i++) {
			if (scoreCmpSub(opt,((char*)(&save.arcade[i]))+4,&player1.score)) {
				return i+1;
			}
		}
		return 0;
	} else {
		return scoreCmpSub(opt,((char*)getScorePtr(opt))+4,&player1.score);
	}
}

//For use in doubles as well
void saveName(uint8_t *dest) {
	uint8_t i,t;
	for (i=0;i<3;++i,++dest) {
		t = player1.namebuffer[i];
		if (t=='-') t=' ';
		*dest = t;
	}
	*dest = 0;  //null-terminate
}

//Source is always LSB 
void numToString(int num, char *dest, uint8_t strwidth) {
	uint8_t i,r,t;
	for (i=strwidth-1; i!=255; --i) {
		r = num%10;
		if (num) {
			dest[i] = r+'0';
		} else {
			dest[i] = ' ';
		}
		num /=10;
	}
	dest[strwidth] = 0;
}


void saveScore(options_t *opt, uint8_t *dest) {
	uint8_t i,t,lzero;
	saveName(dest);
	dest += 4;
	lzero = 0;
	memset(dest,0,10);  //Clear score field and sets null terminator
	for (i=(opt->type==TYPE_FLASH)?4:7; i!=255; --i,++dest) {
		t = player1.score[i];
		if (t || !i) lzero = 1;
		if (!t && !lzero) {
			t = ' ';
		} else {
			t = (t==0x0A)?':':t+'0';
		}
		*dest = t;
	}
}


//##############################################################################
//##############################################################################
//##############################################################################
//##############################################################################

void runGame(options_t *options) {
	uint8_t i,b,c,t,tp,idx;
	int8_t tsig,csig;
	uint8_t moveside_active,moveside_delay;
	uint8_t shuffle_active;
	uint8_t menu_active;
	uint8_t timer_p1,timer_p2;
	kb_key_t kd,kc;
	uint8_t *ptr;
	uint16_t *palptr;
	int longctr;
	uint8_t flash_active,flash_countdown;
	uint8_t score_active,score_countdown;
	int8_t matches_found;
	uint8_t palette_offset;
	int tempscore,x,y,templevel,gos_y;
	gfx_rletsprite_t *rsptr;
	uint8_t drop_timer;
	int ltemp;
	
	
	moveside_delay = MOVESIDE_TIMEOUT;
	drop_timer = menu_active = flash_countdown = flash_active = score_active = shuffle_active = moveside_active = 0;
	gos_y  = 0;
	//Generate game static background
	
	if (options->type == TYPE_ARCADE) {
		palptr = bgp7;
	} else {
		palptr = blockpal[options->type == TYPE_FLASH][options->players];
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
		//Read the keyboard
		kb_Scan();
		kc = kb_Data[1];
		kd = kb_Data[7];
		//Maintain timers
		if (--player1.subsecond == 255) {
			player1.subsecond = ONE_SECOND-1;
			if (player1.secondsleft) --player1.secondsleft;
			if (player1.secondsleft2) --player1.secondsleft2;
		}
		if (((player1.state == GM_PLAYMATCH)||(player1.state == GM_PLAYMOVE))&&
			!player1.subsecond) {
			if (options->type == TYPE_FLASH) {
				//Flash mode -- always incrementing.
				for (c=1,i=0; i<5; ++i) {
					if (i==2) continue;  //skip over colon
					b = (i==1)?5:9; //base 6 if pos 1, else base 10.
					t = player1.score[i] + c;
					if (t>b) {
						t -= (b+1);
						c  = 1;
					} else {
						c = 0;
					}
					player1.score[i] = t;
				}
			} else if (options->time_trial && (options->type == TYPE_ORIGINAL)) {
				//Time trial -- decrementing.
				for (c=1,i=0; i<5; ++i) {
					if (i==2) continue;  //skip over colon
					b = (i==1)?5:9; //base 6 if pos 1, else base 10.
					t = player1.tttimer[i] - c;
					if (t>254) {
						t += (b+1);
						c = 1;
					} else {
						c = 0;
					}
					player1.tttimer[i] = t;
				}
			}
		}
		if ( options->type == TYPE_ORIGINAL && options->time_trial &&
			(player1.state == GM_PLAYMATCH || player1.state == GM_PLAYMOVE) &&
			player1.secondsleft == 0) {
			player1.state = GM_GAMEOVER;
			player1.triad_idx = GRID_SIZE-1; //Re-used for indexing
			player1.subsecond = ONE_SECOND;
			player1.secondsleft = 2;
			gos_y = 72+208+16;
			player1.updating |= UPDATE_NEXT;
			continue;
		}
		//Global key response
		if (kc&kb_Mode && player1.state != GM_NAMEENTRY) return;
		if (kd&(kb_Left|kb_Right)) {
			//Left/right debouncing no matter the mode
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
		if (kd&(kb_Up|kb_Down) && (player1.state != GM_PLAYMOVE)) {
			//Up/down key debouncing (but only if in menu)
			if (menu_active) { 
				kd &= ~(kb_Up|kb_Down);
			} else {
				menu_active = 1;
			}
		} else menu_active = 0;
		if (kc&kb_2nd) {
			//2nd key debouncing
			if (shuffle_active) kc &= ~kb_2nd;
			else shuffle_active = 1;
		} else shuffle_active = 0;
		//Magic gem handler
		if (++player1.magicgemtimer > 2) {
			player1.magicgemtimer=0;
/*			ptr = player1.next_triad;
			if (*ptr>=GRID_MAG1 && *ptr<=GRID_MAG6) {
				for (i=0;i<3;i++,ptr++) {
					if (++ptr[0] > GRID_MAG6) ptr[0] = GRID_MAG1;
				}
				player1.updating |= UPDATE_NEXT;
			} */
			if ((t=player1.triad_idx)<78) {
				for (i=0;i<3;i++,t+=GRID_W) {
					if (player1.grid[t]>=GRID_MAG1 && player1.grid[t]<=GRID_MAG6) {
						player1.grid[t]++;
						if (player1.grid[t]>GRID_MAG6) player1.grid[t] = GRID_MAG1;
					}
				}
			}
		}
		/* ----%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
		if (player1.state == GM_PLAYSTART) {
			redrawboard(options);
			if (options->type == TYPE_ARCADE) {
				if (!player1.secondsleft) player1.secondsleft = 12;
				if (player1.secondsleft == 2 && !player1.subsecond) {
					player1.state = GM_PLAYSTART2;
					player1.subsecond = 96;
					continue;
				}
				if (kc&kb_2nd) {
					player1.state = GM_PLAYSTART2;
					player1.subsecond = 96;
					continue;
				}
				if (kd&kb_Down) {
					player1.menuoption++;
					if (player1.menuoption>2) player1.menuoption = 0;
				}
				if (kd&kb_Up) {
					if (!player1.menuoption) player1.menuoption = 3;
					player1.menuoption--;
				}
				drawarcademenu(&player1,true);
			} else {
				if (!player1.secondsleft) player1.secondsleft = 2;
				if ((!player1.subsecond) && (player1.secondsleft == 1)) {
					player1.state = GM_PLAYMATCH;
					refreshgrid(&player1);
					if (options->time_trial && (options->type != TYPE_FLASH)) {
						player1.secondsleft = 60*3+1;  //3 minutes
					}
					continue;
				}
				x = player1.grid_left;
				y = player1.grid_top;
				gfx_SetTextBGColor(BG_TRANSPARENT);
				gfx_SetTextFGColor(FONT_GOLD);
				gfx_PrintStringXY("game start",x+8,y+104);
			}
			gfx_SwapDraw();
			continue;
			/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
		} else if (player1.state == GM_PLAYSTART2) {
			redrawboard(options);
			if (options->type == TYPE_ARCADE) {
				if (!player1.subsecond) {
					t = player1.menuoption;
					initGameState(options);
					player1.state = GM_PLAYMATCH;
					if (t==0) {
						player1.baselevel = 0;
					} else if (t==1) {
						player1.score[4] = 2;
						player1.baselevel = 5;
						setTimeToMagic(&player1,6);
					} else {
						player1.score[4] = 5;
						player1.baselevel = 10;
						setTimeToMagic(&player1,6);
					}
					player1.drop_max = fallspeed[player1.baselevel];  //rechoose
					player1.updating |= UPDATE_SCORE|UPDATE_LEVEL;
					continue;
				}
				drawarcademenu(&player1,player1.subsecond&4);
			} else {
				//This cannot happen. If it does, quit the game.
				dbg_sprintf(dbgout,"Error: GM_PLAYSTART2 on non-arcade mode.\n");
				break;
			}
			gfx_SwapDraw();
			continue;
			/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
		} else if (player1.state == GM_PLAYMATCH) {
			falldown(&player1);
			
			for (i=GRID_START,t=0;i<(GRID_SIZE-GRID_START);++i) {
				t |= player1.cgrid[i] & (CHANGE_BUF1|CHANGE_BUF2|TILE_FLASHING);
			}
			if (t) player1.cur_delay = MATCH_TIMEOUT; //And reset timeout if change.
			//If flashing, wait until timeout to destroy gems fully.
			if (flash_active) {
				if (!--flash_countdown) {
					flash_active = 0;
					
					if (options->type == TYPE_ARCADE) {
						switch (options->p1_class) {
							case EASIEST:
							default:
								templevel = 50;
								break;
							case EASY:
								templevel = 40;
								break;
							case NORMAL:
								templevel = 35;
								break;
							case HARD:
								templevel = 25;
								break;
						}
					} else {
						templevel = 50;
					}
					templevel = player1.jewels / templevel;
					if (templevel != player1.level) {
						player1.level = templevel;
						player1.updating |= UPDATE_LEVEL;
						if ((player1.level+player1.baselevel) > 29) {
							player1.drop_max = 255;
						} else {
							player1.drop_max = fallspeed[player1.baselevel+player1.level];
						}
					}
					player1.updating |= (UPDATE_SCORE|UPDATE_JEWELS);
					//Add to score now. But only in non-flashcolumns mode
					if (options->type != TYPE_FLASH) {
						for (i=t=0;i<8;i++) {
							if (i<5) {
								t = player1.score[i] + player1.scoreadd[i] + t;
								player1.scoreadd[i] = 0;
							}
							player1.score[i] = t%10;
							t = t/10;
						}
					}
					for (i=0; i<GRID_SIZE; ++i) {
						if (player1.cgrid[i] & TILE_FLASHING) {
							player1.cgrid[i] |= CHANGE_BUF1|CHANGE_BUF2;
							player1.cgrid[i] &= ~TILE_FLASHING;
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
				//	player1.cgrid[GRID_SIZE-3] |= TILE_TARGET_GEM;
				for (i=tp=0; i<GRID_W; ++i) {
					tp |= player1.cgrid[(GRID_SIZE-GRID_W)+i] & TILE_TARGET_GEM;
				}
				
				if (t | ((options->type == TYPE_FLASH) && !tp)) { /* @@@@@@@@@@@@@@@@@@ GAME OVER @@@@@@@@@@@@@@@@@@@ */
					player1.state = GM_GAMEOVER;
					player1.triad_idx = GRID_SIZE-1; //Re-used for indexing
					player1.subsecond = ONE_SECOND;
					player1.secondsleft = 2;
					gos_y = 72+208+16;
					if ((options->type == TYPE_FLASH) && !tp) {
						player1.victory = 1;
					}
					player1.updating |= UPDATE_NEXT;
					continue;
				}
				
				matches_found = gridmatch(&player1);				
				if (matches_found) {
					flash_active = 1;
					flash_countdown = DESTRUCT_TIMEOUT;
					player1.combo++;
					if (matches_found<0) {
						matches_found = -matches_found;
						if (matches_found==3) {
							tempscore = 10000;  //unused magic gem.
						} else {
							i = ((matches_found+2)/3) * (player1.level+player1.baselevel+1);
							tempscore = i * player1.combo * 30;
						}
					} else {
						//Calculate score -- Not perfect but serviceable.
						i = (matches_found+2)/3;
						i = (i>3)?3:i;  //Lim 3
						tempscore = ((int)i) * (player1.level+player1.baselevel+1) * (player1.combo) * 30;
					}
					player1.jewels += matches_found;
					//Extract digits.
					for (i=0,ptr=numbuf; i<5; i++,ptr++) {
						player1.scoreadd[i] = ptr[0] = tempscore%10;
						tempscore /= 10;
					}
					if (player1.jewels > 9999) player1.jewels = 9999; //limit
					//Load digits to buffer.
					y = 111;  //constant in all cases except 1P:2P
					if (player1.playerid == PLAYER1) {
						if (options->players == PLAYER2 || options->type == TYPE_ARCADE) {
							y = 87;
							x = 120;  //player 1 in 2P split screen mode
						} else {
							x = 64;   //Player 1 in 1P or doubles single column
						}
					} else {
						if (options->players == PLAYER2 || options->type == TYPE_ARCADE) {
							x = 160;
						} else {
							x = 216;
						}
					}
					//Process digits into sprites then buffer them.
					player1.scorefallthrough = SCOREFALL_TIMEOUT;
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
					dbg_sprintf(dbgout,"\n");
					player1.cur_delay = DROP_NEXT_DELAY;
					player1.state = GM_DELAYNEXT;
					continue;
				}
			}
			/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
		} else if (player1.state == GM_DELAYNEXT) {
			if (!--player1.cur_delay) {
				player1.combo = 0;
				//Emit triad object.
				player1.triad_idx = t = player1.start_idx;
				dbg_sprintf(dbgout,"Emitting triad objects: ");
				for (i=0;i<3;++i,t+=GRID_W) {
					player1.grid[t] = player1.next_triad[i];
					dbg_sprintf(dbgout,"%i, ",player1.next_triad[i]);
					player1.cgrid[t] = CHANGE_BUF1|CHANGE_BUF2;
					player1.next_triad[i] = 0;
				}
				player1.cur_delay = player1.drop_max;
				player1.stay_delay = LONG_TIMEOUT;
				player1.state = GM_PLAYMOVE;
				continue;
			}
			/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
		} else if (player1.state == GM_PLAYMOVE) {
			//If push down, speed things up everywhere.
			if (kd&kb_Down) {
				player1.stay_delay = 1;
				player1.cur_delay = 64;
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
			
			if (player1.triad_idx >= GRID_START && !player1.next_triad[0]) {
				t = 0;
				for (i=GRID_START; i<GRID_SIZE; i++) {
					if (player1.grid[i]) t++;
				}
				if (t>50 && (!--player1.timetomagic) && options->type == TYPE_ARCADE) {
					for (i=0;i<3;i++) player1.next_triad[i] = GRID_MAG1;						setTimeToMagic(&player1,6);
					setTimeToMagic(&player1,TIME_TO_MAGIC_DEFAULT);
				} else gentriad(&player1);
			}
			//Check if the spot below triad is empty or not on bottom row.
			if ((player1.grid[idx+(GRID_W*3)] == GRID_EMPTY) && (idx<GRID_TBTM)) {
				//dbg_sprintf(dbgout,"DROP COND %i,%i\n",(player1.grid[idx+(GRID_W*3)] == GRID_EMPTY),(idx<GRID_TBTM));
				for (i=0;i<4;++i) {
					//Check if there will be a uint8_t overflow
					if ((((int)drop_timer)+player1.cur_delay)>255) {
						falldown(&player1);
					}
					drop_timer += player1.cur_delay;
				}
				player1.cur_delay = player1.drop_max;
				player1.stay_delay = LONG_TIMEOUT;
			} else {
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
			refreshgrid(&player1);
			redrawboard(options);
			if (!(main_timer&3) && player1.triad_idx < 240) {
				for(i=0;i<6;++i,player1.triad_idx--) {
					player1.grid[player1.triad_idx] = GRID_EXP1;
					player1.cgrid[player1.triad_idx] = CHANGE_BUF1|CHANGE_BUF2;
				}
			}
			x = player1.grid_left;
			y = player1.grid_top;
			gfx_SetClipRegion(x,y,x+96,y+208);
			
			if (!player1.subsecond && !player1.secondsleft) {
				player1.arcaderank = scoreCmp(options);
				if (options->type == TYPE_FLASH && !player1.victory) {
					player1.arcaderank = 0; //Don't allow name entry on failure
				}
				if (player1.arcaderank) {
					//if high score has been achieved
					player1.secondsleft = 31;
					player1.state = GM_NAMEENTRY;
					player1.menuoption = 0;
					player1.curletter = 0;
					player1.secondsleft2 = 0;
					player1.cur_delay = 0;
					memset(&player1.namebuffer,'-',3);
				} else {
					player1.secondsleft = 5;
					player1.state = GM_GAMEWAITING;
				}
			}
			if (gos_y>(72+player1.grid_top)) gos_y-=4;
			
			if (options->type == TYPE_FLASH && player1.victory) {
				//You've done it
				for(i=0,y=0;i<13;i++) {
					x   = youdiditpos[i]+player1.grid_left;
					t   = (((main_timer>>3)-i)&2)==2;
					if (i==6) y = 16;
					rsptr = youdiditspr[i][t];
					gfx_RLETSprite(rsptr,x,gos_y+y);
				}
			} else {
				//Game over
				for(i=0;i<8;i++) {
					x   = gameoverpos[i]+player1.grid_left;
					t   = (((main_timer>>3)-i)&2)==2;
					rsptr = gameoverspr[i][t];
					gfx_RLETSprite(rsptr,x,gos_y);
				}
			}
			gfx_SetClipRegion(0,0,LCD_WIDTH,LCD_HEIGHT);
			gfx_SwapDraw();
			++main_timer;
			continue;
			/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
		} else if (player1.state == GM_NAMEENTRY) {
			refreshgrid(&player1);
			redrawboard(options);
			if (kc|kd) player1.secondsleft2 = 3;
			if (kc&kb_2nd) {
				if (player1.curletter == NEN_PREV && player1.menuoption>0)
					player1.menuoption--;
				else if (player1.curletter == NEN_NEXT && player1.menuoption<3)
					player1.menuoption++;
				else if (player1.curletter == NEN_END) {
					player1.subsecond = player1.secondsleft = 1;
				} else if (player1.curletter<NEN_PREV && player1.menuoption<3) {
					player1.namebuffer[player1.menuoption] = nentrymap[player1.curletter];
					player1.menuoption++;
				}
				if (player1.menuoption >= 3 && player1.curletter<NEN_PREV)
					player1.curletter = NEN_END;
			}
			if (kd&kb_Up) {
				player1.curletter = (player1.curletter+1)&0x1F;
				if (player1.menuoption==3 && player1.curletter<NEN_PREV) player1.curletter = NEN_PREV;
				player1.cur_delay = 4;
				player1.sdir = DIR_UP;
			}
			if (kd&kb_Down) {
				player1.curletter = (player1.curletter-1)&0x1F;
				if (player1.menuoption==3 && player1.curletter<NEN_PREV) player1.curletter = NEN_NEXT;
				player1.cur_delay = 4;
				player1.sdir = DIR_DOWN;
			}
			
			x = player1.grid_left+8;
			y = player1.grid_top+72;
			gfx_SetTextFGColor(FONT_GOLD);
			gfx_SetTextBGColor(BG_TRANSPARENT);
			gfx_PrintStringXY("very good!",x,y);
			x += 24;
			y += 16;
			
			if (options->type == TYPE_ARCADE) {
				gfx_PrintStringXY("rank ",x-8,y);
				gfx_PrintChar(player1.arcaderank+'0');
			}
			
			y += 16;
			gfx_SetTextXY(x,y);
			
			for (i=0;i<4;i++,x+=8) {
				t = player1.namebuffer[i];
				gfx_SetTextXY(x,y);
				if (i<3 && i!=player1.menuoption) {
					gfx_PrintChar(t);
				}
				
				if (i==player1.menuoption) {
					if (!player1.secondsleft2 && main_timer&32 && i<3) {
						gfx_PrintChar(t);
					} else {
						tp = nentrydisp[player1.curletter];
						gfx_SetTextFGColor(FONT_WHITE);
						if (player1.cur_delay) {
							--player1.cur_delay;
							gfx_SetClipRegion(x,y,x+8,y+8);
							gfx_SetTextConfig(gfx_text_clip);
							if (player1.sdir == DIR_DOWN) {
								gfx_SetTextXY(x,y+(player1.cur_delay<<1));
								gfx_PrintChar(tp);
								gfx_SetTextXY(x,y-((4-player1.cur_delay)<<1));
								gfx_PrintChar(nentrydisp[(player1.curletter+1)&0x1F]);
							} else {
								gfx_SetTextXY(x,y+((4-player1.cur_delay)<<1));
								gfx_PrintChar(nentrydisp[(player1.curletter-1)&0x1F]);
								gfx_SetTextXY(x,y-(player1.cur_delay<<1));
								gfx_PrintChar(tp);
							}
							gfx_SetTextXY(x,y);
							gfx_SetTextConfig(gfx_text_noclip);
							gfx_SetClipRegion(0,0,320,240);
						} else {
							gfx_PrintChar(tp);
						}
						gfx_SetTextFGColor(FONT_GOLD);
					}
				}
			}
			
			x = player1.grid_left+24;
			y = player1.grid_top+152;
			gfx_PrintStringXY("time ",x,y);
			gfx_PrintUInt(player1.secondsleft-1,2);
			
			if (!player1.subsecond && player1.secondsleft==1) {
				if (options->type == TYPE_ARCADE) {
					i = player1.arcaderank - 1;
					memmove(&save.arcade[i+1],&save.arcade[i],sizeof(save.arcade[0])*(9-i));
					ptr = (uint8_t*) &save.arcade[i];
					saveScore(options,ptr); //Saves name and score.
					numToString(player1.jewels,(char*)(ptr+4+10),4); //jewels
					numToString(player1.level+player1.baselevel,(char*)(ptr+4+10+5),3);
				} else {
					ptr = (uint8_t*) getScorePtr(options);
					saveScore(options,ptr);
					if (options->players == DOUBLES) {
						//DEBUG: DUMMY DOUBLES ENTRY - REPLICATE P1
						//TODO:  WAIT ON P2 AND RETRIEVE NAME VIA LINK
						strcpy((char*)&player1.namebuffer,"DBG");
						saveName(ptr+4+10);
					}
				}
				player1.state = GM_GAMEWAITING;
				player1.secondsleft = 5;
			}
			gfx_SwapDraw();
			++main_timer;
			continue;
			/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
		} else if (player1.state == GM_GAMEWAITING) {
			refreshgrid(&player1);
			redrawboard(options);
			//if (kc&kb_Mode) return;
			if (kc&kb_2nd) {
				//Only if the player has credits...
				//... not. This isn't an arcade machine!
				initGameState(options);
				player1.updating = UPDATE_SCORE|UPDATE_LEVEL|UPDATE_JEWELS;
				continue;
			}
			if (!player1.subsecond && !player1.secondsleft) return;
			//Game over scroller - more permanent.
			for(i=0;i<8;i++) {
				x   = gameoverpos[i]+player1.grid_left;
				t   = (((main_timer>>3)-i)&2)==2;
				rsptr = gameoverspr[i][t];
				gfx_RLETSprite(rsptr,x,player1.grid_top+72);
			}
			x = player1.grid_left;
			y = player1.grid_top;
			gfx_SetTextFGColor(FONT_GOLD);
			gfx_SetTextBGColor(BG_TRANSPARENT);
			if (main_timer&32) {
				gfx_PrintStringXY("press",x+24,y+96);
				gfx_PrintStringXY("2nd button",x+8,y+112);
			}
			
			gfx_SwapDraw();
			++main_timer;
			continue;
		} else break; //Illegal value - stop playing the game
		++main_timer;
		dbg_sprintf(dbgout,"State %i, cur timer %i, stay timer %i, index %i\n",player1.state,player1.cur_delay,player1.stay_delay,player1.triad_idx);
		redrawboard(options);
		gfx_SwapDraw();
	}
}



