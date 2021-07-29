#include "objinteract.h"
#include "objhistory.h"
#include "effects.h"
#include "map.h"
#include "audio.h"
#include "palettes.h"
#include "regmem.h"
#include "layers.h"
#include "playerobj.h"
#include "direction.h"
#include "debug.h"


#include "sprites/objects/coin.h"
#include "sprites/objects/obj_crate.h"
#include "sprites/objects/victory_tile.h"
#include "sprites/objects/launch_tile.h"
#include "sprites/objects/spikes.h"



// playerobj.c
extern void playerobj_move(int move_x, int move_y);
extern bool playerobj_launch(int launch_dir);
extern void playerobj_victory_start();
// playerhealth.c
extern void playerhealth_take_damage();
// playertongue.c
extern void tongue_detach_obj();
extern void tongue_retract();
extern GameObj *tongue_get_attached_object();

//int oi_pal;

void objint_init();
void crate_update_spr(GameObj *obj);


// interact functions
bool floorint_launch(GameObj *self, GameObj *instigator);
bool floorint_spikes(GameObj *self, GameObj *instigator);
bool floorint_victory(GameObj *self, GameObj *instigator);


////////////////////////
/// Global Functions ///
////////////////////////

void objint_init()
{
	//oi_pal = mem_load_palette(spikesPal);
}



///////////////////////////
/// Object Interactions ///
///////////////////////////

// push a game object
void objint_push_gameobj(GameObj *obj, int push_dir)
{
	if(gameobj_check_properties(obj, OBJPROP_MOVABLE) != OBJPROP_MOVABLE)
		return;
	
	gameobj_set_moving(obj, true, push_dir);
	audio_play_sound(SFX_PUSH_BLOCK);
	create_effect_at_position(ET_SMOKE, obj->tile_pos.x, obj->tile_pos.y, 0);
	//create_effect_at_position(obj->tile_pos.x, obj->tile_pos.y);
}

bool objint_launch_gameobj(GameObj *obj, int launch_dir)
{
	
	if(gameobj_is_player(obj))
	{
		return playerobj_launch(launch_dir);
	}
	else
	{
		GameObj *tongue_obj = tongue_get_attached_object();
		if(obj == tongue_obj)
		{
			tongue_detach_obj();
			tongue_retract();
		}
		Vector2 launch_vec = dir_to_vec(launch_dir);
		Vector2 tile_start = obj->tile_pos;
		Vector2 tile_end;
		vec2_set(&tile_end, tile_start.x + launch_vec.x, tile_start.y + launch_vec.y);
		tile_end = map_constrain_vector(tile_end);
		if((tile_end.x == tile_start.x)&&(tile_end.y == tile_start.y))
		{
			return false;
		}
		if(get_tile_properties(tile_end.x, tile_end.y) & TILEPROP_SOLID)
		{
			return false;
		}
		GameObj *contents = get_tile_contents(tile_end.x, tile_end.y);
		if(contents != NULL && gameobj_check_properties(contents, OBJPROP_SOLID))
		{
			return false;
		}

		gameobj_add_property_flags(obj, OBJPROP_LAUNCHED);
		gameobj_set_moving(obj, true, launch_dir);
		return true;
	}
}

// collect a (valid) game object
void objint_collect(GameObj *target, GameObj *instigator)
{
	if(target == NULL) return;
	if(gameobj_check_properties(target, OBJPROP_PICKUP) != OBJPROP_PICKUP)
		return;
	// TODO: Apply Collect Effect
	audio_play_sound(SFX_COLLECT_COIN);
	time_charge_increase(1);
	create_effect_at_position(ET_SPARKLE, instigator->tile_pos.x, instigator->tile_pos.y, 0);
	remove_tile_contents(target, target->tile_pos.x, target->tile_pos.y);
	gameobj_erase(target);
}



bool objint_check_floor_tile(GameObj *obj, int tile_x, int tile_y)
{
	// hacky fix, replace
//	if(gameobj_is_player(obj))
//		return true;
		
	// floor objs cannot interact with the floor for obvious reasons
	if(gameobj_check_properties(obj, OBJPROP_FLOOROBJ))
		return true;

	bool tile_safe = true;
	ushort props = get_tile_properties(tile_x, tile_y);
	GameObj *floor_obj = get_tile_floor_contents(tile_x, tile_y);
	u16 fo_props = gameobj_check_properties(floor_obj, 0xFFFF);
	if(props & TILEPROP_PAIN)
	{
		tile_safe = false;
	}
	else if((props & TILEPROP_HOLE) && !(fo_props & OBJPROP_SOLID))
	{
		gameobj_fall(obj, tile_x, tile_y);
		tile_safe = false;
	}
	if(floor_obj != NULL)
	{
		tile_safe &= objint_step_on(floor_obj, obj);
	}
	return tile_safe;
}

// step on a floor object
bool objint_step_on(GameObj *target, GameObj *instigator)
{
	if(target == NULL) return true;
	bool safe = true;
	if(target->interact != NULL)
	{
		safe = target->interact(target, instigator);
	}
	// TODO: returning false causes huge slowdown (bc its getting called every frame) so i need to fix that before removing this
	//return true;
	return safe;
}


// take damage
void objint_deal_damage(GameObj *target, GameObj *instigator)
{
	if(target == NULL) return;
	if(gameobj_is_player(target))
	{
		playerhealth_take_damage();
	}
}




void gameobj_fall(GameObj *obj, int tile_x, int tile_y)
{
	// hacky fix, replace
	if(gameobj_is_player(obj))
		return;

	if(tongue_get_attached_object() == obj)
	{
		tongue_detach_obj();
		tongue_retract();
	}

	//ushort props = get_tile_properties(tile_x, tile_y);
	//set_tile_properties(tile_x, tile_y, props & ~TILEPROP_HOLE);
	remove_tile_contents(obj, tile_x, tile_y);
	set_floor_contents(obj, tile_x, tile_y);
	//TODO: ADD SPRITE OF OBJ IN THE HOLE
	gameobj_play_anim(obj);
	audio_play_sound(SFX_FALL);
	//gameobj_hide(obj);
	gameobj_add_property_flags(obj, OBJPROP_FLOOROBJ);
	gameobj_set_layer_priority(obj, LAYER_FLOOROBJ);
}

////////////////////////////
/// Interactable Objects ///
////////////////////////////

GameObj *intobj_create_coin_at_position(int x, int y, u16 props)
{
	Vector2 c_pos;
	vec2_set(&c_pos, x, y);
	int c_tile = mem_load_tiles(spr_coinTiles, spr_coinTilesLen);
	int pal = PAL_ID_OBJS;
	if(props & OBJPROP_TIME_IMMUNITY)
		pal = PAL_ID_OBJS_TIME_IMMUNE;
	props = props|OBJPROP_CANGRAB|OBJPROP_PICKUP;
	GameObj *coin = gameobj_init_dynamic(ATTR0_SQUARE, ATTR1_SIZE_16x16, pal, c_tile, c_pos, false, props);
	register_obj_history(coin);
	gameobj_set_sprite_offset(coin,0,2);
	place_obj_in_tile(coin, x, y);
	AnimationData *coin_anim = animdata_create(c_tile, 4, 4, 0);
	gameobj_set_anim_data(coin, coin_anim, ANIM_FLAG_LOOPING);
	gameobj_play_anim(coin);
	return coin;
}

GameObj *intobj_create_crate_at_position(int x, int y, u16 props)
{
	Vector2 c_pos;
	vec2_set(&c_pos, x, y);
	int c_tile = mem_load_tiles(obj_crateTiles, obj_crateTilesLen);
	int pal = PAL_ID_OBJS;
	if(props & OBJPROP_TIME_IMMUNITY)
		pal = PAL_ID_OBJS_TIME_IMMUNE;
	props = props|OBJPROP_SOLID|OBJPROP_CANGRAB|OBJPROP_MOVABLE;
	GameObj *crate = gameobj_init_dynamic(ATTR0_TALL, ATTR1_SIZE_16x32, pal, c_tile, c_pos, false, props);
	crate->hist = register_obj_history(crate);
	crate->hist->update_func = crate_update_spr;
	gameobj_set_sprite_offset(crate,0,8);
	place_obj_in_tile(crate, x, y);
	AnimationData *fall_anim = animdata_create(c_tile, 8, 5, 0);
	gameobj_set_anim_data(crate, fall_anim, ANIM_FLAG_CLAMP | ANIM_FLAG_LOCK);
	return crate;
}



void crate_update_spr(GameObj *obj)
{
	anim_stop(&obj->anim);
	if(gameobj_check_properties(obj, OBJPROP_FLOOROBJ))
	{
		gameobj_set_layer_priority(obj, LAYER_FLOOROBJ);
		obj->anim.cur_frame = obj->anim.anim_data->frame_ct-1;
		gameobj_set_sprite_id(obj, obj->base_spr_info + (obj->anim.anim_data->tile_offset * (obj->anim.anim_data->frame_ct-1)));
	}
	else
	{
		gameobj_set_layer_priority(obj, LAYER_GAMEOBJ);
		obj->anim.cur_frame = 0;
		gameobj_set_sprite_id(obj, obj->base_spr_info);
	}
}


/////////////////////
/// Floor Objects ///
/////////////////////


GameObj *floorobj_create_victory_tile_at_position(int x, int y)
{
	int vt = mem_load_tiles(victory_tileTiles, victory_tileTilesLen);
	Vector2 vt_pos;
	vt_pos.x = x;
	vt_pos.y = y;
	u16 props = OBJPROP_FLOOROBJ|OBJPROP_TIME_IMMUNITY;
	GameObj *vic_tile = gameobj_init_dynamic(ATTR0_SQUARE, ATTR1_SIZE_16x16, PAL_ID_OBJS, vt, vt_pos, false, props);
	place_obj_in_tile_floor(vic_tile, x, y);
	AnimationData *vt_anim = animdata_create(vt, 4, 4, 0);
	gameobj_set_anim_data(vic_tile, vt_anim, ANIM_FLAG_LOOPING);
	gameobj_play_anim(vic_tile);
	vic_tile->interact = floorint_victory;
	return vic_tile;
}


GameObj *floorobj_create_launch_tile_at_position(int x, int y, int facing)
{
	int lt = mem_load_tiles(launch_tileTiles, launch_tileTilesLen);
	Vector2 lt_pos;
	vec2_set(&lt_pos, x, y);
	u16 props = OBJPROP_LAUNCHPAD|OBJPROP_FLOOROBJ|OBJPROP_TIME_IMMUNITY;
	GameObj *launch_tile = gameobj_init_dynamic(ATTR0_SQUARE, ATTR1_SIZE_16x16, PAL_ID_OBJS, lt, lt_pos, false, props);
	place_obj_in_tile_floor(launch_tile, x, y);
	gameobj_set_facing(launch_tile, facing);
	AnimationData *lt_anim = animdata_create(lt, 4, 4, 4);
	gameobj_set_anim_data(launch_tile, lt_anim, ANIM_FLAG_LOOPING);
	gameobj_play_anim(launch_tile);
	launch_tile->interact = floorint_launch;
	return launch_tile;
}


GameObj *floorobj_create_spikes_at_position(int x, int y)
{
	int s_tile = mem_load_tiles(spikesTiles, spikesTilesLen);
	Vector2 s_pos;
	s_pos.x = x;
	s_pos.y = y;
	GameObj *spikes = gameobj_init_dynamic(ATTR0_SQUARE, ATTR1_SIZE_16x16, PAL_ID_OBJS, s_tile, s_pos, false, 0);
	place_obj_in_tile_floor(spikes, x, y);
	spikes->interact = floorint_spikes;
	return spikes;
}



////////////////////////////////
/// Floor Interact Functions ///
////////////////////////////////

bool floorint_launch(GameObj *self, GameObj *instigator)
{
	return !objint_launch_gameobj(instigator, gameobj_get_facing(self));
}

bool floorint_spikes(GameObj *self, GameObj *instigator)
{
	objint_deal_damage(instigator, self);	// intentional flip of parameters
	return true;
}

bool floorint_victory(GameObj *self, GameObj *instigator)
{
	if(gameobj_is_player(instigator) && !playerobj_is_intangible())
	{
		playerobj_victory_start();
		return false;
	}
	return true;
}