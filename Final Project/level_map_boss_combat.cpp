#include "level_map_internal.h"
#include "level_map_combat_internal.h"

#include <cmath>

namespace LevelMapInternal {
namespace {

float ToRadians(float degrees) {
    return degrees * (kPi / 180.0f);
}

bool PopSnowApeKingCrossBarrage(int& outType) {
    if (g_enemy.bossCrossBarrageQueue.empty()) {
        return false;
    }

    outType = g_enemy.bossCrossBarrageQueue.front();
    g_enemy.bossCrossBarrageQueue.pop_front();
    return true;
}

void EnqueueSnowApeKingCrossBarrage(int barrageType) {
    static_assert(kSnowApeKingCrossBarrageQueueMax > 0, "invalid capacity");

    if (barrageType != kSnowApeKingCrossBarrageTypeRing &&
        barrageType != kSnowApeKingCrossBarrageTypeSpiral) {
        return;
    }

    const size_t queueMax = static_cast<size_t>(kSnowApeKingCrossBarrageQueueMax);
    while (g_enemy.bossCrossBarrageQueue.size() >= queueMax) {
        g_enemy.bossCrossBarrageQueue.pop_front();
    }
    g_enemy.bossCrossBarrageQueue.push_back(barrageType);
}

void SpawnSnowApeKingCrossRingBurst(ULONGLONG nowTick) {
    if (!g_enemy.alive || EnemyDef().kind != GameData::EnemyKind::SnowApeKing) {
        return;
    }

    const int bulletCount =
        (kSnowApeKingCrossRingBulletCount < 4) ? 4 : kSnowApeKingCrossRingBulletCount;
    const float originX = static_cast<float>(g_enemy.x);
    const float originY = static_cast<float>(g_enemy.y);
    const float angleStep = 360.0f / static_cast<float>(bulletCount);

    for (int i = 0; i < bulletCount; ++i) {
        const float angleRadians = ToRadians(static_cast<float>(i) * angleStep);
        SpawnEnemyProjectileFrom(
            originX,
            originY,
            std::cos(angleRadians),
            std::sin(angleRadians),
            kSnowApeKingCrossRingSpeedScale,
            nowTick,
            kSnowApeKingCrossBarrageHitDelayMs,
            kBossCrossRingHitDamage);
    }
}

void EmitSnowApeKingCrossSpiralBurst(ULONGLONG nowTick) {
    if (!g_enemy.alive || EnemyDef().kind != GameData::EnemyKind::SnowApeKing) {
        g_enemy.bossCrossSpiralActive = false;
        g_enemy.bossCrossSpiralBurstsRemaining = 0;
        return;
    }

    const float originX = static_cast<float>(g_enemy.x);
    const float originY = static_cast<float>(g_enemy.y);
    for (int arm = 0; arm < 4; ++arm) {
        const float angleDegrees = g_enemy.bossCrossSpiralBaseAngleDegrees + static_cast<float>(arm) * 90.0f;
        const float angleRadians = ToRadians(angleDegrees);
        SpawnEnemyProjectileFrom(
            originX,
            originY,
            std::cos(angleRadians),
            std::sin(angleRadians),
            kSnowApeKingCrossSpiralSpeedScale,
            nowTick,
            kSnowApeKingCrossBarrageHitDelayMs,
            kBossCrossSpiralHitDamage);
    }

    g_enemy.bossCrossSpiralBaseAngleDegrees += kSnowApeKingCrossSpiralAngleStepDegrees;
    if (std::fabs(g_enemy.bossCrossSpiralBaseAngleDegrees) >= 360.0f) {
        g_enemy.bossCrossSpiralBaseAngleDegrees =
            std::fmod(g_enemy.bossCrossSpiralBaseAngleDegrees, 360.0f);
    }

    --g_enemy.bossCrossSpiralBurstsRemaining;
    if (g_enemy.bossCrossSpiralBurstsRemaining <= 0) {
        g_enemy.bossCrossSpiralActive = false;
        g_enemy.bossCrossSpiralBurstsRemaining = 0;
    }
}

void StartSnowApeKingCrossSpiral(ULONGLONG nowTick) {
    g_enemy.bossCrossSpiralActive = true;
    g_enemy.bossCrossSpiralBurstsRemaining = kSnowApeKingCrossSpiralBurstCount;
    g_enemy.bossCrossSpiralEmitCooldownFrames = 0;

    const float dx = static_cast<float>(g_playerX - g_enemy.x);
    const float dy = static_cast<float>(g_playerY - g_enemy.y);
    g_enemy.bossCrossSpiralBaseAngleDegrees =
        static_cast<float>(std::atan2(dy, dx) * 180.0 / static_cast<double>(kPi));

    EmitSnowApeKingCrossSpiralBurst(nowTick);
    g_enemy.bossCrossSpiralEmitCooldownFrames =
        (kSnowApeKingCrossSpiralBurstIntervalFrames > 0) ? (kSnowApeKingCrossSpiralBurstIntervalFrames - 1) : 0;
}

}

void NotifySnowApeKingHpLossForCrossBarrage(int hpLost) {
    static_assert(kSnowApeKingCrossBarrageHpStep > 0, "invalid step size");

    if (hpLost <= 0) {
        g_enemy.bossCrossBarrageLastHpLoss = 0;
        return;
    }

    const int step = kSnowApeKingCrossBarrageHpStep;
    int lastHpLoss = g_enemy.bossCrossBarrageLastHpLoss;
    if (lastHpLoss < 0) {
        lastHpLoss = 0;
    }

    if (hpLost < lastHpLoss) {
        g_enemy.bossCrossBarrageLastHpLoss = hpLost;
        return;
    }

    // 按首领已损失血量阈值排入交叉弹幕。
    int nextThreshold = ((lastHpLoss / step) + 1) * step;
    while (nextThreshold <= hpLost) {
        TrySpawnPotionDropOnBossHpLossThreshold(g_enemy.x, g_enemy.y);
        if ((nextThreshold % kSnowApeKingCrossBarrageSpiralHpStep) == 0) {
            EnqueueSnowApeKingCrossBarrage(kSnowApeKingCrossBarrageTypeSpiral);
        }
        else {
            EnqueueSnowApeKingCrossBarrage(kSnowApeKingCrossBarrageTypeRing);
        }
        nextThreshold += step;
    }

    g_enemy.bossCrossBarrageLastHpLoss = hpLost;
}

void SpawnSnowApeKingSpikeRow(float dirX, float dirY) {
    const float len = std::sqrt(dirX * dirX + dirY * dirY);
    if (len < 0.01f) {
        return;
    }

    const float ndx = dirX / len;
    const float ndy = dirY / len;

    const float screenLeft = static_cast<float>(g_cameraX);
    const float screenTop = static_cast<float>(g_cameraY);
    const float screenRight = static_cast<float>(g_cameraX + GAME_WINDOW_WIDTH);
    const float screenBottom = static_cast<float>(g_cameraY + GAME_WINDOW_HEIGHT);
    const int enemyFootOffsetY =
        (EnemyDef().kind == GameData::EnemyKind::SnowApeKing) ? kSnowApeKingFootOffsetY : kEnemyFootOffsetY;
    const float originX = static_cast<float>(g_enemy.x);
    const float originY = static_cast<float>(g_enemy.y + enemyFootOffsetY);

    float tMax = 0.0f;
    bool hasBoundaryHit = false;
    auto considerBoundary = [&](float boundary, float origin, float dir) {
        if (std::fabs(dir) < 0.0001f) {
            return;
        }
        const float t = (boundary - origin) / dir;
        if (t > 0.0f && (!hasBoundaryHit || t < tMax)) {
            tMax = t;
            hasBoundaryHit = true;
        }
    };

    considerBoundary(screenLeft, originX, ndx);
    considerBoundary(screenRight, originX, ndx);
    considerBoundary(screenTop, originY, ndy);
    considerBoundary(screenBottom, originY, ndy);

    if (!hasBoundaryHit || tMax < kSnowApeKingSpikeSpacing * 0.6f) {
        tMax = static_cast<float>(GAME_WINDOW_WIDTH > GAME_WINDOW_HEIGHT ? GAME_WINDOW_WIDTH : GAME_WINDOW_HEIGHT);
    }

    const ULONGLONG now = RenderUtils::NowTickMs();
    EnemySpikeRow row;
    row.startX = originX;
    row.startY = originY;
    row.dirX = ndx;
    row.dirY = ndy;
    row.length = tMax;
    row.startTick = now;
    row.maxSpikeCount = static_cast<int>(std::ceil(row.length / kSnowApeKingSpikeSpacing)) + 1;
    if (row.maxSpikeCount < 1) {
        row.maxSpikeCount = 1;
    }
    row.revealedSpikeCount = 1;
    row.nextSpawnTick = now + kSnowApeKingSpikeSpawnIntervalMs;
    row.endTick = now + kSnowApeKingSpikeRowDurationMs + kSnowApeKingSpikeRowHoldMs;
    row.active = true;
    row.hitPlayer = false;

    g_enemySpikeRows.clear();
    g_enemySpikeRows.push_back(row);
}

void UpdateSnowApeKingCrossBarrage(ULONGLONG now) {
    if (EnemyDef().kind != GameData::EnemyKind::SnowApeKing) {
        return;
    }
    if (!g_enemy.alive) {
        ResetEnemyBossRuntimeState();
        return;
    }
    const GameData::EnemyDefinition& enemyDef = EnemyDef();
    const int halfW = enemyDef.drawWidth / 2;
    const int halfH = enemyDef.drawHeight / 2;
    const int screenX = g_enemy.x - g_cameraX;
    const int screenY = g_enemy.y - g_cameraY;
    const bool onScreen =
        screenX >= -halfW && screenX <= GAME_WINDOW_WIDTH + halfW &&
        screenY >= -halfH && screenY <= GAME_WINDOW_HEIGHT + halfH;
    if (!onScreen) {
        g_enemy.bossCrossJumping = false;
        g_enemy.bossCrossJumpEndTick = 0;
        g_enemy.bossCrossSpiralActive = false;
        g_enemy.bossCrossSpiralBurstsRemaining = 0;
        g_enemy.bossCrossSpiralEmitCooldownFrames = 0;
        g_enemy.bossCrossBarrageQueue.clear();
        return;
    }

    if (g_enemy.bossCrossJumping) {
        if (now < g_enemy.bossCrossJumpEndTick) {
            return;
        }

        g_enemy.bossCrossJumping = false;
        g_enemy.bossCrossJumpEndTick = 0;
        int barrageType = 0;
        if (PopSnowApeKingCrossBarrage(barrageType)) {
            if (barrageType == kSnowApeKingCrossBarrageTypeSpiral) {
                StartSnowApeKingCrossSpiral(now);
            }
            else {
                SpawnSnowApeKingCrossRingBurst(now);
            }
        }
        return;
    }

    if (g_enemy.bossCrossSpiralActive && g_enemy.bossCrossSpiralBurstsRemaining > 0) {
        if (g_enemy.bossCrossSpiralEmitCooldownFrames > 0) {
            --g_enemy.bossCrossSpiralEmitCooldownFrames;
            return;
        }

        EmitSnowApeKingCrossSpiralBurst(now);
        g_enemy.bossCrossSpiralEmitCooldownFrames =
            (kSnowApeKingCrossSpiralBurstIntervalFrames > 0) ? (kSnowApeKingCrossSpiralBurstIntervalFrames - 1) : 0;
        return;
    }

    if (!g_enemy.bossCrossBarrageQueue.empty() && !g_enemy.bossRoaring) {
        g_enemy.bossCrossJumping = true;
        g_enemy.bossCrossJumpEndTick = now + kSnowApeKingCrossJumpWindupMs;
        g_enemy.moving = false;
    }
}

}
