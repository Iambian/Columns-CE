#ifndef __columns_defs__
#define __columns_defs__

#define VERSION_INFO "v 0.9 "
#define SAVE_VERSION 5


//Miscellaneous
#define TIME_TO_MAGIC_DEFAULT 30

// Fixed palette entries as defined in convpng.ini
#define BG_TRANSPARENT 0
#define BG_BLACK 1
#define BG_WHITE 2
#define FONT_WHITE 2
#define FONT_GOLD 3
#define FONT_CYAN 4
#define MENU_GOLD 5
#define MENU_MAROON 6
#define MENU_LAVENDER 7
/* NOTE: Codes 8-11 reserved for animated letter static palette swaps */
#define PALSWAP_AREA 192

//Grid and tile information
#define GRID_W 6
#define GRID_H 15
#define GRID_HSTART 2
#define GRID_START GRID_HSTART*GRID_W
#define GRID_SIZE GRID_W*GRID_H
#define TILE_W 16
#define TILE_H 16
#define GRID_TBTM (GRID_SIZE-(GRID_W*3))
#define GRID_BELOW (GRID_W*3)
#define P1_GRIDLEFT 16
#define P2_GRIDLEFT 208
#define CENTER_GRIDLEFT 112

//Timing information
#define MATCH_TIMEOUT 10
#define LONG_TIMEOUT 30
#define MOVESIDE_TIMEOUT 5
#define DESTRUCT_TIMEOUT 38
#define SCOREFALL_TIMEOUT 50
#define ONE_SECOND 64
#define DROP_NEXT_DELAY 16

//Grid buffer masking information
#define CHANGE_BUF1 (1<<7)
#define CHANGE_BUF2 (1<<6)
#define TILE_FLASHING (1<<5)
#define TILE_HALFLINGS (1<<4)
#define TILE_TARGET_GEM (1<<3)

//Field data masking information
#define UPDATE_SCORE ((1<<0)|(1<<4))
#define UPDATE_LEVEL ((1<<1)|(1<<5))
#define UPDATE_JEWELS ((1<<2)|(1<<6))
#define UPDATE_NEXT ((1<<3)|(1<<7))

//Gem identity information
#define GRID_EMPTY 0x00
#define GRID_GEM1 0x01
#define GRID_GEM2 0x02
#define GRID_GEM3 0x03
#define GRID_GEM4 0x04
#define GRID_GEM5 0x05
#define GRID_GEM6 0x06
//Gem explosion information
#define GRID_EXP1 0x07
#define GRID_EXP2 0x08
#define GRID_EXP3 0x09
#define GRID_EXP4 0x0A
#define GRID_EXP5 0x0B
#define GRID_EXP6 0x0C
#define GRID_EXP7 0x0D
//Magic gem information
#define GRID_MAG1 0x0E
#define GRID_MAG2 0x0F
#define GRID_MAG3 0x10
#define GRID_MAG4 0x11
#define GRID_MAG5 0x12
#define GRID_MAG6 0x13




//Game interface IDs
#define BG_CENT 0
#define BG_NEXT 1
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

//Object indices (objidx), as used in posarr[]
#define SOBJ_NEXT 0
#define SOBJ_SCOREMAIN 1
#define SOBJ_LEVELMAIN 2
#define SOBJ_JEWELSMAIN 3
#define SOBJ_CURSCORE 4
#define SOBJ_SCORESUB 5
#define SOBJ_LEVELSUB 6
#define SOBJ_JEWELSSUB 7
//Player modes and number of panes, as (supposed to be) used in posarr[]
#define PMODE_NORMAL 0
#define PMODE_FLASH 1
#define PMODE_SPANE 0
#define PMODE_DPANE 1 

//Character/symbol indices used in the "Game Over"/"You've Done It!" display
#define SYM_APO 0
#define SYM_EXC 1 
#define SYM_A 2
#define SYM_D 3
#define SYM_E 4
#define SYM_G 5
#define SYM_I 6
#define SYM_M 7
#define SYM_N 8
#define SYM_O 9
#define SYM_R 10
#define SYM_T 11
#define SYM_U 12
#define SYM_V 13
#define SYM_Y 14







#endif