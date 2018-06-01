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
#include "game.h"

#include "gfx/title_gfx.h"


char *mainmenu[] = {" select "," arcade"," menu"," options"," quit"};
char *gameselmenu1[] = {"original game","flash columns"};
char *gameselmenu2[] = {"1 player","2 players","doubles"};
char *classes[] = {"novice","amateur","pro"};
char *levelnums[] = {"0","1","2","3","4","5","6","7","8","9"};
char *bgms[] = {"clotho","lathesis","atropos"};
char *noyes[] = {"no","yes"};
char *previewgame[] = {"original","flash columns"};
char *arcdiff[] = {"easiest","easy","normal","hard"};
char *twoplayermode[] = {"none","AI easy","AI normal","AI hard","AI super","2 calc link"};


uint16_t bghs[] = {16776,12548,8320,4096,16644,12416,8192,4096};

uint8_t mainmenustate[] = {GM_ARCADEOPTIONS,GM_GAMEMENU,GM_OPTIONS,255};

uint8_t mainoptcursory[] = {24,64,120,160,184,208};


/* ----------------------- Define your constants here ------------------------*/
uint8_t gamecursory1;    // Up/down changes these and indexes to gamecursorx.
uint8_t gamecursory2;    // Controlled remotely by other calculator.
uint8_t *gamecursorx1[4]; //  Points to the options dialog...
uint8_t *gamecursorx2[4]; //  ...entity each Y position index.


/* ----------------------- Function prototypes goes here ---------------------*/
void drawTitleGFX(void *titleptr);
void *selectNewTitle(void);
void drawMenuBG(void);
void drawScoreBG(void);
void dispCursor(x,y,yidx,xidx,prevcursor);
void *getScorePtr(options_t *options);

/* ----------------------- Define all your functions here --------------------*/
void main_menu_loop(void) {
	uint8_t i,j,k,t,idxlimit;
	uint8_t mx,my;
	uint8_t y,dy,oldy;
	uint8_t curopt;
	uint8_t debounce;
	uint8_t gamestate;
	int x;
	unsigned int dx,oldx,newx,tx;
	kb_key_t kd,kc;
	options_t *arcopt;
	options_t gameopt;
	char *s;
	char **ss;
	void *titleptr;
	
	arcopt = &save.arcopt;
	memset(&gameopt,0,sizeof gameopt);
	
	gamestate = GM_LOADINGTITLE;
	curopt = debounce = 0;
	
	while (1) {
		++main_timer;
		kb_Scan();
		kc = kb_Data[1];
		kd = kb_Data[7];
		if (kc|kd) {
			if (debounce) kc = kd = 0;
			debounce = 1;
		} else debounce = 0;
		
		switch (gamestate) {
			/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
			case GM_LOADINGTITLE:
				titleptr = selectNewTitle();
				drawTitleGFX(titleptr);
				drawTitleGFX(titleptr);
				gamestate = GM_MAINMENU;
				curopt = 0;
				break;
			/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
			case GM_TITLE:
				break;
			/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
			case GM_MAINMENU:
				if (kc&kb_Mode) return;
				gfx_SetTextBGColor(BG_BLACK);
				gfx_SetTextFGColor(FONT_WHITE);
				gfx_SetColor(BG_WHITE);
				gfx_FillRectangle_NoClip(112,137,96,69); //8px taller, downward
				gfx_SetColor(BG_BLACK);
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
					if (kc&kb_2nd) {
						gamestate = mainmenustate[curopt];
						curopt = 0;
					}
				}
				gfx_SetTextBGColor(BG_TRANSPARENT);
				break;
			/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
			case GM_GAMEMENU:
				if (kc&kb_Mode) gamestate = GM_LOADINGTITLE;
				if (kc&kb_2nd) {
					memset(gamecursorx1,0,sizeof gamecursorx1);
					memset(gamecursorx2,0,sizeof gamecursorx2);
					memset(&gameopt,0,sizeof gameopt);
					//Write game type
					gameopt.type = (curopt&1) ? TYPE_FLASH : TYPE_ORIGINAL;
					//Write number of players
					gameopt.players = (enum Players) curopt>>1;
					//Game class is the same across all modes of play
					gamecursorx1[0] = (uint8_t*) &gameopt.p1_class;
					gamecursorx2[0] = (uint8_t*) &gameopt.p2_class;
					//Game level uses the same indices. Is translated on game init
					gamecursorx1[1] = &gameopt.p1_level;
					gamecursorx2[1] = &gameopt.p2_level;
					//All original game and only 2 player flash columns mode uses
					//time_trial flags, tho for flash, it means match play.
					//Otherwise, it's all about the BGM (which this ver don't got)
					if (!(curopt&1) || curopt==3) {
						gamecursorx1[2] = &gameopt.time_trial;
					} else gamecursorx1[2] = &gameopt.bgm;
					
					//Final option is BGM, except in flash 1p/dbls where it's blank.
					if (curopt==1 || curopt==5) gamecursorx1[3] = NULL;
					else                        gamecursorx1[3] = &gameopt.bgm;
					//if (curopt>1) continue; /*disallow 2player modes*/
					gamecursory2 = gamecursory1 = 0;
					gamestate = GM_GAMEOPTIONS;
				}
				
				if (kd) {
					/*
					//Remove ability to select 2p modes by going up or down
					//To be added back in when ways to do 2p is implemented
					if (kd&kb_Up) {
						if (curopt<2) curopt+=6;
						curopt -= 2;
					}
					if (kd&kb_Down) {
						if (curopt>3) curopt-=6;
						curopt += 2;
					}
					*/
					if (kd&kb_Left) {
						if (!(curopt&1)) curopt += 2;
						curopt--;
					}
					if (kd&kb_Right) {
						if (curopt&1) curopt -= 2;
						curopt++;
					}
				}
				drawMenuBG();
				gfx_SetTextBGColor(BG_TRANSPARENT);
				gfx_SetTextFGColor(FONT_GOLD);
				for (i=0,x=48;i<2;i++,x+=120) {
					gfx_PrintStringXY(gameselmenu1[i],x,72);
				}
				gfx_SetTextFGColor(FONT_WHITE);
				gfx_PrintStringXY("menu",144,40);
				/*
				for (i=0,y=96;i<6;y+=24) {
					for(x=48;x<200;i++,x+=120) {
						if (i==curopt) {
							gfx_SetTextFGColor(FONT_CYAN);
							gfx_SetTextXY(x,y);
							gfx_PrintChar(']');
							gfx_SetTextFGColor(FONT_WHITE);
						}
						gfx_PrintStringXY(gameselmenu2[i>>1],x+16,y);
					}
				} */
				for (i=0,y=96;i<6;y+=24) {
					for(x=48;x<200;i++,x+=120) {
						if (i==curopt) {
							gfx_SetTextFGColor(FONT_CYAN);
							gfx_SetTextXY(x,y);
							gfx_PrintChar(']');
							gfx_SetTextFGColor(FONT_WHITE);
						}
						if (i<2)
						gfx_PrintStringXY(gameselmenu2[i>>1],x+16,y);
					}
				}
				break;
			/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
			case GM_GAMEOPTIONS:
				if (kc&kb_Mode) gamestate = GM_GAMEMENU;
				if (kc&kb_2nd) {
					//There really is no need to convert values. All values
					//are mirrored from the actual gameopt struct
					//via pointer shenaningans.
					gamestate = GM_GAMEPREVIEW;
				}
				
				if (kd) {
					//Handle up/down
					idxlimit = (curopt!=1 && curopt!=5)?3:2;
					if (kd&kb_Down) {
						if (gamecursory1<idxlimit) {
							gamecursory1++;
						} else gamecursory1 = 0;
					}
					if (kd&kb_Up) {
						if (gamecursory1) gamecursory1--;
						else gamecursory1 = idxlimit;
					}
					//Handle left/right
					switch (gamecursory1) {
						case 0:
							idxlimit = 2;
							break;
						case 1:
							if (curopt&1) idxlimit = 7;
							else idxlimit = 9;
							break;
						case 2:
							if (curopt==1 || curopt==5) idxlimit = 2;
							else idxlimit = 1;
							break;
						case 3:
							if (curopt==1 || curopt==5) idxlimit = 0;
							else idxlimit = 2;
							break;
						default:
							idxlimit = 0;
							break;
					}
					if (kd&kb_Left) {
						if (*gamecursorx1[gamecursory1]) {
							--*gamecursorx1[gamecursory1];
						} else {
							*gamecursorx1[gamecursory1] = idxlimit;
						}
					}
					if (kd&kb_Right) {
						if (*gamecursorx1[gamecursory1]<idxlimit) {
							++*gamecursorx1[gamecursory1];
						} else {
							*gamecursorx1[gamecursory1] = 0;
						}
					}
				}
				drawMenuBG();
				//Draw menu text here.
				for (x=40,y=32,i=0;i<((curopt==1||curopt==5)?3:4);i++) {
					switch (i) {
						case 0:
							s = "class";
							break;
						case 1:
							if (curopt&1) s = "height";
							else          s = "level";
							break;
						case 2:
							if (curopt==1 || curopt == 5) s = "bgm";
							else if (curopt&1) s = "match";
							else s = "time trial";
							break;
						case 3:
							if (!(curopt == 1 || curopt == 5)) s = "bgm";
							else s = "";
							break;
						default:
							s = "";
							break;
					}
					gfx_SetTextFGColor(FONT_GOLD);
					gfx_PrintStringXY(s,x,y);
					newx = x+24;
					y += 24;
					switch (i) {
						case 0:
							ss = classes;
							idxlimit = 3;
							dx = 72;
							break;
						case 1:
							if (curopt&1) {
								ss = levelnums+2;
								idxlimit = 8;
							} else {
								ss = levelnums;
								idxlimit = 10;
							}
							dx = 16;
							break;
						case 2:
							if ( curopt == 1 || curopt == 5) {
								ss = bgms;
								idxlimit = 3;
								dx = 80;
							} else {
								ss = noyes;
								idxlimit = 2;
								dx = 40;
							}
							break;
						case 3:
							if ( curopt == 1 || curopt == 5) {
								ss = NULL;
								idxlimit = 0;
								dx = 0;
							} else {
								ss = bgms;
								idxlimit = 3;
								dx = 80;
							}
							break;
						default:
							ss = NULL;
							idxlimit = 0;
							dx = 0;
							break;
					}
					
					gfx_SetTextFGColor(FONT_WHITE);
					if (ss) {
						for (j=0;j<idxlimit;j++,newx+=dx) {
							gfx_PrintStringXY(ss[j],newx,y);
							dispCursor(newx,y,i,j,curopt);
						}
					}
					y += 24;
				}
				break;
			/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
			case GM_GAMEPREVIEW:
				if (kc&kb_Mode) gamestate = GM_GAMEOPTIONS;
				if (kc&kb_2nd) {
					//Start the game...
					initGameState(&gameopt);
					runGame(&gameopt);
					gamestate = GM_LOADINGTITLE;
					continue;
				}
				drawMenuBG();
				//Draw menu text
				gfx_SetTextFGColor(FONT_GOLD);
				gfx_PrintStringXY("game  : ",x=96,y=40);
				gfx_SetTextFGColor(FONT_WHITE);
				gfx_PrintString(previewgame[gameopt.type==TYPE_FLASH]);
				y+=16;
				if (curopt!=2 && curopt!=3) {
					gfx_SetTextFGColor(FONT_GOLD);
					gfx_PrintStringXY("class : ",x,y);
					gfx_SetTextFGColor(FONT_WHITE);
					gfx_PrintString(classes[gameopt.p1_class]);
					y+=16;
				}
				if (curopt==1 || curopt == 5) {
					gfx_SetTextFGColor(FONT_GOLD);
					gfx_PrintStringXY("height: ",x,y);
					gfx_SetTextFGColor(FONT_WHITE);
					gfx_PrintString((levelnums+2)[gameopt.p1_level]);
					//y+=16;
				}
		
				if ((!(curopt&1) || curopt ==3) && gameopt.time_trial) {
					gfx_SetTextFGColor(FONT_CYAN);
					if (!(curopt&1)) s = "time trial mode";
					else             s = "match play mode";
					gfx_PrintStringXY(s,x,y);
				}
				y+=24;
				//1 or 2 player stats
				if (curopt==2 || curopt==3) {
					idxlimit = 2;
					x = 24;
				} else {
					idxlimit = 1;
				}
				oldy = y;
				oldx = x;
				for (i=0;i<idxlimit;i++,x+=152) {
					y = oldy;
					if (idxlimit-1) {
						gfx_SetTextFGColor(FONT_GOLD);
						gfx_PrintStringXY("player ",x+24,y);
						gfx_PrintString((levelnums+1)[i]);
						y += 24;
						t = (!i)?gameopt.p1_class:gameopt.p2_class;
						gfx_PrintStringXY("class : ",x,y);
						gfx_SetTextFGColor(FONT_WHITE);
						gfx_PrintString(classes[t]);
						if (curopt==3) {
							y += 16;
							t = (!i)?gameopt.p1_level:gameopt.p2_level;
							gfx_SetTextFGColor(FONT_GOLD);
							gfx_PrintStringXY("height:   ",x,y);
							gfx_SetTextFGColor(FONT_WHITE);
							gfx_PrintString((levelnums+2)[t]);
						}
						y +=24;
					}
					x += (curopt&1)?24:16;
					//Don't display scoring information if matches up with following
					if (!(curopt==3 && gameopt.time_trial)) {
						
						gfx_SetTextFGColor(FONT_GOLD);
						gfx_PrintStringXY("best ",x,y);
						if (curopt&1) s = "time";
						else          s = "score";
						gfx_PrintString(s);
						y+=16;
						s = (char*) getScorePtr(&gameopt);
						if (gameopt.players == DOUBLES) {
							tx = x+16;
						} else {
							tx = x+8;
						}
						gfx_SetTextFGColor(FONT_WHITE);
						gfx_PrintStringXY(s+4,tx,y);
						gfx_SetTextFGColor(FONT_GOLD);
						y+=16;
						gfx_PrintStringXY("by.",x,y);
						y+=16;
						gfx_SetTextFGColor(FONT_WHITE);
						if (gameopt.players == DOUBLES) {
							gfx_PrintStringXY(s+0,x,y);
							gfx_SetTextFGColor(FONT_GOLD);
							gfx_PrintStringXY(" & ",x+32,y);
							gfx_SetTextFGColor(FONT_WHITE);
							gfx_PrintString(s+4+10);
						} else {
							gfx_PrintStringXY(s+0,x+32,y);
						}
					}
					x = oldx;
				}
				break;
			/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
			case GM_ARCADEOPTIONS:
				initGameState(arcopt);
				runGame(arcopt);
				gamestate = GM_ARCADEHIGHSCORES;
				//workaround for the palette change glitch
				for (i=0;i<2;i++) {
					gfx_FillScreen(BG_BLACK);
					gfx_SwapDraw();
				}
				continue;
			/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
			case GM_OPTIONS:
				drawMenuBG();
				
				if (kc&kb_2nd) {
					switch (curopt) {
						case 0:
							arcopt->p1_class = (arcopt->p1_class+1)&3;
							break;
						case 1:
							break;
						case 2:
							break;
						case 3:
							break;
						case 4:
							gamestate = GM_COLORTEST;
							break;
						case 5:
						default:
							gamestate = GM_LOADINGTITLE;
							break;
					}
				}
				
				if (kc&kb_Mode) {
					gamestate = GM_LOADINGTITLE;
					continue;
				}
				
				if (kd&kb_Down) {
					++curopt;
					if (curopt>5) curopt=0;
				}
				if (kd&kb_Up) {
					if (!curopt) curopt=6;
					--curopt;
				}
				

				//NOTE: CONTROL SCHEME NOT CHANGEABLE YET. IT ONLY APPEARS
				//      FOR INSTRUCTIONAL PURPOSES.
				
				gfx_SetTextBGColor(BG_TRANSPARENT);
				//Option headers
				gfx_SetTextFGColor(FONT_GOLD);
				gfx_PrintStringXY("difficulty (arcade)",24,24);
				gfx_PrintStringXY("control",24,64);
				gfx_PrintStringXY("two-player mode",24,120);
				gfx_PrintStringXY("sound test (n/a)",24,160);
				gfx_PrintStringXY("color test",24,184);
				gfx_PrintStringXY("exit",24,208);
				
				//Option values
				gfx_SetTextFGColor(FONT_WHITE);
				gfx_PrintStringXY(arcdiff[arcopt->p1_class],40,40);
				gfx_PrintStringXY("  drop      rotate       quit",32,96);
				gfx_PrintStringXY(twoplayermode[save.twoplayer],40,136);
				
				//Special values
				gfx_SetTextFGColor(FONT_CYAN);
				gfx_PrintStringXY("pad down  2nd button  mode button",32,80);
				//Cursor. Re-uses cyan color setting
				gfx_SetTextXY(8,mainoptcursory[curopt]);
				gfx_PrintChar(']');
				
				break;
			/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
			case GM_ARCADEHIGHSCORES:
				//
				//DEBUG: SHOW ARCADE HIGH SCORE MENU INSTEAD AND FOR NOW
				//
				
				if (kc&kb_Mode) gamestate = GM_LOADINGTITLE;
				drawScoreBG();
				gfx_SetTextFGColor(FONT_GOLD);
				gfx_SetTextBGColor(BG_TRANSPARENT);
				
				gfx_PrintStringXY("score jewels level",80,40);
				for(y=56,i=0; i<9;++i,y+=16) {
					gfx_SetTextXY(16,y);
					gfx_PrintChar(i+'1');
					gfx_PrintChar(' ');
					gfx_PrintString(save.arcade[i].name);
					gfx_PrintStringXY(save.arcade[i].digits,64,y);
					gfx_PrintStringXY(save.arcade[i].jewels,136,y);
					gfx_PrintStringXY(save.arcade[i].level,192,y);
				}
				break;
			/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
			case GM_COLORTEST:
				if (kc&(kb_Mode|kb_2nd)) {
					gamestate = GM_OPTIONS;
					continue;
				}
				dzx7_Turbo(colortest_compressed,gfx_vbuffer);
				gfx_SetTextFGColor(FONT_WHITE);
				gfx_PrintStringXY("color test",120,16);
				break;
			/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
			default:
				return;
				
		}
		gfx_SwapDraw();
	}
}


void drawTitleGFX(void *titleptr) {
	dzx7_Turbo(titleptr,gfx_vbuffer);
	gfx_RLETSprite(titlebanner,48,16);
	gfx_SetTextBGColor(BG_TRANSPARENT);
	gfx_SetTextFGColor(FONT_WHITE);
	gfx_PrintStringXY(VERSION_INFO,272,232);
	gfx_SwapDraw();
}

void *selectNewTitle(void) {
	if (randInt(0,1)) return title1_compressed;
	else return title2_compressed;
}

void drawMenuBG(void) {
	uint8_t i;
	int x,y;  //Can't cast y as uint8_t due to bounds check at end
	
	//Draw top and bottom black borders
	gfx_SetColor(BG_BLACK);
	gfx_FillRectangle_NoClip(0,0,320,8);
	gfx_FillRectangle_NoClip(0,232,320,8);
	//Draw horizontal image border frames
	for(y=8;y<232;y+=72) {
		for(x=0;x<320;x+=8) {
			gfx_Sprite_NoClip(menuborder,x,y);
		}
	}
	//Draw menu fill tiles/sprites
	for(y=16;y<232;y+=72) {
		for(x=0;x<320;x+=80) {
			gfx_Sprite_NoClip(menutile,x,y);
		}
	}
}

void drawScoreBG(void) {
	uint8_t i;
	int x,y;  //Can't cast y as uint8_t due to bounds check at end
	uint8_t xi,yi;
	
	
	gfx_SetPalette(bghs,16,PALSWAP_AREA);  //CONST bgp7 defined in game.c
	//Draw top and bottom black borders
	gfx_SetColor(BG_BLACK);
	gfx_FillRectangle_NoClip(0,0,320,8);     //top border
	gfx_FillRectangle_NoClip(0,232,320,8);   //bottom border
	gfx_FillRectangle_NoClip(232,40,88,160); //Right panel
	for(i=0,x=0,y=8;i<(LCD_WIDTH/32);++i,x+=32) gfx_Sprite_NoClip(hsborder,x,y);
	for(y=40,yi=0;yi<5;y+=32,++yi) {
		for(x=(-24),xi=0;xi<8;x+=32,++xi) {
			gfx_Sprite(((xi^yi)&1)?cyantile:greentile,x,y);
		}
	}
	for(i=0,x=0,y=200;i<(LCD_WIDTH/32);++i,x+=32) gfx_Sprite_NoClip(hsborderf,x,y);
	
}


//use:          dx,y,i.j,curopt
void dispCursor(x,y,yidx,xidx,prevcursor) {
	if (*gamecursorx1[yidx] == xidx && !(gamecursory1==yidx && main_timer&2)) {
		if (yidx<2) gfx_RLETSprite_NoClip(p1sprite,x,y-8);
		else        gfx_RLETSprite_NoClip(downarrow,x,y-8);
	}
	if (prevcursor!=2 && prevcursor !=3) return;
	if (*gamecursorx2[yidx] == xidx && !(gamecursory2==yidx && main_timer&2)) {
		gfx_RLETSprite_NoClip(p2sprite,x,y+8);
	}
}

void *getScorePtr(options_t *opt) {
	uint8_t idx,diffidx;
	void *ptr;

	diffidx = idx = 0;
	if (opt->p1_class < 3) diffidx = opt->p1_class;
	
	//If arcade mode, just get address of first object
	if (opt->type == TYPE_ARCADE) return &save.arcade;
	//Else get pointer to score affected
	if (opt->type == TYPE_ORIGINAL) {
		if (opt->time_trial) ++idx;
		if (opt->players == DOUBLES) {
			ptr = &save.doubles[idx][diffidx];
		} else {
			ptr = &save.singles[idx][diffidx];
		}
	} else {  //TYPE_FLASH
		idx = opt->p1_level;
		if (opt->players == DOUBLES) {
			ptr = &save.flash_doubles[idx][diffidx];
		} else {
			ptr = &save.flash_singles[idx][diffidx];
		}
	}
	return ptr;
}

















