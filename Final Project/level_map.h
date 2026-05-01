#ifndef LEVEL_MAP_H
#define LEVEL_MAP_H

namespace LevelMap {
void Initialize();
void Shutdown();
void ResetCamera();
void HandleMouseButtonDown(int x, int y);
void HandleMouseButtonUp(int x, int y);
void HandleMouseClick(int x, int y);
bool HandleAnyKeyDown();
bool IsBossBattleActive();
void Draw();
}

#endif
