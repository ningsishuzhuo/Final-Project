#include "level_map_internal.h"
#include "level_map_enemy_ai_runtime_internal.h"
#include "level_map_enemy_movement_internal.h"

#include <cmath>

namespace LevelMapInternal {
namespace {

void GetSnowApeAttackAnchor(int& targetX, int& targetY) {
    targetX = g_playerX;
    targetY = g_playerY;

    const float dx = static_cast<float>(g_playerX - g_enemy.x);
    const float dy = static_cast<float>(g_playerY - g_enemy.y);
    const float dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 0.01f) {
        targetX += g_enemy.faceRight ? -static_cast<int>(kSnowApeAttackAnchorDistance) : static_cast<int>(kSnowApeAttackAnchorDistance);
    }
    else {
        targetX = g_playerX - static_cast<int>(std::round(dx / dist * kSnowApeAttackAnchorDistance));
        targetY = g_playerY - static_cast<int>(std::round(dy / dist * kSnowApeAttackAnchorDistance));
    }

    ClampPointToIceRegion(targetX, targetY);
}

void StartEnemyWindup(ULONGLONG now, const GameData::EnemyCombatParams& params) {
    g_enemy.attackState = EnemyInstance::AttackState::Windup;
    g_enemy.slamImpactApplied = false;
    g_enemy.moving = false;
    g_enemy.dodging = false;
    g_enemy.slamTargetX = g_playerX;
    g_enemy.slamTargetY = g_playerY;
    g_enemy.chargeStopX = g_playerX;
    g_enemy.chargeStopY = g_playerY;
    g_enemy.attackStateEndTick = now + 380ULL;
    g_enemy.nextAttackTick = now + static_cast<ULONGLONG>(params.attackIntervalMs);
    UpdateEnemyFacingTowardPlayer();
}

void StartEnemyCharge(const float dx, const float dy, ULONGLONG now, const GameData::EnemyCombatParams& params) {
    const float len = std::sqrt(dx * dx + dy * dy);
    if (len < 1.0f) {
        return;
    }

    g_enemy.attackState = EnemyInstance::AttackState::Charging;
    g_enemy.slamImpactApplied = false;
    g_enemy.dodging = false;
    const float chargeSpeed = GetEnemyMoveSpeed(static_cast<float>(params.chargeSpeed));
    const float stopDistance = kSnowApeAttackStopDistance;
    g_enemy.chargeStopX = g_enemy.slamTargetX - static_cast<int>(dx / len * stopDistance);
    g_enemy.chargeStopY = g_enemy.slamTargetY - static_cast<int>(dy / len * stopDistance);
    ClampPointToIceRegion(g_enemy.chargeStopX, g_enemy.chargeStopY);
    g_enemy.chargeVx = dx / len * chargeSpeed;
    g_enemy.chargeVy = dy / len * chargeSpeed;
    g_enemy.attackStateEndTick = now + static_cast<ULONGLONG>(params.chargeDurationMs);
    g_enemy.faceRight = (g_enemy.chargeVx >= 0.0f);
}

void StartEnemySlam(ULONGLONG now, const GameData::EnemyCombatParams& params) {
    g_enemy.attackState = EnemyInstance::AttackState::Slamming;
    g_enemy.slamImpactApplied = false;
    g_enemy.moving = false;
    g_enemy.chargeVx = 0.0f;
    g_enemy.chargeVy = 0.0f;
    g_enemy.slamTargetX = g_playerX;
    g_enemy.slamTargetY = g_playerY;
    g_enemy.attackStateEndTick = now + static_cast<ULONGLONG>(params.slamDurationMs);
}

}  

void UpdateMeleeEnemyAI(
    const float dx,
    const float dy,
    const float distSquared,
    ULONGLONG now,
    const GameData::EnemyCombatParams& params,
    const GameData::EnemyDefinition& enemyDef,
    bool attackEnabled) {

    if (!attackEnabled) {
        if (g_enemy.attackState != EnemyInstance::AttackState::Idle) {
            ResetEnemyCombatRuntimeState();
        }
        g_enemy.moving = false;
        g_enemy.dodging = false;
        g_enemySawPlayerLastFrame = true;
        return;
    }

    if (g_enemy.attackState == EnemyInstance::AttackState::Windup) {
        g_enemy.moving = false;
        UpdateEnemyFacingTowardPlayer();
        if (now >= g_enemy.attackStateEndTick) {
            const float chargeDx = static_cast<float>(g_enemy.slamTargetX - g_enemy.x);
            const float chargeDy = static_cast<float>(g_enemy.slamTargetY - g_enemy.y);
            StartEnemyCharge(chargeDx, chargeDy, now, params);
        }
        return;
    }

    if (g_enemy.attackState == EnemyInstance::AttackState::Slamming) {
        g_enemy.moving = false;
        UpdateEnemyFacingTowardPlayer();
        if (!g_enemy.slamImpactApplied && now >= g_enemy.attackStateEndTick) {
            const ShockwaveInstance::Variant shockwaveVariant =
                g_enemy.breakStateActive ? ShockwaveInstance::Variant::Empowered : ShockwaveInstance::Variant::Normal;
            SpawnEnemyShockwave(static_cast<float>(g_playerX), static_cast<float>(g_playerY), false, shockwaveVariant);
            g_enemy.slamImpactApplied = true;
            g_enemy.attackState = EnemyInstance::AttackState::Idle;
        }
        return;
    }

    if (g_enemy.attackState == EnemyInstance::AttackState::Charging) {
        g_enemy.moving = true;
        const bool reachedStopPoint = MoveEnemyTowardPoint(
            g_enemy.chargeStopX,
            g_enemy.chargeStopY,
            static_cast<float>(params.chargeSpeed));
        g_enemy.faceRight = (g_enemy.chargeVx >= 0.0f);
        ClampEnemyToIceRegion(enemyDef);

        const float stopDx = static_cast<float>(g_enemy.chargeStopX - g_enemy.x);
        const float stopDy = static_cast<float>(g_enemy.chargeStopY - g_enemy.y);
        const float stopDist = std::sqrt(stopDx * stopDx + stopDy * stopDy);
        if (reachedStopPoint || stopDist <= 20.0f || now >= g_enemy.attackStateEndTick) {
            StartEnemySlam(now, params);
        }
        return;
    }

    g_enemy.moving = false;
    const bool dodgingThisFrame = UpdateEnemyDodgeMovement(params, enemyDef);
    if (!dodgingThisFrame && TryEnemyDodgePlayerProjectile(params, enemyDef)) {
        g_enemy.nextRepositionTick = now + 140;
        return;
    }
    if (dodgingThisFrame) {
        g_enemy.moving = true;
        return;
    }

    if (distSquared <= kSnowApeAttackStartDistance * kSnowApeAttackStartDistance && now >= g_enemy.nextAttackTick) {
        StartEnemyWindup(now, params);
        return;
    }

    int attackAnchorX = g_playerX;
    int attackAnchorY = g_playerY;
    GetSnowApeAttackAnchor(attackAnchorX, attackAnchorY);

    if (distSquared > kSnowApeMeleeCommitDistance * kSnowApeMeleeCommitDistance) {
        const bool reachedPlayer = MoveEnemyTowardPoint(
            attackAnchorX,
            attackAnchorY,
            GetEnemyMoveSpeed(static_cast<float>(params.chaseSpeed)));
        g_enemy.moving = !reachedPlayer;
    }
    else if (distSquared > kSnowApeCloseHoldDistance * kSnowApeCloseHoldDistance) {
        const bool reachedAttackRange = MoveEnemyTowardPoint(
            attackAnchorX,
            attackAnchorY,
            GetEnemyMoveSpeed(static_cast<float>(params.patrolSpeed)));
        g_enemy.moving = !reachedAttackRange;
    }
    else {
        const bool reachedStablePoint = MoveEnemyTowardPoint(
            attackAnchorX,
            attackAnchorY,
            g_enemy.breakStateActive ? 0.75f : 0.55f);
        g_enemy.moving = !reachedStablePoint;
    }

    UpdateEnemyFacingTowardPlayer();
    ClampEnemyToIceRegion(enemyDef);
    g_enemySawPlayerLastFrame = true;
}

}  
