#include "level_map_internal.h"
#include "level_map_combat_helpers_internal.h"
#include "level_map_combat_internal.h"
#include "level_map_damage_rules.h"

#include <cmath>
#include <cstdlib>

namespace LevelMapInternal {
namespace {

constexpr float kPi = 3.14159265358979323846f;

int CountActiveProjectiles(const std::vector<Projectile>& projectiles) {
    int count = 0;
    for (const Projectile& projectile : projectiles) {
        if (projectile.active) {
            ++count;
        }
    }
    return count;
}

}  

void ApplyPlayerHit(int damage) {
    if (g_playerIsDead || g_playerHp <= 0 || damage <= 0) {
        return;
    }

    const ULONGLONG now = RenderUtils::NowTickMs();
    g_playerLastDamageTick = now;
    g_playerArmorRegenTick = 0;
    PlayerStatus::ApplyIncomingDamage(damage, g_playerArmor, g_playerHp);
    ClampPlayerArmor();
    ClampPlayerHp();
    if (g_currentCharacterIndex == AssetPaths::CHARACTER_LOVE &&
        g_playerHp > 0 &&
        g_playerArmor == 0 &&
        !g_lovePurifyState.active &&
        (std::rand() % 100) < kLovePurifyChancePercent) {
        g_lovePurifyState.active = true;
        g_lovePurifyState.startTick = now;
        g_lovePurifyState.endTick = now + kLovePurifyDurationMs;
    }
    if (g_playerHp <= 0) {
        g_playerHp = 0;
        g_playerIsDead = true;
        g_playerDeathStartTick = now;
        g_isMoving = false;
        g_isDashing = false;
        g_moonRainChargeState = {};
        g_moonRainCastState = {};
        g_lovePurifyState = {};
    }

}

void SpawnEnemyProjectile(float dx, float dy) {
    const int defaultDamage = GetEnemyProjectileDamageByTier(EnemyDef().tier);
    SpawnEnemyProjectileFrom(
        static_cast<float>(g_enemy.x),
        static_cast<float>(g_enemy.y),
        dx,
        dy,
        1.0f,
        RenderUtils::NowTickMs(),
        0ULL,
        defaultDamage);
}

void SpawnEnemyProjectileFrom(
    float originX,
    float originY,
    float dx,
    float dy,
    float speedScale,
    ULONGLONG nowTick,
    ULONGLONG hitDelayMs,
    int damage) {
    const float length = std::sqrt(dx * dx + dy * dy);
    if (length < 1.0f) {
        return;
    }
    const float inverseLength = 1.0f / length;
    const float bulletSpeed = static_cast<float>(EnemyParams().bulletSpeed) * speedScale;

    Projectile projectile;
    projectile.x = originX;
    projectile.y = originY;
    projectile.vx = dx * inverseLength * bulletSpeed;
    projectile.vy = dy * inverseLength * bulletSpeed;
    projectile.angleDegrees =
        static_cast<float>(std::atan2(projectile.vy, projectile.vx) * 180.0 / static_cast<double>(kPi)) +
        kEnemyBulletAngleOffsetDegrees;
    projectile.damage = (damage > 0) ? damage : GetEnemyProjectileDamageByTier(EnemyDef().tier);
    projectile.sourceKind = g_enemyKind;
    projectile.hitArmedTick = nowTick + hitDelayMs;
    projectile.active = true;

    PushProjectileCapped(
        g_enemyProjectiles,
        projectile,
        static_cast<size_t>(kEnemyBulletMaxCount),
        g_enemyProjectileOverflowCursor);
}

void SpawnEnemyShockwave(float centerX, float centerY, bool alreadyHitPlayer, ShockwaveInstance::Variant variant) {
    const ShockwaveTuning& tuning = GetShockwaveTuning(variant);

    ShockwaveInstance wave;
    wave.x = centerX;
    wave.y = centerY;
    wave.previousRadius = 0.0f;
    wave.radius = 0.0f;
    wave.triggerTick = RenderUtils::NowTickMs() + tuning.lockDelayMs;
    wave.active = true;
    wave.expanding = false;
    wave.hitPlayer = alreadyHitPlayer;
    wave.variant = variant;

    PushCapped(g_enemyShockwaves, wave, kEnemyShockwaveMaxCount);
}

void UpdateSunSwordQiProjectiles() {
    const float minX = static_cast<float>(g_iceRegionRect.left);
    const float maxX = static_cast<float>(g_iceRegionRect.right);
    const float minY = static_cast<float>(g_iceRegionRect.top);
    const float maxY = static_cast<float>(g_iceRegionRect.bottom);

    size_t writeIndex = 0;
    for (size_t i = 0; i < g_sunSwordQiProjectiles.size(); ++i) {
        SunSwordQiState swordQi = g_sunSwordQiProjectiles[i];
        if (!swordQi.active) {
            continue;
        }

        swordQi.x += swordQi.vx;
        swordQi.y += swordQi.vy;
        const bool outOfBounds =
            swordQi.x < minX ||
            swordQi.y < minY ||
            swordQi.x > maxX ||
            swordQi.y > maxY ||
            swordQi.x < -kSunUltimateSwordQiDrawW ||
            swordQi.y < -kSunUltimateSwordQiDrawH ||
            swordQi.x > kLevelMapWidth + kSunUltimateSwordQiDrawW ||
            swordQi.y > kLevelMapHeight + kSunUltimateSwordQiDrawH;
        if (outOfBounds) {
            continue;
        }

        g_sunSwordQiProjectiles[writeIndex] = swordQi;
        ++writeIndex;
    }
    g_sunSwordQiProjectiles.resize(writeIndex);
}

void UpdateProjectileArray(std::vector<Projectile>& arr, int speedGuardWidth, int speedGuardHeight) {
    const float minX = static_cast<float>(g_iceRegionRect.left);
    const float maxX = static_cast<float>(g_iceRegionRect.right);
    const float minY = static_cast<float>(g_iceRegionRect.top);
    const float maxY = static_cast<float>(g_iceRegionRect.bottom);
    bool hasInactive = false;
    for (Projectile& projectile : arr) {
        if (!projectile.active) {
            hasInactive = true;
            continue;
        }

        projectile.x += projectile.vx;
        projectile.y += projectile.vy;
        if (HandleProjectileObstacleHit(projectile)) {
            hasInactive = true;
            continue;
        }

        const bool hitIceWall =
            projectile.x < minX ||
            projectile.y < minY ||
            projectile.x > maxX ||
            projectile.y > maxY;
        const bool outOfMapSafety =
            projectile.x < -speedGuardWidth ||
            projectile.y < -speedGuardHeight ||
            projectile.x > kLevelMapWidth + speedGuardWidth ||
            projectile.y > kLevelMapHeight + speedGuardHeight;

        if (hitIceWall || outOfMapSafety) {
            projectile.active = false;
            hasInactive = true;
        }
    }

    if (hasInactive) {
        CompactProjectileArray(arr);
    }
}

void HandleCombatCollisions(ULONGLONG now) {
    int activePlayerProjectileCount = CountActiveProjectiles(g_playerProjectiles);
    int activeSunSwordQiCount = 0;
    for (const SunSwordQiState& swordQi : g_sunSwordQiProjectiles) {
        if (swordQi.active) {
            ++activeSunSwordQiCount;
        }
    }

    auto resolvePlayerProjectilesAgainstCurrentEnemy = [&]() {
        if (activePlayerProjectileCount <= 0) {
            return;
        }

        const GameData::EnemyCombatParams& params = EnemyParams();
        if (!g_enemy.alive) {
            return;
        }

        for (Projectile& projectile : g_playerProjectiles) {
            if (!projectile.active) {
                continue;
            }

            const float dx = projectile.x - static_cast<float>(g_enemy.x);
            const float dy = projectile.y - static_cast<float>(g_enemy.y);
            if (std::fabs(dx) > params.hitRadius || std::fabs(dy) > params.hitRadius) {
                continue;
            }

            projectile.active = false;
            --activePlayerProjectileCount;
            ApplyDamageToCurrentEnemy(1);
            if (!g_enemy.alive) {
                break;
            }
        }
    };

    auto resolveSunSwordQiAgainstCurrentEnemy = [&]() {
        if (activeSunSwordQiCount <= 0 || !g_enemy.alive) {
            return;
        }

        const GameData::EnemyCombatParams& params = EnemyParams();
        const float hitRadius =
            static_cast<float>(params.hitRadius + kSunUltimateSwordQiHitRadius);
        const float hitRadiusSq = hitRadius * hitRadius;

        for (SunSwordQiState& swordQi : g_sunSwordQiProjectiles) {
            if (!swordQi.active) {
                continue;
            }

            const float dx = swordQi.x - static_cast<float>(g_enemy.x);
            const float dy = swordQi.y - static_cast<float>(g_enemy.y);
            if (dx * dx + dy * dy > hitRadiusSq) {
                continue;
            }

            swordQi.active = false;
            --activeSunSwordQiCount;
            ApplyDamageToCurrentEnemy(swordQi.baseDamage, true);
            if (!g_enemy.alive || activeSunSwordQiCount <= 0) {
                break;
            }
        }
    };

    if (!g_icefieldEnemies.empty()) {
        EnemyInstance primaryEnemyState = g_enemy;
        const GameData::EnemyKind primaryEnemyKind = g_enemyKind;
        GameData::EnemyCombatParams primaryEnemyParams = g_enemyParams;
        bool primarySawPlayer = g_enemySawPlayerLastFrame;
        const int primaryActiveIndex = g_activeIcefieldEnemyIndex;
        const bool bossFightActive = !IsIcefieldNormalBattleActive() && primaryEnemyState.alive;

        
        
        if (bossFightActive && activePlayerProjectileCount > 0) {
            g_enemy = primaryEnemyState;
            g_enemyKind = primaryEnemyKind;
            SetEnemyTier(GameData::GetEnemyDefinition(primaryEnemyKind).tier);
            g_enemySawPlayerLastFrame = primarySawPlayer;
            g_activeIcefieldEnemyIndex = primaryActiveIndex;
            resolvePlayerProjectilesAgainstCurrentEnemy();

            primaryEnemyState = g_enemy;
            primaryEnemyParams = g_enemyParams;
            primarySawPlayer = g_enemySawPlayerLastFrame;
        }
        if (bossFightActive && activeSunSwordQiCount > 0) {
            g_enemy = primaryEnemyState;
            g_enemyKind = primaryEnemyKind;
            SetEnemyTier(GameData::GetEnemyDefinition(primaryEnemyKind).tier);
            g_enemySawPlayerLastFrame = primarySawPlayer;
            g_activeIcefieldEnemyIndex = primaryActiveIndex;
            resolveSunSwordQiAgainstCurrentEnemy();

            primaryEnemyState = g_enemy;
            primaryEnemyParams = g_enemyParams;
            primarySawPlayer = g_enemySawPlayerLastFrame;
        }

        for (size_t i = 0; i < g_icefieldEnemies.size() &&
            (activePlayerProjectileCount > 0 || activeSunSwordQiCount > 0); ++i) {
            IcefieldEnemy& enemy = g_icefieldEnemies[i];
            if (!enemy.runtime.alive || enemy.runtime.hp <= 0) {
                continue;
            }

            g_enemyKind = enemy.kind;
            SetEnemyTier(enemy.tier);
            g_enemy = enemy.runtime;
            resolvePlayerProjectilesAgainstCurrentEnemy();
            resolveSunSwordQiAgainstCurrentEnemy();
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
    }
    else {
        if (activePlayerProjectileCount > 0) {
            resolvePlayerProjectilesAgainstCurrentEnemy();
        }
        if (activeSunSwordQiCount > 0) {
            resolveSunSwordQiAgainstCurrentEnemy();
        }
    }

    if (activeSunSwordQiCount < static_cast<int>(g_sunSwordQiProjectiles.size())) {
        size_t writeIndex = 0;
        for (size_t i = 0; i < g_sunSwordQiProjectiles.size(); ++i) {
            if (!g_sunSwordQiProjectiles[i].active) {
                continue;
            }
            if (writeIndex != i) {
                g_sunSwordQiProjectiles[writeIndex] = g_sunSwordQiProjectiles[i];
            }
            ++writeIndex;
        }
        g_sunSwordQiProjectiles.resize(writeIndex);
    }

    for (Projectile& projectile : g_enemyProjectiles) {
        if (g_playerIsDead || g_playerHp <= 0) {
            break;
        }
        if (!projectile.active) {
            continue;
        }
        if (projectile.hitArmedTick > 0 && now < projectile.hitArmedTick) {
            continue;
        }

        const float dx = projectile.x - static_cast<float>(g_playerX);
        const float dy = projectile.y - static_cast<float>(g_playerY);
        if (std::fabs(dx) <= static_cast<float>(kEnemyBulletHitRadius) &&
            std::fabs(dy) <= static_cast<float>(kEnemyBulletHitRadius)) {
            projectile.active = false;
            ApplyPlayerHit(projectile.damage);
        }
    }
}

}  

