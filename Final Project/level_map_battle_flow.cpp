#include "level_map_internal.h"
#include "level_map_battle_flow.h"
#include "level_map_enemy_visibility.h"
#include "test_toggles.h"

#include <algorithm>
#include <vector>

namespace LevelMapInternal {

int CountAliveIcefieldEnemies() {
    int aliveCount = 0;
    for (const IcefieldEnemy& enemy : g_icefieldEnemies) {
        if (enemy.runtime.alive && enemy.runtime.hp > 0) {
            ++aliveCount;
        }
    }
    return aliveCount;
}

void ReduceInitialNormalWaveToSixEnemies() {
    if (!TestToggles::Get().startNormalBattleWithSixEnemies) {
        return;
    }

    struct Candidate {
        float distSquared = 0.0f;
        size_t index = 0;
    };
    std::vector<Candidate> alive;
    alive.reserve(g_icefieldEnemies.size());
    for (size_t i = 0; i < g_icefieldEnemies.size(); ++i) {
        const IcefieldEnemy& enemy = g_icefieldEnemies[i];
        if (!enemy.runtime.alive || enemy.runtime.hp <= 0) {
            continue;
        }
        const float dx = static_cast<float>(g_playerX - enemy.runtime.x);
        const float dy = static_cast<float>(g_playerY - enemy.runtime.y);
        alive.push_back({ dx * dx + dy * dy, i });
    }

    std::sort(
        alive.begin(),
        alive.end(),
        [](const Candidate& a, const Candidate& b) { return a.distSquared < b.distSquared; });

    std::vector<bool> keep(g_icefieldEnemies.size(), false);
    const size_t keepCount = (alive.size() < 6U) ? alive.size() : 6U;
    for (size_t i = 0; i < keepCount; ++i) {
        keep[alive[i].index] = true;
    }

    for (size_t i = 0; i < g_icefieldEnemies.size(); ++i) {
        if (keep[i]) {
            continue;
        }
        g_icefieldEnemies[i].runtime.alive = false;
        g_icefieldEnemies[i].runtime.hp = 0;
    }
}

void ActivateIcefieldEnemy(int enemyIndex) {
    if (enemyIndex < 0 || enemyIndex >= static_cast<int>(g_icefieldEnemies.size())) {
        return;
    }

    const IcefieldEnemy& selected = g_icefieldEnemies[static_cast<size_t>(enemyIndex)];
    const bool kindChanged = (g_enemyKind != selected.kind);
    g_enemyKind = selected.kind;
    SetEnemyTier(selected.tier);
    if (kindChanged) {
        LoadEnemyAssets();
    }

    g_enemy = selected.runtime;
    g_enemySawPlayerLastFrame = selected.sawPlayerLastFrame;
    g_activeIcefieldEnemyIndex = enemyIndex;
}

void ActivateNearestAliveIcefieldEnemy() {
    const int selectedIndex = FindNearestAliveIcefieldEnemyIndex();
    ActivateIcefieldEnemy(selectedIndex);
}

void InitializeIcefieldNormalBattle(ULONGLONG now) {
    g_battlePhase = BattlePhase::NormalFight;
    g_activeIcefieldEnemyIndex = -1;
    SpawnIcefieldNormalWave(now);
    ReduceInitialNormalWaveToSixEnemies();
    g_enemyProjectiles.clear();
    g_enemyShockwaves.clear();
    g_enemySpikeRows.clear();

    g_enemyKind = GameData::EnemyKind::Miner;
    SetEnemyTier(GameData::EnemyTier::Normal);
    LoadEnemyAssets();

    const int initialIndex = FindNearestAliveIcefieldEnemyIndex();
    if (initialIndex >= 0) {
        ActivateIcefieldEnemy(initialIndex);
    }
    else {
        StartIcefieldBossBattle(now);
    }
}

void UpdateIcefieldWaveRuntime(ULONGLONG now) {
    UpdateIcefieldEnemyWaveAI(now);
    UpdateIcefieldNormalWaveLifecycle();
}

bool UpdateBossBattleWaveAndRestoreBoss(ULONGLONG now) {
    if (g_icefieldEnemies.empty()) {
        return false;
    }

    const EnemyInstance bossState = g_enemy;
    const GameData::EnemyKind bossKind = g_enemyKind;
    const bool bossSawPlayer = g_enemySawPlayerLastFrame;
    const int bossActiveIndex = g_activeIcefieldEnemyIndex;
    const bool bossVisible = IsBossVisibleOnScreenByState(bossState);

    UpdateIcefieldWaveRuntime(now);

    g_enemy = bossState;
    g_enemyKind = bossKind;
    SetEnemyTier(GameData::GetEnemyDefinition(bossKind).tier);
    g_enemySawPlayerLastFrame = bossSawPlayer;
    g_activeIcefieldEnemyIndex = bossActiveIndex;
    return bossVisible;
}

}  
