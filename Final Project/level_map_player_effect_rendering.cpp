#include "level_map_internal.h"
#include "level_map_rendering_effects_internal.h"

#include <cmath>

namespace LevelMapInternal {
namespace {

float Hash01(unsigned int key) {
    unsigned int x = key * 747796405u + 2891336453u;
    x = ((x >> ((x >> 28u) + 4u)) ^ x) * 277803737u;
    x = (x >> 22u) ^ x;
    return static_cast<float>(x & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
}

void DrawMoonRainAimCircle(float centerWorldX, float centerWorldY, float radius) {
    if (radius < 0.0f) {
        return;
    }

    const int centerX = static_cast<int>(std::round(centerWorldX)) - g_cameraX;
    const int centerY = static_cast<int>(std::round(centerWorldY)) - g_cameraY;
    int drawSize = static_cast<int>(std::round(radius * 2.0f));
    if (drawSize < kMoonRainAimCircleMinDrawSize) {
        drawSize = kMoonRainAimCircleMinDrawSize;
    }
    if (drawSize > kMoonRainAimCircleMaxDrawSize) {
        drawSize = kMoonRainAimCircleMaxDrawSize;
    }

    if (RenderUtils::HasImage(g_moonRainAimCircleImage)) {
        RenderUtils::DrawImageAuto(
            g_moonRainAimCircleImage,
            g_moonRainAimCircleHasAlpha,
            centerX - drawSize / 2,
            centerY - drawSize / 2,
            drawSize,
            drawSize);
        return;
    }

    setlinecolor(RGB(170, 130, 255));
    circle(centerX, centerY, drawSize / 2);
}

void DrawMoonRainArrowBurst(ULONGLONG now) {
    if (!g_moonRainCastState.active) {
        return;
    }

    const ULONGLONG elapsed =
        (now >= g_moonRainCastState.startTick) ? (now - g_moonRainCastState.startTick) : 0ULL;
    const float radius = g_moonRainCastState.radius;
    if (radius <= 0.0f) {
        return;
    }

    const float startY = g_moonRainCastState.centerY - radius - static_cast<float>(kMoonRainArrowSpawnTopOffset);
    const float endY = g_moonRainCastState.centerY + radius * 0.15f;
    const float fallDistance = endY - startY;
    if (fallDistance <= 1.0f) {
        return;
    }

    for (int i = 0; i < kMoonRainArrowCount; ++i) {
        const unsigned int seedBase = 9001u + static_cast<unsigned int>(i) * 131u;
        const float angle = Hash01(seedBase + 3u) * 6.2831853f;
        const float ring = std::sqrt(Hash01(seedBase + 5u)) * radius * 0.96f;
        const float worldX = g_moonRainCastState.centerX + std::cos(angle) * ring;

        const ULONGLONG phaseOffset =
            static_cast<ULONGLONG>(Hash01(seedBase + 7u) * static_cast<float>(kMoonRainArrowFallCycleMs));
        const ULONGLONG local = (elapsed + phaseOffset) % kMoonRainArrowFallCycleMs;
        const float t = static_cast<float>(local) / static_cast<float>(kMoonRainArrowFallCycleMs);
        const float worldY = startY + fallDistance * t;

        const int drawX = static_cast<int>(std::round(worldX)) - g_cameraX - kMoonRainArrowDrawW / 2;
        const int drawY = static_cast<int>(std::round(worldY)) - g_cameraY - kMoonRainArrowDrawH / 2;
        if (RenderUtils::HasImage(g_moonRainArrowImage)) {
            RenderUtils::DrawImageAuto(
                g_moonRainArrowImage,
                g_moonRainArrowHasAlpha,
                drawX,
                drawY,
                kMoonRainArrowDrawW,
                kMoonRainArrowDrawH);
            continue;
        }

        setlinecolor(RGB(180, 150, 255));
        line(drawX + kMoonRainArrowDrawW / 2, drawY, drawX + kMoonRainArrowDrawW / 2, drawY + kMoonRainArrowDrawH);
    }
}

}  

void DrawDashAfterimages(ULONGLONG now) {
    if (g_currentCharacterIndex < 0 || g_currentCharacterIndex >= kCharacterCount) {
        return;
    }
    for (const DashAfterimage& afterimage : g_playerDashAfterimages) {
        const ULONGLONG age = (now >= afterimage.createdTick) ? (now - afterimage.createdTick) : 0ULL;
        const float opacity = 0.45f * (1.0f - static_cast<float>(age) / static_cast<float>(kPlayerDashAfterimageLifetimeMs));
        if (opacity <= 0.0f) {
            continue;
        }

        AnimatedGif& ghostGif = afterimage.moving ? g_walkGifs[g_currentCharacterIndex] : g_idleGifs[g_currentCharacterIndex];
        const int ghostX = afterimage.x - g_cameraX - kPlayerDrawW / 2;
        const int ghostY = afterimage.y - g_cameraY - kPlayerDrawH / 2;
        RenderUtils::DrawAnimatedGifOpacity(ghostGif, ghostX, ghostY, kPlayerDrawW, kPlayerDrawH, opacity, !afterimage.faceRight);
    }
}

void DrawMoonUltimateFireballEffects(ULONGLONG now, bool drawAura, bool drawFireball) {
    const int auraSize = static_cast<int>(std::round(kMoonUltimateFireballRadius * 2.0f));
    for (const MoonUltimateFireballState& fireball : g_moonUltimateFireballs) {
        if (!fireball.active || now >= fireball.endTick) {
            continue;
        }

        if (drawAura) {
            const int auraX = static_cast<int>(std::round(fireball.targetX)) - g_cameraX - auraSize / 2;
            const int auraY = static_cast<int>(std::round(fireball.targetY)) - g_cameraY - auraSize / 2;
            if (RenderUtils::HasImage(g_moonUltimateFireballAuraImage)) {
                RenderUtils::DrawImageAuto(
                    g_moonUltimateFireballAuraImage,
                    g_moonUltimateFireballAuraHasAlpha,
                    auraX,
                    auraY,
                    auraSize,
                    auraSize);
            }
            else {
                setlinecolor(RGB(150, 120, 255));
                circle(auraX + auraSize / 2, auraY + auraSize / 2, auraSize / 2);
            }
        }

        if (!drawFireball) {
            continue;
        }

        float t = 1.0f;
        if (now < fireball.impactTick && fireball.impactTick > fireball.startTick) {
            t = static_cast<float>(now - fireball.startTick) /
                static_cast<float>(fireball.impactTick - fireball.startTick);
            if (t < 0.0f) {
                t = 0.0f;
            }
            if (t > 1.0f) {
                t = 1.0f;
            }
        }

        const float worldX = fireball.startX + (fireball.targetX - fireball.startX) * t;
        const float worldY = fireball.startY + (fireball.targetY - fireball.startY) * t;
        const int drawX = static_cast<int>(std::round(worldX)) - g_cameraX - kMoonUltimateFireballDrawW / 2;
        const int drawY = static_cast<int>(std::round(worldY)) - g_cameraY - kMoonUltimateFireballDrawH / 2;
        if (RenderUtils::HasImage(g_moonUltimateFireballImage)) {
            RenderUtils::DrawImageAuto(
                g_moonUltimateFireballImage,
                g_moonUltimateFireballHasAlpha,
                drawX,
                drawY,
                kMoonUltimateFireballDrawW,
                kMoonUltimateFireballDrawH);
            continue;
        }

        setfillcolor(RGB(255, 120, 120));
        solidcircle(drawX + kMoonUltimateFireballDrawW / 2, drawY + kMoonUltimateFireballDrawH / 2, kMoonUltimateFireballDrawW / 2);
    }
}

void DrawSunSwordQiProjectiles() {
    if (!RenderUtils::HasImage(g_sunUltimateSwordQiImage)) {
        return;
    }

    for (const SunSwordQiState& swordQi : g_sunSwordQiProjectiles) {
        if (!swordQi.active) {
            continue;
        }

        const int centerX = static_cast<int>(std::round(swordQi.x)) - g_cameraX;
        const int centerY = static_cast<int>(std::round(swordQi.y)) - g_cameraY;
        if (!IsScreenVisible(centerX, centerY, kSunUltimateSwordQiDrawW / 2, kSunUltimateSwordQiDrawH / 2)) {
            continue;
        }

        if (RenderUtils::IsGdiplusReady()) {
            RenderUtils::DrawImageFileRotated(
                AssetPaths::GetSunUltimateSwordQiPath(),
                centerX,
                centerY,
                kSunUltimateSwordQiDrawW,
                kSunUltimateSwordQiDrawH,
                swordQi.angleDegrees);
            continue;
        }

        RenderUtils::DrawImageAuto(
            g_sunUltimateSwordQiImage,
            g_sunUltimateSwordQiHasAlpha,
            centerX - kSunUltimateSwordQiDrawW / 2,
            centerY - kSunUltimateSwordQiDrawH / 2,
            kSunUltimateSwordQiDrawW,
            kSunUltimateSwordQiDrawH);
    }
}

void DrawMoonRainSkillEffects(ULONGLONG now) {
    if (g_moonRainChargeState.active) {
        DrawMoonRainAimCircle(
            g_moonRainChargeState.centerX,
            g_moonRainChargeState.centerY,
            g_moonRainChargeState.radius);
    }
    if (g_moonRainCastState.active) {
        DrawMoonRainAimCircle(
            g_moonRainCastState.centerX,
            g_moonRainCastState.centerY,
            g_moonRainCastState.radius);
        DrawMoonRainArrowBurst(now);
    }
    DrawMoonUltimateFireballEffects(now, true, false);
}

}  
