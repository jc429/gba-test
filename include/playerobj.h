#ifndef PLAYEROBJ_H
#define PLAYEROBJ_H

#include "gameobj.h"
#include "sprites/player/spr_player.h"



#define PLAYER_MOVE_SPEED		1		// move speed in pixels per frame
#define PLAYER_TILE_OFFSET		4		// TILE offset for animations (how many tiles is one frame?)
#define PLAYER_FACING_OFFSET	8		// FRAME offset for each direction (multiply by tile offset for full offset)
#define PLAYER_ASYMMETRIC		0		// set 1 if player sheet has unique west-facing sprites
#define ALLOW_DIAGONAL			0		// set 1 to allow the player to move diagonally

typedef enum PlayerAnimID_T{
	PAI_IDLE,
	PAI_HOP,
	PAI_TONGUE,
	PAI_NOM,
	PAI_HURT,
	PAI_DIE,
	PAI_FALL,
	PAI_INTRO,
	PAI_VICTORY,
	PAI_COUNT				// INVALID
} PlayerAnimID;


void playerobj_set_starting_pos(int pos_x, int pos_y);
bool playerobj_check_floor_tile(int tile_x, int tile_y);
int playerobj_current_hop_height();
void playerobj_play_anim(PlayerAnimID pid);
void playerobj_action_primary();			// perform the A press action
void playerobj_action_secondary();			// perform the B press action

GameObj *get_player_obj();

bool playerobj_is_intangible();


#endif //PLAYEROBJ_H