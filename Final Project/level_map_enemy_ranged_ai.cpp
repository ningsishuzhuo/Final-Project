#include "level_map_internal.h"
#include "level_map_enemy_ai_runtime_internal.h"
#include "level_map_enemy_movement_internal.h"

#include <cmath>
#include <cstdlib>

namespace LevelMapInternal {

void UpdateRangedEnemyAI(
    const float dx,
    const float dy,
    const float distSquared,
    ULONGLONG now,
    const GameData::EnemyCombatParams& params,
    const GameData::EnemyDefinition& enemyDef,
    bool attackEnabled) {
    g_enemy.moving = true;

    const bool dodgingThisFrame = UpdateEnemyDodgeMovement(params, enemyDef);
    if (!dodgingThisFrame && TryEnemyDodgePlayerProjectile(params, enemyDef)) {
        g_enemy.nextRepositionTick = now + 160;
    }
    else if (dodgingThisFrame) {
        g_enemy.moving = true;
    }
    else if (distSquared < params.preferredMinDistance * params.preferredMinDistance) {
        float moveX = 0.0f;
        float moveY = 0.0f;
        if (dx > 5.0f) moveX -= static_cast<float>(params.chaseSpeed);
        else if (dx < -5.0f) moveX += static_cast<float>(params.chaseSpeed);
        if (dy > 5.0f) moveY -= static_cast<float>(params.chaseSpeed);
        else if (dy < -5.0f) moveY += static_cast<float>(params.chaseSpeed);
        OffsetEnemyPosition(moveX, moveY);
    }
    else {
        if (now >= g_enemy.nextRepositionTick || (std::abs(g_enemy.roamTargetX - g_enemy.x) < 12 && std::abs(g_enemy.roamTargetY - g_enemy.y) < 12)) {
            UpdateEnemyRoamTarget(params);
            g_enemy.nextRepositionTick = now + static_cast<ULONGLONG>(params.repositionIntervalMs);
        }

        const int tx = g_enemy.roamTargetX - g_enemy.x;
        const int ty = g_enemy.roamTargetY - g_enemy.y;
        float moveX = 0.0f;
        float moveY = 0.0f;
        if (tx > 4) moveX += static_cast<float>(params.patrolSpeed);
        else if (tx < -4) moveX -= static_cast<float>(params.patrolSpeed);
        if (ty > 4) moveY += static_cast<float>(params.patrolSpeed);
        else if (ty < -4) moveY -= static_cast<float>(params.patrolSpeed);

        if (distSquared > params.preferredMaxDistance * params.preferredMaxDistance) {
            if (dx > 8.0f) moveX += static_cast<float>(params.patrolSpeed);
            else if (dx < -8.0f) moveX -= static_cast<float>(params.patrolSpeed);
            if (dy > 8.0f) moveY += static_cast<float>(params.patrolSpeed);
            else if (dy < -8.0f) moveY -= static_cast<float>(params.patrolSpeed);
        }

        ApplyMinerSeparationForce(moveX, moveY);
        const float maxStep = GetEnemyMoveSpeed(static_cast<float>(params.chaseSpeed));
        const float moveLenSquared = moveX * moveX + moveY * moveY;
        if (moveLenSquared > maxStep * maxStep && moveLenSquared > 0.001f) {
            const float moveLen = std::sqrt(moveLenSquared);
            const float scale = maxStep / moveLen;
            moveX *= scale;
            moveY *= scale;
        }

        OffsetEnemyPosition(moveX, moveY);
    }

    UpdateEnemyFacingTowardPlayer();
    ClampEnemyToIceRegion(enemyDef);

    if (g_enemy.nextAttackTick == 0 || g_enemy.nextAttackTick > now + 60000ULL || g_enemy.nextAttackTick + 60000ULL < now) {
        g_enemy.nextAttackTick = now + ((EnemyDef().kind == GameData::EnemyKind::Miner) ? 420ULL : 180ULL);
    }
    if (!g_enemySawPlayerLastFrame) {
        g_enemy.nextAttackTick = now + ((EnemyDef().kind == GameData::EnemyKind::Miner) ? 620ULL : 220ULL);
    }
    g_enemySawPlayerLastFrame = true;

    if (attackEnabled && now >= g_enemy.nextAttackTick) {
        SpawnEnemyProjectile(dx, dy);
        ULONGLONG nextTick = now + static_cast<ULONGLONG>(params.attackIntervalMs);
        if (EnemyDef().kind == GameData::EnemyKind::Miner) {
            const unsigned int seed = static_cast<unsigned int>(
                (g_enemy.x * 131) ^ (g_enemy.y * 57) ^ static_cast<int>(now & 0x3FFULL));
            const ULONGLONG minerExtraCooldown = 380ULL + static_cast<ULONGLONG>(seed % 420U);
            nextTick += minerExtraCooldown;
        }
        g_enemy.nextAttackTick = nextTick;
    }
}

}  
