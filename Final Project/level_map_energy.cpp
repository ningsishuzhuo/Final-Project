#include "level_map_internal.h"

#include <cstddef>
#include <cmath>
#include <cstdlib>

namespace LevelMapInternal {
namespace {

constexpr float kScatterSpeedMin = 1.0f;
constexpr float kScatterSpeedMax = 3.0f;
constexpr int kSettleFramesMin = 16;
constexpr int kSettleFramesMax = 34;
constexpr float kVelocityDamping = 0.88f;
constexpr float kMinVelocity = 0.04f;
constexpr int kPercentBase = 100;
constexpr int kRecoverPotionEnergyValue = 30;
constexpr int kRecoverPotionHpValue = 1;
constexpr int kEnergyPotionEnergyValue = 30;
constexpr int kLifePotionHpValue = 2;

struct DropRenderInfo {
    const IMAGE* image = nullptr;
    bool hasAlpha = false;
    int drawSize = kEnergyDropDrawSize;
    COLORREF fallbackColor = RGB(98, 210, 255);
};

struct EnergyDropReward {
    int count = 0;
    int value = 0;
};

struct PotionDropRates {
    int recoverPercent = 0;
    int energyPercent = 0;
    int lifePercent = 0;
};

void ClampEnergyDropToMap(EnergyDrop& drop) {
    const float minX = static_cast<float>(g_iceRegionRect.left);
    const float maxX = static_cast<float>(g_iceRegionRect.right);
    const float minY = static_cast<float>(g_iceRegionRect.top);
    const float maxY = static_cast<float>(g_iceRegionRect.bottom);

    if (drop.x < minX) {
        drop.x = minX;
        drop.vx = 0.0f;
    }
    else if (drop.x > maxX) {
        drop.x = maxX;
        drop.vx = 0.0f;
    }

    if (drop.y < minY) {
        drop.y = minY;
        drop.vy = 0.0f;
    }
    else if (drop.y > maxY) {
        drop.y = maxY;
        drop.vy = 0.0f;
    }
}

void ResolveEnergyDropAgainstObstacles(EnergyDrop& drop) {
    const float dropCollisionRadius = static_cast<float>(kEnergyDropDrawSize) * 0.5f;
    ResolveCircleOutsideObstacles(drop.x, drop.y, dropCollisionRadius);
    ClampEnergyDropToMap(drop);
}

void GivePlayerEnergy(int value) {
    if (value <= 0) {
        return;
    }
    g_playerEnergy += static_cast<float>(value);
    if (g_playerEnergy > static_cast<float>(g_playerMaxEnergy)) {
        g_playerEnergy = static_cast<float>(g_playerMaxEnergy);
    }
}

void GivePlayerHp(int value) {
    if (value <= 0 || g_playerIsDead) {
        return;
    }
    g_playerHp += value;
    if (g_playerHp > g_playerMaxHp) {
        g_playerHp = g_playerMaxHp;
    }
}

void ApplyDropPickupEffect(const EnergyDrop& drop) {
    switch (drop.kind) {
    case EnergyDrop::Kind::Energy:
        GivePlayerEnergy(drop.value);
        break;
    case EnergyDrop::Kind::RecoverPotion:
        GivePlayerEnergy(kRecoverPotionEnergyValue);
        GivePlayerHp(kRecoverPotionHpValue);
        break;
    case EnergyDrop::Kind::EnergyPotion:
        GivePlayerEnergy(kEnergyPotionEnergyValue);
        break;
    case EnergyDrop::Kind::LifePotion:
        GivePlayerHp(kLifePotionHpValue);
        break;
    }
}

DropRenderInfo GetDropRenderInfo(EnergyDrop::Kind kind) {
    switch (kind) {
    case EnergyDrop::Kind::RecoverPotion:
        return DropRenderInfo{ &g_recoverPotionDropAsset.image, g_recoverPotionDropAsset.hasAlpha, kPotionDropDrawSize, RGB(182, 96, 234) };
    case EnergyDrop::Kind::EnergyPotion:
        return DropRenderInfo{ &g_energyPotionDropAsset.image, g_energyPotionDropAsset.hasAlpha, kPotionDropDrawSize, RGB(94, 172, 255) };
    case EnergyDrop::Kind::LifePotion:
        return DropRenderInfo{ &g_lifePotionDropAsset.image, g_lifePotionDropAsset.hasAlpha, kPotionDropDrawSize, RGB(255, 110, 94) };
    case EnergyDrop::Kind::Energy:
    default:
        return DropRenderInfo{ &g_energyDropAsset.image, g_energyDropAsset.hasAlpha, kEnergyDropDrawSize, RGB(98, 210, 255) };
    }
}

EnergyDropReward GetEnemyEnergyDropReward(const GameData::EnemyDefinition& enemyDef) {
    switch (enemyDef.tier) {
    case GameData::EnemyTier::Boss:
        return { 22, 3 };
    case GameData::EnemyTier::Elite:
        return { 12, 1 };
    case GameData::EnemyTier::Normal:
        return { 6, 1 };
    }
    return { 6, 1 };
}

bool HasPotionDropChance(PotionDropRates rates) {
    return rates.recoverPercent != 0 ||
        rates.energyPercent != 0 ||
        rates.lifePercent != 0;
}

void PushEnergyDropCapped(const EnergyDrop& drop) {
    static_assert(kEnergyDropMaxCount > 0, "invalid capacity");

    const size_t cap = static_cast<size_t>(kEnergyDropMaxCount);
    if (g_energyDrops.size() < cap) {
        g_energyDrops.push_back(drop);
        return;
    }

    const size_t size = g_energyDrops.size();
    const size_t start = g_energyDropOverflowCursor % size;
    for (size_t offset = 0; offset < size; ++offset) {
        const size_t index = (start + offset) % size;
        if (g_energyDrops[index].active) {
            continue;
        }

        g_energyDrops[index] = drop;
        g_energyDropOverflowCursor = (index + 1U) % size;
        return;
    }

    g_energyDrops[start] = drop;
    g_energyDropOverflowCursor = (start + 1U) % size;
}

void SpawnSingleDrop(EnergyDrop::Kind kind, int centerX, int centerY, int value) {
    const float angle = static_cast<float>(std::rand() % 360) * (kPi / 180.0f);
    const float speedFactor = static_cast<float>(std::rand() % 1000) / 1000.0f;
    const float speed = kScatterSpeedMin + (kScatterSpeedMax - kScatterSpeedMin) * speedFactor;

    EnergyDrop drop;
    drop.x = static_cast<float>(centerX);
    drop.y = static_cast<float>(centerY);
    drop.vx = std::cos(angle) * speed;
    drop.vy = std::sin(angle) * speed;
    drop.value = value;
    drop.settleFrames = kSettleFramesMin + (std::rand() % (kSettleFramesMax - kSettleFramesMin + 1));
    drop.kind = kind;
    drop.active = true;
    ClampEnergyDropToMap(drop);
    ResolveEnergyDropAgainstObstacles(drop);
    PushEnergyDropCapped(drop);
}

PotionDropRates GetEnemyPotionDropRates(const GameData::EnemyDefinition& enemyDef) {
    switch (enemyDef.tier) {
    case GameData::EnemyTier::Normal:
        return {
            kPotionDropChanceNormalRecoverPercent,
            kPotionDropChanceNormalEnergyPercent,
            kPotionDropChanceNormalLifePercent
        };
    case GameData::EnemyTier::Elite:
        return {
            kPotionDropChanceEliteRecoverPercent,
            kPotionDropChanceEliteEnergyPercent,
            kPotionDropChanceEliteLifePercent
        };
    case GameData::EnemyTier::Boss:
        return {};
    }
    return {};
}

PotionDropRates GetCratePotionDropRates() {
    return { 0, kPotionDropChanceCrateEnergyPercent, kPotionDropChanceCrateLifePercent };
}

PotionDropRates GetBossPotionDropRates() {
    return {
        kPotionDropChanceBossRecoverPercent,
        kPotionDropChanceBossEnergyPercent,
        kPotionDropChanceBossLifePercent
    };
}

bool TrySpawnPotionDropFromRates(int centerX, int centerY, PotionDropRates rates) {
    const int roll = std::rand() % kPercentBase;
    const int recoverLimit = rates.recoverPercent;
    const int energyLimit = recoverLimit + rates.energyPercent;
    const int lifeLimit = energyLimit + rates.lifePercent;

    if (roll < recoverLimit) {
        SpawnSingleDrop(EnergyDrop::Kind::RecoverPotion, centerX, centerY, 0);
        return true;
    }
    if (roll < energyLimit) {
        SpawnSingleDrop(EnergyDrop::Kind::EnergyPotion, centerX, centerY, 0);
        return true;
    }
    if (roll < lifeLimit) {
        SpawnSingleDrop(EnergyDrop::Kind::LifePotion, centerX, centerY, 0);
        return true;
    }
    return false;
}

bool TrySpawnPotionDropOnEnemyDeath(int centerX, int centerY, const GameData::EnemyDefinition& enemyDef) {
    const PotionDropRates rates = GetEnemyPotionDropRates(enemyDef);
    if (!HasPotionDropChance(rates)) {
        return false;
    }
    return TrySpawnPotionDropFromRates(centerX, centerY, rates);
}

void TrySpawnPotionDropOnCrateDestroyed(int centerX, int centerY) {
    TrySpawnPotionDropFromRates(centerX, centerY, GetCratePotionDropRates());
}

POINT BossPotionDropPoint(int centerX, int centerY) {
    float dirX = static_cast<float>(g_playerX - centerX);
    float dirY = static_cast<float>(g_playerY - centerY);
    const float dirLength = std::sqrt(dirX * dirX + dirY * dirY);
    if (dirLength > 0.001f) {
        dirX /= dirLength;
        dirY /= dirLength;
    }
    else {
        dirX = g_enemy.faceRight ? 1.0f : -1.0f;
        dirY = 0.0f;
    }

    const float dropOffset = static_cast<float>(kSnowApeKingCollisionRadius + kEnergyPickupRadius + kPotionDropDrawSize);
    return {
        static_cast<LONG>(std::round(static_cast<float>(centerX) + dirX * dropOffset)),
        static_cast<LONG>(std::round(static_cast<float>(centerY) + dirY * dropOffset))
    };
}

void TrySpawnPotionDropWithBossRates(int centerX, int centerY) {
    const POINT dropPoint = BossPotionDropPoint(centerX, centerY);
    TrySpawnPotionDropFromRates(dropPoint.x, dropPoint.y, GetBossPotionDropRates());
}

}

void SpawnEnergyDropsOnEnemyDeath(int centerX, int centerY, const GameData::EnemyDefinition& enemyDef) {
    if (TrySpawnPotionDropOnEnemyDeath(centerX, centerY, enemyDef)) {
        return;
    }

    const EnergyDropReward reward = GetEnemyEnergyDropReward(enemyDef);
    if (reward.count <= 0 || reward.value <= 0) {
        return;
    }

    for (int i = 0; i < reward.count; ++i) {
        SpawnSingleDrop(EnergyDrop::Kind::Energy, centerX, centerY, reward.value);
    }
}

void SpawnDropsOnCrateDestroyed(int centerX, int centerY) {
    TrySpawnPotionDropOnCrateDestroyed(centerX, centerY);
}

void TrySpawnPotionDropOnBossHpLossThreshold(int centerX, int centerY) {
    TrySpawnPotionDropWithBossRates(centerX, centerY);
}

void RemoveInactiveEnergyDrops() {
    size_t writeIndex = 0;
    for (size_t i = 0; i < g_energyDrops.size(); ++i) {
        if (!g_energyDrops[i].active) {
            continue;
        }
        if (writeIndex != i) {
            g_energyDrops[writeIndex] = g_energyDrops[i];
        }
        ++writeIndex;
    }

    g_energyDrops.resize(writeIndex);
    if (g_energyDropOverflowCursor >= g_energyDrops.size()) {
        g_energyDropOverflowCursor = 0;
    }
}

void UpdateEnergyDrops() {
    const float pickupRadiusSquared = static_cast<float>(kEnergyPickupRadius * kEnergyPickupRadius);
    bool hasInactive = false;

    for (EnergyDrop& drop : g_energyDrops) {
        if (!drop.active) {
            hasInactive = true;
            continue;
        }

        if (drop.settleFrames > 0) {
            drop.x += drop.vx;
            drop.y += drop.vy;
            drop.vx *= kVelocityDamping;
            drop.vy *= kVelocityDamping;
            if (std::fabs(drop.vx) < kMinVelocity) {
                drop.vx = 0.0f;
            }
            if (std::fabs(drop.vy) < kMinVelocity) {
                drop.vy = 0.0f;
            }
            --drop.settleFrames;
            ClampEnergyDropToMap(drop);
            ResolveEnergyDropAgainstObstacles(drop);
        }

        const float dx = drop.x - static_cast<float>(g_playerX);
        const float dy = drop.y - static_cast<float>(g_playerY);
        if (dx * dx + dy * dy <= pickupRadiusSquared) {
            ApplyDropPickupEffect(drop);
            drop.active = false;
            hasInactive = true;
        }
    }

    if (!hasInactive) {
        return;
    }

    RemoveInactiveEnergyDrops();
}

void DrawEnergyDropsLayered(int minYInclusive, int maxYExclusive) {
    for (const EnergyDrop& drop : g_energyDrops) {
        if (!drop.active) {
            continue;
        }

        if (drop.y < static_cast<float>(minYInclusive) || drop.y >= static_cast<float>(maxYExclusive)) {
            continue;
        }

        const DropRenderInfo renderInfo = GetDropRenderInfo(drop.kind);
        const bool hasIcon = (renderInfo.image != nullptr) && RenderUtils::HasImage(*renderInfo.image);
        const int halfSize = renderInfo.drawSize / 2;
        const int centerX = static_cast<int>(drop.x) - g_cameraX;
        const int centerY = static_cast<int>(drop.y) - g_cameraY;
        if (centerX < -halfSize || centerX > GAME_WINDOW_WIDTH + halfSize ||
            centerY < -halfSize || centerY > GAME_WINDOW_HEIGHT + halfSize) {
            continue;
        }

        if (hasIcon) {
            RenderUtils::DrawImageAuto(
                *renderInfo.image,
                renderInfo.hasAlpha,
                centerX - halfSize,
                centerY - halfSize,
                renderInfo.drawSize,
                renderInfo.drawSize);
        }
        else {
            setfillcolor(renderInfo.fallbackColor);
            solidcircle(centerX, centerY, halfSize);
        }
    }
}

}
