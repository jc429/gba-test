#include "frogtongue.h"
#include "playerobj.h"
#include "input.h"
#include "direction.h"
#include "map.h"
#include "effects.h"
#include "sprites/tongue.h"

#define TSPR_PIECE_H	0
#define TSPR_PIECE_V	1
#define TSPR_TIP_H		2
#define TSPR_TIP_V		3

typedef enum TongueState_T{
	TS_STORED,			// tongue in mouth
	TS_EXTENDING,		// tongue extending from mouth
	TS_ATTACHED,		// tongue extended and attached to an object
	TS_RETRACTING,		// tongue returning to mouth
	TS_CONTRACTING,		// tongue contracting because the player is moving toward it 
	TS_PULLING_OBJ,		// tongue returning to mouth, but dragging an attached obj with it
	TS_PULLING_PL		// tongue returning to mouth, but dragging the player toward the attached obj
} TongueState;

#define TONGUE_EXT_TL	2	// how many tiles forward the tongue can reach
//#define TONGUE_EXT_PX	((TONGUE_EXT_TL*16)+8)		// max reach of tongue
#define EXT_SPD			2	// how many pixels per frame the tongue extends/retracts

// objhistory.c
extern void history_update_all();
// camera.c
extern void camera_update_pos();
// player.c
extern int playerobj_current_hop_height();


void tongue_check();						// check for objects at the apex of tongue stretching
void tongue_attach_obj(GameObj *target);	// attach the tongue to a valid target
void tongue_update_length();				// handles extending/retracting
void tongue_update_sprites();


int tongue_max_tl;									// max tile length of tongue extension (accounting for objects)
int tongue_len_bonus;								// how many extra pixels to add to tongue length (to account for center of player + (potentially) target obj) || typically set to 8 or 16
inline int tongue_max_px(){ return ((tongue_max_tl*16) + tongue_len_bonus); }	// converts tile length to pixel length for tongue

int tongue_extension;		// how far (in pixels) the tongue is currently extended
TongueState tongue_state;

GameObj *tongue_tip;
int base_tile;				// tile_id of the tongue sprite sheet
#define TONGUE_PIECES 6
GameObj *tongue_pieces[TONGUE_PIECES];	// the "neck" of the tongue

GameObj *tongue_owner;		// playerobj, essentially
GameObj *attached_obj;		// what the tongue is stuck on, if anything


void tongue_init(GameObj *owner)
{
	Vector2 pos = owner->tile_pos;
	int pal = owner->pal_bank_id;
	base_tile = mem_load_tiles(tongueTiles, tongueTilesLen);
	tongue_tip = gameobj_init_full(LAYER_GAMEOBJ, ATTR0_SQUARE, ATTR1_SIZE_8x8, pal, base_tile+TSPR_TIP_H, pos.x, pos.y, 0);
	vec2_set(&tongue_tip->spr_off, -4, 0);
	tongue_set_owner(owner);
	tongue_extension = 0;
	tongue_len_bonus = 8;
	attached_obj = NULL;

	for(int i = 0; i < TONGUE_PIECES; i++)
	{
		tongue_pieces[i] = gameobj_init_full(LAYER_GAMEOBJ, ATTR0_SQUARE, ATTR1_SIZE_8x8, pal, base_tile+TSPR_PIECE_H, pos.x, pos.y, 0);
		vec2_set(&tongue_pieces[i]->spr_off, -4, 0);
	}
	tongue_store();
}


void tongue_set_owner(GameObj *owner)
{
	tongue_owner = owner;
}


void tongue_update()
{
	tongue_update_length();
	tongue_update_sprites();
}


// handles extending/retracting
void tongue_update_length()
{
	Vector2 owner_facing = dir_to_vec(gameobj_get_facing(tongue_owner));
	switch (tongue_state)
	{
		case TS_EXTENDING:
			tongue_extension += EXT_SPD;
			if(tongue_extension >= tongue_max_px())
			{
				tongue_check();
				tongue_extension = tongue_max_px();
				if(tongue_max_tl == 0)
				{
					// TODO: play bonk
					tongue_retract();
				}
			}
			break;
		case TS_RETRACTING:
			tongue_extension -= EXT_SPD;
			if(tongue_extension <= 0)
			{
				tongue_extension = 0;
				tongue_store();
			}
			break;
		case TS_CONTRACTING:
			tongue_extension -= PLAYER_MOVE_SPEED;
			if(tongue_extension <= tongue_max_px())
			{
				tongue_extension = tongue_max_px();
				if(tongue_max_tl <= 0)
					tongue_store();
			}
			break;
		case TS_PULLING_OBJ:
			if(tongue_extension > tongue_len_bonus)
			{
				if(tongue_extension - EXT_SPD <= tongue_len_bonus)
				{

					gameobj_change_pixel_pos(attached_obj, owner_facing.x * (tongue_len_bonus - tongue_extension), owner_facing.y * (tongue_len_bonus - tongue_extension));
					gameobj_update_current_tile(attached_obj);
					history_update_all();
				}
				else
					gameobj_change_pixel_pos(attached_obj, owner_facing.x * -EXT_SPD, owner_facing.y * -EXT_SPD);
			}
			tongue_extension -= EXT_SPD;
			if(tongue_extension <= 0)
			{
				tongue_extension = 0;
				tongue_store();
			}
			break;
		case TS_PULLING_PL:
			if(tongue_extension > 8)
			{
				if(tongue_extension - EXT_SPD <= tongue_len_bonus)
				{

					gameobj_change_pixel_pos(tongue_owner, owner_facing.x * (tongue_extension - tongue_len_bonus), owner_facing.y * (tongue_extension - tongue_len_bonus));
					gameobj_update_current_tile(tongue_owner);
					history_update_all();
				}
				else
					gameobj_change_pixel_pos(tongue_owner, owner_facing.x * EXT_SPD, owner_facing.y * EXT_SPD);
			}
			tongue_extension -= EXT_SPD;
			if(tongue_extension <= 0)
			{
				tongue_extension = 0;
				tongue_store();
			}
			camera_update_pos();
			break;
		default:
			break;
	}
	tongue_update_sprites();
}


void tongue_update_sprites()
{
	Vector2 owner_facing = dir_to_vec(gameobj_get_facing(tongue_owner));
	Vector2 p = tongue_owner->pixel_pos;

	if(owner_facing.y == 0)
		p.y = 0;
//	if(owner_facing.x == 0 && attached_obj != NULL)
//		p.y = attached_obj->pixel_pos.y;
	//find distance between tip and owner
	int tonglen = tongue_extension;
	if(attached_obj != NULL)
	{	
		if(owner_facing.x == 0)
			p.y += playerobj_current_hop_height();
		if(owner_facing.y < 0)		// facing north, player below target, shorter len than normal
		{
			tonglen -= playerobj_current_hop_height();
		}
		if(owner_facing.y > 0)		// facing south, player above target, longer len than normal
		{
			tonglen += playerobj_current_hop_height();
		}
	}

	if(tongue_extension > 0)
	{
		p.x += (tongue_extension * owner_facing.x);
		p.y += (tongue_extension * owner_facing.y);
	}


	Vector2 t = tongue_owner->tile_pos;
	gameobj_set_tile_pos(tongue_tip, t.x, t.y);

	gameobj_set_pixel_pos(tongue_tip, p.x, p.y);

	// update tongue pieces 

	


	// split into an increment length for each piece of tongue
	int increment = (tonglen + 2) / TONGUE_PIECES;
	for(int i = 0; i < TONGUE_PIECES; i++)
	{
		if(tongue_extension > 12)
		{
			gameobj_unhide(tongue_pieces[i]);
		}
		else{
			gameobj_hide(tongue_pieces[i]);
			continue;
		}
		Vector2 inc = tongue_owner->pixel_pos;
		
		int spr_off = 2;
		inc.x += (((increment * i) + spr_off) * owner_facing.x);
		inc.y += (((increment * i) + spr_off) * owner_facing.y);
		if(owner_facing.y == 0)
		{
			int yinc = ((ABS(inc.y) - ABS(p.y)) * i)/ TONGUE_PIECES;
			inc.y += yinc; 	
		}
		gameobj_set_tile_pos(tongue_pieces[i], t.x, t.y);
		gameobj_set_pixel_pos(tongue_pieces[i], inc.x, inc.y);
	}

}


void tongue_extend()
{
	if(tongue_state != TS_STORED)
		return;
	tongue_state = TS_EXTENDING;

	Vector2 dir = dir_to_vec(gameobj_get_facing(tongue_owner));
	int tmax = 0;
	tongue_len_bonus = 16;	// reach halfway into the next tile if uninterrupted

	Vector2 v = tongue_owner->tile_pos;
	for(tmax = 0; tmax <= TONGUE_EXT_TL; )
	{
		v.x += dir.x;
		v.y += dir.y;
		if(!check_tile_free(v.x, v.y) || tmax == TONGUE_EXT_TL)
		{
			// trim tongue len if bumping into solid obj
			if(gameobj_check_properties(get_tile_contents(v.x, v.y), OBJPROP_SOLID))
				tongue_len_bonus = 8;
			break;
		}
		tmax++;
	}

	tongue_max_tl = tmax;
	tongue_extension = 0;

	

	if(dir.x != 0)
	{
		tongue_tip->spr_tile_id = base_tile + TSPR_TIP_H;
		for(int i = 0; i < TONGUE_PIECES; i++)
			tongue_pieces[i]->spr_tile_id = base_tile + TSPR_PIECE_H;
	}
	else if(dir.y != 0)
	{
		tongue_tip->spr_tile_id = base_tile + TSPR_TIP_V;
		for(int i = 0; i < TONGUE_PIECES; i++)
			tongue_pieces[i]->spr_tile_id = base_tile + TSPR_PIECE_V;
	}
	

	//for(int i = 0; i < TONGUE_PIECES; i++)
	//	gameobj_unhide(tongue_pieces[i]);
	playerobj_play_anim(PAI_TONGUE);
	gameobj_unhide(tongue_tip);
	input_lock_tongue();
}


void tongue_retract()
{
	if(attached_obj != NULL)
	{
		// pull the attached object
		//create_effect_at_position(ET_SMOKE, attached_obj->tile_pos.x, attached_obj->tile_pos.y);
		tongue_state = TS_PULLING_OBJ;
		input_lock_tongue();
	}
	else
	{
		playerobj_play_anim(PAI_TONGUE);
		tongue_state = TS_RETRACTING;
		tongue_detach_obj();
		input_lock_tongue();
	}
}


void tongue_contract()
{
	tongue_state = TS_CONTRACTING;
	tongue_max_tl -= 1;
	if(tongue_max_tl < 0)
		tongue_max_tl = 0;
}


void tongue_store()
{
	tongue_state = TS_STORED;
	tongue_max_tl = TONGUE_EXT_TL;
	tongue_extension = 0;
	tongue_len_bonus = 8;
	attached_obj = NULL;

	for(int i = 0; i < TONGUE_PIECES; i++)
		gameobj_hide(tongue_pieces[i]);
	gameobj_hide(tongue_tip);

	playerobj_play_anim(PAI_IDLE);
	input_unlock_tongue();
}


//checks if touching a valid object, and attaches if true or retracts if false
void tongue_check()
{
	Vector2 v = tongue_owner->tile_pos;
	Vector2 dir = dir_to_vec(gameobj_get_facing(tongue_owner));
	v.x += (1 + tongue_max_tl) * dir.x;
	v.y += (1 + tongue_max_tl) * dir.y;
	if(!check_tile_free(v.x, v.y))
	{
		GameObj *obj = get_tile_contents(v.x,v.y);
		if(obj != NULL)
		{
			tongue_attach_obj(obj);
			input_unlock_tongue();
			return;
		}
	}
	tongue_retract();
}

void tongue_attach_obj(GameObj *target)
{
		
	attached_obj = target;
	if(target != NULL)
		tongue_state = TS_ATTACHED;
}

void tongue_detach_obj()
{
	attached_obj = NULL;
}

GameObj *tongue_get_attached_object()
{
	return attached_obj;
}

bool check_tongue_out()
{
	return (tongue_state != TS_STORED);
}