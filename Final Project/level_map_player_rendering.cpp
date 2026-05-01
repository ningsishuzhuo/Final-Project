#include "level_map_internal.h"

#include <cmath>

namespace LevelMapInternal {
namespace {

bool FindImageVisibleBounds(
    const IMAGE& image,
    bool hasMeaningfulAlpha,
    int& outX,
    int& outY,
    int& outW,
    int& outH) {
    const int width = image.getwidth();
    const int height = image.getheight();
    if (width <= 0 || height <= 0) {
        return false;
    }

    DWORD* buffer = GetImageBuffer(const_cast<IMAGE*>(&image));
    if (buffer == nullptr) {
        return false;
    }

    int minX = width;
    int minY = height;
    int maxX = -1;
    int maxY = -1;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const DWORD pixel = buffer[y * width + x];
            const unsigned char alpha = static_cast<unsigned char>((pixel >> 24) & 0xFF);
            const bool visible = hasMeaningfulAlpha
                ? (alpha > 8)
                : ((pixel & 0x00FFFFFF) != 0);
            if (!visible) {
                continue;
            }

            if (x < minX) {
                minX = x;
            }
            if (y < minY) {
                minY = y;
            }
            if (x > maxX) {
                maxX = x;
            }
            if (y > maxY) {
                maxY = y;
            }
        }
    }

    if (maxX < minX || maxY < minY) {
        return false;
    }

    outX = minX;
    outY = minY;
    outW = maxX - minX + 1;
    outH = maxY - minY + 1;
    return true;
}

void DrawImageAutoRegion(
    const IMAGE& image,
    bool hasMeaningfulAlpha,
    int srcX,
    int srcY,
    int srcW,
    int srcH,
    int dstX,
    int dstY,
    int dstW,
    int dstH) {
    if (srcW <= 0 || srcH <= 0 || dstW <= 0 || dstH <= 0) {
        return;
    }

    if (hasMeaningfulAlpha) {
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
        AlphaBlend(
            GetImageHDC(),
            dstX,
            dstY,
            dstW,
            dstH,
            GetImageHDC(const_cast<IMAGE*>(&image)),
            srcX,
            srcY,
            srcW,
            srcH,
            bf);
        return;
    }

    TransparentBlt(
        GetImageHDC(),
        dstX,
        dstY,
        dstW,
        dstH,
        GetImageHDC(const_cast<IMAGE*>(&image)),
        srcX,
        srcY,
        srcW,
        srcH,
        RGB(0, 0, 0));
}

int GetPlayerDeathTargetSize(int characterIndex) {
    switch (characterIndex) {
    case AssetPaths::CHARACTER_SUN:
        return 108;
    case AssetPaths::CHARACTER_LOVE:
        return 116;
    case AssetPaths::CHARACTER_MOON:
    default:
        return 112;
    }
}

bool DrawPlayerDeathImage(int characterIndex) {
    if (!RenderUtils::HasImage(g_playerDeathImages[characterIndex])) {
        return false;
    }

    int srcX = 0;
    int srcY = 0;
    int srcW = 0;
    int srcH = 0;
    if (!FindImageVisibleBounds(
        g_playerDeathImages[characterIndex],
        g_playerDeathHasAlpha[characterIndex],
        srcX,
        srcY,
        srcW,
        srcH)) {
        return false;
    }

    const int targetSize = GetPlayerDeathTargetSize(characterIndex);
    int drawW = targetSize;
    int drawH = targetSize;
    if (srcW >= srcH) {
        drawH = max(1, targetSize * srcH / srcW);
    }
    else {
        drawW = max(1, targetSize * srcW / srcH);
    }

    const int drawX = g_playerX - g_cameraX - drawW / 2;
    const int drawY = g_playerY - g_cameraY - drawH / 2;
    DrawImageAutoRegion(
        g_playerDeathImages[characterIndex],
        g_playerDeathHasAlpha[characterIndex],
        srcX,
        srcY,
        srcW,
        srcH,
        drawX,
        drawY,
        drawW,
        drawH);
    return true;
}

void DrawMoonChargeCountdown(int playerScreenX, int playerScreenY) {
    if (g_currentCharacterIndex != AssetPaths::CHARACTER_MOON || !g_moonRainChargeState.active) {
        return;
    }

    const ULONGLONG now = RenderUtils::NowTickMs();
    const ULONGLONG elapsedMs =
        (now >= g_moonRainChargeState.startTick) ? (now - g_moonRainChargeState.startTick) : 0ULL;
    int filledCount = 0;
    if (kMoonRainChargeDurationMs > 0ULL) {
        const ULONGLONG clampedElapsed =
            (elapsedMs > kMoonRainChargeDurationMs) ? kMoonRainChargeDurationMs : elapsedMs;
        filledCount = static_cast<int>(
            (clampedElapsed * static_cast<ULONGLONG>(kMoonRainChargeBoxCount)) / kMoonRainChargeDurationMs);
    }
    if (filledCount < 0) {
        filledCount = 0;
    }
    if (filledCount > kMoonRainChargeBoxCount) {
        filledCount = kMoonRainChargeBoxCount;
    }

    const int totalWidth =
        kMoonRainChargeBoxCount * kMoonRainChargeBoxSize +
        (kMoonRainChargeBoxCount - 1) * kMoonRainChargeBoxGap;
    const int baseX = playerScreenX + (kPlayerDrawW - totalWidth) / 2;
    const int baseY = playerScreenY - 26;

    for (int i = 0; i < kMoonRainChargeBoxCount; ++i) {
        const int left = baseX + i * (kMoonRainChargeBoxSize + kMoonRainChargeBoxGap);
        const int top = baseY;
        const int right = left + kMoonRainChargeBoxSize;
        const int bottom = top + kMoonRainChargeBoxSize;

        if (i < filledCount) {
            setfillcolor(RGB(146, 92, 240));
            solidrectangle(left, top, right, bottom);
            setlinecolor(WHITE);
            rectangle(left, top, right, bottom);
        }
        else {
            setlinecolor(RGB(152, 72, 234));
            rectangle(left, top, right, bottom);
        }
    }
}

}  

void DrawPlayer() {
    int characterIndex = g_currentCharacterIndex;
    if (characterIndex < 0 || characterIndex >= kCharacterCount) {
        characterIndex = 0;
    }

    if (g_playerIsDead) {
        if (DrawPlayerDeathImage(characterIndex)) {
            return;
        }
    }

    const ULONGLONG now = RenderUtils::NowTickMs();
    if (g_moonUltimateState.active && now >= g_moonUltimateState.endTick) {
        g_moonUltimateState = {};
    }
    if (g_sunUltimateState.active && now >= g_sunUltimateState.endTick) {
        g_sunUltimateState = {};
    }
    if (g_loveUltimateState.active && now >= g_loveUltimateState.endTick) {
        g_loveUltimateState = {};
    }
    if (g_lovePurifyState.active && now >= g_lovePurifyState.endTick) {
        g_lovePurifyState = {};
    }

    if (characterIndex == AssetPaths::CHARACTER_LOVE &&
        g_lovePurifyState.active &&
        RenderUtils::HasImage(g_lovePurifyCircleImage)) {
        const int purifyX = g_playerX - g_cameraX - kLovePurifyCircleDrawW / 2;
        const int purifyY = g_playerY + kPlayerFootOffsetY - g_cameraY - kLovePurifyCircleDrawH / 2;
        RenderUtils::DrawImageAuto(
            g_lovePurifyCircleImage,
            g_lovePurifyCircleHasAlpha,
            purifyX,
            purifyY,
            kLovePurifyCircleDrawW,
            kLovePurifyCircleDrawH);
    }

    if (characterIndex == AssetPaths::CHARACTER_MOON &&
        g_moonUltimateState.active &&
        RenderUtils::HasImage(g_moonUltimateWaveImage)) {
        const int waveX = g_playerX - g_cameraX - kMoonUltimateMarkDrawW / 2;
        const int waveY = g_playerY - g_cameraY - kMoonUltimateMarkDrawH / 2 + kMoonUltimateMarkOffsetY;
        RenderUtils::DrawImageAuto(
            g_moonUltimateWaveImage,
            g_moonUltimateWaveHasAlpha,
            waveX,
            waveY,
            kMoonUltimateMarkDrawW,
            kMoonUltimateMarkDrawH);
    }
    else if (characterIndex == AssetPaths::CHARACTER_SUN &&
        g_sunUltimateState.active &&
        RenderUtils::HasImage(g_sunUltimateShieldImage)) {
        const int shieldX = g_playerX - g_cameraX - kSunUltimateMarkDrawW / 2;
        const int shieldY = g_playerY - g_cameraY - kSunUltimateMarkDrawH / 2 + kSunUltimateMarkOffsetY;
        RenderUtils::DrawImageAuto(
            g_sunUltimateShieldImage,
            g_sunUltimateShieldHasAlpha,
            shieldX,
            shieldY,
            kSunUltimateMarkDrawW,
            kSunUltimateMarkDrawH);
    }
    else if (characterIndex == AssetPaths::CHARACTER_LOVE &&
        g_loveUltimateState.active &&
        RenderUtils::HasImage(g_loveUltimateRecoverCircleImage)) {
        const int circleX = g_playerX - g_cameraX - kLoveUltimateMarkDrawW / 2;
        const int circleY = g_playerY - g_cameraY - kLoveUltimateMarkDrawH / 2 + kLoveUltimateMarkOffsetY;
        RenderUtils::DrawImageAuto(
            g_loveUltimateRecoverCircleImage,
            g_loveUltimateRecoverCircleHasAlpha,
            circleX,
            circleY,
            kLoveUltimateMarkDrawW,
            kLoveUltimateMarkDrawH);
    }

    AnimatedGif& playerGif = g_isMoving ? g_walkGifs[characterIndex] : g_idleGifs[characterIndex];
    RenderUtils::UpdateAnimatedGifFrame(playerGif);
    const int playerScreenX = g_playerX - g_cameraX - kPlayerDrawW / 2;
    const int playerScreenY = g_playerY - g_cameraY - kPlayerDrawH / 2;
    RenderUtils::DrawAnimatedGif(playerGif, playerScreenX, playerScreenY, kPlayerDrawW, kPlayerDrawH, !g_faceRight);
    if (characterIndex == AssetPaths::CHARACTER_SUN &&
        g_sunCriticalHitCount >= kSunCriticalReadyHitCount &&
        RenderUtils::HasImage(g_sunCriticalHaloImage)) {
        const int haloX = playerScreenX + kPlayerDrawW / 2 - kSunCriticalHaloDrawSize / 2;
        const int haloY = playerScreenY - kSunCriticalHaloDrawSize + 10;
        RenderUtils::DrawImageAuto(
            g_sunCriticalHaloImage,
            g_sunCriticalHaloHasAlpha,
            haloX,
            haloY,
            kSunCriticalHaloDrawSize,
            kSunCriticalHaloDrawSize);
    }
    DrawMoonChargeCountdown(playerScreenX, playerScreenY);

    if (characterIndex != AssetPaths::CHARACTER_SUN || !g_apolloSlashState.active) {
        return;
    }

    if (now > g_apolloSlashState.endTick) {
        g_apolloSlashState = {};
        return;
    }
    if (!g_apolloSlashGif.IsLoaded()) {
        return;
    }

    RenderUtils::UpdateAnimatedGifFrame(g_apolloSlashGif);
    const int slashCenterX =
        g_playerX - g_cameraX + static_cast<int>(std::round(g_apolloSlashState.dirX * static_cast<float>(kApolloSlashForwardOffset)));
    const int slashCenterY =
        g_playerY - g_cameraY + static_cast<int>(std::round(g_apolloSlashState.dirY * static_cast<float>(kApolloSlashForwardOffset)));
    const int slashX = slashCenterX - kApolloSlashDrawW / 2;
    const int slashY = slashCenterY - kApolloSlashDrawH / 2;
    const bool flip = g_apolloSlashState.dirX < 0.0f;
    RenderUtils::DrawAnimatedGif(g_apolloSlashGif, slashX, slashY, kApolloSlashDrawW, kApolloSlashDrawH, flip);
}

}  
