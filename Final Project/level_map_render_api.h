#ifndef LEVEL_MAP_RENDER_API_H
#define LEVEL_MAP_RENDER_API_H

#include <windows.h>

namespace LevelMapInternal {

void BuildRegions();
void UpdateCameraToPlayer();
void ClampPlayerToMapBounds();

void DrawRegions();
void DrawEnemy();
void DrawPlayer();
void DrawEnergyDropsLayered(int minYInclusive, int maxYExclusive);
void DrawEnemySpikeRowsLayered(int minYInclusive, int maxYExclusive);
void RenderLevelFrame(ULONGLONG now);

void LoadAllCharacterGifs();
void FreeAllCharacterGifs();
void LoadAllCharacterDeathImages();
void FreeAllCharacterDeathImages();
void LoadParticleImage();
void LoadPlayerHpIconImage();
void LoadPlayerArmorIconImage();
void LoadPlayerEnergyIconImage();
void LoadEnergyDropImage();
void UpdatePlayerByKeyboard(ULONGLONG now);

}  

#endif  
