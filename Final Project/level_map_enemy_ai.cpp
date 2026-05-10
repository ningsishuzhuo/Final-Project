#include "level_map_internal.h"
#include "level_map_battle_flow.h"
#include "level_map_enemy_ai_internal.h"
#include "level_map_enemy_ai_runtime_internal.h"
#include "level_map_stage_config.h"
#include "level_map_enemy_visibility.h"

namespace LevelMapInternal {
namespace {

bool g_bossVisibleForCurrentUpdate = false;

bool IsCurrentEnemyAllowedOnScreen(const GameData::EnemyDefinition& enemyDef) {
    if (g_activeIcefieldEnemyIndex < 0 ||
        g_activeIcefieldEnemyIndex >= static_cast<int>(g_icefieldEnemies.size())) {
        const int halfW = enemyDef.drawWidth / 2;
        const int halfH = enemyDef.drawHeight / 2;
        const int sx = g_enemy.x - g_cameraX;
        const int sy = g_enemy.y - g_cameraY;
        return sx >= -halfW && sx <= (GAME_WINDOW_WIDTH + halfW) &&
            sy >= -halfH && sy <= (GAME_WINDOW_HEIGHT + halfH);
    }
    return IsIcefieldEnemyAllowedOnScreen(g_activeIcefieldEnemyIndex, g_bossVisibleForCurrentUpdate);
}

bool IsCurrentMinerWithinAttackQuota() {
    if (!IsIcefieldNormalBattleActive() || EnemyDef().kind != GameData::EnemyKind::Miner) {
        return true;
    }
    return IsIcefieldMinerWithinAttackQuota(g_activeIcefieldEnemyIndex);
}

float EnemyObstacleRadiusForKind(GameData::EnemyKind kind) {
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

void ResolveAndClampEnemyAgainstRegionAndObstacles(const GameData::EnemyDefinition& enemyDef) {
    const float padding = static_cast<float>(enemyDef.regionPadding);
    float minX = static_cast<float>(g_iceRegionRect.left) + padding;
    float maxX = static_cast<float>(g_iceRegionRect.right) - padding;
    float minY = static_cast<float>(g_iceRegionRect.top) + padding;
    float maxY = static_cast<float>(g_iceRegionRect.bottom) - padding;

    if (minX > maxX) {
        const float centerX = static_cast<float>(g_iceRegionRect.left + g_iceRegionRect.right) * 0.5f;
        minX = centerX;
        maxX = centerX;
    }
    if (minY > maxY) {
        const float centerY = static_cast<float>(g_iceRegionRect.top + g_iceRegionRect.bottom) * 0.5f;
        minY = centerY;
        maxY = centerY;
    }

    if (g_enemy.exactX < minX) g_enemy.exactX = minX;
    if (g_enemy.exactX > maxX) g_enemy.exactX = maxX;
    if (g_enemy.exactY < minY) g_enemy.exactY = minY;
    if (g_enemy.exactY > maxY) g_enemy.exactY = maxY;

    const float obstacleRadius = EnemyObstacleRadiusForKind(enemyDef.kind);
    ResolveCircleOutsideObstacles(g_enemy.exactX, g_enemy.exactY, obstacleRadius);

    if (g_enemy.exactX < minX) g_enemy.exactX = minX;
    if (g_enemy.exactX > maxX) g_enemy.exactX = maxX;
    if (g_enemy.exactY < minY) g_enemy.exactY = minY;
    if (g_enemy.exactY > maxY) g_enemy.exactY = maxY;

    RefreshEnemyGridPosition();
}

void UpdateActiveEnemyAIInternal(ULONGLONG now);

void UpdateIcefieldEnemyWaveAIImpl(ULONGLONG now) {
    for (size_t i = 0; i < g_icefieldEnemies.size(); ++i) {
        IcefieldEnemy& enemy = g_icefieldEnemies[i];
        if (!enemy.runtime.alive || enemy.runtime.hp <= 0) {
            continue;
        }

        g_activeIcefieldEnemyIndex = static_cast<int>(i);
        g_enemyKind = enemy.kind;
        SetEnemyTier(enemy.tier);
        g_enemy = enemy.runtime;
        g_enemySawPlayerLastFrame = enemy.sawPlayerLastFrame;
        UpdateActiveEnemyAIInternal(now);
        enemy.runtime = g_enemy;
        enemy.sawPlayerLastFrame = g_enemySawPlayerLastFrame;
    }
}

void UpdateActiveEnemyAIInternal(ULONGLONG now) {
    const GameData::EnemyDefinition& enemyDef = EnemyDef();
    const GameData::EnemyCombatParams params = EnemyParamsForCurrentEnemy(now);
    if (!g_enemy.alive) {
        ResetEnemyCombatRuntimeState();
        return;
    }

    ResolveAndClampEnemyAgainstRegionAndObstacles(enemyDef);

    const bool screenAllowed = IsCurrentEnemyAllowedOnScreen(enemyDef);
    if (!screenAllowed) {
        g_enemy.moving = false;
        g_enemy.dodging = false;
        if (enemyDef.attackMode == GameData::EnemyAttackMode::Melee) {
            ResetEnemyCombatRuntimeState();
        }
        g_enemySawPlayerLastFrame = false;
        return;
    }

    const float dx = static_cast<float>(g_playerX - g_enemy.x);
    const float dy = static_cast<float>(g_playerY - g_enemy.y);
    const float distSquared = dx * dx + dy * dy;
    // 迟滞距离用于避免可见性在边界反复抖动。
    const float trackingDistance = g_enemySawPlayerLastFrame ? kEnemyLosePlayerDistance : kEnemyAcquirePlayerDistance;
    bool canSeePlayer = distSquared <= trackingDistance * trackingDistance;
    if (!canSeePlayer && enemyDef.kind == GameData::EnemyKind::SnowApeKing) {
        canSeePlayer = screenAllowed;
    }
    const bool attackEnabled = screenAllowed && IsCurrentMinerWithinAttackQuota();

    if (!canSeePlayer) {
        g_enemy.moving = false;
        g_enemy.dodging = false;
        if (enemyDef.attackMode == GameData::EnemyAttackMode::Melee) {
            ResetEnemyCombatRuntimeState();
        }
        g_enemySawPlayerLastFrame = false;
        return;
    }

    if (enemyDef.kind == GameData::EnemyKind::SnowApeKing) {
        UpdateSnowApeKingAI(dx, dy, now, params, enemyDef, attackEnabled);
        return;
    }

    switch (enemyDef.attackMode) {
    case GameData::EnemyAttackMode::Ranged:
        UpdateRangedEnemyAI(dx, dy, distSquared, now, params, enemyDef, attackEnabled);
        break;
    case GameData::EnemyAttackMode::Melee:
        UpdateMeleeEnemyAI(dx, dy, distSquared, now, params, enemyDef, attackEnabled);
        break;
    case GameData::EnemyAttackMode::Boss:
        UpdateRangedEnemyAI(dx, dy, distSquared, now, params, enemyDef, attackEnabled);
        break;
    }
}

}

void UpdateIcefieldEnemyWaveAI(ULONGLONG now) {
    UpdateIcefieldEnemyWaveAIImpl(now);
}

void UpdateEnemyAI(ULONGLONG now) {
    const IcefieldStageConfig& stageConfig = GetIcefieldStageConfig();
    if (g_battlePhase == BattlePhase::NormalFight) {
        g_bossVisibleForCurrentUpdate = false;
        UpdateIcefieldWaveRuntime(now);
        if (CountAliveIcefieldEnemies() < stageConfig.bossSwitchAliveThreshold) {
            StartIcefieldBossBattle(now);
            SpawnIcefieldNormalWave(now);
            return;
        }
        ActivateNearestAliveIcefieldEnemy();
        return;
    }

    g_bossVisibleForCurrentUpdate = UpdateBossBattleWaveAndRestoreBoss(now);
    UpdateActiveEnemyAIInternal(now);
}

}



