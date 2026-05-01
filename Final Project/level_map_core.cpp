#include "level_map_internal.h"

#include <array>
#include <cmath>

namespace LevelMapInternal {
namespace {

float CurrentEnemyObstacleRadius() {
    switch (EnemyDef().kind) {
    case GameData::EnemyKind::SnowApeKing:
        return kSnowApeKingObstacleRadius;
    case GameData::EnemyKind::SnowApe:
        return kSnowApeObstacleRadius;
    case GameData::EnemyKind::Miner:
    default:
        return kMinerObstacleRadius;
    }
}

constexpr float kPi = 3.14159265358979323846f;

void ClampEnemyPointToMap(float& x, float& y) {
    const float minX = static_cast<float>(g_iceRegionRect.left);
    const float maxX = static_cast<float>(g_iceRegionRect.right);
    const float minY = static_cast<float>(g_iceRegionRect.top);
    const float maxY = static_cast<float>(g_iceRegionRect.bottom);
    if (x < minX) x = minX;
    if (x > maxX) x = maxX;
    if (y < minY) y = minY;
    if (y > maxY) y = maxY;
}

bool MoveEnemyWithObstacleAvoidance(float targetX, float targetY, float maxStep) {
    if (maxStep <= 0.0f) {
        return false;
    }

    const float radius = CurrentEnemyObstacleRadius();
    ResolveCircleOutsideObstacles(g_enemy.exactX, g_enemy.exactY, radius);
    ClampEnemyPointToMap(g_enemy.exactX, g_enemy.exactY);
    RefreshEnemyGridPosition();

    const float toTargetX = targetX - g_enemy.exactX;
    const float toTargetY = targetY - g_enemy.exactY;
    const float distSquared = toTargetX * toTargetX + toTargetY * toTargetY;
    if (distSquared <= 0.0001f) {
        return true;
    }

    const float distance = std::sqrt(distSquared);
    const float step = (distance < maxStep) ? distance : maxStep;
    if (step <= 0.0001f) {
        return true;
    }

    const float desiredAngle = std::atan2(toTargetY, toTargetX);
    constexpr std::array<float, 25> kAngleOffsets = {
        0.0f,
        15.0f, -15.0f,
        30.0f, -30.0f,
        45.0f, -45.0f,
        60.0f, -60.0f,
        75.0f, -75.0f,
        90.0f, -90.0f,
        105.0f, -105.0f,
        120.0f, -120.0f,
        135.0f, -135.0f,
        150.0f, -150.0f,
        165.0f, -165.0f,
        180.0f
    };
    constexpr std::array<float, 6> kStepScales = { 1.0f, 0.72f, 0.46f, 0.30f, 0.18f, 0.10f };
    constexpr std::array<float, 24> kFullSweepAngles = {
        0.0f, 15.0f, 30.0f, 45.0f, 60.0f, 75.0f,
        90.0f, 105.0f, 120.0f, 135.0f, 150.0f, 165.0f,
        180.0f, 195.0f, 210.0f, 225.0f, 240.0f, 255.0f,
        270.0f, 285.0f, 300.0f, 315.0f, 330.0f, 345.0f
    };
    const float invDistance = 1.0f / distance;
    const float desiredDirX = toTargetX * invDistance;
    const float desiredDirY = toTargetY * invDistance;

    bool hasCandidate = false;
    float bestX = g_enemy.exactX;
    float bestY = g_enemy.exactY;
    float bestScore = 0.0f;

    for (const float stepScale : kStepScales) {
        const float scaledStep = step * stepScale;
        if (scaledStep <= 0.01f) {
            continue;
        }

        for (const float angleOffset : kAngleOffsets) {
            const float radians = desiredAngle + angleOffset * (kPi / 180.0f);
            float candidateX = g_enemy.exactX + std::cos(radians) * scaledStep;
            float candidateY = g_enemy.exactY + std::sin(radians) * scaledStep;
            ClampEnemyPointToMap(candidateX, candidateY);
            if (IsCircleBlockedByObstacles(candidateX, candidateY, radius)) {
                continue;
            }

            const float moveX = candidateX - g_enemy.exactX;
            const float moveY = candidateY - g_enemy.exactY;
            const float moveLen = std::sqrt(moveX * moveX + moveY * moveY);
            if (moveLen <= 0.001f) {
                continue;
            }
            const float moveDirX = moveX / moveLen;
            const float moveDirY = moveY / moveLen;
            const float remainX = targetX - candidateX;
            const float remainY = targetY - candidateY;
            const float remainDistSquared = remainX * remainX + remainY * remainY;
            const float progress = distSquared - remainDistSquared;
            const float alignment = moveDirX * desiredDirX + moveDirY * desiredDirY;
            const float turnPenalty = std::fabs(angleOffset) * 0.004f;
            const float score = remainDistSquared - progress * 0.40f + (1.0f - alignment) * 12.0f + turnPenalty;
            if (!hasCandidate || score < bestScore) {
                hasCandidate = true;
                bestScore = score;
                bestX = candidateX;
                bestY = candidateY;
            }
        }

        if (hasCandidate) {
            break;
        }
    }

    if (!hasCandidate) {
        for (const float stepScale : kStepScales) {
            const float scaledStep = step * stepScale;
            if (scaledStep <= 0.01f) {
                continue;
            }

            for (const float angleDeg : kFullSweepAngles) {
                const float radians = angleDeg * (kPi / 180.0f);
                float candidateX = g_enemy.exactX + std::cos(radians) * scaledStep;
                float candidateY = g_enemy.exactY + std::sin(radians) * scaledStep;
                ClampEnemyPointToMap(candidateX, candidateY);
                if (IsCircleBlockedByObstacles(candidateX, candidateY, radius)) {
                    continue;
                }

                const float moveX = candidateX - g_enemy.exactX;
                const float moveY = candidateY - g_enemy.exactY;
                const float moveLen = std::sqrt(moveX * moveX + moveY * moveY);
                if (moveLen <= 0.001f) {
                    continue;
                }
                const float moveDirX = moveX / moveLen;
                const float moveDirY = moveY / moveLen;
                const float remainX = targetX - candidateX;
                const float remainY = targetY - candidateY;
                const float remainDistSquared = remainX * remainX + remainY * remainY;
                const float progress = distSquared - remainDistSquared;
                const float alignment = moveDirX * desiredDirX + moveDirY * desiredDirY;
                const float score = remainDistSquared - progress * 0.28f + (1.0f - alignment) * 7.5f;
                if (!hasCandidate || score < bestScore) {
                    hasCandidate = true;
                    bestScore = score;
                    bestX = candidateX;
                    bestY = candidateY;
                }
            }

            if (hasCandidate) {
                break;
            }
        }
    }

    if (!hasCandidate) {
        ResolveCircleOutsideObstacles(g_enemy.exactX, g_enemy.exactY, radius);
        ClampEnemyPointToMap(g_enemy.exactX, g_enemy.exactY);
        RefreshEnemyGridPosition();
        return false;
    }

    g_enemy.exactX = bestX;
    g_enemy.exactY = bestY;
    ResolveCircleOutsideObstacles(g_enemy.exactX, g_enemy.exactY, radius);
    ClampEnemyPointToMap(g_enemy.exactX, g_enemy.exactY);
    RefreshEnemyGridPosition();
    return (std::fabs(g_enemy.exactX - targetX) < 0.01f && std::fabs(g_enemy.exactY - targetY) < 0.01f);
}

}

void ClampPlayerToMapBounds() {
    const int radius = kPlayerCollisionRadius;
    int minX = g_iceRegionRect.left + radius;
    int maxX = g_iceRegionRect.right - radius;
    int minY = g_iceRegionRect.top + radius;
    int maxY = g_iceRegionRect.bottom - radius;

    if (minX > maxX) {
        const int centerX = (g_iceRegionRect.left + g_iceRegionRect.right) / 2;
        minX = centerX;
        maxX = centerX;
    }
    if (minY > maxY) {
        const int centerY = (g_iceRegionRect.top + g_iceRegionRect.bottom) / 2;
        minY = centerY;
        maxY = centerY;
    }

    if (g_playerX < minX) g_playerX = minX;
    if (g_playerY < minY) g_playerY = minY;
    if (g_playerX > maxX) g_playerX = maxX;
    if (g_playerY > maxY) g_playerY = maxY;
}

void SetEnemyTier(GameData::EnemyTier tier) {
    g_enemyParams = GameData::GetEnemyCombatParams(GameData::GetEnemyDefinition(g_enemyKind).attackMode, tier);
}

const GameData::EnemyCombatParams& EnemyParams() {
    if (g_enemyParams.hp <= 0) {
        SetEnemyTier(GameData::EnemyTier::Normal);
    }
    return g_enemyParams;
}

GameData::EnemyCombatParams EnemyParamsForCurrentEnemy(ULONGLONG now) {
    GameData::EnemyCombatParams params = EnemyParams();
    if (g_enemy.moonMagicCageEndTick > now) {
        params.chaseSpeed = max(1, params.chaseSpeed * kMoonMagicCageMovePercent / 100);
        params.patrolSpeed = max(1, params.patrolSpeed * kMoonMagicCageMovePercent / 100);
        params.dodgeMoveSpeed = max(1, params.dodgeMoveSpeed * kMoonMagicCageMovePercent / 100);
        params.chargeSpeed = max(1, params.chargeSpeed * kMoonMagicCageMovePercent / 100);
        params.attackIntervalMs *= kMoonMagicCageAttackIntervalMultiplier;
    }
    return params;
}

const GameData::EnemyDefinition& EnemyDef() {
    return GameData::GetEnemyDefinition(g_enemyKind);
}

const ShockwaveTuning& GetShockwaveTuning(ShockwaveInstance::Variant variant) {
    static const ShockwaveTuning kNormal = {
        kNormalShockwaveLockDelayMs,
        kNormalShockwaveTelegraphRadius,
        5,
        60,
        14,
        RGB(34, 84, 144),
        RGB(214, 247, 255),
        RGB(90, 190, 235),
        RGB(40, 115, 185),
        RGB(230, 248, 255)
    };

    static const ShockwaveTuning kEmpowered = {
        kEmpoweredShockwaveLockDelayMs,
        kEmpoweredShockwaveTelegraphRadius,
        7,
        92,
        20,
        RGB(138, 42, 42),
        RGB(255, 182, 182),
        RGB(230, 96, 96),
        RGB(182, 36, 36),
        RGB(255, 222, 222)
    };

    return (variant == ShockwaveInstance::Variant::Empowered) ? kEmpowered : kNormal;
}

void RefreshEnemyGridPosition() {
    g_enemy.x = static_cast<int>(std::round(g_enemy.exactX));
    g_enemy.y = static_cast<int>(std::round(g_enemy.exactY));
}

void SetEnemyExactPosition(float x, float y) {
    g_enemy.exactX = x;
    g_enemy.exactY = y;
    RefreshEnemyGridPosition();
}

void OffsetEnemyPosition(float deltaX, float deltaY) {
    const float moveLengthSquared = deltaX * deltaX + deltaY * deltaY;
    if (moveLengthSquared <= 0.0001f) {
        return;
    }
    const float targetX = g_enemy.exactX + deltaX;
    const float targetY = g_enemy.exactY + deltaY;
    const float maxStep = std::sqrt(moveLengthSquared);
    MoveEnemyWithObstacleAvoidance(targetX, targetY, maxStep);
}

bool MoveEnemyTowardPoint(int targetX, int targetY, float maxStep) {
    return MoveEnemyWithObstacleAvoidance(static_cast<float>(targetX), static_cast<float>(targetY), maxStep);
}

void ResolvePlayerEnemyCollision() {
    if (!g_enemy.alive) {
        return;
    }

    const GameData::EnemyDefinition& enemyDef = EnemyDef();
    const float dx = static_cast<float>(g_playerX - g_enemy.x);
    const float dy = static_cast<float>(g_playerY - g_enemy.y);
    const int enemyCollisionRadius =
        (enemyDef.kind == GameData::EnemyKind::SnowApeKing) ? kSnowApeKingCollisionRadius : kEnemyCollisionRadius;
    const float minDistance = static_cast<float>(kPlayerCollisionRadius + enemyCollisionRadius);
    const float distanceSq = dx * dx + dy * dy;
    if (distanceSq >= minDistance * minDistance) {
        return;
    }

    float distance = std::sqrt(distanceSq);
    float pushX = 1.0f;
    float pushY = 0.0f;
    if (distance > 0.01f) {
        pushX = dx / distance;
        pushY = dy / distance;
    }
    else {
        pushX = (g_faceRight ? -1.0f : 1.0f);
    }

    const float overlap = minDistance - distance;
    const float pushScale = (enemyDef.kind == GameData::EnemyKind::SnowApeKing) ? 1.3f : 1.0f;
    const int previousX = g_playerX;
    const int previousY = g_playerY;
    g_playerX += static_cast<int>(std::round(pushX * overlap * pushScale));
    g_playerY += static_cast<int>(std::round(pushY * overlap * pushScale));
    ClampPlayerToMapBounds();
    ResolvePlayerPositionAgainstObstacles(previousX, previousY);
}

void UpdateCameraToPlayer() {
    g_cameraX = g_playerX - GAME_WINDOW_WIDTH / 2;
    g_cameraY = g_playerY - GAME_WINDOW_HEIGHT / 2;

    const int maxX = (kLevelMapWidth > GAME_WINDOW_WIDTH) ? (kLevelMapWidth - GAME_WINDOW_WIDTH) : 0;
    const int maxY = (kLevelMapHeight > GAME_WINDOW_HEIGHT) ? (kLevelMapHeight - GAME_WINDOW_HEIGHT) : 0;

    if (g_cameraX < 0) g_cameraX = 0;
    if (g_cameraY < 0) g_cameraY = 0;
    if (g_cameraX > maxX) g_cameraX = maxX;
    if (g_cameraY > maxY) g_cameraY = maxY;
}

void BuildRegions() {
    const int insetX = kIceWallThickness;
    const int insetY = kIceWallThickness;
    g_iceRegionRect = {
        insetX,
        insetY,
        kLevelMapWidth - insetX,
        kLevelMapHeight - insetY
    };
}

}  
