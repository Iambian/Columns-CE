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
#include "gfx.h"

#include "font.h"            //Fontset to use
#include "gfx/tiles_gfx.h"   //gems_tiles_compressed, explosion_tiles_compressed
#include "gfx/sprites_gfx.h" //cursor_compressed, grid_compressed
#include "gfx/bg_gfx.h"      //Background stuffs
#include "gfx/score_gfx.h"   //Numerals for scoreboard
#include "gfx/char_gfx.h"    //Fancy glyphs for "Game Over" / "You've Done It!"


/* ----------------------- Define your constants here ------------------------*/
uint8_t *chars[] = {
	s__apo_compressed,
	s__exc_compressed,
	s_A_compressed,
	s_D_compressed,
	s_E_compressed,
	s_G_compressed,
	s_I_compressed,
	s_M_compressed,
	s_N_compressed,
	s_O_compressed,
	s_R_compressed,
	s_T_compressed,
	s_U_compressed,
	s_V_compressed,
	s_Y_compressed,
};


gfx_rletsprite_t *gems_spr[gems_tiles_num];
gfx_rletsprite_t *explosion_spr[explosion_tiles_num];
gfx_rletsprite_t *cursor_spr;
gfx_sprite_t *grid_spr;
gfx_sprite_t *greentile;
gfx_sprite_t *cyantile;

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
gfx_rletsprite_t *downarrow;
gfx_rletsprite_t *flashgems[6][8];
gfx_rletsprite_t *magicgems[6];

gfx_rletsprite_t *arcadeselect;

gfx_rletsprite_t *gameoverspr[8][2];
gfx_rletsprite_t *youdiditspr[13][2];

gfx_sprite_t *hsborder;
gfx_sprite_t *hsborderf;

uint8_t gameoverchr[8] = {SYM_G,SYM_A,SYM_M,SYM_E,SYM_O,SYM_V,SYM_E,SYM_R};
uint8_t youdiditchr[13]= 
{	SYM_Y,SYM_O,SYM_U,SYM_APO,SYM_V,SYM_E,
	SYM_D,SYM_O,SYM_N,SYM_E,SYM_I,SYM_T,SYM_EXC
};

uint8_t gameoverpos[8] = {6,16,26,36,54,64,74,85};
uint8_t youdiditpos[13]= {21,30,41,50,60,70,12,22,33,42,59,67,79};


/* ----------------------- Function prototypes goes here ---------------------*/








/* ----------------------- Define all your functions here --------------------*/

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


void decompAndVaryChars(uint8_t ch, gfx_rletsprite_t **v) {
	uint8_t *baseimg,*ptr;
	uint8_t i;

	baseimg = (uint8_t*) gfx_vbuffer;
	
	dzx7_Turbo(chars[ch],baseimg);
	i = baseimg[0]*baseimg[1];
	v[0] = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)baseimg);
	dzx7_Turbo(chars[ch],baseimg);
	for (ptr=baseimg+2; i; ptr++,i--)  if ((ptr[0]==8)||(ptr[0]==9)) ptr[0]+=2;
	v[1] = gfx_ConvertMallocRLETSprite((gfx_sprite_t*)baseimg);
}

//%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%

void initGraphics(void) {
	uint8_t a,i,j;
	void *baseimg,*flipimg;
	uint8_t *ptr;
	int loop;
	uint8_t *fontspacing;
	
	gfx_Begin();
	gfx_SetDrawBuffer();
	ti_CloseAll();
	
	gfx_SetTextTransparentColor(BG_TRANSPARENT);
	
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
	downarrow = decompAndAllocate(downarrow_compressed);
	arcadeselect = decompAndAllocate(arcadeselect_compressed);
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
	//Flash gem graphics
	for (a=i=0;i<6;++i) {
		for (j=0;j<8;++j,++a) {
			flashgems[i][j] = decompAndAllocate(flashgems_tiles_compressed[a]);
		}
	}
	//Magic gem graphics
	for (i=0;i<6;++i) {
		magicgems[i] = decompAndAllocate(magicgems_tiles_compressed[i]);
	}
	//"Game over" graphics
	for (i=0;i<8;i++) {
		decompAndVaryChars(gameoverchr[i],&gameoverspr[i]);
	}
	//"You've done it" graphics
	for (i=0;i<13;i++) {
		decompAndVaryChars(youdiditchr[i],&youdiditspr[i]);
	}
	//High score border graphics
	hsborder = malloc(32*32+2);
	hsborderf = malloc(32*32+2);
	dzx7_Turbo(hsborder_compressed,hsborder);
	gfx_FlipSpriteX(hsborder,hsborderf);
	
}














