#include "level_map_internal.h"
#include "level_map_obstacle_internal.h"

namespace LevelMapInternal {

std::vector<MapObstacle>& Obstacles() {
    static std::vector<MapObstacle> obstacles;
    return obstacles;
}

void AddObstacle(
    MapObstacleKind kind,
    float x,
    float y,
    int width,
    int height,
    bool destructible) {
    MapObstacle obstacle;
    obstacle.kind = kind;
    obstacle.x = x;
    obstacle.y = y;
    obstacle.width = width;
    obstacle.height = height;
    obstacle.solid = (kind != MapObstacleKind::Rail);
    obstacle.destructible = destructible;
    obstacle.destroyed = false;
    obstacle.hp = destructible ? 1 : 9999;
    Obstacles().push_back(obstacle);
}

bool IsWallKind(MapObstacleKind kind) {
    return kind == MapObstacleKind::IceWall || kind == MapObstacleKind::OreWall;
}

bool IsWallOrCrateKind(MapObstacleKind kind) {
    return IsWallKind(kind) || kind == MapObstacleKind::Crate;
}

int MinDimension(int width, int height) {
    return (width < height) ? width : height;
}

const std::vector<MapObstacle>& GetMapObstacles() {
    return Obstacles();
}

void ResetMapObstacles() {
    BuildObstacleLayout();
}

}  
