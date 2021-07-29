#include "gamesettings.h"
#include "gamedata.h"
#include "tonc.h"

void gamesettings_set_defaults();





void gamesettings_init()
{
	gamesettings_set_defaults();
}



void gamesettings_load_all()
{

}

void gamesettings_save_all()
{

}


#define SETTING_AUDIO_VOLUME_MIN			0
#define SETTING_AUDIO_VOLUME_MAX			16
#define SETTING_AUDIO_VOLUME_DEFAULT		12

static int setting_volume;

int gamesettings_audio_volume_get()
{
	return setting_volume;
}

void gamesettings_audio_volume_set(int volume)
{
	setting_volume = clamp(volume, SETTING_AUDIO_VOLUME_MIN, SETTING_AUDIO_VOLUME_MAX);
}


// move sensitivity dictates how long a direction needs to be held to move in that direction, if the direction is released within the sensitivity window, the player will just turn in place
#define SETTING_MOVE_SENS_MIN		2	
#define SETTING_MOVE_SENS_MAX		30
#define SETTING_MOVE_SENS_DEFAULT	7

static int move_sensitivity;


int gamesettings_move_sensitivity_get()
{
	return move_sensitivity;
}

void gamesettings_move_sensitivity_set(int move_sens)
{
	move_sensitivity = move_sens;
}






void gamesettings_set_defaults()
{
	gamesettings_audio_volume_set(SETTING_AUDIO_VOLUME_DEFAULT);
	gamesettings_move_sensitivity_set(SETTING_MOVE_SENS_DEFAULT);
}
