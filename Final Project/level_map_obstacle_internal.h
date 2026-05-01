#ifndef LEVEL_MAP_OBSTACLE_INTERNAL_H
#define LEVEL_MAP_OBSTACLE_INTERNAL_H

#include "level_map_types.h"

#include <vector>

namespace LevelMapInternal {

constexpr int kWallAndCrateSize = 88;
constexpr int kRailSize = kWallAndCrateSize * 3;
constexpr float kGridSnapForWallOrCrate = kWallAndCrateSize * 0.5f;

std::vector<MapObstacle>& Obstacles();
void AddObstacle(
    MapObstacleKind kind,
    float x,
    float y,
    int width,
    int height,
    bool destructible);
bool IsWallKind(MapObstacleKind kind);
bool IsWallOrCrateKind(MapObstacleKind kind);
int MinDimension(int width, int height);
void BuildObstacleLayout();

}  

#endif  
