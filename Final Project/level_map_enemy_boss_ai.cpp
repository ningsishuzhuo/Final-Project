#include "level_map_internal.h"
#include "level_map_enemy_ai_internal.h"

#include <cmath>
#include <cstdlib>

namespace LevelMapInternal {
namespace {

void UpdateSnowApeKingRoamTarget(const GameData::EnemyDefinition& enemyDef) {
    int dirX = (std::rand() % 201) - 100;
    int dirY = (std::rand() % 201) - 100;
    if (dirX == 0 && dirY == 0) {
        dirX = 1;
    }

    const float len = std::sqrt(static_cast<float>(dirX * dirX + dirY * dirY));
    const int roamStep =
        kSnowApeKingRandomRoamMinStep +
        (std::rand() % (kSnowApeKingRandomRoamMaxStep - kSnowApeKingRandomRoamMinStep + 1));

    int tx = g_enemy.x + static_cast<int>(std::round(static_cast<float>(dirX) / len * static_cast<float>(roamStep)));
    int ty = g_enemy.y + static_cast<int>(std::round(static_cast<float>(dirY) / len * static_cast<float>(roamStep)));

    const int padding = enemyDef.regionPadding;
    if (tx < g_iceRegionRect.left + padding) tx = g_iceRegionRect.left + padding;
    if (tx > g_iceRegionRect.right - padding) tx = g_iceRegionRect.right - padding;
    if (ty < g_iceRegionRect.top + padding) ty = g_iceRegionRect.top + padding;
    if (ty > g_iceRegionRect.bottom - padding) ty = g_iceRegionRect.bottom - padding;
    g_enemy.roamTargetX = tx;
    g_enemy.roamTargetY = ty;
}

void StabilizeSnowApeKingFacingByMove(float moveX, ULONGLONG now) {
    if (std::fabs(moveX) < kSnowApeKingFacingFlipMinMoveX) {
        g_enemy.bossPendingFaceDir = 0;
        g_enemy.bossPendingFaceFrames = 0;
        return;
    }

    const int desiredDir = (moveX > 0.0f) ? 1 : -1;
    const int currentDir = g_enemy.faceRight ? 1 : -1;
    if (desiredDir == currentDir) {
        g_enemy.bossPendingFaceDir = 0;
        g_enemy.bossPendingFaceFrames = 0;
        g_enemy.bossNextFaceFlipTick = now;
        return;
    }

    if (desiredDir == g_enemy.bossPendingFaceDir) {
        ++g_enemy.bossPendingFaceFrames;
    }
    else {
        g_enemy.bossPendingFaceDir = desiredDir;
        g_enemy.bossPendingFaceFrames = 1;
    }

    if (g_enemy.bossPendingFaceFrames >= kSnowApeKingFacingConfirmFrames &&
        now >= g_enemy.bossNextFaceFlipTick) {
        g_enemy.faceRight = (desiredDir > 0);
        g_enemy.bossNextFaceFlipTick = now + kSnowApeKingFacingFlipCooldownMs;
        g_enemy.bossPendingFaceDir = 0;
        g_enemy.bossPendingFaceFrames = 0;
    }
}

bool IsPlayerNearBossViewEdge(float dx, float dy) {
    return std::fabs(dx) >= kSnowApeKingViewChaseThresholdX ||
        std::fabs(dy) >= kSnowApeKingViewChaseThresholdY;
}

bool IsPlayerAwayFromBossViewEdge(float dx, float dy) {
    return std::fabs(dx) <= kSnowApeKingViewReleaseThresholdX &&
        std::fabs(dy) <= kSnowApeKingViewReleaseThresholdY;
}

ULONGLONG RandomDurationMs(ULONGLONG minMs, ULONGLONG maxMs) {
    if (maxMs <= minMs) {
        return minMs;
    }
    const ULONGLONG span = maxMs - minMs + 1ULL;
    return minMs + static_cast<ULONGLONG>(std::rand()) % span;
}

void GetSnowApeKingDirTowardPlayer(float& outDirX, float& outDirY) {
    float dirX = static_cast<float>(g_playerX - g_enemy.x);
    float dirY = static_cast<float>(g_playerY - g_enemy.y);
    const float len = std::sqrt(dirX * dirX + dirY * dirY);
    if (len < 0.001f) {
        outDirX = g_enemy.faceRight ? 1.0f : -1.0f;
        outDirY = 0.0f;
        return;
    }

    outDirX = dirX / len;
    outDirY = dirY / len;
}

void StartSnowApeKingRoar(ULONGLONG now) {
    float dirX = 0.0f;
    float dirY = 0.0f;
    GetSnowApeKingDirTowardPlayer(dirX, dirY);

    g_enemy.bossRoaring = true;
    g_enemy.bossRoarPendingSpikeRow = true;
    g_enemy.bossRoarDirX = dirX;
    g_enemy.bossRoarDirY = dirY;
    g_enemy.bossRoarEndTick = now + kSnowApeKingRoarDurationMs;
    g_enemy.moving = false;
}

}  

void UpdateSnowApeKingAI(
    float dx,
    float dy,
    ULONGLONG now,
    const GameData::EnemyCombatParams& params,
    const GameData::EnemyDefinition& enemyDef,
    bool attackEnabled) {
    if (g_enemy.bossCrossJumping) {
        g_enemy.moving = false;
        UpdateEnemyFacingTowardPlayer();
        g_enemySawPlayerLastFrame = true;
        return;
    }

    if (!g_enemy.bossPressureActive) {
        g_enemy.bossPressureActive = IsPlayerNearBossViewEdge(dx, dy);
    }
    else if (IsPlayerAwayFromBossViewEdge(dx, dy)) {
        g_enemy.bossPressureActive = false;
    }

    const bool playerNearViewEdge = g_enemy.bossPressureActive;

    if (g_enemy.bossPhaseEndTick == 0) {
        g_enemy.bossWalkPhase = true;
        g_enemy.bossPhaseEndTick = now + RandomDurationMs(kSnowApeKingWalkPhaseMinMs, kSnowApeKingWalkPhaseMaxMs);
    }

    if (now >= g_enemy.bossPhaseEndTick) {
        g_enemy.bossWalkPhase = !g_enemy.bossWalkPhase;
        if (g_enemy.bossWalkPhase) {
            UpdateSnowApeKingRoamTarget(enemyDef);
            g_enemy.nextRepositionTick = now + kSnowApeKingRoamRefreshMs;
            g_enemy.bossPhaseEndTick = now + RandomDurationMs(kSnowApeKingWalkPhaseMinMs, kSnowApeKingWalkPhaseMaxMs);
            if (attackEnabled) {
                StartSnowApeKingRoar(now);
            }
        }
        else {
            g_enemy.bossPhaseEndTick = now + RandomDurationMs(kSnowApeKingIdlePhaseMinMs, kSnowApeKingIdlePhaseMaxMs);
        }
    }

    if (playerNearViewEdge) {
        if (!g_enemy.bossWalkPhase) {
            g_enemy.bossWalkPhase = true;
            g_enemy.bossPhaseEndTick = now + RandomDurationMs(kSnowApeKingWalkPhaseMinMs, kSnowApeKingWalkPhaseMaxMs);
            UpdateSnowApeKingRoamTarget(enemyDef);
            g_enemy.nextRepositionTick = now + kSnowApeKingRoamRefreshMs;
            if (attackEnabled) {
                StartSnowApeKingRoar(now);
            }
        }
        else if (g_enemy.bossPhaseEndTick < now + kSnowApeKingPressureWalkMinMs) {
            g_enemy.bossPhaseEndTick = now + kSnowApeKingPressureWalkMinMs;
        }
    }

    if (!attackEnabled) {
        g_enemy.bossRoaring = false;
        g_enemy.bossRoarPendingSpikeRow = false;
    }

    if (g_enemy.bossRoaring) {
        g_enemy.moving = false;
        if (g_enemy.bossRoarDirX > 0.05f) {
            g_enemy.faceRight = true;
        }
        else if (g_enemy.bossRoarDirX < -0.05f) {
            g_enemy.faceRight = false;
        }

        if (now >= g_enemy.bossRoarEndTick) {
            g_enemy.bossRoaring = false;
            if (g_enemy.bossRoarPendingSpikeRow) {
                float attackDirX = 0.0f;
                float attackDirY = 0.0f;
                GetSnowApeKingDirTowardPlayer(attackDirX, attackDirY);
                g_enemy.bossRoarDirX = attackDirX;
                g_enemy.bossRoarDirY = attackDirY;
                if (attackDirX > 0.05f) {
                    g_enemy.faceRight = true;
                }
                else if (attackDirX < -0.05f) {
                    g_enemy.faceRight = false;
                }
                SpawnSnowApeKingSpikeRow(g_enemy.bossRoarDirX, g_enemy.bossRoarDirY);
                g_enemy.bossRoarPendingSpikeRow = false;
            }
        }
        g_enemySawPlayerLastFrame = true;
        return;
    }

    float moveX = 0.0f;
    float moveY = 0.0f;

    if (g_enemy.bossWalkPhase) {
        if (now >= g_enemy.nextRepositionTick ||
            (std::abs(g_enemy.roamTargetX - g_enemy.x) < 20 &&
             std::abs(g_enemy.roamTargetY - g_enemy.y) < 20)) {
            UpdateSnowApeKingRoamTarget(enemyDef);
            g_enemy.nextRepositionTick = now + kSnowApeKingRoamRefreshMs;
        }

        const int tx = g_enemy.roamTargetX - g_enemy.x;
        const int ty = g_enemy.roamTargetY - g_enemy.y;
        if (tx > 4) moveX += static_cast<float>(params.patrolSpeed) * kSnowApeKingRoamDriftScale;
        else if (tx < -4) moveX -= static_cast<float>(params.patrolSpeed) * kSnowApeKingRoamDriftScale;
        if (ty > 4) moveY += static_cast<float>(params.patrolSpeed) * kSnowApeKingRoamDriftScale;
        else if (ty < -4) moveY -= static_cast<float>(params.patrolSpeed) * kSnowApeKingRoamDriftScale;

        if (playerNearViewEdge) {
            const float distToPlayer = std::sqrt(dx * dx + dy * dy);
            if (distToPlayer > 1.0f) {
                const float chaseStep = static_cast<float>(params.chaseSpeed) * kSnowApeKingViewBiasScale;
                moveX += dx / distToPlayer * chaseStep;
                moveY += dy / distToPlayer * chaseStep;
            }
        }
    }

    const float moveLen = std::sqrt(moveX * moveX + moveY * moveY);
    const float maxStep = GetEnemyMoveSpeed(static_cast<float>(params.chaseSpeed));
    if (moveLen > maxStep && moveLen > 0.01f) {
        const float scale = maxStep / moveLen;
        moveX *= scale;
        moveY *= scale;
    }

    StabilizeSnowApeKingFacingByMove(moveX, now);

    OffsetEnemyPosition(moveX, moveY);
    const float padding = static_cast<float>(enemyDef.regionPadding);
    const float minX = static_cast<float>(g_iceRegionRect.left) + padding;
    const float maxX = static_cast<float>(g_iceRegionRect.right) - padding;
    const float minY = static_cast<float>(g_iceRegionRect.top) + padding;
    const float maxY = static_cast<float>(g_iceRegionRect.bottom) - padding;
    if (g_enemy.exactX < minX) g_enemy.exactX = minX;
    if (g_enemy.exactX > maxX) g_enemy.exactX = maxX;
    if (g_enemy.exactY < minY) g_enemy.exactY = minY;
    if (g_enemy.exactY > maxY) g_enemy.exactY = maxY;
    RefreshEnemyGridPosition();

    g_enemy.moving = std::fabs(moveX) > 0.05f || std::fabs(moveY) > 0.05f;
    if (!g_enemy.moving) {
        UpdateEnemyFacingTowardPlayer();
    }
    g_enemySawPlayerLastFrame = true;
}

}  
