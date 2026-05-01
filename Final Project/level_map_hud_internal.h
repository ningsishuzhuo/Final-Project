#ifndef LEVEL_MAP_HUD_INTERNAL_H
#define LEVEL_MAP_HUD_INTERNAL_H

#include <windows.h>

namespace LevelMapInternal {

void DrawUltimateCooldownHud(ULONGLONG now);
void DrawMinimap();
void DrawHud(ULONGLONG now);
void DrawGameOverOverlay(ULONGLONG now);
void DrawVictoryOverlay(ULONGLONG now);

}  

#endif  
