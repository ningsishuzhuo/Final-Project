#ifndef LEVEL_MAP_OBSTACLE_API_H
#define LEVEL_MAP_OBSTACLE_API_H

#include "level_map_types.h"

#include <vector>

namespace LevelMapInternal {

const std::vector<MapObstacle>& GetMapObstacles();
void LoadMapObstacleAssets();
void FreeMapObstacleAssets();
void ResetMapObstacles();
void DrawMapObstacles();
void DrawGroundObstacles();
void DrawMapObstaclesLayered(int minYInclusive, int maxYExclusive);

void ResolvePlayerPositionAgainstObstacles(int previousX, int previousY);
void ResolveEnemyPositionAgainstObstacles(float previousX, float previousY, float radius);
bool HandleProjectileObstacleHit(Projectile& projectile);
void ApplyMeleeObstacleHitInFrontArc(float originX, float originY, float dirX, float dirY, float range);
void ApplyCircleObstacleHit(float centerX, float centerY, float radius);
void ResolvePointOutsideObstacles(float& x, float& y);
bool IsCircleBlockedByObstacles(float x, float y, float radius);
void ResolveCircleOutsideObstacles(float& x, float& y, float radius);

}  

#endif  
