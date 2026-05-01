#ifndef LEVEL_MAP_COMBAT_INTERNAL_H
#define LEVEL_MAP_COMBAT_INTERNAL_H

#include <windows.h>

namespace LevelMapInternal {

void NormalizeAimDirection(float& dirX, float& dirY);
void UpdatePlayerFacingByAim(float dirX);
bool IsPointInFrontArc(float relX, float relY, float dirX, float dirY, float range);
bool IsPointInCircle(float relX, float relY, float radius);
float DistanceSquaredToSegment(
    float px,
    float py,
    float x1,
    float y1,
    float x2,
    float y2,
    float& outNearestRelX,
    float& outNearestRelY);
void ApplyDamageToCurrentEnemy(int damage, bool allowSunCriticalPassive = false);
void SpawnApolloSlashAttack(int clickScreenX, int clickScreenY);

}  

#endif  
