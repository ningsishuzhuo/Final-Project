#include "level_map_internal.h"
#include "level_map_combat_internal.h"

namespace LevelMapInternal {

void UpdateEnemyShockwaves(ULONGLONG now) {
    size_t writeIndex = 0;

    for (size_t i = 0; i < g_enemyShockwaves.size(); ++i) {
        ShockwaveInstance& wave = g_enemyShockwaves[i];
        const ShockwaveTuning& tuning = GetShockwaveTuning(wave.variant);
        if (!wave.active) {
            continue;
        }

        if (!wave.expanding) {
            if (now < wave.triggerTick) {
                if (writeIndex != i) {
                    g_enemyShockwaves[writeIndex] = wave;
                }
                ++writeIndex;
                continue;
            }

            wave.expanding = true;
            wave.previousRadius = static_cast<float>(tuning.telegraphRadius);
            wave.radius = static_cast<float>(tuning.telegraphRadius);
        }

        wave.previousRadius = wave.radius;
        wave.radius += static_cast<float>(tuning.expandSpeed);
        if (!wave.hitPlayer) {
            const float dx = static_cast<float>(g_playerX) - wave.x;
            const float dy = static_cast<float>(g_playerY) - wave.y;
            const float distanceSquared = dx * dx + dy * dy;
            const float bandInner = wave.previousRadius - static_cast<float>(tuning.hitRadius);
            const float bandOuter = wave.radius + static_cast<float>(tuning.hitRadius);
            const float minDistance = bandInner > 0.0f ? bandInner : 0.0f;
            if (distanceSquared >= minDistance * minDistance && distanceSquared <= bandOuter * bandOuter) {
                ApplyPlayerHit(kEnemyShockwaveHitDamage);
                wave.hitPlayer = true;
            }
        }

        if (wave.radius <= static_cast<float>(tuning.maxRadius)) {
            if (writeIndex != i) {
                g_enemyShockwaves[writeIndex] = wave;
            }
            ++writeIndex;
        }
    }

    g_enemyShockwaves.resize(writeIndex);
}

void UpdateEnemySpikeRows(ULONGLONG now) {
    size_t writeIndex = 0;
    for (size_t i = 0; i < g_enemySpikeRows.size(); ++i) {
        EnemySpikeRow& row = g_enemySpikeRows[i];
        if (!row.active || now >= row.endTick) {
            continue;
        }

        if (row.revealedSpikeCount < row.maxSpikeCount && now >= row.nextSpawnTick) {
            ++row.revealedSpikeCount;
            row.nextSpawnTick = now + kSnowApeKingSpikeSpawnIntervalMs;
            if (row.revealedSpikeCount >= row.maxSpikeCount) {
                row.revealedSpikeCount = row.maxSpikeCount;
                row.endTick = now + kSnowApeKingSpikeRowHoldMs;
            }
        }

        if (!row.hitPlayer) {
            const float revealCandidate =
                static_cast<float>(row.revealedSpikeCount) * kSnowApeKingSpikeSpacing;
            const float revealedLength = (revealCandidate < row.length) ? revealCandidate : row.length;

            const float px = static_cast<float>(g_playerX) - row.startX;
            const float py = static_cast<float>(g_playerY) - row.startY;
            const float projection = px * row.dirX + py * row.dirY;
            const float clamped = projection < 0.0f ? 0.0f : (projection > revealedLength ? revealedLength : projection);
            const float nearestX = row.startX + row.dirX * clamped;
            const float nearestY = row.startY + row.dirY * clamped;
            const float dx = static_cast<float>(g_playerX) - nearestX;
            const float dy = static_cast<float>(g_playerY) - nearestY;
            if (dx * dx + dy * dy <= kSnowApeKingSpikeHitRadius * kSnowApeKingSpikeHitRadius) {
                ApplyPlayerHit(kSnowApeKingSpikeHitDamage);
                row.hitPlayer = true;
            }
        }

        if (writeIndex != i) {
            g_enemySpikeRows[writeIndex] = row;
        }
        ++writeIndex;
    }
    g_enemySpikeRows.resize(writeIndex);
}

}
