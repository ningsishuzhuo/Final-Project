#include "level_map_internal.h"
#include "level_map_enemy_visibility.h"
#include "level_map_stage_config.h"

#include <algorithm>

namespace LevelMapInternal {
namespace {

struct OnScreenEnemyLimits {
    int normal = 0;
    int elite = 0;
};

struct ScreenEnemy {
    float distSquared = 0.0f;
    size_t index = 0;
};

bool IsAliveEnemy(const IcefieldEnemy& enemy) {
    return enemy.runtime.alive && enemy.runtime.hp > 0;
}

float DistanceSquaredFromPlayer(int worldX, int worldY) {
    const float dx = static_cast<float>(g_playerX - worldX);
    const float dy = static_cast<float>(g_playerY - worldY);
    return dx * dx + dy * dy;
}

bool IsWorldEntityOnScreen(int worldX, int worldY, int halfW, int halfH) {
    const int sx = worldX - g_cameraX;
    const int sy = worldY - g_cameraY;
    return sx >= -halfW && sx <= (GAME_WINDOW_WIDTH + halfW) &&
        sy >= -halfH && sy <= (GAME_WINDOW_HEIGHT + halfH);
}

OnScreenEnemyLimits CurrentOnScreenEnemyLimits(bool bossVisible) {
    const IcefieldStageConfig& stageConfig = GetIcefieldStageConfig();
    if (bossVisible) {
        return {
            stageConfig.normalVisibleLimitWhenBossVisible,
            stageConfig.eliteVisibleLimitWhenBossVisible
        };
    }
    return { stageConfig.normalVisibleLimit, stageConfig.eliteVisibleLimit };
}

bool IsVisibleIcefieldEnemy(const IcefieldEnemy& enemy) {
    if (!IsAliveEnemy(enemy)) {
        return false;
    }
    const GameData::EnemyDefinition& def = GameData::GetEnemyDefinition(enemy.kind);
    return IsEnemyVisibleOnScreenByState(enemy.runtime, def);
}

size_t VisibleLimitForTier(const OnScreenEnemyLimits& limits, GameData::EnemyTier tier) {
    const int limit = (tier == GameData::EnemyTier::Elite) ? limits.elite : limits.normal;
    return (limit > 0) ? static_cast<size_t>(limit) : 0U;
}

void MarkNearestEnemies(std::vector<ScreenEnemy>& enemies, size_t limit, std::vector<bool>& allowOnScreen) {
    std::sort(
        enemies.begin(),
        enemies.end(),
        [](const ScreenEnemy& a, const ScreenEnemy& b) { return a.distSquared < b.distSquared; });

    const size_t count = (enemies.size() < limit) ? enemies.size() : limit;
    for (size_t i = 0; i < count; ++i) {
        allowOnScreen[enemies[i].index] = true;
    }
}

}  

bool IsEnemyVisibleOnScreenByState(const EnemyInstance& enemy, const GameData::EnemyDefinition& enemyDef) {
    return IsWorldEntityOnScreen(enemy.x, enemy.y, enemyDef.drawWidth / 2, enemyDef.drawHeight / 2);
}

bool IsBossVisibleOnScreenByState(const EnemyInstance& bossState) {
    if (!bossState.alive) {
        return false;
    }
    const GameData::EnemyDefinition& bossDef = GameData::GetEnemyDefinition(GameData::EnemyKind::SnowApeKing);
    return IsEnemyVisibleOnScreenByState(bossState, bossDef);
}

void GetCurrentOnScreenEnemyLimits(bool bossVisible, int& normalLimit, int& eliteLimit) {
    const OnScreenEnemyLimits limits = CurrentOnScreenEnemyLimits(bossVisible);
    normalLimit = limits.normal;
    eliteLimit = limits.elite;
}

void BuildIcefieldVisibleMask(bool bossVisible, std::vector<bool>& allowOnScreen) {
    allowOnScreen.assign(g_icefieldEnemies.size(), false);
    if (g_icefieldEnemies.empty()) {
        return;
    }

    std::vector<ScreenEnemy> normalOnScreen;
    std::vector<ScreenEnemy> eliteOnScreen;
    normalOnScreen.reserve(g_icefieldEnemies.size());
    eliteOnScreen.reserve(g_icefieldEnemies.size());

    for (size_t i = 0; i < g_icefieldEnemies.size(); ++i) {
        const IcefieldEnemy& enemy = g_icefieldEnemies[i];
        if (!IsVisibleIcefieldEnemy(enemy)) {
            continue;
        }

        const ScreenEnemy entry{ DistanceSquaredFromPlayer(enemy.runtime.x, enemy.runtime.y), i };
        if (enemy.tier == GameData::EnemyTier::Elite) {
            eliteOnScreen.push_back(entry);
        }
        else {
            normalOnScreen.push_back(entry);
        }
    }

    const OnScreenEnemyLimits limits = CurrentOnScreenEnemyLimits(bossVisible);
    MarkNearestEnemies(eliteOnScreen, VisibleLimitForTier(limits, GameData::EnemyTier::Elite), allowOnScreen);
    MarkNearestEnemies(normalOnScreen, VisibleLimitForTier(limits, GameData::EnemyTier::Normal), allowOnScreen);
}

bool IsIcefieldEnemyAllowedOnScreen(int enemyIndex, bool bossVisible) {
    if (enemyIndex < 0 || enemyIndex >= static_cast<int>(g_icefieldEnemies.size())) {
        return true;
    }

    const IcefieldEnemy& selfEnemy = g_icefieldEnemies[static_cast<size_t>(enemyIndex)];
    if (!IsAliveEnemy(selfEnemy)) {
        return false;
    }

    if (!IsVisibleIcefieldEnemy(selfEnemy)) {
        return false;
    }

    const float selfDistSquared = DistanceSquaredFromPlayer(selfEnemy.runtime.x, selfEnemy.runtime.y);

    const OnScreenEnemyLimits limits = CurrentOnScreenEnemyLimits(bossVisible);
    const size_t tierLimit = VisibleLimitForTier(limits, selfEnemy.tier);

    size_t closerVisibleSameTierCount = 0;
    for (size_t i = 0; i < g_icefieldEnemies.size(); ++i) {
        if (static_cast<int>(i) == enemyIndex) {
            continue;
        }
        const IcefieldEnemy& other = g_icefieldEnemies[i];
        if (other.tier != selfEnemy.tier || !IsVisibleIcefieldEnemy(other)) {
            continue;
        }

        const float distSquared = DistanceSquaredFromPlayer(other.runtime.x, other.runtime.y);
        if (distSquared < selfDistSquared) {
            ++closerVisibleSameTierCount;
            if (closerVisibleSameTierCount >= tierLimit) {
                return false;
            }
        }
    }

    return true;
}

bool IsIcefieldMinerWithinAttackQuota(int enemyIndex) {
    if (enemyIndex < 0 || enemyIndex >= static_cast<int>(g_icefieldEnemies.size())) {
        return true;
    }

    const IcefieldEnemy& selfEnemy = g_icefieldEnemies[static_cast<size_t>(enemyIndex)];
    if (selfEnemy.kind != GameData::EnemyKind::Miner || !IsAliveEnemy(selfEnemy)) {
        return true;
    }

    const float selfDistSquared = DistanceSquaredFromPlayer(selfEnemy.runtime.x, selfEnemy.runtime.y);

    int closerMinerCount = 0;
    for (size_t i = 0; i < g_icefieldEnemies.size(); ++i) {
        if (static_cast<int>(i) == enemyIndex) {
            continue;
        }

        const IcefieldEnemy& other = g_icefieldEnemies[i];
        if (other.kind != GameData::EnemyKind::Miner || !IsVisibleIcefieldEnemy(other)) {
            continue;
        }

        const float distSquared = DistanceSquaredFromPlayer(other.runtime.x, other.runtime.y);
        if (distSquared < selfDistSquared) {
            ++closerMinerCount;
            if (closerMinerCount >= kMaxMinerAttackersOnScreen) {
                return false;
            }
        }
    }

    return true;
}

}  
