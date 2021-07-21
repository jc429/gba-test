#include <tonc.h>
#include "playerhealth.h"
#include "objhistory.h"
#include "audio.h"
#include "debug.h"


extern void playerobj_damaged_start();
extern void playerobj_die_start();

static int player_health = PLAYER_HP_MAX;
static bool player_damaged;
static bool player_dead;


void playerhealth_init()
{
	playerhealth_reset();
}

void playerhealth_reset()
{
	player_damaged = false;
	player_dead = false;
	player_health = PLAYER_HP_MAX;
}

int playerhealth_get()
{
	return player_health;
}

// flag the player as damaged 
void playerhealth_take_damage()
{
	player_damaged = true;
}


void playerhealth_damage_check()
{
	if(player_damaged || player_dead) return;
	if(player_damaged)
	{
		if(!DEBUG_UNLIMITED_HP)
			player_health--;
		audio_play_sound(SFX_FROG_HIT);
		// TODO: maybe change this to not rewind? 
		history_step_back(1);
		playerobj_damaged_start();
	}
	player_damaged = false;
}

void playerhealth_death_check()
{
	if(player_dead) return;
	if(player_health <= 0 && !player_dead)
	{
		playerhealth_die();
	}
}

void playerhealth_die()
{
	player_dead = true;
	player_damaged = false;
	// play sound effect + anim, set timer, then reset level when timer expires
	playerobj_die_start();
}

bool playerhealth_is_dead()
{
	return player_dead;
}


void playerhealth_heal(int heal_amt)
{
	// show a sparkle, increase health
	player_health = clamp(player_health + heal_amt, 0, PLAYER_HP_MAX);
}

void playerhealth_reduce_hp(int dmg_amt)
{
	if(!DEBUG_UNLIMITED_HP)
		player_health = clamp(player_health - dmg_amt, 0, PLAYER_HP_MAX);
}