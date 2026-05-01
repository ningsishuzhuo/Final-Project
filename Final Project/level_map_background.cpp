#include "level_map_internal.h"

#include <array>
#include <cstdint>

namespace LevelMapInternal {
namespace {

constexpr int kIceGroundTileSize = 44;
constexpr COLORREF kIceVoidColor = RGB(10, 28, 48);
constexpr COLORREF kIceGroundBaseColor = RGB(178, 199, 208);
constexpr std::array<COLORREF, 6> kIceGroundPalette = {
    RGB(188, 206, 214),
    RGB(176, 197, 206),
    RGB(166, 190, 201),
    RGB(157, 183, 196),
    RGB(148, 176, 191),
    RGB(198, 214, 222)
};
constexpr std::array<COLORREF, 5> kIceGroundAccentPalette = {
    RGB(131, 168, 185),
    RGB(143, 177, 193),
    RGB(120, 156, 175),
    RGB(204, 220, 227),
    RGB(154, 186, 200)
};

struct IceWallVisual {
    bool loaded = false;
    IMAGE image;
    bool hasImage = false;
    bool hasAlpha = false;
};

unsigned int HashTile(int tileX, int tileY) {
    const unsigned int x = static_cast<unsigned int>(tileX);
    const unsigned int y = static_cast<unsigned int>(tileY);
    unsigned int h = x * 73856093u;
    h ^= y * 19349663u;
    h ^= (x + y) * 83492791u;
    return h;
}

int FloorDiv(int value, int divisor) {
    if (divisor <= 0) {
        return 0;
    }
    if (value >= 0) {
        return value / divisor;
    }
    return -(((-value) + divisor - 1) / divisor);
}

int MaxInt(int a, int b) {
    return (a > b) ? a : b;
}

int MinInt(int a, int b) {
    return (a < b) ? a : b;
}

IceWallVisual& GetIceWallVisual() {
    static IceWallVisual visual;
    if (visual.loaded) {
        return visual;
    }

    RenderUtils::LoadImageFlexible(visual.image, AssetPaths::GetIcefieldWallPath(), 0, 0);
    visual.hasImage = RenderUtils::HasImage(visual.image);
    visual.hasAlpha = visual.hasImage && RenderUtils::HasMeaningfulAlpha(visual.image);
    visual.loaded = true;
    return visual;
}

void DrawIceGroundPixelStyle() {
    const int groundLeft = g_iceRegionRect.left;
    const int groundTop = g_iceRegionRect.top;
    const int groundRight = g_iceRegionRect.right;
    const int groundBottom = g_iceRegionRect.bottom;
    const int iceLeft = groundLeft - g_cameraX;
    const int iceTop = groundTop - g_cameraY;
    const int iceRight = groundRight - g_cameraX;
    const int iceBottom = groundBottom - g_cameraY;

    setfillcolor(kIceGroundBaseColor);
    solidrectangle(iceLeft, iceTop, iceRight, iceBottom);

    const int tileSize = kIceGroundTileSize;
    const int minTileX = FloorDiv(groundLeft, tileSize);
    const int minTileY = FloorDiv(groundTop, tileSize);
    const int maxTileX = FloorDiv(groundRight + tileSize - 1, tileSize);
    const int maxTileY = FloorDiv(groundBottom + tileSize - 1, tileSize);

    for (int tileY = minTileY; tileY <= maxTileY; ++tileY) {
        for (int tileX = minTileX; tileX <= maxTileX; ++tileX) {
            const int worldX = tileX * tileSize;
            const int worldY = tileY * tileSize;
            const int clippedLeft = MaxInt(worldX, groundLeft);
            const int clippedTop = MaxInt(worldY, groundTop);
            const int clippedRight = MinInt(worldX + tileSize, groundRight);
            const int clippedBottom = MinInt(worldY + tileSize, groundBottom);
            if (clippedLeft >= clippedRight || clippedTop >= clippedBottom) {
                continue;
            }

            const int sx = clippedLeft - g_cameraX;
            const int sy = clippedTop - g_cameraY;
            const int ex = clippedRight - g_cameraX;
            const int ey = clippedBottom - g_cameraY;

            const unsigned int h = HashTile(tileX, tileY);
            const COLORREF baseColor = kIceGroundPalette[h % kIceGroundPalette.size()];
            setfillcolor(baseColor);
            solidrectangle(sx, sy, ex, ey);

            if ((h & 7u) <= 2u) {
                const int insetX = 5 + static_cast<int>((h >> 4) % 10u);
                const int insetY = 5 + static_cast<int>((h >> 8) % 10u);
                const int patchW = tileSize / 2 + static_cast<int>((h >> 12) % 8u);
                const int patchH = tileSize / 2 + static_cast<int>((h >> 16) % 8u);
                const int patchLeftWorld = worldX + insetX;
                const int patchTopWorld = worldY + insetY;
                const int patchRightWorld = patchLeftWorld + patchW;
                const int patchBottomWorld = patchTopWorld + patchH;
                const int patchLeft = MaxInt(patchLeftWorld, groundLeft) - g_cameraX;
                const int patchTop = MaxInt(patchTopWorld, groundTop) - g_cameraY;
                const int patchRight = MinInt(patchRightWorld, groundRight) - g_cameraX;
                const int patchBottom = MinInt(patchBottomWorld, groundBottom) - g_cameraY;
                if (patchLeft >= patchRight || patchTop >= patchBottom) {
                    continue;
                }
                const COLORREF accentColor = kIceGroundAccentPalette[(h >> 20) % kIceGroundAccentPalette.size()];
                setfillcolor(accentColor);
                solidrectangle(patchLeft, patchTop, patchRight, patchBottom);
            }
        }
    }
}

void DrawFallbackIceWall() {
    const int iceLeft = -g_cameraX;
    const int iceTop = -g_cameraY;
    const int iceRight = kLevelMapWidth - g_cameraX;
    const int iceBottom = kLevelMapHeight - g_cameraY;

    setfillcolor(RGB(207, 220, 228));
    solidrectangle(iceLeft, iceTop, iceRight, iceTop + kIceWallThickness);
    solidrectangle(iceLeft, iceBottom - kIceWallThickness, iceRight, iceBottom);
    solidrectangle(iceLeft, iceTop, iceLeft + kIceWallThickness, iceBottom);
    solidrectangle(iceRight - kIceWallThickness, iceTop, iceRight, iceBottom);

    setfillcolor(RGB(160, 188, 201));
    solidrectangle(iceLeft, iceTop + kIceWallThickness - 8, iceRight, iceTop + kIceWallThickness + 6);
    solidrectangle(iceLeft, iceBottom - kIceWallThickness - 6, iceRight, iceBottom - kIceWallThickness + 8);
}

void DrawIceWallPerimeter() {
    IceWallVisual& wall = GetIceWallVisual();
    if (!wall.hasImage) {
        DrawFallbackIceWall();
        return;
    }

    const int tileW = kIceWallThickness;
    const int tileH = kIceWallThickness;
    const int bottomY = kLevelMapHeight - tileH;
    const int rightX = kLevelMapWidth - tileW;

    const int minWorldX = MaxInt(0, g_cameraX - tileW);
    const int maxWorldX = MinInt(kLevelMapWidth - tileW, g_cameraX + GAME_WINDOW_WIDTH + tileW);
    const int startWorldX = FloorDiv(minWorldX, tileW) * tileW;
    for (int worldX = startWorldX; worldX <= maxWorldX; worldX += tileW) {
        const int sx = worldX - g_cameraX;
        const int topSy = -g_cameraY;
        const int bottomSy = bottomY - g_cameraY;
        RenderUtils::DrawImageAuto(wall.image, wall.hasAlpha, sx, topSy, tileW, tileH);
        RenderUtils::DrawImageAuto(wall.image, wall.hasAlpha, sx, bottomSy, tileW, tileH);
    }

    const int minWorldY = MaxInt(0, g_cameraY - tileH);
    const int maxWorldY = MinInt(kLevelMapHeight - tileH, g_cameraY + GAME_WINDOW_HEIGHT + tileH);
    const int startWorldY = FloorDiv(minWorldY, tileH) * tileH;
    for (int worldY = startWorldY; worldY <= maxWorldY; worldY += tileH) {
        const int sy = worldY - g_cameraY;
        const int leftSx = -g_cameraX;
        const int rightSx = rightX - g_cameraX;
        RenderUtils::DrawImageAuto(wall.image, wall.hasAlpha, leftSx, sy, tileW, tileH);
        RenderUtils::DrawImageAuto(wall.image, wall.hasAlpha, rightSx, sy, tileW, tileH);
    }

    const int innerLeft = g_iceRegionRect.left - g_cameraX;
    const int innerTop = g_iceRegionRect.top - g_cameraY;
    const int innerRight = g_iceRegionRect.right - g_cameraX;
    const int innerBottom = g_iceRegionRect.bottom - g_cameraY;

    setfillcolor(RGB(214, 229, 236));
    solidrectangle(innerLeft - 2, innerTop - 10, innerRight + 2, innerTop - 2);
    solidrectangle(innerLeft - 2, innerBottom + 2, innerRight + 2, innerBottom + 10);
    solidrectangle(innerLeft - 10, innerTop - 2, innerLeft - 2, innerBottom + 2);
    solidrectangle(innerRight + 2, innerTop - 2, innerRight + 10, innerBottom + 2);

    setfillcolor(RGB(128, 156, 170));
    solidrectangle(innerLeft - 1, innerTop - 2, innerRight + 1, innerTop + 8);
    solidrectangle(innerLeft - 1, innerBottom - 8, innerRight + 1, innerBottom + 2);
    solidrectangle(innerLeft - 2, innerTop - 1, innerLeft + 8, innerBottom + 1);
    solidrectangle(innerRight - 8, innerTop - 1, innerRight + 2, innerBottom + 1);

    setlinecolor(RGB(108, 137, 152));
    for (int x = innerLeft; x < innerRight; x += 52) {
        line(x, innerTop - 1, x + 8, innerTop + 7);
        line(x, innerBottom + 1, x + 8, innerBottom - 7);
    }
    for (int y = innerTop; y < innerBottom; y += 52) {
        line(innerLeft - 1, y, innerLeft + 7, y + 8);
        line(innerRight + 1, y, innerRight - 7, y + 8);
    }

    setlinecolor(RGB(142, 169, 182));
    rectangle(-g_cameraX, -g_cameraY, kLevelMapWidth - g_cameraX, kLevelMapHeight - g_cameraY);
}

}  

void DrawRegions() {
    setfillcolor(kIceVoidColor);
    solidrectangle(0, 0, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT);
    DrawIceGroundPixelStyle();
    DrawIceWallPerimeter();
}

}  
