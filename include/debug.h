#ifndef DEBUG_H
#define DEBUG_H



#define DEBUG_SKIP_INTROS 		0			// set to skip straight to gameplay
#define DEBUG_MUTE_SONGS 		0			// set to mute audio tracks
#define DEBUG_MUTE_SFX 			0			// set to mute sound effects
#define DEBUG_MUTE_ALL			1			// set to mute everything
#define DEBUG_CLEAR_SRAM		0			// set to wipe SRAM upon game boot
#define DEBUG_UNLOCK_ALL		1			// unlock all levels
#define DEBUG_UNLIMITED_HP		1			// hp will not deplete
#define DEBUG_UNLIMITED_GAUGE	1			// time gauge will not deplete

void debug_write_text(char *str);
void debug_write_int(int num);

#endif //DEBUG_H