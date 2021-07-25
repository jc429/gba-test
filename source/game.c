#include <string.h>
#include <tonc.h>
#include "debug.h"
#include "game.h"
#include "regmem.h"
#include "gameobj.h"
#include "layers.h"
#include "map.h"
#include "objhistory.h"
#include "input.h"
#include "objinteract.h"
#include "pausemenu.h"
#include "audio.h"


// ui.c
extern void ui_start();
extern void ui_erase();
extern void set_action_count_immediate(int count);
// playerobj.c
extern void playerobj_level_intro_start();
extern void playerobj_timestop_start();
// playerhealth.c
extern void playerhealth_damage_check();
extern void playerhealth_death_check();
// camera.c
extern void camera_center();
extern void camera_find_target();
// main.c
extern void go_to_logo();
extern void go_to_title();
extern void go_to_level_select();
extern void go_to_main_game();


void set_game_state(GameState state);


void game_update_main();





///////////////
/// Globals ///
///////////////

static bool turn_active;							// is the game currently performing a turn?

static GameState game_state;						// what state the game is currently in


static bool game_paused;							// is the game currently paused?

int world_offset_x;
int world_offset_y;



void main_game_start()
{
	input_lock(INPLCK_SYS);
	
	load_map_from_current_data();

	turn_count_set(0);
	set_action_count_immediate(0);
	game_paused = false;
	turn_active = false;
	history_reset();

	ui_start();
	
	reg_set_main();
	// update all obj histories once
	history_update_all();
	
	camera_center();
	camera_find_target();

	// play level intro animation 
	playerobj_level_intro_start();
	input_unlock(INPLCK_SYS);
	set_game_state(GS_MAIN_GAME);
	audio_play_track(MOD_LEVEL_SEL);
}


void main_game_end()
{
	//map_init();
	//playerobj_init();
	//ui_init();
	//effects_init();
	pausemenu_close();
	ui_erase();
	audio_stop_all();
	mem_clear_obj_palettes();
	mem_clear_tiles();
	gameobj_erase_all();
	REGBGOFS_reset_all();
	// wipe all sbb and cbb data
	//memset16(VRAM, 0, 32768);
	
}


void game_update_main()
{
	
	if(!input_locked())
	{
		if(key_hit(KEY_START))
		{
			pausemenu_open();
		}
		if(history_mode_active())
		{
			if(key_hit(KEY_L))
			{
				history_step_back(1);
				// play clock tick sound
				audio_play_sound(SFX_CLOCK_TICK);
			}
			else if(key_hit(KEY_R))
			{
				//history_return_to_present();
				history_step_forward(1);
				// play clock tick sound
				audio_play_sound(SFX_CLOCK_TICK);
			}
			else if(key_hit(KEY_DIR | KEY_A | KEY_B))
			{
				playerobj_timestop_start();
				history_mode_disable();
			}
		}
		else
		{
			if(key_hit(KEY_SHOULDER) && time_charge_use())
			{
				playerobj_timestop_start();
				history_mode_enable();
			}
		}
	}

	// game_update_main_temp();
}






///////////////////////
/// Turns & Actions ///
///////////////////////

// set the game state to "objects are moving"
void set_turn_active()
{
	turn_active = true;
	// start the turn counter animation
	turn_count_increment();
}

void set_turn_inactive()
{
	turn_active = false;
}

bool check_turn_active()
{
	return turn_active;
}

// update that occurs when the player takes an action
void action_update()
{
	// if an action was taken in the past, clear the previous future 
	history_clear_future();
}

// update that occurs after all pieces have settled
void finalize_turn()
{
	

	// floor check
	bool safe = gameobj_check_floor_dynamic();

	// check if the player has been damaged this turn, and apply damage if so
	playerhealth_damage_check();
	// check if the player has died 
	playerhealth_death_check();

	if(safe)
	{
		// update all obj histories
		history_update_all();
		// turn_count_increment();
		input_unlock(INPLCK_PLAYER);
		// deactivate turn 
		turn_active = false;
	}
}



//////////////////
/// Game State ///
//////////////////

void set_game_state(GameState state)
{
	game_state = state;
}

GameState get_game_state()
{
	return game_state;
}




/////////////////////
/// Pause/Unpause ///
/////////////////////

void set_game_paused(bool paused)
{
	game_paused = paused;
}


bool check_game_paused()
{
	return game_paused;
}


////////////////////
/// World Offset ///
////////////////////

// set the world offset values
void set_world_offset(int off_x, int off_y)
{
	world_offset_x = off_x;
	world_offset_y = off_y;
}

// get the current world offset
Vector2 get_world_offset()
{
	Vector2 v;
	v.x = world_offset_x;
	v.y = world_offset_y;
	return v;
}

// push changes to the world offset (done every frame)
void update_world_pos()
{
	// main map
	REG_BG1HOFS = world_offset_x;
	REG_BG1VOFS = world_offset_y;
	// overlay map
	REG_BG2HOFS = world_offset_x;
	REG_BG2VOFS = world_offset_y;
}

// reset all reg bg offsets, in addition to the world offset
void REGBGOFS_reset_all()
{
	world_offset_x = 0;
	world_offset_y = 0;
	REG_BG0HOFS = 0;
	REG_BG0VOFS = 0;
	REG_BG1HOFS = 0;
	REG_BG1VOFS = 0;
	REG_BG2HOFS = 0;
	REG_BG2VOFS = 0;
	REG_BG3HOFS = 0;
	REG_BG3VOFS = 0;
}



