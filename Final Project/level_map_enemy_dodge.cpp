#include "level_map_internal.h"
#include "level_map_enemy_movement_internal.h"

#include <cmath>

namespace LevelMapInternal {
namespace {

float DistanceSquared(float dx, float dy) {
    return dx * dx + dy * dy;
}

bool IsDodgeTargetReached() {
    const float dx = static_cast<float>(g_enemy.dodgeTargetX) - g_enemy.exactX;
    const float dy = static_cast<float>(g_enemy.dodgeTargetY) - g_enemy.exactY;
    return std::fabs(dx) < 0.01f && std::fabs(dy) < 0.01f;
}

void FaceTowardDodgeTarget() {
    if (g_enemy.dodgeTargetX > g_enemy.x + kEnemyFaceTurnDeadzone) {
        g_enemy.faceRight = true;
    }
    else if (g_enemy.dodgeTargetX < g_enemy.x - kEnemyFaceTurnDeadzone) {
        g_enemy.faceRight = false;
    }
}

bool IsMeleeEnemyTooCloseToDodge(const GameData::EnemyDefinition& enemyDef) {
    if (enemyDef.attackMode != GameData::EnemyAttackMode::Melee) {
        return false;
    }

    const float dxToPlayer = static_cast<float>(g_playerX - g_enemy.x);
    const float dyToPlayer = static_cast<float>(g_playerY - g_enemy.y);
    return DistanceSquared(dxToPlayer, dyToPlayer) <
        kSnowApeProjectileDodgeMinDistance * kSnowApeProjectileDodgeMinDistance;
}

bool ProjectileCanThreatenEnemy(
    const Projectile& projectile,
    const GameData::EnemyCombatParams& params,
    float& outPerpX,
    float& outPerpY) {
    const float relX = static_cast<float>(g_enemy.x) - projectile.x;
    const float relY = static_cast<float>(g_enemy.y) - projectile.y;
    const float dodgeDetectRadiusSquared = static_cast<float>(params.dodgeDetectRadius * params.dodgeDetectRadius);
    if (DistanceSquared(relX, relY) > dodgeDetectRadiusSquared) {
        return false;
    }

    const float velocitySquared = DistanceSquared(projectile.vx, projectile.vy);
    if (velocitySquared < 0.01f) {
        return false;
    }

    const float t = (relX * projectile.vx + relY * projectile.vy) / velocitySquared;
    if (t < 0.0f || t > params.dodgePredictionFrames) {
        return false;
    }

    const float closestX = projectile.x + projectile.vx * t;
    const float closestY = projectile.y + projectile.vy * t;
    const float missX = static_cast<float>(g_enemy.x) - closestX;
    const float missY = static_cast<float>(g_enemy.y) - closestY;
    const float dodgeLaneHalfWidthSquared = params.dodgeLaneHalfWidth * params.dodgeLaneHalfWidth;
    if (DistanceSquared(missX, missY) > dodgeLaneHalfWidthSquared) {
        return false;
    }

    outPerpX = -projectile.vy;
    outPerpY = projectile.vx;
    const float perpLen = std::sqrt(DistanceSquared(outPerpX, outPerpY));
    if (perpLen < 0.01f) {
        return false;
    }

    outPerpX /= perpLen;
    outPerpY /= perpLen;
    if (missX * outPerpX + missY * outPerpY < 0.0f) {
        outPerpX = -outPerpX;
        outPerpY = -outPerpY;
    }
    return true;
}

bool TrySetDodgeTarget(float perpX, float perpY, int dodgeStep) {
    const int dodgeX = static_cast<int>(perpX * static_cast<float>(dodgeStep));
    const int dodgeY = static_cast<int>(perpY * static_cast<float>(dodgeStep));
    if (dodgeX == 0 && dodgeY == 0) {
        return false;
    }

    g_enemy.dodgeTargetX = g_enemy.x + dodgeX;
    g_enemy.dodgeTargetY = g_enemy.y + dodgeY;
    ClampPointToIceRegion(g_enemy.dodgeTargetX, g_enemy.dodgeTargetY);
    if (g_enemy.dodgeTargetX == g_enemy.x && g_enemy.dodgeTargetY == g_enemy.y) {
        return false;
    }

    g_enemy.dodging = true;
    g_enemy.faceRight = (dodgeX >= 0);
    return true;
}

}  

bool UpdateEnemyDodgeMovement(const GameData::EnemyCombatParams& params, const GameData::EnemyDefinition& enemyDef) {
    if (!g_enemy.dodging) {
        return false;
    }

    const float speed = GetEnemyMoveSpeed(static_cast<float>(params.dodgeMoveSpeed));

    if (IsDodgeTargetReached()) {
        g_enemy.dodging = false;
        return false;
    }

    if (MoveEnemyTowardPoint(g_enemy.dodgeTargetX, g_enemy.dodgeTargetY, speed)) {
        g_enemy.dodging = false;
    }

    FaceTowardDodgeTarget();
    ClampEnemyToIceRegion(enemyDef);
    return true;
}

bool TryEnemyDodgePlayerProjectile(
    const GameData::EnemyCombatParams& params,
    const GameData::EnemyDefinition& enemyDef) {
    if (g_enemy.dodging || g_enemy.attackState != EnemyInstance::AttackState::Idle) {
        return false;
    }

    if (IsMeleeEnemyTooCloseToDodge(enemyDef)) {
        return false;
    }

    for (const Projectile& projectile : g_playerProjectiles) {
        if (!projectile.active) {
            continue;
        }

        float perpX = 0.0f;
        float perpY = 0.0f;
        if (!ProjectileCanThreatenEnemy(projectile, params, perpX, perpY)) {
            continue;
        }

        if (TrySetDodgeTarget(perpX, perpY, params.dodgeStep)) {
            return true;
        }
    }

    return false;
}

}  
