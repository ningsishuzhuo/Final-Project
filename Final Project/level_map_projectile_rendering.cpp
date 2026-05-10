#include "level_map_internal.h"
#include "level_map_rendering_effects_internal.h"

#include <array>

namespace LevelMapInternal {
void DrawProjectileArrayLayered(
    const std::vector<Projectile>& arr,
    const IMAGE& image,
    bool hasAlpha,
    int w,
    int h,
    int minYInclusive,
    int maxYExclusive) {
    if (!RenderUtils::HasImage(image) && !RenderUtils::HasImage(g_loveUltimateBulletAsset.image)) {
        return;
    }

    for (const Projectile& projectile : arr) {
        if (!projectile.active) {
            continue;
        }
        if (!IsWithinLayerRange(projectile.y, minYInclusive, maxYExclusive)) {
            continue;
        }

        const int centerX = static_cast<int>(projectile.x) - g_cameraX;
        const int centerY = static_cast<int>(projectile.y) - g_cameraY;
        if (!IsScreenVisible(centerX, centerY, w / 2, h / 2)) {
            continue;
        }

        if (projectile.loveUltimateBullet && RenderUtils::HasImage(g_loveUltimateBulletAsset.image)) {
            if (RenderUtils::IsGdiplusReady()) {
                RenderUtils::DrawImageFileRotated(
                    AssetPaths::GetLoveUltimateBulletPath(),
                    centerX,
                    centerY,
                    kLoveUltimateBulletDrawW,
                    kLoveUltimateBulletDrawH,
                    projectile.angleDegrees);
                continue;
            }

            RenderUtils::DrawImageAuto(
                g_loveUltimateBulletAsset.image,
                g_loveUltimateBulletAsset.hasAlpha,
                centerX - kLoveUltimateBulletDrawW / 2,
                centerY - kLoveUltimateBulletDrawH / 2,
                kLoveUltimateBulletDrawW,
                kLoveUltimateBulletDrawH);
            continue;
        }

        if (RenderUtils::HasImage(image)) {
            const int sx = centerX - w / 2;
            const int sy = centerY - h / 2;
            RenderUtils::DrawImageAuto(image, hasAlpha, sx, sy, w, h);
        }
    }
}

struct CachedEnemyBulletVisual {
    bool loaded = false;
    IMAGE image;
    bool hasImage = false;
    bool hasAlpha = false;
};

constexpr int kEnemyKindCount = 3;

int EnemyKindIndex(GameData::EnemyKind kind) {
    const int index = static_cast<int>(kind);
    return (index >= 0 && index < kEnemyKindCount) ? index : 0;
}

CachedEnemyBulletVisual& EnemyBulletVisualCache(GameData::EnemyKind kind) {
    static std::array<CachedEnemyBulletVisual, kEnemyKindCount> caches = {};
    CachedEnemyBulletVisual& cache = caches[EnemyKindIndex(kind)];
    if (cache.loaded) {
        return cache;
    }

    const GameData::EnemyDefinition& def = GameData::GetEnemyDefinition(kind);
    if (def.bulletImagePath != nullptr) {
        RenderUtils::LoadImageFlexible(cache.image, def.bulletImagePath, 0, 0);
    }
    cache.hasImage = RenderUtils::HasImage(cache.image);
    cache.hasAlpha = cache.hasImage && RenderUtils::HasMeaningfulAlpha(cache.image);
    cache.loaded = true;
    return cache;
}

void DrawEnemyProjectileArrayLayered(int w, int h, int minYInclusive, int maxYExclusive) {
    if (w <= 0 || h <= 0) {
        const GameData::EnemyDefinition& minerDef = GameData::GetEnemyDefinition(GameData::EnemyKind::Miner);
        w = (minerDef.bulletDrawWidth > 0) ? minerDef.bulletDrawWidth : 34;
        h = (minerDef.bulletDrawHeight > 0) ? minerDef.bulletDrawHeight : 18;
    }

    for (const Projectile& projectile : g_enemyProjectiles) {
        if (!projectile.active) {
            continue;
        }
        if (!IsWithinLayerRange(projectile.y, minYInclusive, maxYExclusive)) {
            continue;
        }

        const int centerX = static_cast<int>(projectile.x) - g_cameraX;
        const int centerY = static_cast<int>(projectile.y) - g_cameraY;
        if (!IsScreenVisible(centerX, centerY, w / 2, h / 2)) {
            continue;
        }

        const GameData::EnemyDefinition& sourceDef = GameData::GetEnemyDefinition(projectile.sourceKind);
        const int drawW = (sourceDef.bulletDrawWidth > 0) ? sourceDef.bulletDrawWidth : w;
        const int drawH = (sourceDef.bulletDrawHeight > 0) ? sourceDef.bulletDrawHeight : h;
        if (projectile.sourceKind == GameData::EnemyKind::SnowApeKing &&
            sourceDef.bulletImagePath != nullptr &&
            RenderUtils::IsGdiplusReady()) {
            RenderUtils::DrawImageFileRotated(
                sourceDef.bulletImagePath,
                centerX,
                centerY,
                drawW,
                drawH,
                projectile.angleDegrees);
            continue;
        }
        CachedEnemyBulletVisual& bulletVisual = EnemyBulletVisualCache(projectile.sourceKind);
        if (bulletVisual.hasImage) {
            RenderUtils::DrawImageAutoRotated(
                bulletVisual.image,
                bulletVisual.hasAlpha,
                centerX,
                centerY,
                drawW,
                drawH,
                projectile.angleDegrees);
            continue;
        }

        setlinecolor(RGB(80, 235, 255));
        line(
            centerX,
            centerY,
            centerX + static_cast<int>(projectile.vx * 2.2f),
            centerY + static_cast<int>(projectile.vy * 2.2f));
        setfillcolor(RGB(40, 190, 255));
        solidcircle(centerX, centerY, 5);
        setfillcolor(RGB(226, 250, 255));
        solidcircle(centerX, centerY, 3);
    }
}
}  
