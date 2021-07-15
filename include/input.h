#ifndef INPUT_H
#define INPUT_H

#include <tonc.h>

typedef enum LockFlag_T {
	INPLCK_SYS		= 0x01,
	INPLCK_UI		= 0x02,
	INPLCK_PLAYER	= 0x04,
	INPLCK_TONGUE	= 0x08,
	INPLCK_PAUSE	= 0x10,
	INPLCK_TIMER	= 0x20
} LockFlag;

bool input_locked();
byte input_current_lock();
void input_unlock_override_all();

void input_lock(byte lock_flag);
void input_unlock(byte lock_flag);

#define KEY_NONDIR KEY_FIRE|KEY_SHOULDER|KEY_SPECIAL

// if a direction is held less than this number of frames, the player will only change their facing direction instead of moving
#define INPUT_MOV_BUFFER	7

#endif //INPUT_H