#include "level_map_internal.h"
#include "level_map_enemy_movement_internal.h"

#include <cmath>
#include <cstdlib>

namespace LevelMapInternal {
namespace {

float EnemyObstacleRadius(GameData::EnemyKind kind) {
    switch (kind) {
    case GameData::EnemyKind::SnowApeKing:
        return kSnowApeKingObstacleRadius;
    case GameData::EnemyKind::SnowApe:
        return kSnowApeObstacleRadius;
    case GameData::EnemyKind::Miner:
    default:
        return kMinerObstacleRadius;
    }
}

bool IsAliveMiner(const IcefieldEnemy& enemy) {
    return enemy.kind == GameData::EnemyKind::Miner && enemy.runtime.alive && enemy.runtime.hp > 0;
}

void FaceTowardX(int targetX) {
    if (targetX > g_enemy.x + kEnemyFaceTurnDeadzone) {
        g_enemy.faceRight = true;
    }
    else if (targetX < g_enemy.x - kEnemyFaceTurnDeadzone) {
        g_enemy.faceRight = false;
    }
}

}  

void ClampEnemyToIceRegion(const GameData::EnemyDefinition& enemyDef) {
    const float padding = static_cast<float>(enemyDef.regionPadding);
    const float minX = static_cast<float>(g_iceRegionRect.left) + padding;
    const float maxX = static_cast<float>(g_iceRegionRect.right) - padding;
    const float minY = static_cast<float>(g_iceRegionRect.top) + padding;
    const float maxY = static_cast<float>(g_iceRegionRect.bottom) - padding;

    if (g_enemy.exactX < minX) g_enemy.exactX = minX;
    if (g_enemy.exactX > maxX) g_enemy.exactX = maxX;
    if (g_enemy.exactY < minY) g_enemy.exactY = minY;
    if (g_enemy.exactY > maxY) g_enemy.exactY = maxY;

    ResolveCircleOutsideObstacles(g_enemy.exactX, g_enemy.exactY, EnemyObstacleRadius(enemyDef.kind));

    RefreshEnemyGridPosition();
}

void UpdateEnemyRoamTarget(const GameData::EnemyCombatParams& params) {
    const int minD = static_cast<int>(params.preferredMinDistance);
    const int maxD = static_cast<int>(params.preferredMaxDistance);

    int dirX = (std::rand() % 201) - 100;
    int dirY = (std::rand() % 201) - 100;
    if (dirX == 0 && dirY == 0) dirX = 1;

    const float len = std::sqrt(static_cast<float>(dirX * dirX + dirY * dirY));
    const int dist = minD + (std::rand() % (maxD - minD + 1));

    int tx = 0;
    int ty = 0;
    if (IsIcefieldNormalBattleActive() && EnemyDef().kind == GameData::EnemyKind::Miner) {
        tx = g_enemy.x + static_cast<int>(dirX / len * (dist / 2));
        ty = g_enemy.y + static_cast<int>(dirY / len * (dist / 2));
    }
    else {
        tx = g_playerX + static_cast<int>(dirX / len * dist);
        ty = g_playerY + static_cast<int>(dirY / len * dist);
    }

    ClampPointToIceRegion(tx, ty);

    g_enemy.roamTargetX = tx;
    g_enemy.roamTargetY = ty;
}

void ApplyMinerSeparationForce(float& moveX, float& moveY) {
    if (!IsIcefieldNormalBattleActive() || EnemyDef().kind != GameData::EnemyKind::Miner) {
        return;
    }

    constexpr float kSeparationRadius = 180.0f;
    constexpr float kSeparationRadiusSquared = kSeparationRadius * kSeparationRadius;
    constexpr float kSeparationStrength = 1.35f;

    float separationX = 0.0f;
    float separationY = 0.0f;
    for (const IcefieldEnemy& candidate : g_icefieldEnemies) {
        if (!IsAliveMiner(candidate)) {
            continue;
        }

        const float dx = g_enemy.exactX - candidate.runtime.exactX;
        const float dy = g_enemy.exactY - candidate.runtime.exactY;
        const float distanceSquared = dx * dx + dy * dy;
        if (distanceSquared < 0.5f || distanceSquared > kSeparationRadiusSquared) {
            continue;
        }

        const float distance = std::sqrt(distanceSquared);
        const float invDistance = 1.0f / distance;
        const float weight = (kSeparationRadius - distance) / kSeparationRadius;
        separationX += dx * invDistance * weight;
        separationY += dy * invDistance * weight;
    }

    moveX += separationX * kSeparationStrength;
    moveY += separationY * kSeparationStrength;
}

float GetEnemyMoveSpeed(float baseSpeed) {
    if (g_enemy.breakStateActive &&
        (g_enemyKind == GameData::EnemyKind::SnowApe || g_enemyKind == GameData::EnemyKind::SnowApeKing)) {
        return baseSpeed * kSnowApeBreakSpeedMultiplier;
    }
    return baseSpeed;
}

void UpdateEnemyFacingTowardPlayer() {
    FaceTowardX(g_playerX);
}

void ClampPointToIceRegion(int& x, int& y) {
    const int padding = EnemyDef().regionPadding;
    if (x < g_iceRegionRect.left + padding) x = g_iceRegionRect.left + padding;
    if (x > g_iceRegionRect.right - padding) x = g_iceRegionRect.right - padding;
    if (y < g_iceRegionRect.top + padding) y = g_iceRegionRect.top + padding;
    if (y > g_iceRegionRect.bottom - padding) y = g_iceRegionRect.bottom - padding;
}

}  
