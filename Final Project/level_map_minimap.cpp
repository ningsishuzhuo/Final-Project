#include "level_map_internal.h"
#include "level_map_hud_internal.h"

namespace LevelMapInternal {
namespace {
void DrawMinimapPoint(int mapX, int mapY, int innerW, int innerH, int worldX, int worldY, int radius, COLORREF color) {
    if (innerW <= 0 || innerH <= 0) {
        return;
    }

    int pointX = mapX + kMinimapPadding + worldX * innerW / kLevelMapWidth;
    int pointY = mapY + kMinimapPadding + worldY * innerH / kLevelMapHeight;
    const int minX = mapX + kMinimapPadding + radius;
    const int minY = mapY + kMinimapPadding + radius;
    const int maxX = mapX + kMinimapPadding + innerW - radius;
    const int maxY = mapY + kMinimapPadding + innerH - radius;
    if (pointX < minX) pointX = minX;
    if (pointX > maxX) pointX = maxX;
    if (pointY < minY) pointY = minY;
    if (pointY > maxY) pointY = maxY;

    setfillcolor(color);
    solidcircle(pointX, pointY, radius);
}

void DrawMinimapObstacle(int mapX, int mapY, int innerW, int innerH, const MapObstacle& obstacle) {
    if (obstacle.destroyed || obstacle.kind == MapObstacleKind::Rail) {
        return;
    }

    const int centerX = static_cast<int>(obstacle.x);
    const int centerY = static_cast<int>(obstacle.y);
    int pointX = mapX + kMinimapPadding + centerX * innerW / kLevelMapWidth;
    int pointY = mapY + kMinimapPadding + centerY * innerH / kLevelMapHeight;
    const int halfSize = kMinimapObstacleDotSize / 2;
    const int minX = mapX + kMinimapPadding + halfSize;
    const int minY = mapY + kMinimapPadding + halfSize;
    const int maxX = mapX + kMinimapPadding + innerW - halfSize;
    const int maxY = mapY + kMinimapPadding + innerH - halfSize;
    if (pointX < minX) pointX = minX;
    if (pointX > maxX) pointX = maxX;
    if (pointY < minY) pointY = minY;
    if (pointY > maxY) pointY = maxY;

    setfillcolor(RGB(245, 248, 255));
    solidrectangle(pointX - halfSize, pointY - halfSize, pointX + halfSize, pointY + halfSize);
}

void DrawTranslucentMinimapBackground(int mapX, int mapY, int mapW, int mapH) {
    static IMAGE background;
    if (background.getwidth() != mapW || background.getheight() != mapH) {
        background.Resize(mapW, mapH);
        DWORD* pixels = GetImageBuffer(&background);
        if (pixels != nullptr) {
            const int pixelCount = mapW * mapH;
            for (int i = 0; i < pixelCount; ++i) {
                pixels[i] = (static_cast<DWORD>(168) << 24);
            }
        }
    }

    RenderUtils::DrawImageAuto(background, true, mapX, mapY, mapW, mapH);
}

}  

void DrawMinimap() {
    if ((GetAsyncKeyState(VK_TAB) & 0x8000) == 0) {
        return;
    }

    const int mapW = GAME_WINDOW_WIDTH * kMinimapScreenNumerator / kMinimapScreenDenominator;
    const int mapH = GAME_WINDOW_HEIGHT * kMinimapScreenNumerator / kMinimapScreenDenominator;
    const int mapX = (GAME_WINDOW_WIDTH - mapW) / 2;
    const int mapY = (GAME_WINDOW_HEIGHT - mapH) / 2;
    const int innerW = mapW - kMinimapPadding * 2;
    const int innerH = mapH - kMinimapPadding * 2;

    DrawTranslucentMinimapBackground(mapX, mapY, mapW, mapH);
    setlinecolor(RGB(210, 216, 226));
    rectangle(mapX, mapY, mapX + mapW, mapY + mapH);

    for (const MapObstacle& obstacle : GetMapObstacles()) {
        DrawMinimapObstacle(mapX, mapY, innerW, innerH, obstacle);
    }

    for (const IcefieldEnemy& enemy : g_icefieldEnemies) {
        if (!enemy.runtime.alive || enemy.runtime.hp <= 0) {
            continue;
        }
        DrawMinimapPoint(
            mapX,
            mapY,
            innerW,
            innerH,
            enemy.runtime.x,
            enemy.runtime.y,
            kMinimapEnemyDotRadius,
            RGB(220, 42, 52));
    }

    if (g_battlePhase == BattlePhase::BossFight && g_enemy.alive && g_enemy.hp > 0) {
        DrawMinimapPoint(
            mapX,
            mapY,
            innerW,
            innerH,
            g_enemy.x,
            g_enemy.y,
            kMinimapEnemyDotRadius,
            RGB(220, 42, 52));
    }

    DrawMinimapPoint(
        mapX,
        mapY,
        innerW,
        innerH,
        g_playerX,
        g_playerY,
        kMinimapPlayerDotRadius,
        RGB(80, 230, 110));
}

}  
