#include "level_map_internal.h"
#include "level_map_battle_flow.h"

namespace LevelMapInternal {

void StartIcefieldBossBattle(ULONGLONG now) {
    g_activeIcefieldEnemyIndex = -1;
    g_battlePhase = BattlePhase::BossFight;

    const bool kindChanged = (g_enemyKind != GameData::EnemyKind::SnowApeKing);
    g_enemyKind = GameData::EnemyKind::SnowApeKing;
    const GameData::EnemyDefinition& enemyDef = EnemyDef();
    SetEnemyTier(enemyDef.tier);
    const GameData::EnemyCombatParams& params = EnemyParams();
    if (kindChanged) {
        LoadEnemyAssets();
    }

    g_enemy.alive = true;
    g_enemy.faceRight = false;
    g_enemy.breakStateActive = false;
    ResetEnemyCombatRuntimeState();
    ResetEnemyBossRuntimeState();
    g_enemy.bossWalkPhase = true;
    g_enemy.bossPressureActive = false;
    g_enemy.bossRoarDirX = -1.0f;
    g_enemy.bossRoarDirY = 0.0f;
    g_enemy.moonMagicCageEndTick = 0;
    g_enemy.hp = (g_enemyKind == GameData::EnemyKind::SnowApeKing) ? 250 : params.hp;
    g_enemy.maxHp = g_enemy.hp;

    const int halfW = enemyDef.drawWidth / 2;
    const int halfH = enemyDef.drawHeight / 2;
    const int screenSpawnPaddingX = halfW + enemyDef.regionPadding;
    const int screenSpawnPaddingY = halfH + enemyDef.regionPadding;
    const int mapMinX = g_iceRegionRect.left + enemyDef.regionPadding;
    const int mapMaxX = g_iceRegionRect.right - enemyDef.regionPadding;
    const int mapMinY = g_iceRegionRect.top + enemyDef.regionPadding;
    const int mapMaxY = g_iceRegionRect.bottom - enemyDef.regionPadding;
    int spawnMinX = g_cameraX + screenSpawnPaddingX;
    int spawnMaxX = g_cameraX + GAME_WINDOW_WIDTH - screenSpawnPaddingX;
    int spawnMinY = g_cameraY + screenSpawnPaddingY;
    int spawnMaxY = g_cameraY + GAME_WINDOW_HEIGHT - screenSpawnPaddingY;

    if (spawnMinX < mapMinX) spawnMinX = mapMinX;
    if (spawnMaxX > mapMaxX) spawnMaxX = mapMaxX;
    if (spawnMinY < mapMinY) spawnMinY = mapMinY;
    if (spawnMaxY > mapMaxY) spawnMaxY = mapMaxY;

    int spawnX = g_playerX;
    int spawnY = g_playerY;
    if (spawnMinX <= spawnMaxX && spawnMinY <= spawnMaxY) {
        spawnX = spawnMinX + (std::rand() % (spawnMaxX - spawnMinX + 1));
        spawnY = spawnMinY + (std::rand() % (spawnMaxY - spawnMinY + 1));
    }
    ClampPointToIceRegion(spawnX, spawnY);
    SetEnemyExactPosition(static_cast<float>(spawnX), static_cast<float>(spawnY));
    g_enemy.roamTargetX = g_enemy.x;
    g_enemy.roamTargetY = g_enemy.y;
    g_enemy.dodgeTargetX = g_enemy.x;
    g_enemy.dodgeTargetY = g_enemy.y;
    g_enemy.chargeStopX = g_enemy.x;
    g_enemy.chargeStopY = g_enemy.y;
    g_enemy.slamTargetX = g_playerX;
    g_enemy.slamTargetY = g_playerY;
    g_enemy.bossPhaseEndTick = 0;
    g_enemy.nextRepositionTick = now + 200;
    g_enemy.nextAttackTick = now + 800;
    g_enemy.attackStateEndTick = 0;
    g_enemySawPlayerLastFrame = false;
}

bool IsIcefieldNormalBattleActive() {
    return g_battlePhase == BattlePhase::NormalFight;
}

void GetIcefieldRemainingEnemyCounts(int& normalCount, int& eliteCount) {
    normalCount = 0;
    eliteCount = 0;
    for (const IcefieldEnemy& enemy : g_icefieldEnemies) {
        if (!enemy.runtime.alive || enemy.runtime.hp <= 0) {
            continue;
        }
        if (enemy.tier == GameData::EnemyTier::Elite) {
            ++eliteCount;
        }
        else {
            ++normalCount;
        }
    }
}

void SpawnEnemyInIceRegion() {
    const ULONGLONG now = RenderUtils::NowTickMs();
    InitializeIcefieldNormalBattle(now);
}

}  
