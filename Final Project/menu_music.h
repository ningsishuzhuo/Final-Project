#ifndef MENU_MUSIC_H
#define MENU_MUSIC_H

#include "globals.h"

namespace MenuMusic {

void Initialize();
void Shutdown();
void OnStateChanged(GameState state);
int GetBackgroundVolumeLevel();
int GetGameVolumeLevel();
void AdjustBackgroundVolume(int delta);
void AdjustGameVolume(int delta);

}  

#endif  
