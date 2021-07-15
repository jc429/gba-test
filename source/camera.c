#include <tonc.h>
#include "game.h"
#include "map.h"
#include "vector2.h"
#include "gameobj.h"

#define SCREEN_HALF_WIDTH 			120		// half screen width
#define SCREEN_HALF_HEIGHT 			80		// half screen height
#define CAMERA_BOUNDS_HORIZONTAL	56		// 2.5 tiles
#define CAMERA_BOUNDS_VERTICAL		56
#define PLAYER_SIZE					16		// player size in pixels


void set_camera_pos(int target_x, int target_y);
void camera_update_pos();
void camera_set_target(GameObj *target);
void camera_find_target(GameObj *target);
void camera_center();
void camera_push_changes();

extern void set_world_offset(int off_x, int off_y);


extern int world_offset_x;
extern int world_offset_y;

static int cam_bound_h = (SCREEN_HALF_WIDTH) - CAMERA_BOUNDS_HORIZONTAL;
static int cam_bound_v = (SCREEN_HALF_HEIGHT) - CAMERA_BOUNDS_VERTICAL;

static GameObj *cam_target;

void set_camera_pos(int target_x, int target_y)
{
	Vector2 pos;
	//try to constrain player within camera bounds
	pos.x = clamp(world_offset_x + SCREEN_HALF_WIDTH, target_x - cam_bound_h + PLAYER_SIZE, target_x + cam_bound_h);
	pos.y = clamp(world_offset_y + SCREEN_HALF_HEIGHT, target_y - cam_bound_v + PLAYER_SIZE, target_y + cam_bound_v);
	
	//maintain map bounds above all else
	pos.x = clamp(pos.x, SCREEN_HALF_WIDTH, (MAP_SIZE_X * GAME_TILE_SIZE) - SCREEN_HALF_WIDTH);
	pos.y = clamp(pos.y, SCREEN_HALF_HEIGHT, (MAP_SIZE_Y * GAME_TILE_SIZE) - SCREEN_HALF_HEIGHT);
	
	// subtract halfbounds to get top left corner again
	set_world_offset(pos.x - SCREEN_HALF_WIDTH, pos.y - SCREEN_HALF_HEIGHT);
}

void camera_update_pos()
{
	if(cam_target != NULL)
	{
		Vector2 v = gameobj_get_pixel_pos(cam_target);
		set_camera_pos(v.x, v.y);
	}
}

void camera_set_target(GameObj *target)
{
	cam_target = target;
}

void camera_find_target(GameObj *target)
{
	Vector2 v = gameobj_get_pixel_pos(target);
	camera_update_pos(v.x, v.y);
}

void camera_center()
{
	//set_camera_pos(128,128);
	set_world_offset(8, 46);
}
