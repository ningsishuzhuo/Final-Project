#include "level_map_internal.h"
#include "level_map_hud_internal.h"

namespace LevelMapInternal {
namespace {
bool GetCurrentUltimateTiming(ULONGLONG& startTick, ULONGLONG& endTick) {
    switch (g_currentCharacterIndex) {
    case AssetPaths::CHARACTER_MOON:
        if (!g_moonUltimateState.active) {
            return false;
        }
        startTick = g_moonUltimateState.startTick;
        endTick = g_moonUltimateState.endTick;
        return true;
    case AssetPaths::CHARACTER_SUN:
        if (!g_sunUltimateState.active) {
            return false;
        }
        startTick = g_sunUltimateState.startTick;
        endTick = g_sunUltimateState.endTick;
        return true;
    case AssetPaths::CHARACTER_LOVE:
        if (!g_loveUltimateState.active) {
            return false;
        }
        startTick = g_loveUltimateState.startTick;
        endTick = g_loveUltimateState.endTick;
        return true;
    default:
        return false;
    }
}

int ScaleDurationToPixels(ULONGLONG elapsed, ULONGLONG duration, int pixelSize) {
    if (duration == 0ULL) {
        return pixelSize;
    }
    if (elapsed >= duration) {
        return pixelSize;
    }
    return static_cast<int>((elapsed * static_cast<ULONGLONG>(pixelSize)) / duration);
}

bool GetUltimateHudVisibleYBounds(const IMAGE& image, int iconSize, int& outTop, int& outBottom) {
    const int width = image.getwidth();
    const int height = image.getheight();
    if (width <= 0 || height <= 0 || iconSize <= 0) {
        return false;
    }

    DWORD* pixels = GetImageBuffer(const_cast<IMAGE*>(&image));
    if (pixels == nullptr) {
        return false;
    }

    int minY = height;
    int maxY = -1;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const DWORD pixel = pixels[y * width + x];
            const int alpha = static_cast<int>((pixel >> 24) & 0xFF);
            if (alpha <= 8) {
                continue;
            }

            if (y < minY) {
                minY = y;
            }
            if (y > maxY) {
                maxY = y;
            }
        }
    }

    if (maxY < minY) {
        return false;
    }

    outTop = (minY * iconSize) / height;
    outBottom = ((maxY + 1) * iconSize + height - 1) / height;
    if (outTop < 0) {
        outTop = 0;
    }
    if (outBottom > iconSize) {
        outBottom = iconSize;
    }
    if (outBottom <= outTop) {
        outBottom = outTop + 1;
    }
    return true;
}

bool BuildUltimateHudCompositeIcon(
    IMAGE& composite,
    const IMAGE& colorIcon,
    const IMAGE& grayIcon,
    int iconSize,
    int colorStartY) {
    const int colorW = colorIcon.getwidth();
    const int colorH = colorIcon.getheight();
    const int grayW = grayIcon.getwidth();
    const int grayH = grayIcon.getheight();
    if (iconSize <= 0 || colorW <= 0 || colorH <= 0 || grayW <= 0 || grayH <= 0) {
        return false;
    }

    if (colorStartY < 0) {
        colorStartY = 0;
    }
    if (colorStartY > iconSize) {
        colorStartY = iconSize;
    }

    composite.Resize(iconSize, iconSize);
    DWORD* dst = GetImageBuffer(&composite);
    DWORD* color = GetImageBuffer(const_cast<IMAGE*>(&colorIcon));
    DWORD* gray = GetImageBuffer(const_cast<IMAGE*>(&grayIcon));
    if (dst == nullptr || color == nullptr || gray == nullptr) {
        composite.Resize(0, 0);
        return false;
    }

    for (int y = 0; y < iconSize; ++y) {
        const bool useColor = y >= colorStartY;
        const int srcY = useColor ? (y * colorH / iconSize) : (y * grayH / iconSize);
        for (int x = 0; x < iconSize; ++x) {
            if (useColor) {
                const int srcX = x * colorW / iconSize;
                dst[y * iconSize + x] = color[srcY * colorW + srcX];
            }
            else {
                const int srcX = x * grayW / iconSize;
                dst[y * iconSize + x] = gray[srcY * grayW + srcX];
            }
        }
    }

    return true;
}

}  

void DrawUltimateCooldownHud(ULONGLONG now) {
    if (g_currentCharacterIndex < 0 || g_currentCharacterIndex >= kCharacterCount) {
        return;
    }

    const IMAGE& colorIcon = g_ultimateCooldownIconAssets[g_currentCharacterIndex].image;
    const IMAGE& grayIcon = g_ultimateCooldownGrayIconAssets[g_currentCharacterIndex].image;
    if (!RenderUtils::HasImage(colorIcon) || !RenderUtils::HasImage(grayIcon)) {
        return;
    }

    const int iconSize = kUltimateHudIconDrawSize;
    const int iconX = GAME_WINDOW_WIDTH - kUltimateHudIconRight - iconSize;
    const int iconY = kUltimateHudIconTop;
    const bool colorAlpha = g_ultimateCooldownIconAssets[g_currentCharacterIndex].hasAlpha;
    const bool compositeAlpha =
        colorAlpha || g_ultimateCooldownGrayIconAssets[g_currentCharacterIndex].hasAlpha;
    static IMAGE compositeIcon;
    static bool visibleBoundsCached[kCharacterCount] = {};
    static int visibleTop[kCharacterCount] = {};
    static int visibleBottom[kCharacterCount] = {};
    if (!visibleBoundsCached[g_currentCharacterIndex]) {
        if (!GetUltimateHudVisibleYBounds(
            colorIcon,
            iconSize,
            visibleTop[g_currentCharacterIndex],
            visibleBottom[g_currentCharacterIndex])) {
            visibleTop[g_currentCharacterIndex] = 0;
            visibleBottom[g_currentCharacterIndex] = iconSize;
        }
        visibleBoundsCached[g_currentCharacterIndex] = true;
    }
    const int effectTop = visibleTop[g_currentCharacterIndex];
    const int effectBottom = visibleBottom[g_currentCharacterIndex];
    const int effectHeight = effectBottom - effectTop;

    ULONGLONG startTick = 0;
    ULONGLONG endTick = 0;
    const bool active = GetCurrentUltimateTiming(startTick, endTick) && now < endTick;
    if (active) {
        const ULONGLONG elapsed = (now >= startTick) ? (now - startTick) : 0ULL;
        const int grayH = ScaleDurationToPixels(elapsed, kMoonUltimateVisualDurationMs, effectHeight);
        if (BuildUltimateHudCompositeIcon(compositeIcon, colorIcon, grayIcon, iconSize, effectTop + grayH)) {
            RenderUtils::DrawImageAuto(compositeIcon, compositeAlpha, iconX, iconY, iconSize, iconSize);
        }
        return;
    }

    const ULONGLONG nextCastTick = g_characterUltimateNextCastTick[g_currentCharacterIndex];
    if (nextCastTick > now) {
        const ULONGLONG cooldownStartTick =
            (nextCastTick > kCharacterUltimateCooldownMs) ? (nextCastTick - kCharacterUltimateCooldownMs) : 0ULL;
        const ULONGLONG elapsed = (now >= cooldownStartTick) ? (now - cooldownStartTick) : 0ULL;
        const int colorH = ScaleDurationToPixels(elapsed, kCharacterUltimateCooldownMs, effectHeight);
        if (BuildUltimateHudCompositeIcon(compositeIcon, colorIcon, grayIcon, iconSize, effectBottom - colorH)) {
            RenderUtils::DrawImageAuto(compositeIcon, compositeAlpha, iconX, iconY, iconSize, iconSize);
        }
        return;
    }

    RenderUtils::DrawImageAuto(colorIcon, colorAlpha, iconX, iconY, iconSize, iconSize);
}

}  
