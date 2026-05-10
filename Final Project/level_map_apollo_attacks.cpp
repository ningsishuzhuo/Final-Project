#include "level_map_internal.h"
#include "level_map_combat_helpers_internal.h"
#include "level_map_combat_internal.h"

#include <cmath>
#include <cstdlib>

namespace LevelMapInternal {
namespace {

int RollApolloSlashDamage() {
    const bool critical =
        (std::rand() % 100) < kApolloSlashCritChancePercent;
    const int multiplier = critical ? kApolloSlashCritMultiplier : 1;
    return kApolloSlashBaseDamage * multiplier;
}

void ArmApolloSlashVisual(float dirX, float dirY, ULONGLONG now) {
    g_apolloSlashState.active = true;
    g_apolloSlashState.startTick = now;
    g_apolloSlashState.endTick = now + kApolloSlashDurationMs;
    g_apolloSlashState.dirX = dirX;
    g_apolloSlashState.dirY = dirY;

    if (g_apolloSlashGif.IsLoaded() && g_apolloSlashGif.frameCount > 0) {
        g_apolloSlashGif.currentFrame = 0;
        g_apolloSlashGif.image->SelectActiveFrame(&g_apolloSlashGif.dimensionGuid, 0);
        if (!g_apolloSlashGif.delaysMs.empty()) {
            g_apolloSlashGif.nextTick = now + static_cast<ULONGLONG>(g_apolloSlashGif.delaysMs[0]);
        }
    }
}

void DispelEnemyAttacksByApolloSlash(float dirX, float dirY) {
    for (Projectile& projectile : g_enemyProjectiles) {
        if (!projectile.active) {
            continue;
        }

        const float relX = projectile.x - static_cast<float>(g_playerX);
        const float relY = projectile.y - static_cast<float>(g_playerY);
        if (!IsPointInFrontArc(relX, relY, dirX, dirY, kApolloSlashDispelRange)) {
            continue;
        }

        projectile.active = false;
    }

    for (ShockwaveInstance& shockwave : g_enemyShockwaves) {
        if (!shockwave.active) {
            continue;
        }

        const float relX = shockwave.x - static_cast<float>(g_playerX);
        const float relY = shockwave.y - static_cast<float>(g_playerY);
        if (!IsPointInFrontArc(relX, relY, dirX, dirY, kApolloSlashDispelRange)) {
            continue;
        }

        shockwave.active = false;
    }

    for (EnemySpikeRow& spikeRow : g_enemySpikeRows) {
        if (!spikeRow.active) {
            continue;
        }

        const float x1 = spikeRow.startX;
        const float y1 = spikeRow.startY;
        const float x2 = spikeRow.startX + spikeRow.dirX * spikeRow.length;
        const float y2 = spikeRow.startY + spikeRow.dirY * spikeRow.length;
        float nearestRelX = 0.0f;
        float nearestRelY = 0.0f;
        const float distSq = DistanceSquaredToSegment(
            static_cast<float>(g_playerX),
            static_cast<float>(g_playerY),
            x1,
            y1,
            x2,
            y2,
            nearestRelX,
            nearestRelY);
        if (distSq > kApolloSlashDispelRange * kApolloSlashDispelRange) {
            continue;
        }
        if (nearestRelX * dirX + nearestRelY * dirY < 0.0f) {
            continue;
        }

        spikeRow.active = false;
    }
}

void ExecuteApolloSlashAgainstCurrentEnemy(float dirX, float dirY) {
    if (!g_enemy.alive || g_enemy.hp <= 0) {
        return;
    }

    const float relX = static_cast<float>(g_enemy.x - g_playerX);
    const float relY = static_cast<float>(g_enemy.y - g_playerY);
    const float hitRange = kApolloSlashHitRange + static_cast<float>(EnemyParams().hitRadius);
    if (!IsPointInFrontArc(relX, relY, dirX, dirY, hitRange)) {
        return;
    }

    ApplyDamageToCurrentEnemy(RollApolloSlashDamage(), true);
}

bool IsSunUltimateCurrentlyActive(ULONGLONG now) {
    return g_currentCharacterIndex == AssetPaths::CHARACTER_SUN &&
        g_sunUltimateState.active &&
        now < g_sunUltimateState.endTick;
}

void PushSunSwordQi(const SunSwordQiState& swordQi) {
    static_assert(kSunUltimateSwordQiMaxCount > 0, "invalid capacity");

    if (g_sunSwordQiProjectiles.size() < static_cast<size_t>(kSunUltimateSwordQiMaxCount)) {
        g_sunSwordQiProjectiles.push_back(swordQi);
        return;
    }

    for (size_t i = 1; i < g_sunSwordQiProjectiles.size(); ++i) {
        g_sunSwordQiProjectiles[i - 1] = g_sunSwordQiProjectiles[i];
    }
    g_sunSwordQiProjectiles.back() = swordQi;
}

void SpawnSunSwordQiProjectile(float dirX, float dirY) {
    SunSwordQiState swordQi;
    swordQi.active = true;
    swordQi.x = static_cast<float>(g_playerX) + dirX * static_cast<float>(kApolloSlashForwardOffset);
    swordQi.y = static_cast<float>(g_playerY) + dirY * static_cast<float>(kApolloSlashForwardOffset);
    swordQi.vx = dirX * static_cast<float>(kSunUltimateSwordQiSpeed);
    swordQi.vy = dirY * static_cast<float>(kSunUltimateSwordQiSpeed);
    swordQi.angleDegrees =
        static_cast<float>(std::atan2(swordQi.vy, swordQi.vx) * 180.0 / static_cast<double>(kPi));
    swordQi.baseDamage = kSunUltimateSwordQiDamage;
    PushSunSwordQi(swordQi);
}

}

void SpawnApolloSlashAttack(int clickScreenX, int clickScreenY) {
    const ULONGLONG now = RenderUtils::NowTickMs();
    if (now < g_apolloSlashNextAvailableTick) {
        return;
    }
    const bool sunUltimateActive = IsSunUltimateCurrentlyActive(now);
    if (sunUltimateActive && g_playerEnergy < static_cast<float>(kSunUltimateSwordQiEnergyCost)) {
        return;
    }

    const float targetWorldX = static_cast<float>(clickScreenX + g_cameraX);
    const float targetWorldY = static_cast<float>(clickScreenY + g_cameraY);
    float dirX = targetWorldX - static_cast<float>(g_playerX);
    float dirY = targetWorldY - static_cast<float>(g_playerY);
    NormalizeAimDirection(dirX, dirY);
    UpdatePlayerFacingByAim(dirX);

    g_apolloSlashNextAvailableTick = now + kApolloSlashCooldownMs;
    ArmApolloSlashVisual(dirX, dirY, now);
    if (sunUltimateActive) {
        g_playerEnergy -= static_cast<float>(kSunUltimateSwordQiEnergyCost);
        ClampPlayerEnergy();
        SpawnSunSwordQiProjectile(dirX, dirY);
    }
    DispelEnemyAttacksByApolloSlash(dirX, dirY);
    ApplyMeleeObstacleHitInFrontArc(
        static_cast<float>(g_playerX),
        static_cast<float>(g_playerY),
        dirX,
        dirY,
        kApolloSlashHitRange);

    if (!g_icefieldEnemies.empty()) {
        EnemyInstance primaryEnemyState = g_enemy;
        const GameData::EnemyKind primaryEnemyKind = g_enemyKind;
        GameData::EnemyCombatParams primaryEnemyParams = g_enemyParams;
        const bool primarySawPlayer = g_enemySawPlayerLastFrame;
        const int primaryActiveIndex = g_activeIcefieldEnemyIndex;

        const bool bossFightActive = !IsIcefieldNormalBattleActive() && primaryEnemyState.alive;
        if (bossFightActive) {
            g_enemy = primaryEnemyState;
            g_enemyKind = primaryEnemyKind;
            SetEnemyTier(GameData::GetEnemyDefinition(primaryEnemyKind).tier);
            ExecuteApolloSlashAgainstCurrentEnemy(dirX, dirY);
            primaryEnemyState = g_enemy;
            primaryEnemyParams = g_enemyParams;
        }

        for (size_t i = 0; i < g_icefieldEnemies.size(); ++i) {
            IcefieldEnemy& enemy = g_icefieldEnemies[i];
            if (!enemy.runtime.alive || enemy.runtime.hp <= 0) {
                continue;
            }

            g_enemyKind = enemy.kind;
            SetEnemyTier(enemy.tier);
            g_enemy = enemy.runtime;
            ExecuteApolloSlashAgainstCurrentEnemy(dirX, dirY);
            enemy.runtime = g_enemy;
        }

        if (IsIcefieldNormalBattleActive() &&
            g_activeIcefieldEnemyIndex >= 0 &&
            g_activeIcefieldEnemyIndex < static_cast<int>(g_icefieldEnemies.size())) {
            const IcefieldEnemy& activeEnemy = g_icefieldEnemies[static_cast<size_t>(g_activeIcefieldEnemyIndex)];
            g_enemyKind = activeEnemy.kind;
            SetEnemyTier(activeEnemy.tier);
            g_enemy = activeEnemy.runtime;
            g_enemySawPlayerLastFrame = activeEnemy.sawPlayerLastFrame;
        }
        else {
            g_enemy = primaryEnemyState;
            g_enemyKind = primaryEnemyKind;
            g_enemyParams = primaryEnemyParams;
            g_enemySawPlayerLastFrame = primarySawPlayer;
            g_activeIcefieldEnemyIndex = primaryActiveIndex;
        }
        return;
    }

    ExecuteApolloSlashAgainstCurrentEnemy(dirX, dirY);
}

}
