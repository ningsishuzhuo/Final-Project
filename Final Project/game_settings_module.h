#ifndef GAME_SETTINGS_MODULE_H
#define GAME_SETTINGS_MODULE_H

#include "globals.h"

namespace GameSettingsModule {

void Initialize();
void Shutdown();
void EnterSettings();
void Draw();
void HandleMouseMove(int x, int y);
void HandleMouseClick(int x, int y, GameState& currentState);

}  

#endif  
