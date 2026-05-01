#include "level_map_internal.h"
#include "level_map_combat_helpers_internal.h"
#include "level_map_combat_internal.h"

#include <cmath>

namespace LevelMapInternal {
namespace {

constexpr float kPi = 3.14159265358979323846f;

bool IsLoveUltimateCurrentlyActive(ULONGLONG now) {
    return g_currentCharacterIndex == AssetPaths::CHARACTER_LOVE &&
        g_loveUltimateState.active &&
        now < g_loveUltimateState.endTick;
}

}  

void SpawnPlayerProjectile(int clickScreenX, int clickScreenY) {
    const ULONGLONG now = RenderUtils::NowTickMs();
    if (g_currentCharacterIndex == AssetPaths::CHARACTER_SUN) {
        SpawnApolloSlashAttack(clickScreenX, clickScreenY);
        return;
    }

    if (!GameData::GetCharacterDefinition(g_currentCharacterIndex).usesProjectileAttack) {
        return;
    }

    const bool loveUltimateActive = IsLoveUltimateCurrentlyActive(now);
    const int energyCost = kPlayerBulletEnergyCost *
        (loveUltimateActive ? kLoveUltimateEnergyCostMultiplier : 1);
    const bool hasRequiredImage = loveUltimateActive
        ? RenderUtils::HasImage(g_loveUltimateBulletImage)
        : RenderUtils::HasImage(g_particleImage);
    if (g_playerEnergy < static_cast<float>(energyCost) || !hasRequiredImage) {
        return;
    }

    const float targetWorldX = static_cast<float>(clickScreenX + g_cameraX);
    const float targetWorldY = static_cast<float>(clickScreenY + g_cameraY);
    float dirX = targetWorldX - static_cast<float>(g_playerX);
    float dirY = targetWorldY - static_cast<float>(g_playerY);
    NormalizeAimDirection(dirX, dirY);
    UpdatePlayerFacingByAim(dirX);

    const int projectileCount = loveUltimateActive ? 2 : 1;
    const float perpX = -dirY;
    const float perpY = dirX;
    for (int i = 0; i < projectileCount; ++i) {
        const float offset = loveUltimateActive
            ? ((i == 0) ? -static_cast<float>(kLoveUltimateBulletRowOffset) : static_cast<float>(kLoveUltimateBulletRowOffset))
            : 0.0f;

        Projectile projectile;
        projectile.x = static_cast<float>(g_playerX) + perpX * offset;
        projectile.y = static_cast<float>(g_playerY) + perpY * offset;
        projectile.vx = dirX * static_cast<float>(kPlayerProjectileSpeed);
        projectile.vy = dirY * static_cast<float>(kPlayerProjectileSpeed);
        projectile.angleDegrees =
            static_cast<float>(std::atan2(projectile.vy, projectile.vx) * 180.0 / static_cast<double>(kPi));
        projectile.hitArmedTick = 0;
        projectile.loveUltimateBullet = loveUltimateActive;
        projectile.active = true;

        PushProjectileCapped(
            g_playerProjectiles,
            projectile,
            static_cast<size_t>(kPlayerProjectileMaxCount),
            g_playerProjectileOverflowCursor);
    }

    g_playerEnergy -= static_cast<float>(energyCost);
    ClampPlayerEnergy();
}

}  
