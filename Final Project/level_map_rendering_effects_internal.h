#ifndef LEVEL_MAP_RENDERING_EFFECTS_INTERNAL_H
#define LEVEL_MAP_RENDERING_EFFECTS_INTERNAL_H

#include "level_map_types.h"

#include <graphics.h>
#include <windows.h>

#include <vector>

namespace LevelMapInternal {

bool IsWithinLayerRange(float y, int minYInclusive, int maxYExclusive);
bool IsScreenVisible(int centerX, int centerY, int radiusX, int radiusY);
void DrawProjectileArrayLayered(
    const std::vector<Projectile>& arr,
    const IMAGE& image,
    bool hasAlpha,
    int w,
    int h,
    int minYInclusive,
    int maxYExclusive);
void DrawEnemyProjectileArrayLayered(int w, int h, int minYInclusive, int maxYExclusive);
void DrawEnemyShockwavesLayered(ULONGLONG now, int minYInclusive, int maxYExclusive);
void DrawDashAfterimages(ULONGLONG now);
void DrawMoonRainSkillEffects(ULONGLONG now);
void DrawMoonUltimateFireballEffects(ULONGLONG now, bool drawAura, bool drawFireball);
void DrawSunSwordQiProjectiles();
void DrawGrid();

}  

#endif  
