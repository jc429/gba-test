#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

void gamesettings_init();

void gamesettings_load_all();
void gamesettings_save_all();

int gamesettings_audio_volume_get();
void gamesettings_audio_volume_set(int volume);
int gamesettings_move_sensitivity_get();
void gamesettings_move_sensitivity_set(int move_sens);


#endif //GAMESETTINGS_H