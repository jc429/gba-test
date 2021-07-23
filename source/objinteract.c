#include "objinteract.h"
#include "objhistory.h"
#include "effects.h"
#include "map.h"
#include "audio.h"
#include "palettes.h"
#include "regmem.h"
#include "layers.h"


#include "sprites/objects/coin.h"
#include "sprites/objects/obj_crate.h"
#include "sprites/objects/victory_tile.h"
#include "sprites/objects/spikes.h"

// playerhealth.c
extern void playerhealth_take_damage();
// playertongue.c
extern void tongue_detach_obj();
extern void tongue_retract();
extern GameObj *tongue_get_attached_object();

//int oi_pal;

void objint_init();
void crate_update_spr(GameObj *obj);

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

// collect a (valid) game object
void objint_collect(GameObj *target, GameObj *instigator)
{
	if(target == NULL) return;
	if(gameobj_check_properties(target, OBJPROP_PICKUP) != OBJPROP_PICKUP)
		return;
	// TODO: Apply Collect Effect
	audio_play_sound(SFX_COLLECT_COIN);
	time_charge_increase(1);
	create_effect_at_position(ET_SMOKE, target->tile_pos.x, target->tile_pos.y, 0);
	remove_tile_contents(target, target->tile_pos.x, target->tile_pos.y);
	gameobj_erase(target);
}

// step on a floor object
void objint_step_on(GameObj *target, GameObj *instigator)
{
	if(target == NULL) return;

	// if spikes, damage instigator
	objint_deal_damage(instigator, target);		// switching instigator and target here is intentional

	//create_effect_at_position(ET_SMOKE, target->tile_pos.x, target->tile_pos.y);
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



bool objint_check_floor_tile(GameObj *obj, int tile_x, int tile_y)
{
	// hacky fix, replace
	if(gameobj_is_player(obj))
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
	return tile_safe;
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

GameObj *intobj_create_coin_at_position(int x, int y)
{
	Vector2 c_pos;
	vec2_set(&c_pos, x, y);
	int c_tile = mem_load_tiles(spr_coinTiles, spr_coinTilesLen);
	GameObj *coin = gameobj_init_full(LAYER_GAMEOBJ, ATTR0_SQUARE, ATTR1_SIZE_16x16, PAL_ID_OBJS, c_tile, c_pos, false, OBJPROP_PICKUP);
	register_obj_history(coin);
	gameobj_set_sprite_offset(coin,0,2);
	place_obj_in_tile(coin, x, y);
	AnimationData *coin_anim = animdata_create(c_tile, 4, 4, 0);
	gameobj_set_anim_data(coin, coin_anim, ANIM_FLAG_LOOPING);
	gameobj_play_anim(coin);
	return coin;
}

GameObj *intobj_create_crate_at_position(int x, int y)
{
	Vector2 c_pos;
	vec2_set(&c_pos, x, y);
	int c_tile = mem_load_tiles(obj_crateTiles, obj_crateTilesLen);
	GameObj *crate = gameobj_init_full(LAYER_GAMEOBJ, ATTR0_TALL, ATTR1_SIZE_16x32, PAL_ID_OBJS, c_tile, c_pos, false, OBJPROP_SOLID|OBJPROP_MOVABLE);
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
	GameObj *vic_tile = gameobj_init_full(LAYER_GAMEOBJ, ATTR0_SQUARE, ATTR1_SIZE_16x16, PAL_ID_OBJS, vt, vt_pos, false, OBJPROP_TIME_IMMUNITY);
	place_obj_in_tile_floor(vic_tile, x, y);
	AnimationData *vt_anim = animdata_create(vt, 4, 4, 0);
	gameobj_set_anim_data(vic_tile, vt_anim, ANIM_FLAG_LOOPING);
	gameobj_play_anim(vic_tile);
	return vic_tile;
}


GameObj *floorobj_create_spikes_at_position(int x, int y)
{
	int s_tile = mem_load_tiles(spikesTiles, spikesTilesLen);
	Vector2 s_pos;
	s_pos.x = x;
	s_pos.y = y;
	GameObj *spikes = gameobj_init_full(LAYER_GAMEOBJ, ATTR0_SQUARE, ATTR1_SIZE_16x16, PAL_ID_OBJS, s_tile, s_pos, false, 0);
	place_obj_in_tile_floor(spikes, x, y);
	
	return spikes;
}

