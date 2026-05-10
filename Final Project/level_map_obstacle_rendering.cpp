#include "level_map_internal.h"
#include "level_map_obstacle_internal.h"

#include <array>
#include <climits>
#include <cmath>

namespace LevelMapInternal {
namespace {

struct ObstacleVisual {
    IMAGE image;
    bool hasImage = false;
    bool hasAlpha = false;
};

struct ObstacleAssets {
    std::array<ObstacleVisual, 8> visuals = {};
    AnimatedGif torchGif;
};

ObstacleAssets& Assets() {
    static ObstacleAssets assets;
    return assets;
}

int KindIndex(MapObstacleKind kind) {
    return static_cast<int>(kind);
}

ObstacleVisual& Visual(MapObstacleKind kind) {
    return Assets().visuals[KindIndex(kind)];
}

bool IsGroundOnlyKind(MapObstacleKind kind) {
    return kind == MapObstacleKind::Rail;
}

void LoadObstacleImage(MapObstacleKind kind, const TCHAR* path) {
    ObstacleVisual& visual = Visual(kind);
    RenderUtils::LoadImageFlexible(visual.image, path, 0, 0);
    visual.hasImage = RenderUtils::HasImage(visual.image);
    visual.hasAlpha = visual.hasImage && RenderUtils::HasMeaningfulAlpha(visual.image);
}

int ObstacleFootY(const MapObstacle& obstacle) {
    return static_cast<int>(std::round(obstacle.y + static_cast<float>(obstacle.height) * 0.5f));
}

void DrawSingleObstacle(const MapObstacle& obstacle) {
    const int drawX = static_cast<int>(std::round(obstacle.x)) - g_cameraX - obstacle.width / 2;
    const int drawY = static_cast<int>(std::round(obstacle.y)) - g_cameraY - obstacle.height / 2;
    if (obstacle.kind == MapObstacleKind::Torch && Assets().torchGif.IsLoaded()) {
        RenderUtils::DrawAnimatedGif(Assets().torchGif, drawX, drawY, obstacle.width, obstacle.height, false);
        return;
    }

    const ObstacleVisual& visual = Visual(obstacle.kind);
    if (visual.hasImage) {
        RenderUtils::DrawImageAuto(visual.image, visual.hasAlpha, drawX, drawY, obstacle.width, obstacle.height);
        return;
    }

    setfillcolor(RGB(110, 124, 138));
    solidrectangle(drawX, drawY, drawX + obstacle.width, drawY + obstacle.height);
}

void UpdateTorchFrame(ULONGLONG& lastUpdateTick) {
    if (!Assets().torchGif.IsLoaded()) {
        return;
    }

    const ULONGLONG now = RenderUtils::NowTickMs();
    if (now != lastUpdateTick) {
        RenderUtils::UpdateAnimatedGifFrame(Assets().torchGif);
        lastUpdateTick = now;
    }
}

}  

void LoadMapObstacleAssets() {
    LoadObstacleImage(MapObstacleKind::Rock, AssetPaths::GetIcefieldRockPath());
    LoadObstacleImage(MapObstacleKind::Minecart, AssetPaths::GetIcefieldMinecartPath());
    LoadObstacleImage(MapObstacleKind::BrokenMinecart, AssetPaths::GetIcefieldBrokenMinecartPath());
    LoadObstacleImage(MapObstacleKind::OreWall, AssetPaths::GetIcefieldOreWallPath());
    LoadObstacleImage(MapObstacleKind::IceWall, AssetPaths::GetIcefieldWallPath());
    LoadObstacleImage(MapObstacleKind::Rail, AssetPaths::GetIcefieldRailPath());
    LoadObstacleImage(MapObstacleKind::Crate, AssetPaths::GetIcefieldCratePath());
    RenderUtils::LoadAnimatedGif(Assets().torchGif, AssetPaths::GetIcefieldTorchPath());
}

void FreeMapObstacleAssets() {
    for (ObstacleVisual& visual : Assets().visuals) {
        visual.image.Resize(0, 0);
        visual.hasImage = false;
        visual.hasAlpha = false;
    }
    Assets().torchGif.Reset();
    Obstacles().clear();
}

void DrawMapObstacles() {
    DrawMapObstaclesLayered(INT_MIN, INT_MAX);
}

void DrawGroundObstacles() {
    static ULONGLONG s_lastTorchUpdateTick = 0;
    UpdateTorchFrame(s_lastTorchUpdateTick);

    for (const MapObstacle& obstacle : Obstacles()) {
        if (obstacle.destructible && obstacle.destroyed) {
            continue;
        }
        if (!IsGroundOnlyKind(obstacle.kind)) {
            continue;
        }
        DrawSingleObstacle(obstacle);
    }
}

void DrawMapObstaclesLayered(int minYInclusive, int maxYExclusive) {
    static ULONGLONG s_lastTorchUpdateTick = 0;
    UpdateTorchFrame(s_lastTorchUpdateTick);

    for (const MapObstacle& obstacle : Obstacles()) {
        if (obstacle.destructible && obstacle.destroyed) {
            continue;
        }
        if (IsGroundOnlyKind(obstacle.kind)) {
            continue;
        }
        const int footY = ObstacleFootY(obstacle);
        if (footY < minYInclusive || footY >= maxYExclusive) {
            continue;
        }
        DrawSingleObstacle(obstacle);
    }
}

}  
