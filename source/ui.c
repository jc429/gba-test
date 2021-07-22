#include <tonc.h>

#include "gameobj.h"
#include "animation.h"
#include "playerhealth.h"
#include "objhistory.h"
#include "input.h"
#include "layers.h"
#include "regmem.h"
#include "palettes.h"
#include "sprites/ui/heart.h"
#include "sprites/ui/numbers.h"
#include "sprites/ui/gear.h"
#include "sprites/ui/timegauge.h"


void ui_init();
void ui_start();
void ui_erase();
void ui_update();
void ui_show();
void ui_hide();

void ui_update_hearts();
void ui_update_time_gauge();

void reset_action_count();
void set_action_count_immediate(int count);
void set_action_count(int count);
void increment_action_counter();
void decrement_action_counter();
void ui_animate_gear_forward();
void ui_animate_gear_backward();


// int ui_palette;										// shared palette for ui elements
////////////////////
// Action Counter //
////////////////////
#define ACTION_COUNTER_DIGITS		3				// how many digits the action counter takes
#define DIGIT_ANIM_LENGTH			4				// how many frames it takes to roll from one digit to the next
#define ACTION_COUNTER_OFFSET_X		2				// offset from left side of screen
#define ACTION_COUNTER_OFFSET_Y		-2				// offset from bottom of screen
GameObj *action_counter[ACTION_COUNTER_DIGITS];		// action counter objects
int a_tile;											// tile of digit 0 


static int true_action_count = 0;					// the actual action count, for gameplay purposes
static int displayed_action_count = 0;				// the displayed action count, which will roll up or down until reaching the true action count
static int count_rolling;							// whether or not to animate the counter changing (and in which direction)
static int c_frame = 0;


////////////////
// Time Gauge //
////////////////
int t_tile;
#define TIME_GAUGE_SEGMENTS 4
#define TG_END_OFFSET_L	0
#define TG_END_OFFSET_R 2
#define TG_OFFSET_MID 4
GameObj *time_gauge[TIME_GAUGE_SEGMENTS];


////////////
// HP Bar //
////////////
GameObj *hearts[PLAYER_HP_MAX];	//health bar 
int h_tile;



//////////
// Gear //
//////////

GameObj *gear;
int g_tile;
int g_anim;


// initialize and set up ui
void ui_init()
{
	// TODO: change to uiPal
	//ui_palette = mem_load_palette(numbersPal);

	// init action counter
	a_tile = mem_load_tiles(numbersTiles, numbersTilesLen);
	for(int i = 0; i < ACTION_COUNTER_DIGITS; i++)
	{
		action_counter[i] = gameobj_init();
		gameobj_set_layer_priority(action_counter[i], LAYER_OVERLAY);
		//action_counter[i]->layer_priority = LAYER_OVERLAY;
		int ac_x = ACTION_COUNTER_OFFSET_X + i*8;
		int ac_y = ACTION_COUNTER_OFFSET_Y + 152;
		gameobj_update_attr_full(action_counter[i], ATTR0_SQUARE, ATTR1_SIZE_8x8, PAL_ID_UI, a_tile, ac_x, ac_y, true, 0);
	}
	reset_action_count();


	// init time gauge
	t_tile = mem_load_tiles(timegaugeTiles, timegaugeTilesLen);
	for(int i = 0; i < TIME_GAUGE_SEGMENTS; i++)
	{
		time_gauge[i] = gameobj_init();
		gameobj_set_layer_priority(time_gauge[i], LAYER_OVERLAY);
		//time_gauge[i]->layer_priority = LAYER_OVERLAY;
		int tg_x = 26 + (i*8);
		int tg_y = SCREEN_HEIGHT - 12;
		int tg_t = t_tile + (2*TG_END_OFFSET_L);
		if(i == TIME_GAUGE_SEGMENTS-1)
			tg_t += (2*TG_END_OFFSET_R);
		else if(i > 0)
			tg_t += (2*TG_OFFSET_MID);
		gameobj_update_attr_full(time_gauge[i], ATTR0_TALL, ATTR1_SIZE_8x16, PAL_ID_UI, tg_t, tg_x, tg_y, true, 0);

	}

	// init hp icons
	h_tile = mem_load_tiles(heartTiles, heartTilesLen);
	for(int i = 0; i < PLAYER_HP_MAX; i++)
	{
		int h_x = (i*8) + 0 - (3*(i>>1));
		int h_y = 0;
		hearts[i] = gameobj_init_full(LAYER_OVERLAY, ATTR0_TALL, ATTR1_SIZE_8x16, PAL_ID_UI, h_tile, h_x, h_y, true, 0);
		gameobj_set_base_spr_id(hearts[i], h_tile);
		//hearts[i]->base_spr_info = h_tile;
		if(i % 2)
			gameobj_set_flip_h(hearts[i], true);
	}


	// init gear
	gear = gameobj_init();
	g_tile = mem_load_tiles(gearTiles, gearTilesLen);
	gameobj_set_layer_priority(gear, LAYER_OVERLAY);
	//gear->layer_priority = LAYER_OVERLAY;
	gameobj_update_attr_full(gear, ATTR0_SQUARE, ATTR1_SIZE_32x32, PAL_ID_UI, g_tile, 0, 128, true, 0);
	AnimationData *gear_anim = animdata_create(g_tile, ANIM_OFFSET_32x32, 3, 0);
	gameobj_set_anim_data(gear, gear_anim, 0);
	//gameobj_set_anim_info(gear, 3, ANIM_OFFSET_32x32, 0, false);
	g_anim = 1;

}

// prepare ui for gameplay
void ui_start()
{
	reset_action_count();
	ui_show();
}

void ui_erase()
{
	reset_action_count();
	for(int i = 0; i < ACTION_COUNTER_DIGITS; i++)
	{
		gameobj_erase(action_counter[i]);
	}
	for(int i = 0; i < PLAYER_HP_MAX; i++)
	{
		//hearts[i]->spr_tile_id = 0;
		//gameobj_update_attr_full(hearts[i],ATTR0_SQUARE, ATTR1);
		gameobj_erase(hearts[i]);
	}
	for(int i = 0; i < TIME_GAUGE_SEGMENTS; i++)
	{
		gameobj_erase(time_gauge[i]);
	}
	gameobj_erase(gear);
}

// gameplay update
void ui_update()
{
	ui_update_hearts();
	ui_update_time_gauge();
}

void ui_show()
{
	for(int i = 0; i < ACTION_COUNTER_DIGITS; i++)
	{
		gameobj_unhide(action_counter[i]);
	}
	for(int i = 0; i < PLAYER_HP_MAX; i++)
	{
		gameobj_unhide(hearts[i]);
	}
	for(int i = 0; i < TIME_GAUGE_SEGMENTS; i++)
	{
		gameobj_unhide(time_gauge[i]);
	}
	gameobj_unhide(gear);
	count_rolling = 0;
	set_action_count_immediate(true_action_count);
}

void ui_hide()
{
	for(int i = 0; i < ACTION_COUNTER_DIGITS; i++)
	{
		gameobj_hide(action_counter[i]);
	}
	for(int i = 0; i < PLAYER_HP_MAX; i++)
	{
		gameobj_hide(hearts[i]);
	}
	for(int i = 0; i < TIME_GAUGE_SEGMENTS; i++)
	{
		gameobj_hide(time_gauge[i]);
	}
	gameobj_hide(gear);
	count_rolling = 0;
	set_action_count_immediate(true_action_count);
}


void ui_update_hearts()
{
	int player_health = playerhealth_get();
	
	for(int i = 0; i < PLAYER_HP_MAX; i++)
	{
		if(i >= player_health)
			gameobj_set_sprite_id(hearts[i], h_tile+2);
			//hearts[i]->spr_tile_id = h_tile + 2;
		else
			gameobj_set_sprite_id(hearts[i], h_tile);
			//hearts[i]->spr_tile_id = h_tile;
		gameobj_update_attr(hearts[i]);
		if(i % 2)
			gameobj_set_flip_h(hearts[i], true);
	}
}

void ui_update_time_gauge()
{
	int time_charges = time_charges_check();
	if(time_charges > 0)
		gameobj_set_sprite_id(time_gauge[0], t_tile + 2*(TG_END_OFFSET_L));
		//time_gauge[0]->spr_tile_id = t_tile + 2*(TG_END_OFFSET_L);
	else
		gameobj_set_sprite_id(time_gauge[0], t_tile + 2*(1+TG_END_OFFSET_L));
		//time_gauge[0]->spr_tile_id = t_tile + 2*(1+TG_END_OFFSET_L);
	if(time_charges == 5)
		gameobj_set_sprite_id(time_gauge[TIME_GAUGE_SEGMENTS-1], t_tile + 2*(TG_END_OFFSET_R));
		//time_gauge[TIME_GAUGE_SEGMENTS-1]->spr_tile_id = t_tile + 2*(TG_END_OFFSET_R);
	else
		gameobj_set_sprite_id(time_gauge[TIME_GAUGE_SEGMENTS-1], t_tile + 2*(1+TG_END_OFFSET_R));
		//time_gauge[TIME_GAUGE_SEGMENTS-1]->spr_tile_id = t_tile + 2*(1+TG_END_OFFSET_R);

	for(int i = 1; i < TIME_GAUGE_SEGMENTS-1; i++)
	{
		int seg = 1 + (2*(i-1));
		int fr = t_tile;
		if(time_charges < seg)
			fr += 2*(3+TG_OFFSET_MID);
		else if(time_charges == seg)
			fr += 2*(2+TG_OFFSET_MID);
		else if(time_charges == seg+1)
			fr += 2*(1+TG_OFFSET_MID);
		else
			fr += 2*(TG_OFFSET_MID);
		gameobj_set_sprite_id(time_gauge[i], fr);
		//time_gauge[i]->spr_tile_id = fr;
	}
}

// animation update
void ui_update_anim()
{

	// Action Counter
	if(count_rolling != 0)
	{
		// update ones digit first
		int f_offset = gameobj_get_sprite_id(action_counter[2]) - a_tile;
		//int f_offset = action_counter[2]->spr_tile_id - a_tile;
		f_offset = ((10 * DIGIT_ANIM_LENGTH) + (f_offset + count_rolling)) % (10 * DIGIT_ANIM_LENGTH);
		gameobj_set_base_spr_id(action_counter[2], a_tile+f_offset);
		//action_counter[2]->spr_tile_id = a_tile + f_offset;
		
		//if rolling up to a 0 or down to a 9
		if(((count_rolling > 0) && (displayed_action_count % 10 == 9)) || ((count_rolling < 0) && (displayed_action_count % 10 == 0)))
		{
			// update tens digit as well
			f_offset = gameobj_get_sprite_id(action_counter[1]) - a_tile;
			//f_offset = action_counter[1]->spr_tile_id - a_tile;
			f_offset = ((10 * DIGIT_ANIM_LENGTH) + (f_offset + count_rolling)) % (10 * DIGIT_ANIM_LENGTH);
			gameobj_set_base_spr_id(action_counter[1], a_tile+f_offset);
			//action_counter[1]->spr_tile_id = a_tile + f_offset;

			//repeat process for hundreds digit
			if(((count_rolling > 0) && (displayed_action_count % 100 == 99)) || ((count_rolling < 0) && (displayed_action_count % 100 == 0)))
			{
				f_offset = gameobj_get_sprite_id(action_counter[0]) - a_tile;
				//f_offset = action_counter[0]->spr_tile_id - a_tile;
				f_offset = ((10 * DIGIT_ANIM_LENGTH) + (f_offset + count_rolling)) % (10 * DIGIT_ANIM_LENGTH);
				gameobj_set_base_spr_id(action_counter[0], a_tile+f_offset);
				//action_counter[0]->spr_tile_id = a_tile + f_offset;
			}
		}

		c_frame++;
		if(c_frame == DIGIT_ANIM_LENGTH)
		{
			c_frame = 0;
			displayed_action_count += count_rolling;
			if(displayed_action_count == true_action_count)
			{
				input_unlock(INPLCK_UI);
				count_rolling = 0;
				set_action_count_immediate(true_action_count);		//should be redundant if everything works properly, but just in case
			}
		}
	}

}




void reset_action_count()
{
	true_action_count = 0;
	displayed_action_count = 0;
	for(int i = 0; i < ACTION_COUNTER_DIGITS; i++)
	{
		gameobj_set_sprite_id(action_counter[i], a_tile);
		//action_counter[i]->spr_tile_id = a_tile;
		//gameobj_update_attr(action_counter[i]);
	}
	count_rolling = 0;
	set_action_count_immediate(0);
	input_unlock(INPLCK_UI);
}

void set_action_count_immediate(int count)
{
	true_action_count = count;
	displayed_action_count = count;
	// hundreds digit
	gameobj_set_sprite_id(action_counter[0], a_tile + (DIGIT_ANIM_LENGTH * ((count / 100) % 10)));
	//action_counter[0]->spr_tile_id = a_tile + (DIGIT_ANIM_LENGTH * ((count / 100) % 10));
	gameobj_update_attr(action_counter[0]);
	// tens digit
	gameobj_set_sprite_id(action_counter[1], a_tile + (DIGIT_ANIM_LENGTH * ((count / 10) % 10)));
	//action_counter[1]->spr_tile_id = a_tile + (DIGIT_ANIM_LENGTH * ((count / 10) % 10));
	gameobj_update_attr(action_counter[1]);
	// ones digit
	gameobj_set_sprite_id(action_counter[2], a_tile + (DIGIT_ANIM_LENGTH * (count % 10)));
	//action_counter[2]->spr_tile_id = a_tile + (DIGIT_ANIM_LENGTH * (count % 10));
	gameobj_update_attr(action_counter[2]);
	count_rolling = 0;
	input_unlock(INPLCK_UI);
}

void set_action_count(int count)
{
	true_action_count = count;
	if(true_action_count < displayed_action_count)
	{
		count_rolling = -1;
		c_frame = 0;
		ui_animate_gear_backward();
		input_lock(INPLCK_UI);
	}
	if(true_action_count > displayed_action_count)
	{
		count_rolling = 1;
		c_frame = 0;
		ui_animate_gear_forward();
		input_lock(INPLCK_UI);
	}
}

void increment_action_counter()
{
	displayed_action_count++;
	count_rolling = 1;
	c_frame = 0;
	ui_animate_gear_forward();
}

void decrement_action_counter()
{
	displayed_action_count--;
	count_rolling = -1;
	c_frame = 0;
	ui_animate_gear_backward();
}

void ui_animate_gear_forward()
{
	anim_play_forward(&gear->anim);
}

void ui_animate_gear_backward()
{
	anim_play_reversed(&gear->anim);
}

