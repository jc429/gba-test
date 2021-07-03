#include <maxmod.h>
#include <tonc.h>
#include "audio.h"
#include "debug.h"


static bool muted;

static inline bool audio_muted() { return (muted | DEBUG_MUTE_ALL ); }
static inline bool songs_muted() { return (muted | DEBUG_MUTE_SONGS | DEBUG_MUTE_ALL); }
static inline bool sfx_muted() { return (muted | DEBUG_MUTE_SFX | DEBUG_MUTE_ALL); }



void audio_init()
{
	muted = false;
	// Initialize maxmod with default settings
	// pass soundbank address, and allocate 8 channels.
	mmInitDefault((mm_addr)soundbank_bin, 8);
}


void audio_update()
{
	//test
}

void audio_mute()
{
	muted = true;
}

void audio_unmute()
{
	muted = false;
}

void audio_stop()
{
	mmStop();
}


void audio_play_track(int track_id)
{
	if(songs_muted()) return;

	if(track_id < 0 || track_id >= MSL_NSONGS)
		track_id = 0;

	if(track_id == MOD_TITLE_THEME)
	{
		mmSetModuleVolume(384);
		mmSetModuleTempo(512);
	}
	mmStart(track_id, MM_PLAY_LOOP);
}


void audio_play_sound(int sound_id)
{
	if(sfx_muted()) return;

	if(sound_id < 0 || sound_id >= MSL_NSAMPS)
		sound_id = 0;

	mmEffect(sound_id);
}