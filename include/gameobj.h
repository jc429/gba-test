#ifndef GAMEOBJ_H
#define GAMEOBJ_H

#include "vector2.h"
#include "layers.h"
#include "animation.h"


#define OBJ_USE_MASK		0x80
#define OBJ_ID_MASK			0x7F

typedef struct struct_GameObj {
	u8 game_obj_id;						// first bit: in use or not || other 7 bits: actual obj id
	OBJ_ATTR *attr;

	//u16 attr0;						// AABC DDEE FFFF FFFF || A = SHAPE, B = COLORMODE, C = MOSAIC, D = GFXMODE, E = OBJMODE, F = YPOS
	//u16 attr1;						// AABC xxxD DDDD DDDD || A = SIZE, B = VFLIP, C = HFLIP, D = XPOS || BCxxx = AFFINDEX
	//u16 attr2;						// AAAA BBCC CCCC CCCC || A = PALBANK, B = PRIORITY, C = TILEINDEX
	
	u16 base_sprite_id;					// needs to be stored separately from attr2 to handle animations etc || SIZE: 10 bits
	// these are literally attr2
	//u16 spr_tile_id;					// index of upperleft tile in obj memory (SIZE: 10 bits)
	//u8 pal_bank_id;						// index of palette in pal memory (SIZE: 4 bits)
	//u8 layer_priority;					// draw order layer_priority in layer (0 = drawn on top) (SIZE: 2 bits)

	//u8 spr_shape;						// shape of sprite (SIZE: 2 bits)
	//u8 spr_size;						// size of sprite (SIZE: 2 bits)
	
	Vector2 tile_pos;					// position on map (in tiles) || ignored in FIXED_POS mode
	Vector2 pixel_pos;					// position relative to tile (in pixels) or position on screen if in FIXED_POS mode
	Vector2 spr_off;					// offset from top left pixel of sprite to top left corner of its position

	u16 obj_properties;					// flags for various gameplay-related properties of a given gameobj

	Animation anim;						// animation info
	struct struct_ObjHistory *hist;		// object history - used for time travel
} GameObj;

inline bool get_obj_used(GameObj *obj)
{
	return (obj->game_obj_id & OBJ_USE_MASK) > 0;
};

inline bool get_obj_id(GameObj *obj)
{
	return (obj->game_obj_id & OBJ_ID_MASK);
};

/////////////////////////
/// Object Properties ///
/////////////////////////
#define OBJPROP_SOLID			0x0001		// does the object take up a tile, or can the player/another object step on it?
#define OBJPROP_PICKUP			0x0002		// does the object get destroyed when the player eats/steps on it?
#define OBJPROP_MOVABLE			0x0004		// can the player push this object by walking into it?
#define OBJPROP_MOVING			0x0008		// is the object currently moving

#define OBJPROP_EDIBLE			0x0010		// can the frog consume this?

#define OBJPROP_TIME_IMMUNITY	0x0200		// grants immunity to time-based shenanigans
#define OBJPROP_HIDDEN			0x0400		// when activated the obj will disappear from view (and collision)
#define OBJPROP_FIXED_POS		0x0800		// does the object remain in a fixed position on screen? (mostly for UI elements)
// last 4 bits are treated a little differently
#define OBJPROP_FACING_MASK		0x3000		// bits 13+14 refer to the direction the GameObj is facing (see direction.h for specific values)
#define OBJPROP_FACING_BIT_OFFSET	12
#define OBJPROP_MOVING_MASK		0xC000		// bits 15+16 refer to the direction the GameObj is moving (see direction.h for specific values)
#define OBJPROP_MOVING_BIT_OFFSET	14

inline u16 objprop_ignore_time(GameObj *obj)
{
	return (obj->obj_properties & (OBJPROP_FIXED_POS | OBJPROP_HIDDEN | OBJPROP_TIME_IMMUNITY));
};



////////////////////////////////////////
/// Inline functions for Sprite Data ///
////////////////////////////////////////
	//u16 spr_tile_id;						// index of upperleft tile in obj memory (SIZE: 10 bits)
	//u8 pal_bank_id;						// index of palette in pal memory (SIZE: 4 bits)
	//u8 layer_priority;					// draw order layer_priority in layer (0 = drawn on top) (SIZE: 2 bits)

inline u16 gameobj_get_sprite_id(GameObj *obj)
{	return ((obj->attr->attr2)>>ATTR2_ID_SHIFT)&ATTR2_ID_MASK;	};

inline void gameobj_set_sprite_id(GameObj *obj, u16 spr_id)
{	obj->attr->attr2 = ((obj->attr->attr2 & ~ATTR2_ID_MASK) | ATTR2_ID(spr_id&ATTR2_ID_MASK));	};

inline void gameobj_set_base_spr_id(GameObj *obj, u16 spr_id)
{	obj->base_sprite_id = spr_id;	};

inline u16 gameobj_get_base_spr_id(GameObj *obj)
{	return obj->base_sprite_id;	};

inline u8 gameobj_get_pal_id(GameObj *obj)
{	return ((obj->attr->attr2)>>ATTR2_PALBANK_SHIFT)&0x0F;	};

inline void gameobj_set_pal_id(GameObj *obj, u8 pal)
{	obj->attr->attr2 = ((obj->attr->attr2 & ~ATTR2_PALBANK_MASK) | ATTR2_PALBANK(pal&0x0F));	};

inline u8 gameobj_get_layer_priority(GameObj *obj)
{	return ((obj->attr->attr2)>>ATTR2_PRIO_SHIFT)&0x03;	};

inline void gameobj_set_layer_priority(GameObj *obj, u8 layer)
{	obj->attr->attr2 = ((obj->attr->attr2 & ~ATTR2_PRIO_MASK) | ATTR2_PRIO(layer&0x03));	};


inline u16 gameobj_get_sprite_shape(GameObj *obj)
{	return (obj->attr->attr0 & ATTR0_SHAPE_MASK);	};

inline void gameobj_set_sprite_shape(GameObj *obj, u16 shape)
{	obj->attr->attr0 = ((obj->attr->attr0 & ~ATTR0_SHAPE_MASK) | (shape & ATTR0_SHAPE_MASK));	};

inline u16 gameobj_get_sprite_size(GameObj *obj)
{	return (obj->attr->attr1 & ATTR1_SIZE_MASK);	};

inline void gameobj_set_sprite_size(GameObj *obj, u16 size)
{	obj->attr->attr1 = ((obj->attr->attr1 & ~ATTR1_SIZE_MASK) | (size & ATTR1_SIZE_MASK));	};



//////////////////////////////////////////////////////////////////////////////////////////////////////////


GameObj *gameobj_init();
GameObj *gameobj_init_full(u16 layer_priority, u16 attr0_shape, u16 attr1_size, u8 palbank, u16 spr_id, int x, int y, u16 properties);
GameObj *gameobj_duplicate(GameObj *src);															// duplicate a GameObj into another slot in memory
GameObj *gameobj_clone(GameObj *dest, GameObj *src);												// copy all attributes of a GameObj into another existing GameObj
void gameobj_erase(GameObj *obj);																	// wipe all attributes of a GameObj and mark it as unused
void gameobj_erase_all();																			// wipe all attributes of all GameObjs

void gameobj_main_update(GameObj *obj);
void gameobj_update_attr(GameObj *obj);
void gameobj_update_attr_full(GameObj *obj, u16 attr0_shape, u16 attr1_size, u8 palbank, u16 spr_id, int x, int y, u16 properties);

void gameobj_set_property_flags(GameObj *obj, u16 properties);
void gameobj_add_property_flags(GameObj *obj, u16 properties);
void gameobj_remove_property_flags(GameObj *obj, u16 properties);
u16 gameobj_check_properties(GameObj *obj, u16 properties);
u16 gameobj_get_properties(GameObj *obj);
void gameobj_unhide(GameObj *obj);
void gameobj_hide(GameObj *obj);

// Graphics Functions
void gameobj_set_anim_data(GameObj *obj, AnimationData *anim_data, u8 flags);
void gameobj_play_anim(GameObj *obj);
void gameobj_set_sprite_offset(GameObj *obj, int x, int y);
void gameobj_flip_h(GameObj *obj);														// flip an object horizontally across the vertical axis
void gameobj_flip_v(GameObj *obj);														// flip an object vertically across the horizontal axis
void gameobj_set_flip(GameObj *obj, bool flip_h, bool flip_v);							// set the horizontal and vertical flip state
void gameobj_set_flip_h(GameObj *obj, bool flip_h);										// set the horizontal flip state
void gameobj_set_flip_v(GameObj *obj, bool flip_v);										// set the vertical flip state

// Position Functions
void gameobj_set_tile_pos(GameObj *obj, int x, int y);
void gameobj_set_tile_pos_by_id(GameObj *obj, int tile_id);
void gameobj_set_pixel_pos(GameObj *obj, int x, int y);
void gameobj_change_pixel_pos(GameObj *obj, int move_x, int move_y);					// move a GameObj by (x,y) pixels
Vector2 gameobj_get_pixel_pos(GameObj *obj);											// get the pixel position of a GameObj as a Vector2
void gameobj_update_current_tile(GameObj *obj);											// remove obj from old tile, update tile + pixel vectors, assign obj to new tile

// Facing Functions
void gameobj_set_facing(GameObj *obj, int facing);
int gameobj_get_facing(GameObj *obj);

// Movement  Functions
void gameobj_set_move_dir(GameObj *obj, int move_dir);
int gameobj_get_move_dir(GameObj *obj);
void gameobj_set_moving(GameObj *obj, bool moving, int move_dir);
void gameobj_set_moving_vec(GameObj *obj, bool moving, Vector2 move_dir);
bool gameobj_is_moving(GameObj *obj);													
bool gameobj_all_at_rest();																// returns true when all GameObjs have finished moving



void gameobj_hide_all();
void gameobj_unhide_all();

bool gameobj_is_player(GameObj *obj);													// checks whether or not a given GameObj is the PlayerObj



#endif //GAMEOBJ_H