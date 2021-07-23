#include "effects.h"
#include "regmem.h"
#include "direction.h"
#include "palettes.h"
#include "game.h"
#include "gameobj.h"
#include "sprites/effects/eff_dust.h"
#include "sprites/effects/eff_smoke.h"
#include "sprites/effects/eff_sparkle.h"
#include "sprites/effects/eff_teleport.h"

typedef struct struct_Effect {
	bool in_use;						// is this effect currently in use?
	byte eff_duration;					// how long the effect will last before fading, in anim_frames
	byte eff_timer;						// remaining life of effect
	GameObj *obj;						// effect GameObj data
}Effect;

void effects_init();
void effects_anim_update();
Effect *get_free_effect();

GameObj *eff_templates[ET_COUNT];		// hidden templates for the different effects
const byte eff_durations[ET_COUNT] = {5, 5, 5, 5};
#define EFFECTS_MAX 8
Effect effects[EFFECTS_MAX];			// effects that will be visible in game


void effects_init()
{
	// init free effects
	for(int i = 0; i < EFFECTS_MAX; i++)
	{
		effects[i].in_use = false;
		effects[i].eff_duration = 0;
		effects[i].eff_timer = 0;
		effects[i].obj = gameobj_init();
	}

	

	// dust
	int dust_tile = mem_load_tiles(eff_dustTiles, eff_dustTilesLen);
	Vector2 eff_pos;
	eff_templates[ET_DUST] = gameobj_init_full(LAYER_GAMEOBJ, ATTR0_SQUARE, ATTR1_SIZE_8x8, PAL_ID_EFF, dust_tile, eff_pos, false, 0);
	gameobj_set_sprite_offset(eff_templates[ET_DUST], -8, -8);
	AnimationData *dust_animdata = animdata_create(dust_tile, ANIM_OFFSET_8x8, 4, 0);
	gameobj_set_anim_data(eff_templates[ET_DUST], dust_animdata, ANIM_FLAG_CLAMP);
	gameobj_hide(eff_templates[ET_DUST]);

	// init smoke_cloud
	int smoke_tile = mem_load_tiles(eff_smokeTiles, eff_smokeTilesLen);
	eff_templates[ET_SMOKE] = gameobj_init_full(LAYER_GAMEOBJ, ATTR0_SQUARE, ATTR1_SIZE_16x16, PAL_ID_EFF, smoke_tile, eff_pos, false, 0);
	gameobj_set_sprite_offset(eff_templates[ET_SMOKE], 0, 0);
	AnimationData *smoke_animdata = animdata_create(smoke_tile, ANIM_OFFSET_16x16, 4, 0);
	gameobj_set_anim_data(eff_templates[ET_SMOKE], smoke_animdata, 0);
	gameobj_hide(eff_templates[ET_SMOKE]);

	// init sparkle
	int sparkle_tile = mem_load_tiles(eff_sparkleTiles, eff_sparkleTilesLen);
	eff_templates[ET_SPARKLE] = gameobj_init_full(LAYER_GAMEOBJ, ATTR0_SQUARE, ATTR1_SIZE_16x16, PAL_ID_EFF, sparkle_tile, eff_pos, false, 0);
	gameobj_set_sprite_offset(eff_templates[ET_SPARKLE], 0, 0);
	AnimationData *sparkle_animdata = animdata_create(sparkle_tile, ANIM_OFFSET_16x16, 4, 0);
	gameobj_set_anim_data(eff_templates[ET_SPARKLE], sparkle_animdata, 0);
	gameobj_hide(eff_templates[ET_SPARKLE]);

	// init teleport
	int tele_tile = mem_load_tiles(eff_teleportTiles, eff_teleportTilesLen);
	eff_templates[ET_TELEPORT] = gameobj_init_full(LAYER_GAMEOBJ, ATTR0_SQUARE, ATTR1_SIZE_16x16, PAL_ID_EFF, tele_tile, eff_pos, false, 0);
	gameobj_set_sprite_offset(eff_templates[ET_TELEPORT], 0, 0);
	AnimationData *tele_animdata = animdata_create(tele_tile, ANIM_OFFSET_16x16, 4, 0);
	gameobj_set_anim_data(eff_templates[ET_TELEPORT], tele_animdata, 0);
	gameobj_hide(eff_templates[ET_TELEPORT]);
}


void effects_anim_update()
{
	for(int i = 0; i < EFFECTS_MAX; i++)
	{
		if(effects[i].in_use && effects[i].eff_timer > 0)
		{
			effects[i].eff_timer--;
			if(effects[i].eff_timer <= 0)
			{
				gameobj_hide(effects[i].obj);
				effects[i].in_use = false;
			}
		}
	}
}

Effect *get_free_effect()
{
	for(int i = 0; i < EFFECTS_MAX; i++)
	{
		if(!effects[i].in_use)
		{
			effects[i].in_use = true;
			return &effects[i];
		}
	}
	return NULL;
}




void create_effect_at_tile(EffectType eff_type, int tile_id)
{
	Effect *eff = get_free_effect();
	// if no free effect slots, dont bother
	if(eff == NULL)
		return;
	gameobj_clone(eff->obj, eff_templates[eff_type]);
	gameobj_set_tile_pos_by_id(eff->obj, tile_id);
	gameobj_unhide(eff->obj);
	gameobj_play_anim(eff->obj);
	eff->eff_duration = eff_durations[eff_type];
	eff->eff_timer = eff->eff_duration;
}

void create_effect_at_position(EffectType eff_type, int tile_x, int tile_y, int facing)
{
	Effect *eff = get_free_effect();
	// if no free effect slots, dont bother
	if(eff == NULL)
		return;
	gameobj_clone(eff->obj, eff_templates[eff_type]);
	gameobj_set_tile_pos(eff->obj, tile_x, tile_y);
	gameobj_unhide(eff->obj);
	if(facing == DIRECTION_WEST)
		gameobj_set_flip_h(eff->obj, true);
	gameobj_play_anim(eff->obj);
	eff->eff_duration = eff_durations[eff_type];
	eff->eff_timer = eff->eff_duration;
}