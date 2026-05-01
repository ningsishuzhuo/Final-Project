#include "level_map_internal.h"
#include "level_map_rendering_effects_internal.h"

#include <cmath>

namespace LevelMapInternal {
namespace {

void DrawEnemySpikeRowsLayeredImpl(int minYInclusive, int maxYExclusive) {
    if (!RenderUtils::HasImage(g_enemyIceSpikeImage)) {
        return;
    }

    const int spikeW = 64;
    const int spikeH = 36;
    for (const EnemySpikeRow& row : g_enemySpikeRows) {
        if (!row.active) {
            continue;
        }

        const float visibleCandidate =
            static_cast<float>(row.revealedSpikeCount) * kSnowApeKingSpikeSpacing;
        const float visibleLength = (visibleCandidate < row.length) ? visibleCandidate : row.length;

        const float step = kSnowApeKingSpikeSpacing;
        for (float d = 0.0f; d <= visibleLength; d += step) {
            const float wx = row.startX + row.dirX * d;
            const float wy = row.startY + row.dirY * d;
            if (!IsWithinLayerRange(wy, minYInclusive, maxYExclusive)) {
                continue;
            }

            const int centerX = static_cast<int>(std::round(wx)) - g_cameraX;
            const int centerY = static_cast<int>(std::round(wy)) - g_cameraY;
            if (!IsScreenVisible(centerX, centerY, spikeW / 2, spikeH / 2)) {
                continue;
            }

            
            RenderUtils::DrawImageAuto(
                g_enemyIceSpikeImage,
                g_enemyIceSpikeHasAlpha,
                centerX - spikeW / 2,
                centerY - spikeH,
                spikeW,
                spikeH);
        }
    }
}

}  

void DrawEnemyShockwavesLayered(ULONGLONG now, int minYInclusive, int maxYExclusive) {
    for (const ShockwaveInstance& wave : g_enemyShockwaves) {
        if (!wave.active) {
            continue;
        }
        if (!IsWithinLayerRange(wave.y, minYInclusive, maxYExclusive)) {
            continue;
        }

        const ShockwaveTuning& tuning = GetShockwaveTuning(wave.variant);
        const int centerX = static_cast<int>(wave.x) - g_cameraX;
        const int centerY = static_cast<int>(wave.y) - g_cameraY;
        const int visibleRadius = wave.expanding ? static_cast<int>(wave.radius) + tuning.hitRadius + 4 : tuning.telegraphRadius + 10;
        if (!IsScreenVisible(centerX, centerY, visibleRadius, visibleRadius)) {
            continue;
        }
        if (!wave.expanding) {
            const float remain = static_cast<float>(wave.triggerTick > now ? (wave.triggerTick - now) : 0ULL);
            const float telegraphProgress =
                (tuning.lockDelayMs > 0ULL) ? (1.0f - remain / static_cast<float>(tuning.lockDelayMs)) : 1.0f;
            const int telegraphRadius = tuning.telegraphRadius + static_cast<int>(telegraphProgress * 6.0f);

            setfillcolor(tuning.fillColor);
            solidcircle(centerX, centerY, telegraphRadius);
            setlinecolor(tuning.outerColor);
            circle(centerX, centerY, telegraphRadius);
            circle(centerX, centerY, telegraphRadius + 3);
            continue;
        }

        const float progress = wave.radius / static_cast<float>(tuning.maxRadius);
        const int outerRadius = static_cast<int>(wave.radius);
        const int innerRadius = outerRadius - tuning.hitRadius;
        const int coreRadius = outerRadius - tuning.hitRadius / 2;
        const int fillRadius = innerRadius > 0 ? innerRadius : outerRadius;

        setfillcolor(tuning.fillColor);
        solidcircle(centerX, centerY, fillRadius);

        setlinecolor(wave.hitPlayer ? tuning.flashColor : tuning.outerColor);
        circle(centerX, centerY, outerRadius);

        if (innerRadius > 4) {
            setlinecolor(tuning.innerColor);
            circle(centerX, centerY, innerRadius);
        }

        if (coreRadius > 4) {
            setlinecolor(tuning.coreColor);
            circle(centerX, centerY, coreRadius);
        }

        if (progress < 0.55f) {
            setlinecolor(tuning.flashColor);
            circle(centerX, centerY, outerRadius + 2);
        }
    }
}

void DrawEnemySpikeRowsLayered(int minYInclusive, int maxYExclusive) {
    DrawEnemySpikeRowsLayeredImpl(minYInclusive, maxYExclusive);
}

}  
