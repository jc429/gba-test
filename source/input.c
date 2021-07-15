#include "input.h"


///////////////////
/// Input Locks ///
///////////////////

static byte inp_lck = 0;

bool input_locked()
{
	return (inp_lck > 0);
}

byte input_current_lock()
{
	return inp_lck;
}


void input_unlock_override_all()
{
	inp_lck = 0;
}

///////////////////////////////

void input_lock(byte lock_flag)
{
	inp_lck |= lock_flag;
}

void input_unlock(byte lock_flag)
{
	inp_lck &= ~lock_flag;
}
