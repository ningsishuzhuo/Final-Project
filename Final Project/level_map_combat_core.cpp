#include "level_map_internal.h"
#include "level_map_combat_helpers_internal.h"
#include "level_map_combat_internal.h"

#include <cmath>
#include <cstdlib>

namespace LevelMapInternal {
namespace {

bool UsesHeavyProjectileDamage(GameData::EnemyTier tier) {
    return tier == GameData::EnemyTier::Boss || tier == GameData::EnemyTier::Elite;
}

}

int GetEnemyProjectileDamageByTier(GameData::EnemyTier tier) {
    return UsesHeavyProjectileDamage(tier) ? kEliteEnemyHitDamage : kNormalEnemyHitDamage;
}

void NormalizeAimDirection(float& dirX, float& dirY) {
    const float length = std::sqrt(dirX * dirX + dirY * dirY);
    if (length < 1.0f) {
        dirX = g_faceRight ? 1.0f : -1.0f;
        dirY = 0.0f;
        return;
    }

    const float invLength = 1.0f / length;
    dirX *= invLength;
    dirY *= invLength;
}

void UpdatePlayerFacingByAim(float dirX) {
    if (dirX < 0.0f) {
        g_faceRight = false;
    }
    else if (dirX > 0.0f) {
        g_faceRight = true;
    }
}

bool IsPointInFrontArc(float relX, float relY, float dirX, float dirY, float range) {
    const float rangeSq = range * range;
    if (relX * relX + relY * relY > rangeSq) {
        return false;
    }
    return relX * dirX + relY * dirY >= 0.0f;
}

bool IsPointInCircle(float relX, float relY, float radius) {
    const float radiusSq = radius * radius;
    return relX * relX + relY * relY <= radiusSq;
}

float DistanceSquaredToSegment(
    float px,
    float py,
    float x1,
    float y1,
    float x2,
    float y2,
    float& outNearestRelX,
    float& outNearestRelY) {
    const float segX = x2 - x1;
    const float segY = y2 - y1;
    const float segLenSq = segX * segX + segY * segY;
    if (segLenSq <= 0.0001f) {
        outNearestRelX = x1 - px;
        outNearestRelY = y1 - py;
        return outNearestRelX * outNearestRelX + outNearestRelY * outNearestRelY;
    }

    const float tUnclamped = ((px - x1) * segX + (py - y1) * segY) / segLenSq;
    const float t = (tUnclamped < 0.0f) ? 0.0f : ((tUnclamped > 1.0f) ? 1.0f : tUnclamped);
    const float nearestX = x1 + segX * t;
    const float nearestY = y1 + segY * t;
    outNearestRelX = nearestX - px;
    outNearestRelY = nearestY - py;
    return outNearestRelX * outNearestRelX + outNearestRelY * outNearestRelY;
}

int ApplySunCriticalPassiveToDamage(int damage) {
    if (g_currentCharacterIndex != AssetPaths::CHARACTER_SUN || damage <= 0) {
        return damage;
    }

    // 日角色被动按命中次数触发暴击。
    if (g_sunCriticalHitCount >= kSunCriticalReadyHitCount) {
        g_sunCriticalHitCount = 0;
        g_playerEnergy -= static_cast<float>(kSunCriticalEnergyCost);
        ClampPlayerEnergy();
        return damage * kSunCriticalMultiplier;
    }

    ++g_sunCriticalHitCount;
    if (g_sunCriticalHitCount > kSunCriticalReadyHitCount) {
        g_sunCriticalHitCount = kSunCriticalReadyHitCount;
    }
    return damage;
}

void TryTriggerMoonMagicCagePassive() {
    if (g_currentCharacterIndex != AssetPaths::CHARACTER_MOON ||
        !g_enemy.alive ||
        g_enemy.hp <= 0) {
        return;
    }

    if ((std::rand() % 100) >= kMoonMagicCageChancePercent) {
        return;
    }

    const ULONGLONG now = RenderUtils::NowTickMs();
    const bool wasActive = g_enemy.moonMagicCageEndTick > now;
    if (!wasActive) {
        if (g_enemy.nextAttackTick > now) {
            g_enemy.nextAttackTick = now + (g_enemy.nextAttackTick - now) * kMoonMagicCageAttackIntervalMultiplier;
        }
        if (g_enemy.bossPhaseEndTick > now) {
            g_enemy.bossPhaseEndTick = now + (g_enemy.bossPhaseEndTick - now) * kMoonMagicCageAttackIntervalMultiplier;
        }
    }
    g_enemy.moonMagicCageEndTick = now + kMoonMagicCageDurationMs;
}

void ApplyDamageToCurrentEnemy(int damage, bool allowSunCriticalPassive) {
    if (damage <= 0 || !g_enemy.alive || g_enemy.hp <= 0) {
        return;
    }

    if (allowSunCriticalPassive) {
        damage = ApplySunCriticalPassiveToDamage(damage);
    }

    const GameData::EnemyDefinition& enemyDef = EnemyDef();
    g_enemy.hp -= damage;
    if (g_enemy.hp > 0) {
        TryTriggerMoonMagicCagePassive();
    }
    if (enemyDef.kind == GameData::EnemyKind::SnowApeKing && g_enemy.hp > 0) {
        const int hpLost = g_enemy.maxHp - g_enemy.hp;
        NotifySnowApeKingHpLossForCrossBarrage(hpLost);
    }
    if (!g_enemy.breakStateActive &&
        enemyDef.tier == GameData::EnemyTier::Elite &&
        g_enemy.hp > 0 &&
        g_enemy.hp <= g_enemy.maxHp / 2) {
        g_enemy.breakStateActive = true;
    }
    if (g_enemy.hp <= 0) {
        SpawnEnergyDropsOnEnemyDeath(g_enemy.x, g_enemy.y, enemyDef);
        g_enemy.alive = false;
        g_enemy.hp = 0;
        ResetEnemyCombatRuntimeState();
        ResetEnemyBossRuntimeState();
    }
}

}


