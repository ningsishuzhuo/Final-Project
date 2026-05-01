#include "level_map_internal.h"
#include "level_map_stage_config.h"

#include <cmath>
#include <cstdlib>
#include <vector>

namespace LevelMapInternal {
namespace {

int RandomRangeInclusive(int minValue, int maxValue) {
    if (maxValue <= minValue) {
        return minValue;
    }
    const int span = maxValue - minValue + 1;
    return minValue + (std::rand() % span);
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

}  

void SpawnIcefieldNormalWave(ULONGLONG now) {
    const IcefieldStageConfig& stageConfig = GetIcefieldStageConfig();
    const int totalEnemyCount = GetIcefieldTotalEnemyCount();

    g_icefieldEnemies.clear();
    g_icefieldEnemies.reserve(static_cast<size_t>(totalEnemyCount));

    const int rows = (stageConfig.spawnGridRows > 0) ? stageConfig.spawnGridRows : 1;
    const int cols = (stageConfig.spawnGridCols > 0) ? stageConfig.spawnGridCols : 1;
    const int totalCells = rows * cols;

    const int mapLeft = g_iceRegionRect.left;
    const int mapTop = g_iceRegionRect.top;
    const int mapWidth = g_iceRegionRect.right - g_iceRegionRect.left;
    const int mapHeight = g_iceRegionRect.bottom - g_iceRegionRect.top;
    const int cellW = mapWidth / cols;
    const int cellH = mapHeight / rows;

    std::vector<int> shuffledCells;
    shuffledCells.reserve(static_cast<size_t>(totalCells));
    for (int i = 0; i < totalCells; ++i) {
        shuffledCells.push_back(i);
    }
    for (int i = totalCells - 1; i > 0; --i) {
        const int j = std::rand() % (i + 1);
        const int temp = shuffledCells[static_cast<size_t>(i)];
        shuffledCells[static_cast<size_t>(i)] = shuffledCells[static_cast<size_t>(j)];
        shuffledCells[static_cast<size_t>(j)] = temp;
    }

    std::vector<bool> eliteCell(static_cast<size_t>(totalCells), false);
    for (int i = 0; i < stageConfig.eliteEnemyCount && i < totalCells; ++i) {
        eliteCell[static_cast<size_t>(shuffledCells[static_cast<size_t>(i)])] = true;
    }

    int normalSerial = 1;
    int eliteSerial = 1;

    const int spawnCount = (totalEnemyCount < totalCells) ? totalEnemyCount : totalCells;
    for (int spawnIndex = 0; spawnIndex < spawnCount; ++spawnIndex) {
        const int cellIndex = shuffledCells[static_cast<size_t>(spawnIndex)];
        const int row = cellIndex / cols;
        const int col = cellIndex % cols;
        const bool isElite = eliteCell[static_cast<size_t>(cellIndex)];

        const int cellLeft = mapLeft + col * cellW;
        const int cellTop = mapTop + row * cellH;
        const int cellRight = (col == cols - 1) ? g_iceRegionRect.right : (cellLeft + cellW);
        const int cellBottom = (row == rows - 1) ? g_iceRegionRect.bottom : (cellTop + cellH);

        const int marginX = (cellW >= 96) ? 28 : 12;
        const int marginY = (cellH >= 96) ? 28 : 12;
        const int spawnMinX = cellLeft + marginX;
        const int spawnMaxX = cellRight - marginX;
        const int spawnMinY = cellTop + marginY;
        const int spawnMaxY = cellBottom - marginY;

        IcefieldEnemy enemy;
        enemy.kind = isElite ? GameData::EnemyKind::SnowApe : GameData::EnemyKind::Miner;
        enemy.tier = isElite ? GameData::EnemyTier::Elite : GameData::EnemyTier::Normal;
        enemy.runtime.alive = true;
        enemy.runtime.faceRight = ((std::rand() & 1) == 0);
        enemy.runtime.breakStateActive = false;
        enemy.runtime.attackState = EnemyInstance::AttackState::Idle;
        enemy.runtime.slamImpactApplied = false;
        enemy.runtime.x = RandomRangeInclusive(spawnMinX, spawnMaxX);
        enemy.runtime.y = RandomRangeInclusive(spawnMinY, spawnMaxY);
        enemy.runtime.exactX = static_cast<float>(enemy.runtime.x);
        enemy.runtime.exactY = static_cast<float>(enemy.runtime.y);
        const float spawnObstacleRadius = EnemyObstacleRadiusForKind(enemy.kind);
        ResolveCircleOutsideObstacles(enemy.runtime.exactX, enemy.runtime.exactY, spawnObstacleRadius);
        enemy.runtime.x = static_cast<int>(std::round(enemy.runtime.exactX));
        enemy.runtime.y = static_cast<int>(std::round(enemy.runtime.exactY));
        ClampPointToIceRegion(enemy.runtime.x, enemy.runtime.y);
        enemy.runtime.exactX = static_cast<float>(enemy.runtime.x);
        enemy.runtime.exactY = static_cast<float>(enemy.runtime.y);
        enemy.runtime.roamTargetX = enemy.runtime.x;
        enemy.runtime.roamTargetY = enemy.runtime.y;
        enemy.runtime.dodgeTargetX = enemy.runtime.x;
        enemy.runtime.dodgeTargetY = enemy.runtime.y;
        enemy.runtime.chargeStopX = enemy.runtime.x;
        enemy.runtime.chargeStopY = enemy.runtime.y;
        enemy.runtime.slamTargetX = g_playerX;
        enemy.runtime.slamTargetY = g_playerY;
        enemy.runtime.nextRepositionTick = now + static_cast<ULONGLONG>(RandomRangeInclusive(100, 520));
        enemy.runtime.nextAttackTick = now + static_cast<ULONGLONG>(RandomRangeInclusive(220, 840));
        enemy.runtime.attackStateEndTick = 0;

        enemy.serialNumber = isElite ? eliteSerial++ : normalSerial++;
        enemy.runtime.maxHp = isElite ? 10 : 5;
        enemy.runtime.hp = enemy.runtime.maxHp;

        g_icefieldEnemies.push_back(enemy);
    }
}

bool UpdateIcefieldNormalWaveLifecycle() {
    return false;
}

int FindNearestAliveIcefieldEnemyIndex() {
    int bestIndex = -1;
    float bestDistanceSquared = 0.0f;

    for (size_t i = 0; i < g_icefieldEnemies.size(); ++i) {
        const IcefieldEnemy& enemy = g_icefieldEnemies[i];
        if (!enemy.runtime.alive || enemy.runtime.hp <= 0) {
            continue;
        }

        const float dx = static_cast<float>(g_playerX - enemy.runtime.x);
        const float dy = static_cast<float>(g_playerY - enemy.runtime.y);
        const float distanceSquared = dx * dx + dy * dy;
        if (bestIndex < 0 || distanceSquared < bestDistanceSquared) {
            bestIndex = static_cast<int>(i);
            bestDistanceSquared = distanceSquared;
        }
    }

    return bestIndex;
}

}  
