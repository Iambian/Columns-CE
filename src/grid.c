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
//#include "menu.h"
#include "gfx.h"
//#include "game.h"


#include "grid.h"





/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void refreshgrid(entity_t *e) {
	uint8_t i;
	for(i=0;i<GRID_SIZE;i++) {
		e->cgrid[i] |= CHANGE_BUF1|CHANGE_BUF2;
	}
}



void gfx_Sprite_NoClip_Safe(gfx_sprite_t *spr,int x, int y) {
	uint8_t *ptr,err;
	ptr = (uint8_t*) spr;
	err = 0;
	
	if (ptr[0]!=16 || ptr[1]!=16) {
		dbg_sprintf(dbgerr,"Incorrect sprite size. Found: (%i,%i)\n",ptr[0],ptr[1]);
		err++;
	}
	if (x>(319-16) || x<0 || y>(239-16) || y<0) {
		dbg_sprintf(dbgerr,"Sprite out of bounds. Found: (%i,%i)\n",x,y);
		err++;
	}
	if (err) dbg_Debugger();
	
	gfx_Sprite_NoClip(spr,x,y);
}

void gfx_RLETSprite_NoClip_Safe(gfx_rletsprite_t *spr,int x, int y) {
	uint8_t *ptr,err;
	ptr = (uint8_t*)spr;
	err = 0;
	
	if (ptr[0]!=16 || ptr[1]!=16) {
		dbg_sprintf(dbgerr,"Incorrect sprite size. Found: (%i,%i)\n",ptr[0],ptr[1]);
		err++;
	}
	if (x>(319-16) || x<0 || y>(239-16) || y<0) {
		dbg_sprintf(dbgerr,"Sprite out of bounds. Found: (%i,%i)\n",x,y);
		err++;
	}
	if (err) dbg_Debugger();
	
	gfx_RLETSprite_NoClip(spr,x,y);
}

void gfx_RLETSprite_Safe(gfx_rletsprite_t *spr,int x, int y) {
	uint8_t *ptr,err;
	ptr = (uint8_t*)spr;
	err = 0;
	
	if (ptr[0]!=16 || ptr[1]!=16) {
		dbg_sprintf(dbgerr,"Incorrect sprite size. Found: (%i,%i)\n",ptr[0],ptr[1]);
		err++;
	}
	if (x>(319-16) || x<0 || y>(239-16) || y<0) {
		dbg_sprintf(dbgerr,"Sprite out of bounds. Found: (%i,%i)\n",x,y);
		err++;
	}
	if (err) dbg_Debugger();
	
	gfx_RLETSprite(spr,x,y);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
//Draw if any of the flags were set, but clear only mask_buf flag from dgrid
void drawgrid(entity_t *e,uint8_t mask_buf) {
	uint8_t tilestate,tileid,grididx,gridx,gridy,y,y1,i;
	uint8_t ismagic;
	gfx_rletsprite_t *rspr;
	int x,x1;

	x1 = e->grid_left;
	y1 = e->grid_top;
	gfx_SetClipRegion(x1,y1,x1+(GRID_W*TILE_W),y1+(13*TILE_W));
	
	y = y1+(12*TILE_H); //bottom row
	grididx = GRID_SIZE-1;
	for (gridy = GRID_HSTART; gridy < GRID_H; gridy++) {
		x = x1+((GRID_W-1)*TILE_W);
		for (gridx = 0 ; gridx < GRID_W; gridx++) {
			tilestate = e->cgrid[grididx];
			tileid = e->grid[grididx];
			
			ismagic=0;
			if (tileid >= GRID_MAG1 && tileid <=GRID_MAG6) ismagic++;
			
			if ((tilestate & (CHANGE_BUF1|CHANGE_BUF2|TILE_FLASHING|TILE_TARGET_GEM) || ismagic)) {
				tilestate &= ~mask_buf; //Acknowledge render
				//@@@
				gfx_Sprite_NoClip(grid_spr,x,y);
				//gfx_Sprite_NoClip_Safe(grid_spr,x,y);
				if (tileid >= GRID_EXP1 && tileid <= GRID_EXP7) {
					//@@@
					gfx_RLETSprite_NoClip((gfx_rletsprite_t*)explosion_spr[tileid-GRID_EXP1],x,y);
					//gfx_RLETSprite_NoClip_Safe((gfx_rletsprite_t*)explosion_spr[tileid-GRID_EXP1],x,y);
					if (!(main_timer&1)) {
						tileid++;
						if (tileid > GRID_EXP7) {
							tileid = GRID_EMPTY;
							tilestate &= ~TILE_TARGET_GEM;
						}
					}
					tilestate |= CHANGE_BUF1 | CHANGE_BUF2; //Reverse acknowledgement.
				} else if (tileid >= GRID_GEM1 && tileid <=GRID_GEM6 || ismagic) {
					if (!(tilestate&TILE_FLASHING) || main_timer&4) {
						if (ismagic) {
							rspr = (gfx_rletsprite_t*)magicgems[tileid-GRID_MAG1];
						} else {
							if ((tilestate & TILE_TARGET_GEM) && (main_timer & 0x20)) {
								rspr = (gfx_rletsprite_t*)flashgems[tileid-GRID_GEM1][(main_timer>>3)&0x07];
							} else {
								rspr = (gfx_rletsprite_t*)gems_spr[tileid-GRID_GEM1];
							}
						}
						if (tilestate&TILE_HALFLINGS) {
							//@@@
							gfx_RLETSprite(rspr,x,y+8);
							//gfx_RLETSprite_Safe(rspr,x,y+8);
						} else {
							//@@@
							gfx_RLETSprite_NoClip(rspr,x,y);
							//gfx_RLETSprite_NoClip_Safe(rspr,x,y);
						}
					}
				}
				e->grid[grididx] = tileid;
			}
			e->cgrid[grididx] = tilestate;
			x -= TILE_W;
			grididx--;
		}
		y -= TILE_H;
	}
	//Dealing with the half tile crap above the screen
	
	x = e->grid_left+((GRID_W-1)*TILE_W);
	for (gridx=0;gridx<GRID_W;gridx++) {
		tileid = e->grid[grididx];
		tilestate = e->cgrid[grididx];
		if (tilestate&TILE_HALFLINGS ) {
			//Magic gem implement
			if (tileid >= GRID_MAG1 && tileid<=GRID_MAG6) {
				rspr = magicgems[tileid-GRID_MAG1];
				gfx_RLETSprite(rspr,x,y+8);
				//gfx_RLETSprite_Safe(rspr,x,y+8);
			}		
			//Everything else
			else if ((tilestate&mask_buf) && tileid >= GRID_GEM1 && tileid <=GRID_GEM6) {
				//@@@
				gfx_RLETSprite((gfx_rletsprite_t*)gems_spr[tileid-GRID_GEM1],x,y+8);
				//gfx_RLETSprite_Safe((gfx_rletsprite_t*)gems_spr[tileid-GRID_GEM1],x,y+8);
			}
		}
		e->cgrid[grididx] = tilestate & ~mask_buf;
		x -= TILE_W;
		grididx--;
	}
	
	gfx_SetClipRegion(0,0,LCD_WIDTH,LCD_HEIGHT);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
//Known problem: Does not properly transfer grid properties on falldown.
//This problem affects TILE_TARGET_GEM in Flash Columns mode if the target is
//not on the bottom row. We should also implement half drop support eventually.
void falldown(entity_t *e) {
	uint8_t i,ip;
	
	//for(i=0;i<(2*GRID_W);++i) e->cgrid[i] = 0; //Preclear change state of unseen rows
	
	i = e->triad_idx+GRID_BELOW;
//	dbg_sprintf(dbgout,"Cond1 %i, Cond2 %i\n",i<GRID_SIZE,e->grid[i]==GRID_EMPTY);
	if (i<GRID_SIZE && e->grid[i]==GRID_EMPTY) {
		//Only move down if tile already halfway in down position
		if (e->cgrid[e->triad_idx]&TILE_HALFLINGS) e->triad_idx += GRID_W;
	}
	//dbg_sprintf(dbgout,"Falldown new idx %i\n",e->triad_idx);
	i = GRID_SIZE-1, ip = i-GRID_W;
	for (;i>GRID_W-1;--i,--ip) {
		if (e->grid[i]==GRID_EMPTY && e->grid[ip]!=GRID_EMPTY) {
			//If destination is empty, move towards it.
			if (e->cgrid[ip]&TILE_HALFLINGS) {
				e->grid[i]   = e->grid[ip];
				e->grid[ip]  = GRID_EMPTY;
				e->cgrid[ip] &= ~TILE_HALFLINGS;
			} else {
				e->cgrid[ip] |= TILE_HALFLINGS;
			}
			e->cgrid[i]  |= CHANGE_BUF1|CHANGE_BUF2;
			e->cgrid[ip] |= CHANGE_BUF1|CHANGE_BUF2;
		} else if (e->cgrid[i]&TILE_HALFLINGS) {
			//If destination is becoming vacated, only allow tiles not moving
			//yet to begin movement by adding TILE_HALFLINGS. Else illegals.
			e->cgrid[ip] |= TILE_HALFLINGS | CHANGE_BUF1|CHANGE_BUF2;
			e->cgrid[i]  |= CHANGE_BUF1|CHANGE_BUF2;
			e->cgrid[i+GRID_W] |= CHANGE_BUF1|CHANGE_BUF2;
		}
		if (e->grid[i]==GRID_EMPTY) e->cgrid[i] &= ~TILE_HALFLINGS;
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void movedir(entity_t *e, enum Direction dir) {
	int8_t i,idx,oldidx,temp;
	temp = 0;
	//Move idx to left or right if we aren't on a field boundary
	oldidx = idx = e->triad_idx;
	if ((dir<0 && idx%GRID_W) || (dir>0 && ((idx+1)%GRID_W))) {
		idx += dir;
		//dbg_sprintf(dbgout,"Current state %i\n",idx);
		//dbg_sprintf(dbgout,"Dir change to %i\n",dir);
	} else return;
	//Do not change anything else if we tried to move on top of something else.
	
	if (e->cgrid[oldidx]&TILE_HALFLINGS)
	{
		i=3;
	} else {
		i=2;
	}
	if (e->grid[idx+(GRID_W*i)] != GRID_EMPTY) return;
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
	if (idx<GRID_SIZE && temp&TILE_HALFLINGS) {
		e->cgrid[idx] |= CHANGE_BUF1 | CHANGE_BUF2;
		e->cgrid[oldidx] |= CHANGE_BUF1 | CHANGE_BUF2;
	}
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
//Using the TILE_FLASHING flag to mark and detect tiles that have matched.
int8_t gridmatch(entity_t *e) {
	uint8_t i,t,matches,dist;
	uint8_t curidx,curx,cury;
	uint8_t tempidx,tempx,tempy;
	uint8_t limx,limy,limidx;
	uint8_t ismagic;
	
	//Ensure grid is free of anything flashing. This shouldn't happen, though.
	for(i=0;i<GRID_SIZE;++i) e->cgrid[i] &= ~TILE_FLASHING;
	
	ismagic = curidx = 0;
	t = e->grid[e->triad_idx];
	
	if (t>=GRID_MAG1 && t<=GRID_MAG6) {
		ismagic++;
		//Make sure that the magic gems are flashing
		for (i=0,curidx=e->triad_idx;i<3;i++,curidx+=GRID_W) {
			e->cgrid[curidx] |= TILE_FLASHING;
		}
		//And check to see if the magic gems are actually touching anything
		if (curidx<GRID_SIZE) {
			//And if so, match against all gems found.
			t = e->grid[curidx];
			for (i=0;i<GRID_SIZE;i++) {
				if (e->grid[i] == t) e->cgrid[i] |= TILE_FLASHING;
			}
		}
	} else {
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
	}
	//Check how many tiles have been set to flashing and return that number.
	for(matches=i=0; i<GRID_SIZE; ++i) {
		if (e->cgrid[i]&TILE_FLASHING) ++matches;
	}
	if (ismagic) matches = -matches;
	//dbg_sprintf(dbgout,"Matches found: %i\n",matches);
	return matches;
}



uint8_t grid4j[] = {
	1,2,3,4,1,2,
	1,4,1,2,3,2,
	3,2,3,4,1,4,
	3,4,1,2,3,4,
	1,2,3,4,1,2,
	4,3,1,2,4,3,
	1,2,3,4,2,1,
	3,4,1,2,3,4
};

uint8_t grid5j[] = {
	3,4,5,2,3,1,
	4,2,1,4,5,5,
	2,4,1,2,3,3,
	1,2,3,5,5,2,
	3,4,5,1,3,4,
	5,1,3,3,5,2,
	3,4,5,2,1,4,
	1,2,3,4,5,2
};
	
uint8_t grid6j[] = {
	4,6,5,2,1,3,
	1,3,4,6,5,2,
	5,2,1,3,4,6,
	3,1,4,6,3,1,
	6,4,5,2,2,5,
	5,2,1,3,4,6,
	5,4,3,5,6,6,
	1,2,1,3,4,2,
};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
//Called from initGameState().
//Assumes height=9. Caller is responsible for trimming the results
void genflashgrid(entity_t *e) {
	uint8_t maxtypes;
	uint8_t *jptr;
	uint8_t i,gidx;
	
	maxtypes = e->max_types;
	i = randInt(0,7) * GRID_W;
	gidx = (GRID_H-9) * GRID_W;
	
	if (maxtypes==3)      jptr = grid4j;
	else if (maxtypes==4) jptr = grid5j;
	else                  jptr = grid6j;
	
	for (; gidx<GRID_SIZE; ++gidx,i=(i+1)%48) e->grid[gidx] = jptr[i];
	
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void trimgrid(entity_t *e, uint8_t height) {
	uint8_t i,end;
	
	end = (GRID_H-height)*GRID_W;
	if (end>GRID_SIZE) return;  //in case of uninit or bad input (i see u p2)
	for(i=(GRID_H-9)*GRID_W; i<end; ++i) e->grid[i] = 0;
}












