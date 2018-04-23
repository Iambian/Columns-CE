/*
 *--------------------------------------
 * Program Name: Columns CE
 * Author: rawrf.
 * License: rawrf.
 * Description: rawrf.
 *--------------------------------------
*/

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
#include <decompress.h>
#include <fileioc.h>

#include "defs.h"
#include "types.h"
#include "main.h"
#include "menu.h"
#include "game.h"
#include "gfx.h"




score_t tempscore;
savefile_t save;
uint8_t main_timer;

char *filename = "ColumDAT";

int randI(int imin,int imax);
void seedRand(void);


//******************************************************************************
void main(void) {
	ti_var_t slot;
	seedRand();
	initGraphics();
	/* Initialize game variables */
	memset(&save,0,sizeof save);
	save.arcopt.type = TYPE_ARCADE;
	save.arcopt.players = PLAYER1;
	save.arcopt.p1_class = NORMAL;
	save.arcopt.p2_class = NORMAL;
	save.version = SAVE_VERSION;
	
	ti_CloseAll();
	if (slot = ti_Open(filename,"r")) {
		if (ti_GetC(slot)==SAVE_VERSION) {
			ti_Rewind(slot);
			ti_Read(&save,1,sizeof save,slot);
		} else {
			ti_Close(slot);
			ti_Delete(filename);
		}
	}
	ti_CloseAll();
	main_timer = 0;
	
	//#####################
	main_menu_loop();
	//#####################
	
	if (slot = ti_Open(filename,"w")) {
		ti_Write(&save,1,sizeof save,slot);
		//ti_SetArchiveStatus(true,slot);
		ti_Close(slot);
	}
	while (kb_AnyKey());
	gfx_End();
}

//------------------------------------------------------------------------------


int randI(int imin,int imax) {  //Because apparently randInt is actually a macro.
	return randInt(imin,imax);
}

void seedRand() {
	asm("	LD A,R");  //Grab semirandom value from register R
	asm("	LD B,A");  //And seed _RandInt by running it R times.
	asm("__loop13487:");
	asm("	PUSH BC");
	asm("		LD BC,255");
	asm("		PUSH BC");
	asm("		LD BC,0");
	asm("		PUSH BC");
	asm("		CALL _randI"); //which we can't do directly because randInt is
	asm("		POP BC");      //a macro, not a function. Had to wrap it in one.
	asm("		POP BC");
	asm("	POP BC");
	asm("	DJNZ __loop13487");
	return;
}


