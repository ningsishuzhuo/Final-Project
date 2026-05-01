#include "level_map_internal.h"
#include "level_map_combat_helpers_internal.h"
#include "level_map_combat_internal.h"

namespace LevelMapInternal {
namespace {

void ApplyMoonRainDamageToCurrentEnemy(float centerX, float centerY, float radius, int damage) {
    if (!g_enemy.alive || g_enemy.hp <= 0 || damage <= 0) {
        return;
    }

    const float relX = static_cast<float>(g_enemy.x) - centerX;
    const float relY = static_cast<float>(g_enemy.y) - centerY;
    const float effectiveRadius = radius + static_cast<float>(EnemyParams().hitRadius);
    if (!IsPointInCircle(relX, relY, effectiveRadius)) {
        return;
    }

    ApplyDamageToCurrentEnemy(damage);
}

void ApplyMoonRainDamageToEnemies(float centerX, float centerY, float radius, int damage) {
    if (damage <= 0 || radius <= 0.0f) {
        return;
    }
    ApplyCircleObstacleHit(centerX, centerY, radius);

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
            ApplyMoonRainDamageToCurrentEnemy(centerX, centerY, radius, damage);
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
            ApplyMoonRainDamageToCurrentEnemy(centerX, centerY, radius, damage);
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

    ApplyMoonRainDamageToCurrentEnemy(centerX, centerY, radius, damage);
}

bool IsMoonUltimateCurrentlyActive(ULONGLONG now) {
    return g_currentCharacterIndex == AssetPaths::CHARACTER_MOON &&
        g_moonUltimateState.active &&
        now < g_moonUltimateState.endTick;
}

void UpdateMoonUltimateFireballs(ULONGLONG now) {
    size_t writeIndex = 0;
    for (size_t i = 0; i < g_moonUltimateFireballs.size(); ++i) {
        MoonUltimateFireballState& fireball = g_moonUltimateFireballs[i];
        if (!fireball.active) {
            continue;
        }

        if (!fireball.damageApplied && now >= fireball.impactTick) {
            ApplyMoonRainDamageToEnemies(
                fireball.targetX,
                fireball.targetY,
                kMoonUltimateFireballRadius,
                kMoonUltimateFireballDamage);
            fireball.damageApplied = true;
        }

        if (now >= fireball.endTick) {
            continue;
        }

        if (writeIndex != i) {
            g_moonUltimateFireballs[writeIndex] = fireball;
        }
        ++writeIndex;
    }
    g_moonUltimateFireballs.resize(writeIndex);
}

void StartMoonRainCastFromCharge(ULONGLONG now) {
    if (!g_moonRainChargeState.active) {
        return;
    }

    const ULONGLONG elapsedMs =
        (now >= g_moonRainChargeState.startTick) ? (now - g_moonRainChargeState.startTick) : 0ULL;
    int chargeSeconds = 0;
    if (kMoonRainChargeDurationMs > 0ULL) {
        const ULONGLONG clampedElapsed =
            (elapsedMs > kMoonRainChargeDurationMs) ? kMoonRainChargeDurationMs : elapsedMs;
        chargeSeconds = static_cast<int>(
            (clampedElapsed * static_cast<ULONGLONG>(kMoonRainChargeScalingMaxSeconds)) /
            kMoonRainChargeDurationMs);
    }
    if (chargeSeconds > kMoonRainChargeScalingMaxSeconds) {
        chargeSeconds = kMoonRainChargeScalingMaxSeconds;
    }
    if (chargeSeconds < 1 && kMoonRainChargeScalingMaxSeconds > 0) {
        chargeSeconds = 1;
    }

    int energyCost = kMoonRainEnergyCostBase + chargeSeconds * kMoonRainEnergyCostBonusPerSecond;
    if (energyCost > kMoonRainEnergyCostCap) {
        energyCost = kMoonRainEnergyCostCap;
    }
    if (g_playerEnergy < static_cast<float>(energyCost)) {
        g_moonRainChargeState = {};
        return;
    }

    int damagePerTick = kMoonRainDamageBasePerTick + chargeSeconds * kMoonRainDamageBonusPerSecond;
    if (damagePerTick > kMoonRainDamagePerTickCap) {
        damagePerTick = kMoonRainDamagePerTickCap;
    }

    g_playerEnergy -= static_cast<float>(energyCost);
    ClampPlayerEnergy();
    g_moonRainCastState.active = true;
    g_moonRainCastState.centerX = g_moonRainChargeState.centerX;
    g_moonRainCastState.centerY = g_moonRainChargeState.centerY;
    g_moonRainCastState.radius = g_moonRainChargeState.radius;
    g_moonRainCastState.damagePerTick = damagePerTick;
    g_moonRainCastState.energyCost = energyCost;
    g_moonRainCastState.damageApplied = false;
    g_moonRainCastState.startTick = now;
    g_moonRainCastState.endTick = now + kMoonRainDurationMs;
    g_moonRainCastState.nextDamageTick = now;
    g_moonRainChargeState = {};
}

bool IsMoonRainChargeReleaseRequested() {
    return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0;
}

void UpdateMoonRainChargeRadius(ULONGLONG now) {
    if (!g_moonRainChargeState.active) {
        return;
    }

    const ULONGLONG elapsedMs =
        (now >= g_moonRainChargeState.startTick) ? (now - g_moonRainChargeState.startTick) : 0ULL;
    if (elapsedMs >= kMoonRainChargeDurationMs) {
        g_moonRainChargeState.radius = kMoonRainMaxRadius;
        return;
    }

    const float growth = static_cast<float>(elapsedMs) * (kMoonRainChargeGrowthPerSecond / 1000.0f);
    g_moonRainChargeState.radius = (growth < kMoonRainMaxRadius) ? growth : kMoonRainMaxRadius;
}

}  

void BeginMoonRainCharge(int clickScreenX, int clickScreenY) {
    if (g_currentCharacterIndex != AssetPaths::CHARACTER_MOON || g_playerIsDead) {
        return;
    }
    if (g_moonRainChargeState.active) {
        return;
    }

    const float worldX = static_cast<float>(clickScreenX + g_cameraX);
    const float worldY = static_cast<float>(clickScreenY + g_cameraY);
    g_moonRainChargeState.active = true;
    g_moonRainChargeState.centerX = worldX;
    g_moonRainChargeState.centerY = worldY;
    g_moonRainChargeState.radius = 0.0f;
    g_moonRainChargeState.startTick = RenderUtils::NowTickMs();
    g_moonRainCastState = {};
}

void SpawnMoonUltimateFireball(int clickScreenX, int clickScreenY) {
    const ULONGLONG now = RenderUtils::NowTickMs();
    if (!IsMoonUltimateCurrentlyActive(now)) {
        return;
    }
    if (now < g_moonUltimateFireballNextCastTick) {
        return;
    }

    MoonUltimateFireballState fireball;
    fireball.active = true;
    fireball.damageApplied = false;
    fireball.startX = static_cast<float>(g_cameraX - kMoonUltimateFireballDrawW);
    fireball.startY = static_cast<float>(g_cameraY - kMoonUltimateFireballDrawH);
    fireball.targetX = static_cast<float>(clickScreenX + g_cameraX);
    fireball.targetY = static_cast<float>(clickScreenY + g_cameraY);
    fireball.startTick = now;
    fireball.impactTick = now + kMoonUltimateFireballFallDurationMs;
    fireball.endTick = fireball.impactTick + kMoonUltimateFireballImpactHoldMs;
    g_moonUltimateFireballs.push_back(fireball);
    g_moonUltimateFireballNextCastTick = now + kMoonUltimateFireballCooldownMs;
}

void ReleaseMoonRainCharge() {
    if (!g_moonRainChargeState.active || g_currentCharacterIndex != AssetPaths::CHARACTER_MOON || g_playerIsDead) {
        return;
    }

    const ULONGLONG now = RenderUtils::NowTickMs();
    StartMoonRainCastFromCharge(now);
}

void UpdateMoonRainSkill(ULONGLONG now) {
    UpdateMoonUltimateFireballs(now);

    if (g_moonRainChargeState.active) {
        UpdateMoonRainChargeRadius(now);
        if (IsMoonRainChargeReleaseRequested()) {
            StartMoonRainCastFromCharge(now);
        }
    }

    if (!g_moonRainCastState.active) {
        return;
    }

    if (now >= g_moonRainCastState.endTick) {
        g_moonRainCastState = {};
        return;
    }

    if (!g_moonRainCastState.damageApplied && now >= g_moonRainCastState.nextDamageTick) {
        ApplyMoonRainDamageToEnemies(
            g_moonRainCastState.centerX,
            g_moonRainCastState.centerY,
            g_moonRainCastState.radius,
            g_moonRainCastState.damagePerTick);
        g_moonRainCastState.damageApplied = true;
    }
}

}  

