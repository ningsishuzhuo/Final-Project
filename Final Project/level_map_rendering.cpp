#include "level_map_internal.h"
#include "level_map_hud_internal.h"
#include "level_map_actor_rendering_internal.h"
#include "level_map_rendering_effects_internal.h"

#include <climits>

namespace LevelMapInternal {

void RenderLevelFrame(ULONGLONG now) {
    setbkcolor(BLACK);
    cleardevice();

    DrawRegions();
    DrawGroundObstacles();
    DrawMoonRainSkillEffects(now);
    DrawGrid();

    const GameData::EnemyDefinition& enemyDef = EnemyDef();
    int enemyBulletDrawWidth = enemyDef.bulletDrawWidth;
    int enemyBulletDrawHeight = enemyDef.bulletDrawHeight;
    if (IsIcefieldNormalBattleActive()) {
        const GameData::EnemyDefinition& minerDef = GameData::GetEnemyDefinition(GameData::EnemyKind::Miner);
        if (minerDef.bulletDrawWidth > enemyBulletDrawWidth) {
            enemyBulletDrawWidth = minerDef.bulletDrawWidth;
        }
        if (minerDef.bulletDrawHeight > enemyBulletDrawHeight) {
            enemyBulletDrawHeight = minerDef.bulletDrawHeight;
        }
    }

    const bool hasIcefieldEnemies = !g_icefieldEnemies.empty();
    if (IsIcefieldNormalBattleActive() || hasIcefieldEnemies) {
        DrawEnemyShockwavesLayered(now, INT_MIN, INT_MAX);
        DrawEnemySpikeRowsLayered(INT_MIN, INT_MAX);
        DrawEnemyProjectileArrayLayered(enemyBulletDrawWidth, enemyBulletDrawHeight, INT_MIN, INT_MAX);
        DrawProjectileArrayLayered(
            g_playerProjectiles,
            g_particleAsset.image,
            g_particleAsset.hasAlpha,
            kPlayerProjectileDrawW,
            kPlayerProjectileDrawH,
            INT_MIN,
            INT_MAX);
        DrawIcefieldActorsSorted();
    }
    else {
        const int playerFootY = g_playerY + kPlayerFootOffsetY;
        const int enemyFootOffsetY = (EnemyDef().drawHeight * 3) / 8;
        const int enemyFootY = g_enemy.y + enemyFootOffsetY;
        const int backFootY = (playerFootY < enemyFootY) ? playerFootY : enemyFootY;
        const int frontFootY = (playerFootY < enemyFootY) ? enemyFootY : playerFootY;

        DrawMapObstaclesLayered(INT_MIN, backFootY);
        DrawEnergyDropsLayered(INT_MIN, backFootY);
        DrawEnemyShockwavesLayered(now, INT_MIN, backFootY);
        DrawEnemySpikeRowsLayered(INT_MIN, backFootY);
        DrawEnemyProjectileArrayLayered(enemyBulletDrawWidth, enemyBulletDrawHeight, INT_MIN, backFootY);
        DrawProjectileArrayLayered(
            g_playerProjectiles,
            g_particleAsset.image,
            g_particleAsset.hasAlpha,
            kPlayerProjectileDrawW,
            kPlayerProjectileDrawH,
            INT_MIN,
            backFootY);

        if (!g_enemy.alive) {
            DrawEnemy();
            DrawPlayer();
        }
        else if (playerFootY < enemyFootY) {
            DrawPlayer();
        }
        else {
            DrawEnemy();
        }

        DrawMapObstaclesLayered(backFootY, frontFootY);
        DrawEnergyDropsLayered(backFootY, frontFootY);
        DrawEnemyShockwavesLayered(now, backFootY, frontFootY);
        DrawEnemySpikeRowsLayered(backFootY, frontFootY);
        DrawEnemyProjectileArrayLayered(enemyBulletDrawWidth, enemyBulletDrawHeight, backFootY, frontFootY);
        DrawProjectileArrayLayered(
            g_playerProjectiles,
            g_particleAsset.image,
            g_particleAsset.hasAlpha,
            kPlayerProjectileDrawW,
            kPlayerProjectileDrawH,
            backFootY,
            frontFootY);

        if (g_enemy.alive) {
            if (playerFootY < enemyFootY) {
                DrawEnemy();
            }
            else {
                DrawPlayer();
            }
        }

        DrawMapObstaclesLayered(frontFootY, INT_MAX);
        DrawEnergyDropsLayered(frontFootY, INT_MAX);
        DrawEnemyShockwavesLayered(now, frontFootY, INT_MAX);
        DrawEnemySpikeRowsLayered(frontFootY, INT_MAX);
        DrawEnemyProjectileArrayLayered(enemyBulletDrawWidth, enemyBulletDrawHeight, frontFootY, INT_MAX);
        DrawProjectileArrayLayered(
            g_playerProjectiles,
            g_particleAsset.image,
            g_particleAsset.hasAlpha,
            kPlayerProjectileDrawW,
            kPlayerProjectileDrawH,
            frontFootY,
            INT_MAX);
    }

    DrawDashAfterimages(now);
    DrawMoonUltimateFireballEffects(now, false, true);
    DrawSunSwordQiProjectiles();
    DrawHud(now);
    if (g_playerIsDead) {
        DrawGameOverOverlay(now);
    }
    else if (g_gameVictory) {
        DrawVictoryOverlay(now);
    }
}

}  


