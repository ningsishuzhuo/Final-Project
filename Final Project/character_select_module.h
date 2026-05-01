#ifndef CHARACTER_SELECT_MODULE_H
#define CHARACTER_SELECT_MODULE_H

#include "globals.h"

namespace CharacterSelectModule {
void Initialize();
void Shutdown();
void Draw();
void HandleMouseMove(int x, int y);
void HandleMouseClick(int x, int y, GameState& currentState);
int GetSelectedCharacterIndex();
}

#endif
