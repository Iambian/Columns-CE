#ifndef __columns_types__
#define __columns_types__

#include <stdint.h>
#include <graphx.h>

extern enum GameState {
	GM_TITLE=0,
	GM_MAINMENU,
	GM_ARCADEOPTIONS,
	GM_GAMEMENU,GM_GAMEOPTIONS,GM_GAMEPREVIEW,
	GM_PLAYSTART,GM_PLAYSTART2,GM_PLAYMATCH,GM_PLAYMOVE,GM_GAMEOVER,
	GM_OPTIONS,GM_GOTOMAIN
};
extern enum GameType {
	TYPE_ARCADE = 0, TYPE_ORIGINAL, TYPE_FLASH
};
extern enum Players {
	PLAYER1 = 0, PLAYER2, DOUBLES
};
extern enum Difficulty { 
	NOVICE = 3, AMATEUR, PRO,
	EASIEST, EASY, NORMAL, HARD
};
extern enum Direction {DIR_LEFT = -1,DIR_RIGHT = 1};

//Used in entity_t
typedef struct numsprite_t {
	gfx_rletsprite_t *sprite;
	int ypos;
	int xpos;
} numsprite_t;

//Information about each player's state
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
	uint8_t start_idx;     //Index to start a triad from. Either 2 or 3, dep on mode
	uint8_t subsecond;     // 1/64th of a second.
	uint8_t secondsleft;   // For menu timing
	uint8_t menuoption;    // Current menu option
	uint8_t baselevel;     //this plus level based on jewels destroyed
	
	
	uint8_t next_triad[3]; //next 3 blocks, top to bottom.
	uint8_t grid[GRID_SIZE];  //Gem/explosion IDs
	uint8_t cgrid[GRID_SIZE]; //Board state flags (changing, flashing, etc)
	numsprite_t nums[5];      //Up to five digits
	int scoreybase;           //Y position of base scorebox
	uint8_t scorefallthrough; //Countdown frm 16 for drop. Higher vals ignored
	uint8_t updating;         //Flags based on UPDATE_XXXX defines
} entity_t;

//Information about the gameplay options selected prior to starting the game
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

//Various score formats
typedef struct score_t {
	char name[4];     //3 character name, zero-terminated
	char digits[10];  //8 digit score or 5 digit time, zero terminated.
} score_t;
typedef struct dblscore_t {
	char name[4];     //3 character name, zero-terminated
	char digits[10];  //8 digit score or 5 digit time, zero terminated.
	char name2[4];    //[IF USED] 3 ch name, zero-terminated.
} dblscore_t;
typedef struct arcscore_t {
	char name[4];     //3 ch name, 0-term
	char digits[10];  //8 ch score, 0-term
	char jewels[5];   //4 ch jewels (max 9999), 0-term
	char level[4];    //3 ch levels (theoretical max 285), 0-term
} arcscore_t;




#endif