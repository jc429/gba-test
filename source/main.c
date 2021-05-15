#include <string.h>
#include <tonc.h>

#include "layers.h"
#include "game.h"
#include "input.h"



extern void init_objs();	//temp
extern void init_map();		//temp
extern void game_update_temp();	//temp

extern void animations_init();
extern void gameobj_init_all();
extern void playerobj_init();
extern void ui_init();
extern void map_init();

extern void ui_start();

extern void playerobj_update();
extern void ui_update();
extern void ui_update_anim();
extern void update_world_pos();
extern void camera_update_pos();
// gameobj.c
extern void gameobj_push_all_updates();
extern void gameobj_update_anim_all();
// objhistory.c
extern void history_clear_future();
extern void history_update_all();
extern int turn_count_get();
extern void turn_count_set(int turn_count);
extern void turn_count_increment();
extern void turn_count_decrement();


void main_game_loop();

// init functions
void game_init();
void reg_init();
void timer_init();

void game_start();									// after initializing everything, this is called to set the game in motion, then we enter the game loop

// update functions
void animations_update();							// update graphics and animation
void game_update();									// update gameplay elements
void action_update();								// update that occurs when the player takes an action
void finalize_turn();								// update that occurs after all pieces have settled

int current_turn();									// get the current turn
void set_current_turn(int turn_count);				// set the current turn



static GameState game_state;						// what state the game is currently in
static bool game_paused;							

// placeholder
void test_init_tte_se4();
void test_run_tte_se4();
void bg_demo();
void update_text_temp();
void win_textbox(uint bgnr, int left, int top, int right, int bottom, uint bldy);



// The Big One
int main(void)
{
	game_init();
	

	// temp
	init_objs(); 	
	init_map();
	test_init_tte_se4();
	test_run_tte_se4();
	//
	game_start();

	main_game_loop();


	return 0;
}

// Where all the magic happens
void main_game_loop()
{
	
	while (1) 
	{
		//vid_vsync();		//resource hog
		VBlankIntrWait();	//slower but saves power
		key_poll();

		animations_update();

		game_update();
		

		
	}
}



//////////////////////////////////
/// Primary Game Init Function ///
//////////////////////////////////

//all initialization and setup goes in here
void game_init()
{	
	game_state = GS_DEBUG;

	// GBA setup
	irq_init(NULL);
	//irq_add(II_VBLANK, NULL);
	irq_enable(II_VBLANK);
	reg_init(); 
	
	// game setup
	animations_init();
	gameobj_init_all();
	map_init();
	playerobj_init();
	ui_init();
}	



////////////////////////////
/// Other Initialization ///
////////////////////////////

// initialize the bg + obj registers 
void reg_init()
{
	// set up BG0 for text
	REG_BG0CNT= BG_CBB(LAYER_TEXT) | BG_SBB(30);
	// set up BG1 for a 8bpp 32x32t map, using charblock 0 and screenblock 31
	REG_BG1CNT= BG_PRIO(LAYER_BACKGROUND) | BG_CBB(0) | BG_SBB(30) | BG_8BPP | BG_REG_32x32;
	REG_DISPCNT= DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_OBJ | DCNT_OBJ_1D;
	//REG_DISPCNT= DCNT_MODE0 | DCNT_BG0;
}

// initialize game timers (currently unused?)
void timer_init()
{
	// Overflow every ~1 second:
	// 0x4000 ticks @ FREQ_1024

	// REG_TM2D = -0x4000;				// 111 ticks to overflow - should produce 60hz cycle?
	// REG_TM2CNT = TM_FREQ_1024;		// use 1024 cycle timer
	// REG_TM2CNT = TM_ENABLE;
	// REG_TM3CNT= TM_ENABLE | TM_CASCADE;		// tm1 cascades into tm0
}


//////////////////
/// Game Start ///
//////////////////

void game_start()
{
	turn_count_set(0);
	game_paused = false;
	ui_start();
	
	// update all obj histories once
	history_update_all();
}


/////////////////////////////
/// Game Update Functions ///
/////////////////////////////

// update graphics and animation
void animations_update()
{
	static uint32_t anim_sync;
	static uint32_t ui_anim_sync;

	anim_sync++;
	if(anim_sync > ANIM_SPEED)
	{
		// animation update functions go in here
		gameobj_update_anim_all();
		anim_sync %= ANIM_SPEED;
	}
	ui_anim_sync++;
	if(ui_anim_sync > UI_ANIM_SPEED)
	{
		//ui looked weird at the slower anim speed so it runs on its own cycle
		ui_update_anim();
		ui_anim_sync %= UI_ANIM_SPEED;
	}
}


// update gameplay elements
void game_update()
{
	// gameplay update functions go out here
	update_text_temp();
	playerobj_update();				// update player first
	game_update_temp();				// update other gameobjs 
	update_world_pos();				// push the map around
	ui_update();
	camera_update_pos();

	// update gameobj attrs based on gameplay changes
	gameobj_push_all_updates();
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
	// update all obj histories
	history_update_all();

	turn_count_increment();

	input_unlock();
}

/////////////////////////////
/// Testing & Placeholder ///
/////////////////////////////

void test_init_tte_se4()
{

	// (1) Init for text
//	tte_init_chr4c(
//		0,									// BG number.
//		BG_CBB(0)|BG_SBB(16),				// BG control.
//		0xF000,								// Screen-entry base
//		bytes2word(13,15,15,15),				// Color attributes.
//		CLR_MAG,							// Ink color
//		&verdana9_b4Font,					// Verdana 9, with shade.
//		(fnDrawg)chr4c_drawg_b4cts_fast);	// b4cts renderer, asm version
	

	// --- (1) Base TTE init for tilemaps ---
	tte_init_se(
		0,						// Background number (BG 0)
		BG_CBB(0)|BG_SBB(31),	// BG control (for REG_BGxCNT)
		0,						// Tile offset (special cattr)
		CLR_WHITE,				// Ink color
		14,						// BitUnpack offset (on-pixel = 15)
		NULL,					// Default font (sys8) 
		NULL);					// Default renderer (se_drawg_s)


	tte_init_con();

	// --- (2) Init some colors ---
	//pal_bg_bank[1][15]= CLR_RED;
	//pal_bg_bank[2][15]= CLR_GREEN;
	//pal_bg_bank[3][15]= CLR_BLUE;
	//pal_bg_bank[4][15]= CLR_WHITE;
	//pal_bg_bank[5][15]= CLR_MAG;

	// (3) Create and print to a text box.
	//win_textbox(0, 8, 160-32+4, 232, 160-4, 8);

	

}



void win_textbox(uint bgnr, int left, int top, int right, int bottom, uint bldy)
{
	REG_WIN0H= left<<8 | right;
	REG_WIN0V=  top<<8 | bottom;
	REG_WIN0CNT= WIN_ALL | WIN_BLD;
	REG_WINOUTCNT= WIN_ALL;

	REG_BLDCNT= (BLD_ALL&~BIT(bgnr)) | BLD_BLACK;
	REG_BLDY= bldy;

	REG_DISPCNT |= DCNT_WIN0;

	tte_set_margins(left, top, right, bottom);
}


void test_run_tte_se4()
{
//	tte_erase_screen();

	// --- (3) Print some text ---
	//tte_set_margins(16,16,144,144);
	tte_set_pos(8, 8);
	// "Hello world in different colors"
	tte_write("\n Hahahahaha\n");
	tte_write(" #{cx:0x1000}It took me an entire day\n");
	tte_write(" #{cx:0x2000}But I made text and sprites \n show up at the same time\n");
	tte_write(" #{cx:0x5000}It was a 1 line fix....\n");

	// Color use explained
	//tte_set_pos(8, 84);
	//tte_write("#{cx:0x0000}C#{cx:0x1000}o#{cx:0x2000}l");
	//tte_write("#{cx:0x3000}o#{cx:0x4000}r#{cx:0x5000}s");
	//tte_write("#{cx:0} provided by \\#{cx:#}.");

}



void update_text_temp()
{
	if(key_hit(KEY_SELECT))
	{
		static bool txt_hidden;
		if(txt_hidden)
			test_run_tte_se4();
		else
			tte_erase_screen();
		txt_hidden = !txt_hidden;
		//tte_erase_rect(0,0,64,32);
	}
}
